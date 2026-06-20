//main.cpp

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "SCD40_sensor.h"
#include "DS3231_RTC.h"
// #include "BME280_sensor.h"
// #include "TEMT6000_sensor.h"
#include "Joystick.h"
#include <SPIFFS.h>
#include "WeatherFetcher.h"
#include <WiFi.h>
#include "secrets.h"
#include "CryptoFetcher.h"
#include "CurrencyFetcher.h"


// ========== Объекты ==========
TFT_eSPI tft = TFT_eSPI(); 
SCD40_sensor scd40;
DS3231_RTC rtc;
// BME280_sensor bme280;
// TEMT6000_sensor light;
Joystick joy;
WeatherFetcher weather;
CryptoFetcher crypto;
CurrencyFetcher currency;

// ========== Пины ==========
// Джойстик
#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 15

// TEMT6000
#define LIGHT_PIN 36

// ========== Переменные для экрана и меню ==========
int currentScreen = 0; // 0 - главный (часы+сводка), 1 - CO2+темп, 2 - давление, 3 - освещённость, 4 - погода(API)
const int numScreens = 6;
unsigned long lastRTCupdate = 0;
unsigned long lastSensorReadAll = 0;
const unsigned long RTC_INTERVAL = 1000;
const unsigned long SENSOR_INTERVAL = 2000;

//api weather модуль
unsigned long lastWeatherUpdate = 0;
const unsigned long WEATHER_INTERVAL = 30 * 60 * 1000UL; // 30 минут
bool weatherInitialized = false;

//wifi
bool wifiConnected = false;
unsigned long wifiConnectStart = 0;
const unsigned long WIFI_TIMEOUT = 10000;  // 10 секунд на попытку подключения

//crypto
bool cryptoInitialized = false;

//currency
bool currencyInitialized = false;

// === Прототипы функций ===
void initDisplay();
void updateDisplay();
void drawScreen0(); // главный экран
void drawScreen1(); // CO2 и климат от SCD40
void drawScreen2(); // давление и температура BME280
void drawScreen3(); // освещённость
void drawScreen4(); // погода
void drawScreen5(); // crypto активы
void drawBlock();

// ====================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ======================

// Универсальная функция для преобразования Unix Timestamp в строку "ЧЧ:ММ"
String formatTimestamp(unsigned long timestampUTC, int offsetSeconds){
    unsigned long localTimestamp = timestampUTC + offsetSeconds;
    time_t rawTime = localTimestamp;
    struct tm* timeinfo = gmtime(&rawTime);
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
    return String(buffer);
}

// Функция для расчёта продолжитеьности дня
String getDayLightDuration(unsigned long sunriseTimestamp, unsigned long sunsetTimestamp){
    unsigned long durationSeconds = sunsetTimestamp - sunriseTimestamp;

    int hours = durationSeconds/3600;
    int minutes = (durationSeconds % 3600)/60;

    char buffer[6];
    sprintf(buffer, "%02d:%02d", hours, minutes);
    return String(buffer);
}

//Функция для направления ветра
String getWindDirection(int deg) {
    if (deg < 22.5) return "С";
    if (deg < 67.5) return "СВ";
    if (deg < 112.5) return "В";
    if (deg < 157.5) return "ЮВ";
    if (deg < 202.5) return "Ю";
    if (deg < 247.5) return "ЮЗ";
    if (deg < 292.5) return "З";
    if (deg < 337.5) return "СЗ";
    return "С";
}

uint16_t getColorForChange(float change){
    if (change > 0) return TFT_GREEN;
    if (change < 0) return TFT_RED;
    return TFT_WHITE;
}

uint16_t getColorForFearGreed(int value) {
    if (value <= 25) return TFT_ORANGE;
    if (value <= 45) return TFT_YELLOW;
    if (value <= 55) return TFT_WHITE;
    if (value <= 75) return TFT_GREENYELLOW;
    return TFT_GREEN;
}


// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);
  Serial.println("AegisDesk starting..."); 
    
    initDisplay(); // инициализация дисплея

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    wifiConnectStart=millis();
    wifiConnected = false;

    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS mount failed!");
        // можно вывести на экран
        tft.setTextColor(TFT_RED, TFT_BLACK, true);
        tft.setCursor(10, 10);
        tft.print("SPIFFS ERROR");
        while(1); // стоп
    }
    
    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setTextFont(0);


    // Инициализация SCD40
    if (!scd40.init(21, 22)) {
        tft.setCursor(10, 120);
        tft.setTextColor(TFT_RED, TFT_BLACK, true);
        tft.println("SCD40 ERROR!");
        Serial.println("SCD40 initialization failed");
    }
    if (!rtc.init(21, 22)) {
        tft.setCursor(10, 150);
        tft.setTextColor(TFT_RED, TFT_BLACK, true);
        tft.println("DS3231 ERROR!");
        Serial.println("DS3231 initialization failed");
    }
    // if (!bme280.init(21, 22)) {
    //     tft.setCursor(10, 150);
    //     tft.setTextColor(TFT_RED, TFT_BLACK, true);
    //     tft.println("BME280 ERROR!");
    //     Serial.println("BME280 initialization failed");
    // }
    // light.init(LIGHT_PIN);
    joy.init(JOY_X, JOY_Y, JOY_SW);

    delay(2000);
    tft.fillScreen(TFT_BLACK);
    drawScreen0(); // рисуем начальный экран
} 



// ====================== LOOP ======================
void loop() {
    unsigned long loopStart = micros();
    // 1. Обновляем джойстик и обрабатываем нажатия (переключение экранов)
    joy.update();
    if (joy.justPressed()){

        // Принудительно обновить погоду, если текущий экран погоды
        if (currentScreen == 4) {
            weather.update();
            drawScreen4();
        }
    }
    int dir = joy.getDirection();
    if (dir == 3) {
        currentScreen--;
        if (currentScreen < 0) currentScreen = numScreens -1;
        updateDisplay();
        delay(50);
    } else if (dir == 4) {
        currentScreen++;
        if (currentScreen >= numScreens) currentScreen = 0;
        updateDisplay();
        delay(50);
    }

    // 2. Обновление RTC (каждую секунду)
    if (millis() - lastRTCupdate >= RTC_INTERVAL) {
        rtc.update();
        lastRTCupdate = millis();
        // На главном экране время нужно обновлять часто
        if (currentScreen == 0) {
            drawScreen0();
        }
    }

    // 3. Обновление датчиков SCD40 (он сам управляет интервалом внутри read)
    if (scd40.read()) {
        // при обновлении данных CO2 можно перерисовать экран CO2, если он активен
        if (currentScreen == 1) drawScreen1();
        // а также обновить главный экран (если на нём есть CO2)
        if (currentScreen == 0) drawScreen0();
    }
    // 4. Обновление BMe280, TEMT6000 (раз в 2 секунды)
    // if (millis() - lastSensorReadAll >= SENSOR_INTERVAL) {
    //     bme280.read();
    //     light.read();
    //     lastSensorReadAll = millis();
    //     if (currentScreen == 2) drawScreen2();
    //     if (currentScreen == 3) drawScreen3();
    //     if (currentScreen == 0) drawScreen0();
    // }

    // 5. Управление WiFi
    if (!wifiConnected) {
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.println("Wifi Connected!");
            // Теперь можно инициализировать погоду (если не была инициализирована)
            if (!weatherInitialized) {
                weather.init(CITY_NAME, OPENWEATHER_API_KEY);
                weatherInitialized = true;
            }
            if (!cryptoInitialized && wifiConnected) {
                crypto.init(COINMARKETCAP_API_KEY);
                cryptoInitialized = true;
            }
            if(!currencyInitialized && wifiConnected){
                currency.init();
                currencyInitialized = true;
            }
        } else if (millis() - wifiConnectStart > WIFI_TIMEOUT) {
            static unsigned long lastReconnectAttempt = 0;
            if (millis() - lastReconnectAttempt > 10000) {
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                wifiConnectStart = millis();
                lastReconnectAttempt = millis();
                Serial.println("retrying WiFi connection...");
            }
        }
    } else {
        if (WiFi.status() != WL_CONNECTED) {
            wifiConnected = false;
            Serial.println("Wifi Lost, Will reconnect");
        } 
    }

    if (wifiConnected && weatherInitialized) {
        if (millis() - lastWeatherUpdate >= WEATHER_INTERVAL) {
            weather.update();
            lastWeatherUpdate = millis(); 
        }
        if (currentScreen == 4) drawScreen4();
        }
        if (wifiConnected && cryptoInitialized) {
            crypto.update();
            if (currentScreen == 5) {
                drawScreen5();
            }
        }
        if (wifiConnected && currencyInitialized){
            currency.update();
            if (currentScreen == 5){
                drawScreen5();
            }
        }

  vTaskDelay(50/portTICK_PERIOD_MS);    // Небольшая задержка, чтобы не грузить процессор
}


// ====================== ФУНКЦИИ ДИСПЛЕЯ ======================
void initDisplay() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  // Управляем подсветкой дисплея
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); // включаем подсветку на максимум
}

// перерисовываем текущий экран
void updateDisplay() {
    tft.fillScreen(TFT_BLACK);
    switch (currentScreen) {
    case 0: drawScreen0(); break;    
    case 1:drawScreen1(); break;
    case 2:drawScreen2(); break;
    case 3:drawScreen3(); break;
    case 4:drawScreen4(); break;
    case 5:drawScreen5(); break;
    }
}


// Рисует блок с заголовком и значением
void drawBlock(int x, int y, int w, int h, uint16_t borderColor, String title, String value, uint16_t valueColor = TFT_WHITE) {
    tft.fillRect(x, y, w, h, TFT_BLACK);
    tft.drawRect(x, y, w, h, borderColor);  // рамка
    tft.setTextColor(borderColor, TFT_BLACK);
    tft.setTextSize(2);                     // крупный заголовок
    tft.setCursor(x + 10, y + 8);
    tft.print(title);
    tft.setTextColor(valueColor, TFT_BLACK, true);
    tft.setTextSize(3);                     // очень крупное значение
    tft.setCursor(x + 10, y + 40);
    // Если значение длинное (например, "1234 ppm" - 8 символов), уменьшаем шрифт до size 2
    if (value.length() > 7) tft.setTextSize(2);
    tft.print(value);
    tft.setTextSize(1); // сброс
}

// Рисует горизонтальную шкалу (прогресс-бар)
void drawProgressBar(int x, int y, int w, int h, uint16_t bgColor, uint16_t fillColor, float percent) {
    if (percent > 1.0) percent = 1.0;
    if (percent < 0) percent = 0;
    int fillWidth = (int)(w * percent);
    tft.fillRect(x, y, w, h, bgColor);
    if (fillWidth > 0) {
        tft.fillRect(x, y, fillWidth, h, fillColor);
    }
    tft.drawRect(x, y, w, h, TFT_WHITE);
}

// ====================== ГЛАВНЫЙ ЭКРАН ======================
void drawScreen0() {
    int x0 = 13, y0 = 70, w = 210, h = 90, gap = 20;

    // ---- Статус сети WIFI ----
    tft.setTextSize(1);
    if (wifiConnected) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK, true);
        tft.setCursor(380, 10);
        tft.print("WIFI");
    } else {
        tft.setTextColor(TFT_RED, TFT_BLACK, true);
        tft.setCursor(380, 10);
        tft.print("No WIFI");
    }

    // ---- Время и дата ----
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(15, 15);
    tft.print(rtc.getTimeString());

    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(240, 15);
    tft.print(rtc.getDateTimeString().substring(0, 10));

    tft.drawLine(5, 55, 440, 55, TFT_DARKGREY);

    // ---- CO2 ----
    uint16_t co2color = TFT_GREEN;
    if      (scd40.getCO2() > 1200) co2color = TFT_RED;
    else if (scd40.getCO2() > 800)  co2color = TFT_YELLOW;

    tft.drawRect(x0, y0, w, h, co2color);
    tft.setTextColor(co2color, TFT_BLACK, true);
    tft.setCursor(x0 + 10, y0 + 8);  tft.print("Углекислый газ");
    tft.setTextColor(co2color, TFT_BLACK, true);
    tft.setCursor(x0 + 10, y0 + 40); tft.print(String(scd40.getCO2()) + " ppm   ");

    // ---- T / H ----
    tft.drawRect(x0 + w + gap, y0, w, h, TFT_ORANGE);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK, true);
    tft.setCursor(x0 + w + gap + 10, y0 + 8);  tft.print("Темпер / Влажн");
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(x0 + w + gap + 10, y0 + 40);
    tft.print(String(scd40.getTemperature(), 1) + "C / " + String(scd40.getHumidity(), 1) + "%  ");


    // ---- Давление ----
    tft.drawRect(x0, y0 + h + gap, w, h, TFT_CYAN);
    tft.setTextColor(TFT_CYAN, TFT_BLACK, true);
    tft.setCursor(x0 + 10, y0 + h + gap + 8);  tft.print("Давление ");
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(x0 + 10, y0 + h + gap + 40);
    tft.print(String("Заглушка"));

    // ---- Освещённость ----
    tft.drawRect(x0 + w + gap, y0 + h + gap, w, h, TFT_YELLOW);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK, true);
    tft.setCursor(x0 + w + gap + 10, y0 + h + gap + 8);  tft.print("Уровень света    ");
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(x0 + w + gap + 10, y0 + h + gap + 40);
    tft.print(String("Заглушка"));

}

// ====================== ЭКРАН CO2 ======================
void drawScreen1() {
    // Статичные заголовки — setTextColor с фоном, повторная печать не мерцает
    tft.setTextColor(TFT_CYAN, TFT_BLACK, true);
    tft.setCursor(15, 15);
    tft.print("Уровень углекислого газа");

    uint16_t co2 = scd40.getCO2();
    uint16_t color = TFT_GREEN;
    if      (co2 > 1200) color = TFT_RED;
    else if (co2 > 800)  color = TFT_YELLOW;

    // Крупное значение
    tft.setTextColor(color, TFT_BLACK, true);
    tft.setCursor(25, 80);
    tft.setTextColor(color, TFT_BLACK, true);
    char buf[16];
    sprintf(buf, "%-4d ppm", co2); // CO2 до 9999, 4 символа
    tft.print(buf);


    // Прогресс-бар — две части вместо fillRect на весь бар
    float pct = constrain((float)co2 / 1200.0, 0.0, 1.0);
    int bx = 25, by = 160, bw = 420, bh = 24;
    int fw = (int)(bw * pct);
    tft.fillRect(bx,      by, fw,      bh, color);
    tft.fillRect(bx + fw, by, bw - fw, bh, TFT_DARKGREY);
    tft.drawRect(bx,      by, bw,      bh, TFT_WHITE);

    // Метки бара
    tft.setTextColor(TFT_PURPLE, TFT_BLACK, true);
    tft.setCursor(25,  195); tft.print("Хороший ");
    tft.setCursor(185, 195); tft.print("Умеренный ");
    tft.setCursor(360, 195); tft.print("Высокий ");

    // Температура и влажность
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 235);
    tft.print("Температура: "); tft.print(scd40.getTemperature(), 1); tft.print(" C   ");
    tft.setCursor(25, 265);
    tft.print("Влажность:  "); tft.print(scd40.getHumidity(), 1);    tft.print(" %   ");

}

// ====================== ЭКРАН ДАВЛЕНИЯ ======================
void drawScreen2() {
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK, true);
    tft.setCursor(15, 15);
    tft.print("Экран заглушка");

    //float press = bme280.getPressure_hPa();
    // tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    // tft.setCursor(25, 70);
    // uint16_t color = TFT_DARKGREY;
    // if      (press < 1010) color = TFT_ORANGE;
    // else if (press > 1015) color = TFT_SKYBLUE;
    // else                   color = TFT_PURPLE; 
    // tft.setTextColor(color, TFT_BLACK, true);
    // char buf[16];
    // sprintf(buf, "%-7.1f hPa", press); // 1013.2, 7 символов
    // tft.print(buf);


    // // Строка статуса — фиксированная ширина пробелами
    // tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK, true);
    // tft.setCursor(25, 140);
    // if      (press > 1015) tft.print("Стабильное высокое давление          ");
    // else if (press < 1010) tft.print("Низкое давление - риск дождя высокий ");
    // else                   tft.print("Нормальное давление                  ");

    // tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    // tft.setCursor(25, 190);
    // tft.print("Влажность: ");
    // tft.print(bme280.getHumidity(), 1); 
    // tft.print(" %");

    // tft.setTextSize(2);
    // tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    // tft.setCursor(25, 230);
    // tft.print("Температура: ");
    // tft.print(bme280.getTemperature_C(), 1); 
    // tft.print(" C");

    // tft.setTextColor(TFT_DARKGREY, TFT_BLACK, true);
    // tft.setCursor(25, 300);
    // tft.print("Над уровнем моря: 1015 hPa");

}

// ====================== ЭКРАН ОСВЕЩЁННОСТИ ======================
void drawScreen3() {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK, true);
    tft.setCursor(15, 15);
    tft.print("Экран заглушка");

    // float lux = light.getLux();
    // tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    // tft.setCursor(25, 100);
    // char buf[12];
    // sprintf(buf, "%-4.0f lx", lux);
    // tft.print(buf);


    // // Прогресс-бар
    // float pct = constrain(lux / 2000.0, 0.0, 1.0);
    // int bx = 25, by = 170, bw = 400, bh = 24;
    // int fw = (int)(bw * pct);
    // tft.fillRect(bx,      by, fw,      bh, TFT_YELLOW);
    // tft.fillRect(bx + fw, by, bw - fw, bh, TFT_DARKGREY);
    // tft.drawRect(bx,      by, bw,      bh, TFT_WHITE);

    // tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    // tft.setCursor(25,  210); tft.print("Темно  ");
    // tft.setCursor(195, 210); tft.print("Офис");
    // tft.setCursor(350, 210); tft.print("Солнечно ");

}

// ====================== ЭКРАН ПОГОДЫ(API) ======================
void drawScreen4() {
    int tzOffset = weather.getTimezoneOffset();
    String sunriceStr = formatTimestamp(weather.getSunrise(), tzOffset);
    String sunsetStr = formatTimestamp(weather.getSunset(), tzOffset);
    String daylight = getDayLightDuration(weather.getSunrise(), weather.getSunset());

    // Заголовок
    tft.setTextColor(TFT_BLUE, TFT_BLACK, true);
    tft.setCursor(15, 15);
    tft.print("Погода: " + weather.getCity());

    // Проверка валидности данных
    if (!weather.isDataFresh() && WiFi.status() != WL_CONNECTED) {
        tft.setTextColor(TFT_RED, TFT_BLACK, true);
        tft.setCursor(15, 60);
        tft.print("Нет соединения с интернетом");
        tft.setCursor(15, 100);
        tft.print("Показываю кэш");
    }

    // Температура
    tft.setTextColor(TFT_PINK, TFT_BLACK, true);
    tft.setCursor(25, 50);
    char tempBuf[16];
    sprintf(tempBuf, "%.1f C", weather.getTemp());
    tft.print(tempBuf);
    
    // Ощущается как
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 90);
    tft.print("Ощущается как: " + String(weather.getFeelsLike(), 1) + " C");
    
    // Влажность
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 120);
    tft.print("Влажность: " + String(weather.getHumidity()) + " %");
    
    // Давление
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 150);
    tft.print("Давление: " + String(weather.getPressure_hPa(), 1) + " hPa");
    
    // Ветер
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 180);
    tft.print("Ветер: " + String(weather.getWindSpeed(), 1) + " м/с, " + getWindDirection(weather.getWindDeg()));
    
    // Облачность
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 210);
    tft.print("Облачность: " + String(weather.getClouds()) + " %");
    
    // Описание
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK, true);
    tft.setCursor(25, 240);
    tft.print(weather.getDescription());
    
    // Закат/восход
    tft.setTextColor(TFT_ORANGE, TFT_BLACK, true);
    tft.setCursor(25, 270);
    tft.print("Восх/Зак(Прод): " + String(sunriceStr) + " / ");
    tft.print(String(sunsetStr) + " (" + daylight + ")");

    // Время последнего обновления
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK, true);
    tft.setCursor(25, 300);
    unsigned long age = (millis() - weather.getLastUpdateTime()) / 60000UL; // минуты
    tft.print("Обновлено " + String(age) + " мин назад");
    
}

// ====================== ЭКРАН КРИПТЫ(API) ======================
void drawScreen5(){

    tft.setTextColor(TFT_GOLD, TFT_BLACK, true);
    tft.setCursor(15, 10);
    tft.print("КРИПТА");

    if (!crypto.isDataValid()){
        tft.setTextColor(TFT_RED, TFT_BLACK, true);
        tft.setCursor(15, 50);
        tft.print("NO DATA");
        tft.setCursor(15, 80);
        tft.print(crypto.getLastError());
        return;
    }

     // === Левая колонка: цены монет ===
    int x = 15;
    int y = 50;
    int lineHeight = 35;
    
    // Bitcoin
    float btc = crypto.getBTCPrice();
    float btcCh = crypto.getBTCChange24h();
    tft.setTextColor(TFT_ORANGE, TFT_BLACK, true);
    tft.setCursor(x, y);
    tft.print("BTC:");
    tft.setTextColor(getColorForChange(btcCh), TFT_BLACK, true);
    tft.printf(" $%.2f (%.2f%%)", btc, btcCh);
    
    // Ethereum
    float eth = crypto.getETHPrice();
    float ethCh = crypto.getETHChange24h();
    tft.setCursor(x, y + lineHeight);
    tft.setTextColor(TFT_BLUE, TFT_BLACK, true);
    tft.print("ETH:");
    tft.setTextColor(getColorForChange(ethCh), TFT_BLACK, true);
    tft.printf(" $%.2f (%.2f%%)", eth, ethCh);
    
    // Solana
    float sol = crypto.getSOLPrice();
    float solCh = crypto.getSOLChange24h();
    tft.setCursor(x, y + lineHeight * 2);
    tft.setTextColor(TFT_PURPLE, TFT_BLACK, true);
    tft.print("SOL:");
    tft.setTextColor(getColorForChange(solCh), TFT_BLACK, true);
    tft.printf(" $%.2f (%.2f%%)", sol, solCh);
    
    // XRP
    float xrp = crypto.getXRPPrice();
    float xrpCh = crypto.getXRPChange24h();
    tft.setCursor(x, y + lineHeight * 3);
    tft.setTextColor(TFT_SKYBLUE, TFT_BLACK, true);
    tft.print("XRP:");
    tft.setTextColor(getColorForChange(xrpCh), TFT_BLACK, true);
    tft.printf(" $%.4f (%.2f%%)", xrp, xrpCh);
    
    // Chainlink
    float link = crypto.getLINKPrice();
    float linkCh = crypto.getLINKChange24h();
    tft.setCursor(x, y + lineHeight * 4);
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK, true);
    tft.print("LINK:");
    tft.setTextColor(getColorForChange(linkCh), TFT_BLACK, true);
    tft.printf(" $%.2f (%.2f%%)", link, linkCh);
    
    // === Правая колонка: рыночные индексы ===
    int rightX = 290;
    int rightY = 50;
    int rightLineHeight = 65;
    
    // Altcoin Season Index
    int altIndex = crypto.getAltcoinIndex();
    uint16_t altColor;
    String altStatus;
    if (altIndex > 75) {
        altColor = TFT_GREEN;
        altStatus = "АЛЬТСЕЗОН!";
    } else if (altIndex < 25) {
        altColor = TFT_RED;
        altStatus = "BTC СЕЗОН";
    } else {
        altColor = TFT_YELLOW;
        altStatus = "Смешано";
    }
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(rightX, rightY);
    tft.print("Индекс альтсез");
    tft.setTextColor(altColor, TFT_BLACK, true);
    tft.setCursor(rightX, rightY + 25);
    tft.printf("%d (%s)", altIndex, altStatus.c_str());
    
    // Fear & Greed Index
    int fg = crypto.getFearGreedValue();
    String fgClass = crypto.getFearGreedClassification();
    uint16_t fgColor = getColorForFearGreed(fg);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(rightX, rightY + rightLineHeight);
    tft.print("Жадн/Страх");
    tft.setTextColor(fgColor, TFT_BLACK, true);
    tft.setCursor(rightX, rightY + rightLineHeight + 25);
    tft.printf("%d (%s)", fg, fgClass.c_str());
    
    int startY = rightY + rightLineHeight + 70;

    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(rightX, startY);
    tft.print("Курсы валют");

    // USD
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK, true);
    tft.setCursor(rightX, startY + 25);
    tft.printf("USD: %.2f", currency.getUSDRate());
    tft.printf("тг");
    // RUB
    tft.setTextColor(TFT_SKYBLUE, TFT_BLACK, true);
    tft.setCursor(rightX, startY + 50);
    tft.printf("RUB: %.2f", currency.getRUBRate());
    tft.printf("тг");


    // CNY
    tft.setTextColor(TFT_ORANGE, TFT_BLACK, true);
    tft.setCursor(rightX, startY + 75);
    tft.printf("CNY: %.2f", currency.getCNYRate());
    tft.printf("тг");

    // Время последнего обновления
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK, true);
    tft.setCursor(25, 300);
    unsigned long age = (millis() - weather.getLastUpdateTime()) / 60000UL; // минуты
    tft.print("Обновлено " + String(age) + " мин назад");
    
}