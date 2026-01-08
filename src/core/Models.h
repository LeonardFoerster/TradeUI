#pragma once
#include <vector>
#include <random>


struct Candle
{
    double time;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

struct OrderBookEntry
{
    double price;
    double volume;
    bool is_bid;
};

struct Trade
{
    double time;
    double price;
    double amount;
    bool is_buy;
};

struct PositionInfo
{
    double amount = 0.0;
    double entry_price = 0.0;
    double unrealized_pnl = 0.0;
};

enum OrderType
{
    ORDER_LIMIT = 0,
    ORDER_MARKET = 1,
    ORDER_FOK = 2
};

struct MyOrder
{
    int id;
    bool is_buy;
    double price;
    double amount;
    int order_type;
    double time;
    bool reduce_only = false;
};

struct TradingState
{
    std::vector<Candle> candles;
    std::vector<OrderBookEntry> bids;
    std::vector<OrderBookEntry> asks;
    std::vector<Trade> trade_history;

    double current_price = 42000.0;
    double last_update_time = 0.0;
    std::mt19937 rng;

    double balance = 50000.0;
    double equity = 50000.0;
    
    std::vector<double> equity_history;
    double max_equity = 50000.0;
    double max_drawdown = 0.0;
    int total_trades_count = 0;
    int winning_trades = 0;
    double gross_profit = 0.0;
    double gross_loss = 0.0;

    PositionInfo long_pos;
    PositionInfo short_pos;
    
    std::vector<MyOrder> open_orders;
    std::vector<MyOrder> order_history;
    int order_id_counter = 1;

    char symbol[16] = "BTC/USD";
    int timeframe_idx = 1; 
    
    int order_type = 0;
    bool is_reduce_mode = false;
    float order_amount = 0.1f;
    float order_price = 42000.0f;

    double simulation_update_interval_s = 1.0;
    int simulation_interval_idx = 2;
    bool is_paused = false;
};
