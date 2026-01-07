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
    
    // Init Equity History
    state.equity_history.push_back(state.equity);
}

std::vector<Candle> TradingEngine::GetCandles(int timeframe_idx) const {
    // Map idx to minutes
    // 0=1m, 1=5m, 2=15m, 3=1h, 4=4h, 5=D
    int minutes = 1;
    switch(timeframe_idx) {
        case 0: minutes = 1; break;
        case 1: minutes = 5; break;
        case 2: minutes = 15; break;
        case 3: minutes = 60; break;
        case 4: minutes = 240; break;
        case 5: minutes = 1440; break;
        default: minutes = 1; break;
    }

    if (minutes == 1 || state.candles.empty()) {
        return state.candles;
    }

    std::vector<Candle> aggregated;
    double group_seconds = minutes * 60.0;
    
    Candle current_candle = {};
    bool first = true;
    double bucket_start_time = 0.0;

    for (const auto& c : state.candles) {
        // Calculate which bucket this timestamp belongs to
        // We use floor to align to 00:00, 00:15 etc.
        double t = c.time;
        double bucket = std::floor(t / group_seconds) * group_seconds;

        if (first) {
            bucket_start_time = bucket;
            current_candle = c;
            current_candle.time = bucket; // Force align time
            first = false;
        } else {
            if (bucket != bucket_start_time) {
                // Push finished candle
                aggregated.push_back(current_candle);
                
                // Start new candle
                bucket_start_time = bucket;
                current_candle = c;
                current_candle.time = bucket;
            } else {
                // Update current candle
                current_candle.high = std::max(current_candle.high, c.high);
                current_candle.low = std::min(current_candle.low, c.low);
                current_candle.close = c.close;
                current_candle.volume += c.volume;
            }
        }
    }
    // Push last partial candle
    if (!first) {
        aggregated.push_back(current_candle);
    }

    return aggregated;
}

void TradingEngine::UpdateAccount() {
    // Long PnL
    if (state.long_pos.amount > 0.0) {
        state.long_pos.unrealized_pnl = (state.current_price - state.long_pos.entry_price) * state.long_pos.amount;
    } else {
        state.long_pos.unrealized_pnl = 0.0;
    }

    // Short PnL (amount is negative)
    if (state.short_pos.amount < 0.0) {
        state.short_pos.unrealized_pnl = (state.current_price - state.short_pos.entry_price) * state.short_pos.amount;
    } else {
        state.short_pos.unrealized_pnl = 0.0;
    }

    state.equity = state.balance + state.long_pos.unrealized_pnl + state.short_pos.unrealized_pnl;

    // Performance Stats (Drawdown)
    if (state.equity > state.max_equity) state.max_equity = state.equity;
    if (state.max_equity > 0) {
        double dd = (state.max_equity - state.equity) / state.max_equity * 100.0;
        if (dd > state.max_drawdown) state.max_drawdown = dd;
    }
    
    // History Recording (Limit size)
    state.equity_history.push_back(state.equity);
    if (state.equity_history.size() > 5000) {
        state.equity_history.erase(state.equity_history.begin());
    }
}

void TradingEngine::ExecuteFill(bool is_buy, double price, double amount, bool reduce_only, int order_type) {
    // Hedge Mode Logic
    PositionInfo* target_pos = nullptr;
    double qty = 0.0;
    bool is_opening = !reduce_only;

    if (is_buy) {
        if (!reduce_only) {
            // Open Long
            target_pos = &state.long_pos;
            qty = amount;
        } else {
            // Close Short (Buy to Cover)
            target_pos = &state.short_pos;
            qty = amount; 
        }
    } else {
        if (!reduce_only) {
            // Open Short
            target_pos = &state.short_pos;
            qty = -amount;
        } else {
            // Close Long (Sell)
            target_pos = &state.long_pos;
            qty = -amount; 
        }
    }

    if (!target_pos) return;

    if (is_opening) {
        // Adding to position (Weighted Average Entry)
        double old_val = std::abs(target_pos->amount) * target_pos->entry_price;
        double new_val = std::abs(qty) * price;
        double new_amt = std::abs(target_pos->amount) + std::abs(qty);
        
        if (new_amt > 0) {
            target_pos->entry_price = (old_val + new_val) / new_amt;
        } else {
            target_pos->entry_price = price;
        }
        target_pos->amount += qty;

    } else {
        // Closing position (Realize PnL)
        double current_amt = target_pos->amount;
        double close_qty = qty;

        // Cap logic
        if (is_buy) { 
             if ((current_amt + close_qty) > 0.0) close_qty = -current_amt; // Cap at 0
        } else {
             if ((current_amt + close_qty) < 0.0) close_qty = -current_amt;
        }

        double realized = 0.0;
        if (target_pos == &state.long_pos) {
            realized = (price - target_pos->entry_price) * std::abs(close_qty);
        } else {
            realized = (target_pos->entry_price - price) * std::abs(close_qty);
        }
        
        state.balance += realized;
        target_pos->amount += close_qty;

        // Track Trade Stats
        if (std::abs(close_qty) > 0.000001) {
             state.total_trades_count++;
             if (realized > 0) {
                 state.winning_trades++;
                 state.gross_profit += realized;
             } else {
                 state.gross_loss += std::abs(realized);
             }
        }
    }
    
    // Cleanup tiny residuals
    if (std::abs(target_pos->amount) < 0.000001) {
        target_pos->amount = 0;
        target_pos->entry_price = 0;
    }

    // Add to History
    double current_time = state.candles.empty() ? 0.0 : state.candles.back().time;
    state.order_history.insert(state.order_history.begin(), {0, is_buy, price, amount, order_type, current_time, reduce_only});
    if (state.order_history.size() > 50) state.order_history.pop_back();
    
    UpdateAccount();
}

void TradingEngine::PlaceOrder(bool is_buy, int order_type, double price, double amount, bool reduce_only) {
    if (order_type == ORDER_MARKET) {
        double fill_p = is_buy ? state.asks[0].price : state.bids[0].price;
        ExecuteFill(is_buy, fill_p, amount, reduce_only, ORDER_MARKET);
    } 
    else if (order_type == ORDER_LIMIT) {
        double current_time = state.candles.empty() ? 0.0 : state.candles.back().time;
        state.open_orders.push_back({state.order_id_counter++, is_buy, price, amount, ORDER_LIMIT, current_time, reduce_only});
    }
    else if (order_type == ORDER_FOK) {
        auto& book = is_buy ? state.asks : state.bids;
        double filled_vol = 0.0;
        double weighted_price_sum = 0.0;
        bool possible = false;
        
        for (const auto& level : book) {
            if (is_buy) {
                if (level.price > price) break; 
            } else {
                if (level.price < price) break; 
            }
            
            double take = std::min(amount - filled_vol, level.volume);
            weighted_price_sum += take * level.price;
            filled_vol += take;
            
            if (filled_vol >= amount - 0.000001) {
                possible = true;
                break;
            }
        }
        
        if (possible) {
            double avg_price = weighted_price_sum / amount;
            ExecuteFill(is_buy, avg_price, amount, reduce_only, ORDER_FOK);
        } else {
            std::cout << "FOK Order Killed" << std::endl;
        }
    }
}

void TradingEngine::CancelOrder(int index) {
    if (index >= 0 && index < (int)state.open_orders.size()) {
        state.open_orders.erase(state.open_orders.begin() + index);
    }
}

void TradingEngine::ClosePosition(bool close_long, bool close_short) {
    if (close_long && state.long_pos.amount > 0.0) {
        PlaceOrder(false, ORDER_MARKET, 0, state.long_pos.amount, true); 
    }
    if (close_short && state.short_pos.amount < 0.0) {
        PlaceOrder(true, ORDER_MARKET, 0, std::abs(state.short_pos.amount), true); 
    }
}

void TradingEngine::CheckLimitOrders() {
    for (auto it = state.open_orders.begin(); it != state.open_orders.end();) {
        bool hit = false;
        if (it->is_buy && state.current_price <= it->price) hit = true;
        else if (!it->is_buy && state.current_price >= it->price) hit = true;
        
        if (hit) {
            ExecuteFill(it->is_buy, it->price, it->amount, it->reduce_only, ORDER_LIMIT);
            it = state.open_orders.erase(it);
        } else {
            ++it;
        }
    }
}

void TradingEngine::GenerateMarketData() {
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

    if (state.candles.size() > 2000) state.candles.erase(state.candles.begin());

    state.bids.clear();
    state.asks.clear();
    
    double p = state.current_price;
    for(int i=0; i<15; ++i) {
        p -= std::uniform_real_distribution<double>(1.0, 5.0)(state.rng);
        state.bids.push_back({p, std::uniform_real_distribution<double>(0.1, 5.0)(state.rng), true});
    }
    p = state.current_price;
    for(int i=0; i<15; ++i) {
        p += std::uniform_real_distribution<double>(1.0, 5.0)(state.rng);
        state.asks.push_back({p, std::uniform_real_distribution<double>(0.1, 5.0)(state.rng), false});
    }

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

    static double accumulator = 0.0;
    accumulator += dt;
    if (accumulator < state.simulation_update_interval_s) return;
    accumulator = 0.0;

    GenerateMarketData();
    CheckLimitOrders();
    UpdateAccount();
}
