//WeatherFetcher.cpp

#include "WeatherFetcher.h"

WeatherFetcher::WeatherFetcher() : _temp(0), _feelsLike(0), _humidity(0), _pressure(0), _windSpeed(0), 
        _winDeg(0), _clouds(0), _description(""), _iconCode(""), _lastUpdateTime(0), _dataValid(false), 
        _lastError(""), _sunrise(0), _sunset(0), _timezoneOffset(0) {}

void WeatherFetcher::init(String city, String apiKey){
    _city = city;
    _apiKey = apiKey;
    _fetchData();     // Первый запрос делаем сразу при инициализации (если WiFi есть)
}

bool WeatherFetcher::isDataFresh() const {
    return _dataValid && ( millis() - _lastUpdateTime < _updateInterval);
}

void WeatherFetcher::update(){
    // Обновляем только если прошло достаточно времени и есть WiFi
    if (!isDataFresh() && WiFi.status() == WL_CONNECTED){
        _fetchData();
    }
}

bool WeatherFetcher::_fetchData() {
    if (_apiKey.length() == 0 || _city.length() == 0) {
        _lastError = "No Api key or city";
        _dataValid = false;
        return false;
    }

    HTTPClient http;
    // URL для текущей погоды в метрических единицах (цельсий, м/с, гПа)
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + _city +
                 "&units=metric&lang=ru&appid=" + _apiKey;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != 200) {
        _lastError = "HTTP error: " + String(httpCode);
        _dataValid = false;
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    // Парсинг JSON
    _parseJson(payload);
    _lastUpdateTime = millis();
    _dataValid = true;
    _lastError = "";
    return true;
}

void WeatherFetcher::_parseJson(const String& json) {
    // Используем DynamicJsonDocument с запасом памяти (около 2KB)
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        _lastError = "Json parse failed";
        _dataValid = false;
        return;
    }

    // Извлекаем поля согласно документации OpenWeatherMap
    JsonObject main = doc["main"];
    _temp = main["temp"];
    _feelsLike = main["feels_like"];
    _humidity = main["humidity"];
    _pressure = main["pressure"];

    JsonObject wind = doc["wind"];
    _windSpeed = wind["speed"];
    _winDeg = wind["deg"];

    JsonObject clouds = doc["clouds"];
    _clouds = clouds["all"]; // облачность %

    JsonArray weather = doc["weather"];
    if (weather.size() > 0) {
        _description = weather[0]["description"].as<String>();
        _iconCode = weather[0]["icon"].as<String>();
    } else {
        _description = "N/A";
        _iconCode = "01d";
    }

    JsonObject sys = doc["sys"];
    _sunrise = sys["sunrise"];
    _sunset = sys["sunset"];

    _timezoneOffset  = doc["timezone"] | 0;

    //{"coord":{"lon":69.1628,"lat":54.8753},
    //"weather":[{"id":804,"main":"Clouds","description":"пасмурно","icon":"04n"}],
    //"base":"stations",
    //"main":{"temp":13.21,"feels_like":12.15,"temp_min":13.21,
        //"temp_max":13.21,"pressure":1015,"humidity":60,"sea_level":1015,
        //"grnd_level":999},"visibility":10000,
    //"wind":{"speed":5.42,"deg":241,"gust":13.23},
    //"clouds":{"all":100},"dt":1779483762,
    //"sys":{"country":"KZ","sunrise":1779491054,"sunset":1779550573},
    //"timezone":18000,"id":1520172,"name":"Петропавловск","cod":200}
}