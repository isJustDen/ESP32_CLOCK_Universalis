//main.cpp

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "SCD40_sensor.h"
#include "DS3231_RTC.h"
#include "BME280_sensor.h"
#include "TEMT6000_sensor.h"
#include "Joystick.h"
#include <SPIFFS.h>

// ========== Объекты ==========
TFT_eSPI tft = TFT_eSPI(); 
SCD40_sensor scd40;
DS3231_RTC rtc;
BME280_sensor bme280;
TEMT6000_sensor light;
Joystick joy;

// ========== Пины ==========
// Джойстик
#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 15
// TEMT6000
#define LIGHT_PIN 36

// ========== Переменные для экрана и меню ==========
int currentScreen = 0; // 0 - главный (часы+сводка), 1 - CO2+темп, 2 - давление, 3 - освещённость
const int numScreens = 4;
unsigned long lastRTCupdate = 0;
unsigned long lastSensorReadAll = 0;
const unsigned long RTC_INTERVAL = 1000;
const unsigned long SENSOR_INTERVAL = 2000;

// === Прототипы функций ===
void initDisplay();
void updateDisplay();
void drawScreen0(); // главный экран
void drawScreen1(); // CO2 и климат от SCD40
void drawScreen2(); // давление и температура BME280
void drawScreen3(); // освещённость
void drawBlock();



// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);
  Serial.println("AegisDesk starting..."); 

  initDisplay(); // инициализация дисплея

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
    if (!bme280.init(21, 22)) {
        tft.setCursor(10, 150);
        tft.setTextColor(TFT_RED, TFT_BLACK, true);
        tft.println("BME280 ERROR!");
        Serial.println("BME280 initialization failed");
    }
    light.init(LIGHT_PIN);
    joy.init(JOY_X, JOY_Y, JOY_SW);

    delay(2000);
    tft.fillScreen(TFT_BLACK);
     drawScreen0(); // рисуем начальный экран
} 



// ====================== LOOP ======================
void loop() {
    // 1. Обновляем джойстик и обрабатываем нажатия (переключение экранов)
    joy.update();
    if (joy.justPressed()){
        // нажатие на джойстик: вызывает дополнительное действие, например, обновить все датчики принудительно
        // пока пропустим
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
            drawScreen0;
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
    if (millis() - lastSensorReadAll >= SENSOR_INTERVAL) {
        bme280.read();
        light.read();
        lastSensorReadAll = millis();
        if (currentScreen == 2) drawScreen2();
        if (currentScreen == 3) drawScreen3();
        if (currentScreen == 0) drawScreen0();
    }

  vTaskDelay(25/portTICK_PERIOD_MS);    // Небольшая задержка, чтобы не грузить процессор
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

    // ---- Время и дата ----
    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(15, 15);
    tft.print(rtc.getTimeString());
    tft.unloadFont();

    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(240, 15);
    tft.print(rtc.getDateTimeString().substring(0, 10));
    tft.unloadFont();

    tft.drawLine(5, 55, 440, 55, TFT_DARKGREY);

    // ---- CO2 ----
    uint16_t co2color = TFT_GREEN;
    if      (scd40.getCO2() > 1200) co2color = TFT_RED;
    else if (scd40.getCO2() > 800)  co2color = TFT_YELLOW;

    tft.drawRect(x0, y0, w, h, co2color);
    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(co2color, TFT_BLACK, true);
    tft.setCursor(x0 + 10, y0 + 8);  tft.print("Углекислый газ");
    tft.setTextColor(co2color, TFT_BLACK, true);
    tft.setCursor(x0 + 10, y0 + 40); tft.print(String(scd40.getCO2()) + " ppm   ");
    tft.unloadFont();

    // ---- T / H ----
    tft.drawRect(x0 + w + gap, y0, w, h, TFT_ORANGE);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK, true);
    tft.loadFont("Arial24", SPIFFS);
    tft.setCursor(x0 + w + gap + 10, y0 + 8);  tft.print("Темпер / Влажн");
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(x0 + w + gap + 10, y0 + 40);
    tft.print(String(scd40.getTemperature(), 1) + "C / " + String(scd40.getHumidity(), 1) + "%  ");
    tft.unloadFont();


    // ---- Давление ----
    tft.drawRect(x0, y0 + h + gap, w, h, TFT_CYAN);
    tft.setTextColor(TFT_CYAN, TFT_BLACK, true);
    tft.loadFont("Arial24", SPIFFS);
    tft.setCursor(x0 + 10, y0 + h + gap + 8);  tft.print("Давление ");
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(x0 + 10, y0 + h + gap + 40);
    tft.print(String((bme280.getPressure_hPa())* 0.75006, 1) + " mmHg");
    tft.unloadFont();

    // ---- Освещённость ----
    tft.drawRect(x0 + w + gap, y0 + h + gap, w, h, TFT_YELLOW);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK, true);
    tft.loadFont("Arial24", SPIFFS);
    tft.setCursor(x0 + w + gap + 10, y0 + h + gap + 8);  tft.print("Уровень света    ");
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(x0 + w + gap + 10, y0 + h + gap + 40);
    tft.print(String(light.getLux(), 0) + " lx       ");
    tft.unloadFont();

    // ---- Подсказка ----
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK, true);
    tft.loadFont("Arial24", SPIFFS);
    tft.setCursor(15, 320);
    tft.print("<  > для изменения экрана");
    tft.unloadFont();

}

// ====================== ЭКРАН CO2 ======================
void drawScreen1() {
    // Статичные заголовки — setTextColor с фоном, повторная печать не мерцает
    tft.setTextColor(TFT_CYAN, TFT_BLACK, true);
    tft.loadFont("Arial24", SPIFFS);
    tft.setCursor(15, 15);
    tft.print("Уровень углекислого газа");
    tft.unloadFont();

    uint16_t co2 = scd40.getCO2();
    uint16_t color = TFT_GREEN;
    if      (co2 > 1200) color = TFT_RED;
    else if (co2 > 800)  color = TFT_YELLOW;

    // Крупное значение
    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(color, TFT_BLACK, true);
    tft.setCursor(25, 80);
    tft.setTextColor(color, TFT_BLACK, true);
    char buf[16];
    sprintf(buf, "%-4d ppm", co2); // CO2 до 9999, 4 символа
    tft.print(buf);
    tft.unloadFont();


    // Прогресс-бар — две части вместо fillRect на весь бар
    float pct = constrain((float)co2 / 1200.0, 0.0, 1.0);
    int bx = 25, by = 160, bw = 420, bh = 24;
    int fw = (int)(bw * pct);
    tft.fillRect(bx,      by, fw,      bh, color);
    tft.fillRect(bx + fw, by, bw - fw, bh, TFT_DARKGREY);
    tft.drawRect(bx,      by, bw,      bh, TFT_WHITE);

    // Метки бара
    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(TFT_PURPLE, TFT_BLACK, true);
    tft.setCursor(25,  195); tft.print("Хороший ");
    tft.setCursor(185, 195); tft.print("Умеренный ");
    tft.setCursor(360, 195); tft.print("Высокий ");
    tft.unloadFont();

    // Температура и влажность
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.loadFont("Arial24", SPIFFS);
    tft.setCursor(25, 235);
    tft.print("Температура: "); tft.print(scd40.getTemperature(), 1); tft.print(" C   ");
    tft.setCursor(25, 265);
    tft.print("Влажность:  "); tft.print(scd40.getHumidity(), 1);    tft.print(" %   ");
    tft.unloadFont();


    tft.setTextColor(TFT_DARKGREY, TFT_BLACK, true);
    tft.setCursor(15, 340);
    tft.loadFont("Arial24", SPIFFS);
    tft.print("<  > для изменения экрана");
    tft.unloadFont();

}

// ====================== ЭКРАН ДАВЛЕНИЯ ======================
void drawScreen2() {
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK, true);
    tft.loadFont("Arial24", SPIFFS);
    tft.setCursor(15, 15);
    tft.print("Атмосферное давление");
    tft.unloadFont();

    float press = bme280.getPressure_hPa();
    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 70);
    uint16_t color = TFT_DARKGREY;
    if      (press < 1010) color = TFT_ORANGE;
    else if (press > 1015) color = TFT_SKYBLUE;
    else                   color = TFT_PURPLE; 
    tft.setTextColor(color, TFT_BLACK, true);
    char buf[16];
    sprintf(buf, "%-7.1f hPa", press); // 1013.2, 7 символов
    tft.print(buf);
    tft.unloadFont();


    // Строка статуса — фиксированная ширина пробелами
    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK, true);
    tft.setCursor(25, 140);
    if      (press > 1015) tft.print("Стабильное высокое давление          ");
    else if (press < 1010) tft.print("Низкое давление - риск дождя высокий ");
    else                   tft.print("Нормальное давление                  ");

    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 190);
    tft.print("Влажность: ");
    tft.print(bme280.getHumidity(), 1); 
    tft.print(" %");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 230);
    tft.print("Температура: ");
    tft.print(bme280.getTemperature_C(), 1); 
    tft.print(" C");

    tft.setTextColor(TFT_DARKGREY, TFT_BLACK, true);
    tft.setCursor(25, 270);
    tft.print("Над уровнем моря: 1013 hPa");
    tft.unloadFont();

    tft.setCursor(15, 340);
    tft.loadFont("Arial24", SPIFFS);
    tft.print("<  > для изменения экрана");
    tft.unloadFont();

}

// ====================== ЭКРАН ОСВЕЩЁННОСТИ ======================
void drawScreen3() {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK, true);
    tft.loadFont("Arial24", SPIFFS);
    tft.setCursor(15, 15);
    tft.print("Освещенность");
    tft.unloadFont();

    float lux = light.getLux();
    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25, 100);
    char buf[12];
    sprintf(buf, "%-4.0f lx", lux);
    tft.print(buf);
    tft.unloadFont();


    // Прогресс-бар
    float pct = constrain(lux / 2000.0, 0.0, 1.0);
    int bx = 25, by = 170, bw = 400, bh = 24;
    int fw = (int)(bw * pct);
    tft.fillRect(bx,      by, fw,      bh, TFT_YELLOW);
    tft.fillRect(bx + fw, by, bw - fw, bh, TFT_DARKGREY);
    tft.drawRect(bx,      by, bw,      bh, TFT_WHITE);

    tft.loadFont("Arial24", SPIFFS);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setCursor(25,  210); tft.print("Темно  ");
    tft.setCursor(195, 210); tft.print("Офис");
    tft.setCursor(350, 210); tft.print("Солнечно ");
    tft.unloadFont();

    tft.setTextColor(TFT_DARKGREY, TFT_BLACK, true);
    tft.setCursor(15, 340);
    tft.loadFont("Arial24", SPIFFS);
    tft.print("<  > для изменения экрана");
    tft.unloadFont();

}