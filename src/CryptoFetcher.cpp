//CryptoFetcher.cpp

#include "CryptoFetcher.h"

CryptoFetcher::CryptoFetcher(): 
        _btcPrice(0), _btcChange(0), _ethPrice(0), _ethChange(0), _solPrice(0), _solChange(0), 
        _xrpPrice(0), _xrpChange(0), _linkPrice(0), _linkChange(0),  
        _altcoinIndex(0), _fearGreedValue(0), _fearGreedClassification(""),
        _dataValid(false), _lastUpdateTime(0), _lastError("") {}

void CryptoFetcher::init(String apiKey){
    _apiKey = apiKey;
    _fetchPrices();
    _fetchMarketIndices();
}

bool CryptoFetcher::isDataFresh() const {
    return _dataValid && (millis() - _lastUpdateTime < _updateInterval);
}

void CryptoFetcher::update() {
    if (!isDataFresh() && WiFi.status() == WL_CONNECTED){
        bool priceOk =     _fetchPrices();
        bool indicesOk = _fetchMarketIndices();

        if (priceOk && indicesOk) {
            _lastUpdateTime = millis();
            _dataValid = true;
        } else {
            _dataValid = false;
            _lastError = priceOk ? "Indices failed" : "Pricdes failed";
        }
    }
}

bool CryptoFetcher::forceUpdate() {
    if (WiFi.status() != WL_CONNECTED) {
        _lastError = "No WiFi connection";
        return false;
    }
    bool pricesOk = _fetchPrices();
    bool indicesOk = _fetchMarketIndices();
    if (pricesOk && indicesOk) {
        _lastUpdateTime = millis();
        _dataValid = true;
        _lastError = "";
        return true;
    }
    _dataValid = false;
    return false;
}

bool CryptoFetcher:: _fetchPrices() {
    if(_apiKey.length() == 0){
        _lastError = "NO API KEY";
        return false;
    }

    HTTPClient http;

    String url = "https://pro-api.coinmarketcap.com/v3/cryptocurrency/quotes/latest?id=1,1027,5426,52,1975&convert=USD";

    http.begin(url);
    http.addHeader("X-CMC_PRO_API_KEY", _apiKey);
    http.addHeader("Accept", "application/json");

    unsigned long t0 = millis();
    int httpCode = http.GET();
    Serial.print("PRICES HTTP: "); Serial.print(millis() - t0); 
    Serial.print(" ms, code: "); Serial.println(httpCode);

    if (httpCode != 200){
        _lastError = "Price HTTP error" + String(httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    _parsePrices(payload);
    return true;
}

void CryptoFetcher::_parsePrices(const String& json) {
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, json);
    if(error) {
        _lastError = "Price JSON parse failed";
        return;
    }

    if (doc.containsKey("status")){
        int errorCode = doc["status"]["error_code"] | 0;
        if (errorCode != 0) {
            _lastError = doc["status"]["error_message"] | "Unknown API error";
            _dataValid = false;
            return;
        }
    } 

     JsonArray dataArray = doc["data"];
    if (dataArray.isNull() || dataArray.size() == 0) {
        _lastError = "Empty data array";
        return;
    }

    // Проходим по массиву и ищем каждый ID
    for (JsonObject item : dataArray) {
        int id = item["id"] | 0;
        JsonArray quotes = item["quote"];
        if (quotes.isNull() || quotes.size() == 0) continue;
        JsonObject usdQuote = quotes[0];
        if (usdQuote.isNull()) continue;
        
        float price = usdQuote["price"] | 0.0;
        float change = usdQuote["percent_change_24h"] | 0.0;
        
        switch (id) {
            case 1:   // Bitcoin
                _btcPrice = price;
                _btcChange = change;
                break;
            case 1027: // Ethereum
                _ethPrice = price;
                _ethChange = change;
                break;
            case 5426: // Solana
                _solPrice = price;
                _solChange = change;
                break;
            case 52:   // XRP
                _xrpPrice = price;
                _xrpChange = change;
                break;
            case 1975: // Chainlink
                _linkPrice = price;
                _linkChange = change;
                break;
        }
    }

    // Если ни одна цена не обновилась – ошибка
    if (_btcPrice == 0 && _ethPrice == 0 && _solPrice == 0 && _xrpPrice == 0 && _linkPrice == 0) {
        _lastError = "No valid price data found";
        _dataValid = false;
    } else {
        _dataValid = true;
        _lastError = "";
    }
}

// --- Запрос рыночных индексов: альтсезон и страх/жадность ---
bool CryptoFetcher::_fetchMarketIndices() {
    bool success = true;

    // 1. Altcoin Season Index
    HTTPClient http;
    String urlAlt = "https://pro-api.coinmarketcap.com/v1/altcoin-season-index/latest";
    http.begin(urlAlt);
    http.addHeader("X-CMC_PRO_API_KEY", _apiKey);
    http.addHeader("Accept", "application/json");

    unsigned long t0 = millis();
    int httpCodeAlt = http.GET();
    Serial.print("ALTCOIN HTTP: "); Serial.print(millis() - t0);
    Serial.print(" ms, code: "); Serial.println(httpCodeAlt);

    if (httpCodeAlt == 200) {
        String payload = http.getString();
        _parseAltcoinIndex(payload);
    } else {
        success = false;
        _lastError = "Altcoin index error" + String(httpCodeAlt);
    }
    http.end();

    // 2. Fear & Greed Index
    String urlFng = "https://pro-api.coinmarketcap.com/v3/fear-and-greed/latest";
    http.begin(urlFng);
    http.addHeader("X-CMC_PRO_API_KEY", _apiKey);
    http.addHeader("Accept", "application/json");

    unsigned long t1 = millis(); 
    int httpCodeFng = http.GET();
    Serial.print("FEARGREED HTTP: "); Serial.print(millis() - t1);
    Serial.print(" ms, code: "); Serial.println(httpCodeFng);

    if (httpCodeFng == 200) {
        String payload = http.getString();
        _parseFearGreed(payload);
    } else {
        success = false;
        _lastError = "Fear & Greed error" + String(httpCodeFng);
    }
    http.end();
    
    return success;
}

// --- Парсинг Altcoin Season Index ---
void CryptoFetcher::_parseAltcoinIndex(const String& json){
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);
    if (error){
        _lastError = "Altcoin JSON parse failed";
        return;
    }

    JsonObject data = doc["data"];
    _altcoinIndex = data["altcoin_index"] | 0;
}

// --- Парсинг Fear & Greed Index ---
void CryptoFetcher::_parseFearGreed(const String& json){
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);
    if (error){
        _lastError = "Fear & Greed JSON parse failed";
        return;
    }

    JsonObject data = doc["data"];
    _fearGreedValue = data["value"] | 0;
    _fearGreedClassification = data["value_classification"].as<String>();
}