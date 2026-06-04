//CurrencyFetcher.cpp

#include "CurrencyFetcher.h"

CurrencyFetcher::CurrencyFetcher() : _usdToKzt(0), _rubToKzt(0), _cnyToKzt(0),_eurToUsd(0),
                                    _dataValid(false), _lastUpdateTime(0), _lastError("") {}

void CurrencyFetcher::init(){
    Serial.println("CurrencyFetcher initialized");
    vTaskDelay(2000/portTICK_PERIOD_MS);
    _fetchData();
}

void CurrencyFetcher::update(){
    if ( (!_dataValid || (millis() - _lastUpdateTime >= _updateInterval)) && WiFi.status() == WL_CONNECTED ) {
        if (_fetchData()) {
            _lastUpdateTime = millis();
            _dataValid = true;
            _lastError = "";
            Serial.println("Currency data updated.");
        } else {
            _dataValid = false;
            Serial.println("Currency update failed");
        }
    }
}

bool CurrencyFetcher::forceUpdate(){
    if (WiFi.status() != WL_CONNECTED) {
        _lastError = "No WiFi connection";
        return false;
    }
    if (_fetchData()) {
        _lastUpdateTime = millis();
        _dataValid = true;
        return true;
    }
    return false;
}

bool CurrencyFetcher::_fetchData(){
    HTTPClient http;

    String url = "https://open.er-api.com/v6/latest/USD";

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != 200) {
        _lastError = "Currency HTTP error: " + String(httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    _parseJson(payload);
    return true;
}

void CurrencyFetcher::_parseJson(const String& json){
    const size_t capacity = JSON_OBJECT_SIZE(5) + 256;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        _lastError = "Currency JSON parse failed" + String(error.c_str());
        return;
    }

   String resultStatus = doc["result"] | "";
    if (resultStatus != "success") {
        _lastError = "API error: Request failed";
        return;
    }

    JsonObject rates = doc["rates"];
    if (rates.isNull()){
        _lastError = "No 'rates' object in json reponce";
        return;
    }

    float kzt_rate = rates["KZT"] | 0.0;
    float rub_rate = rates["RUB"] | 0.0;
    float cny_rate = rates["CNY"] | 0.0;

    if (kzt_rate == 0.0 || rub_rate == 0.0 || cny_rate == 0.0) {
        _lastError = "One or more currency rates npt found or invalid";
        return;
    }

    _usdToKzt = kzt_rate; //1 доллар 486тг
    _rubToKzt = _usdToKzt/ rub_rate;
    _cnyToKzt = _usdToKzt/ cny_rate;
    _dataValid = true;
    _lastError = "";
}
