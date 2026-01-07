#include "TradingEngine.h"
#include <chrono>
#include <algorithm>
#include <cmath>
#include <iostream>

TradingEngine::TradingEngine() {
    // Constructor
}

void TradingEngine::Init() {
    state.rng.seed(std::random_device{}());
    
    // Generate 200 initial candles so the chart isn't empty
    double now = (double)std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    double price = 42000.0;
    std::normal_distribution<double> walk(0.0, 50.0); // Price volatility
    std::uniform_real_distribution<double> noise(0.0, 15.0); // High/Low noise
    std::uniform_real_distribution<double> vol(1.0, 100.0); // Volume

    // Go back 200 minutes (assuming 1m candles for base sim)
    double time_step = 60.0;
    double start_time = now - (200 * time_step);

    for (int i = 0; i < 200; ++i) {
        double t = start_time + (i * time_step);
        double move = walk(state.rng);
        double open = price;
        double close = open + move;
        double high = std::max(open, close) + noise(state.rng);
        double low = std::min(open, close) - noise(state.rng);
        
        state.candles.push_back({t, open, high, low, close, vol(state.rng)});
        price = close;
    }
    state.current_price = price;
    state.order_price = (float)price;
}

void TradingEngine::UpdateAccount() {
    if (state.position.amount != 0.0) {
        state.position.unrealized_pnl = (state.current_price - state.position.entry_price) * state.position.amount;
    } else {
        state.position.unrealized_pnl = 0.0;
    }
    state.equity = state.balance + state.position.unrealized_pnl;
}

void TradingEngine::ExecuteFill(bool is_buy, double price, double amount) {
    double qty = is_buy ? amount : -amount;
    double new_total = state.position.amount + qty;
    
    // Check if adding to position (same side) or closing/flipping
    bool same_side = (state.position.amount >= 0 && qty >= 0) || (state.position.amount <= 0 && qty <= 0);
    
    if (state.position.amount == 0.0 || same_side) {
        // Weighted Average Entry Price
        double old_val = std::abs(state.position.amount) * state.position.entry_price;
        double new_val = std::abs(qty) * price;
        state.position.entry_price = (old_val + new_val) / (std::abs(state.position.amount) + std::abs(qty));
        state.position.amount += qty;
    } else {
        // Reducing or Flipping
        if (std::abs(qty) > std::abs(state.position.amount)) {
            // Flip Position
            double pnl = (price - state.position.entry_price) * state.position.amount;
            state.balance += pnl;
            
            // Open remainder in new direction
            double remainder = new_total; 
            state.position.amount = remainder;
            state.position.entry_price = price;
        } else {
            // Reduce Position
            double pnl = (price - state.position.entry_price) * (-qty);
            state.balance += pnl;
            state.position.amount += qty;
        }
    }
    
    // Cleanup tiny residuals
    if (std::abs(state.position.amount) < 0.000001) {
        state.position.amount = 0;
        state.position.entry_price = 0;
    }

    // Add to History
    state.order_history.insert(state.order_history.begin(), {0, is_buy, price, amount, false});
    if (state.order_history.size() > 50) state.order_history.pop_back();
    
    UpdateAccount();
}

void TradingEngine::PlaceOrder(bool is_buy, bool is_market, double price, double amount) {
    if (is_market) {
        // Simple fill simulation: fill at top of book
        double fill_p = is_buy ? state.asks[0].price : state.bids[0].price;
        ExecuteFill(is_buy, fill_p, amount);
    } else {
        state.open_orders.push_back({state.order_id_counter++, is_buy, price, amount, true});
    }
}

void TradingEngine::CancelOrder(int index) {
    if (index >= 0 && index < (int)state.open_orders.size()) {
        state.open_orders.erase(state.open_orders.begin() + index);
    }
}

void TradingEngine::ClosePosition() {
    if (state.position.amount == 0.0) return;
    bool close_buy = state.position.amount < 0; 
    PlaceOrder(close_buy, true, 0, std::abs(state.position.amount));
}

void TradingEngine::CheckLimitOrders() {
    for (auto it = state.open_orders.begin(); it != state.open_orders.end();) {
        bool hit = false;
        if (it->is_buy && state.current_price <= it->price) hit = true;
        else if (!it->is_buy && state.current_price >= it->price) hit = true;
        
        if (hit) {
            ExecuteFill(it->is_buy, it->price, it->amount);
            it = state.open_orders.erase(it);
        } else {
            ++it;
        }
    }
}

void TradingEngine::GenerateMarketData() {
    // 1. Add new candle data (or update last candle)
    std::normal_distribution<double> walk(0.0, 30.0);
    std::uniform_real_distribution<double> noise(0.0, 10.0);
    std::uniform_real_distribution<double> vol_dist(0.5, 10.0);

    double move = walk(state.rng);
    double prev_close = state.candles.back().close;
    double new_open = prev_close;
    double new_close = new_open + move;
    double new_high = std::max(new_open, new_close) + noise(state.rng);
    double new_low = std::min(new_open, new_close) - noise(state.rng);
    double new_time = state.candles.back().time + 60.0;

    state.candles.push_back({new_time, new_open, new_high, new_low, new_close, vol_dist(state.rng)});
    state.current_price = new_close;

    // Keep memory usage check
    if (state.candles.size() > 2000) state.candles.erase(state.candles.begin());

    // 2. Generate Order Book
    state.bids.clear();
    state.asks.clear();
    
    // Bids
    double p = state.current_price;
    for(int i=0; i<15; ++i) {
        p -= std::uniform_real_distribution<double>(1.0, 5.0)(state.rng);
        state.bids.push_back({p, std::uniform_real_distribution<double>(0.1, 5.0)(state.rng), true});
    }
    // Asks
    p = state.current_price;
    for(int i=0; i<15; ++i) {
        p += std::uniform_real_distribution<double>(1.0, 5.0)(state.rng);
        state.asks.push_back({p, std::uniform_real_distribution<double>(0.1, 5.0)(state.rng), false});
    }

    // 3. Generate a Trade
    if (std::uniform_real_distribution<double>(0.0, 1.0)(state.rng) > 0.3) {
        bool is_buy = std::uniform_int_distribution<int>(0, 1)(state.rng);
        state.trade_history.insert(state.trade_history.begin(), {
            new_time, 
            state.current_price + (is_buy ? 1.0 : -1.0), 
            std::uniform_real_distribution<double>(0.01, 2.0)(state.rng), 
            is_buy
        });
        if (state.trade_history.size() > 50) state.trade_history.pop_back();
    }
}

void TradingEngine::Update(double dt) {
    if (state.is_paused) return;

    // Only update every 1 second (or configured interval)
    static double accumulator = 0.0;
    accumulator += dt;
    if (accumulator < state.simulation_update_interval_s) return;
    accumulator = 0.0;

    GenerateMarketData();
    CheckLimitOrders();
    UpdateAccount();
}
