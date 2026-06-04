//CurrencyFetcher.h

#ifndef CURRENCY_FETCHER_H
#define CURRENCY_FETCHER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class CurrencyFetcher {
    public: 
        CurrencyFetcher();

        void init();
        void update();

        float getUSDRate() const { return _usdToKzt; }
        float getRUBRate() const { return _rubToKzt; }
        float getCNYRate() const { return _cnyToKzt; }

        bool isDataValid() const {return _dataValid;}
        String getLastError() const {return _lastError;}
        unsigned long getLastUpdateTime() const {return _lastUpdateTime;}

        bool forceUpdate();

    private:
        bool _fetchData();
        void _parseJson(const String& json);

        float _usdToKzt;
        float _rubToKzt;
        float _cnyToKzt;
        float _eurToUsd;

        bool _dataValid;
        String _lastError;
        unsigned long _lastUpdateTime;
        const unsigned long _updateInterval = 10*60*1000UL; // 10 минут
};

#endif