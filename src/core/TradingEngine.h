#pragma once
#include "Models.h"
#include <vector>
#include <random>

class TradingEngine
{
public:
    TradingState state;

    TradingEngine();
    void Init();
    void Update(double dt);
    
    void PlaceOrder(bool is_buy, int order_type, double price, double amount, bool reduce_only = false);
    void CancelOrder(int index);
    void ClosePosition(bool close_long, bool close_short);

    std::vector<Candle> GetCandles(int timeframe_idx) const;

private:
    void UpdateAccount();
    void ExecuteFill(bool is_buy, double price, double amount, bool reduce_only, int order_type);
    void CheckLimitOrders();
    void GenerateMarketData();
};
