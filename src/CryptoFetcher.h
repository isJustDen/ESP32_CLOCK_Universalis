//CryptoFetcher.h

#ifndef CRYPTO_FETCHER_H
#define CRYPTO_FETCHER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class CryptoFetcher {
    public: 
        CryptoFetcher();

        void init(String apiKey);

        void update();

        bool isDataFresh() const;

        // === Цены и изменения для отдельных монет ===
        float getBTCPrice() const { return _btcPrice; }
        float getBTCChange24h() const { return _btcChange; }

        float getETHPrice() const { return _ethPrice; }
        float getETHChange24h() const { return _ethChange; }

        float getSOLPrice() const { return _solPrice; }
        float getSOLChange24h() const { return _solChange; }

        float getXRPPrice() const { return _xrpPrice; }
        float getXRPChange24h() const { return _xrpChange; }

        float getLINKPrice() const { return _linkPrice; }
        float getLINKChange24h() const { return _linkChange; }

        // === Рыночные индикаторы ===
        int getAltcoinIndex() const { return _altcoinIndex; }
        int getFearGreedValue() const { return _fearGreedValue; }
        String getFearGreedClassification() const { return _fearGreedClassification; }

        // Общее состояние
        bool isDataValid() const { return _dataValid; }
        unsigned long getLastUpdateTime() const { return _lastUpdateTime; }
        String getLastError() const { return _lastError; }

        bool forceUpdate();

    private:
        bool _fetchPrices();
        bool _fetchMarketIndices();
        void _parsePrices(const String& json);
        void _parseAltcoinIndex(const String& json);
        void _parseFearGreed(const String& json);

        float _btcPrice, _btcChange;
        float _ethPrice, _ethChange;
        float _solPrice, _solChange;
        float _xrpPrice, _xrpChange;
        float _linkPrice, _linkChange;

        int _altcoinIndex;
        int _fearGreedValue;
        String _fearGreedClassification;

        String _apiKey;
        bool _dataValid;
        String _lastError;
        unsigned long _lastUpdateTime;
        const unsigned long _updateInterval = 20 * 60 * 1000UL; //20 мин
        
};

#endif