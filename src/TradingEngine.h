#pragma once
#include "Models.h"
#include <vector>
#include <random>

class TradingEngine {
public:
    TradingState state;

    TradingEngine();
    void Init();
    void Update(double dt);
    
    // Trading Actions
    void PlaceOrder(bool is_buy, bool is_market, double price, double amount);
    void CancelOrder(int index);
    void ClosePosition();

private:
    void UpdateAccount();
    void ExecuteFill(bool is_buy, double price, double amount);
    void CheckLimitOrders();
    void GenerateMarketData();
};
