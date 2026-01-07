#pragma once
#include <vector>
#include <random>

// =================================================================================================
// DATA MODEL
// =================================================================================================

struct Candle {
    double time;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

struct OrderBookEntry {
    double price;
    double volume;
    bool is_bid; // true = bid (buy), false = ask (sell)
};

struct Trade {
    double time;
    double price;
    double amount;
    bool is_buy; // true = green, false = red
};

struct MyPosition {
    double amount = 0.0;
    double entry_price = 0.0;
    double unrealized_pnl = 0.0;
};

struct MyOrder {
    int id;
    bool is_buy;
    double price;
    double amount;
    bool is_limit; 
};

struct TradingState {
    // Market Data
    std::vector<Candle> candles;
    std::vector<OrderBookEntry> bids;
    std::vector<OrderBookEntry> asks;
    std::vector<Trade> trade_history;

    // Simulation State
    double current_price = 42000.0;
    double last_update_time = 0.0;
    std::mt19937 rng;

    // Account
    double balance = 50000.0;
    double equity = 50000.0;
    MyPosition position;
    std::vector<MyOrder> open_orders;
    std::vector<MyOrder> order_history;
    int order_id_counter = 1;

    // UI State (Persistent View State)
    char symbol[16] = "BTC/USD";
    int timeframe_idx = 1; 
    
    // Order Entry State
    int order_type = 0; // 0=Limit, 1=Market
    float order_amount = 0.1f;
    float order_price = 42000.0f;

    // Simulation Speed Control
    double simulation_update_interval_s = 1.0; // Default 1 second
    int simulation_interval_idx = 2; // Index for 1.0s in intervals array
    bool is_paused = false;
};
