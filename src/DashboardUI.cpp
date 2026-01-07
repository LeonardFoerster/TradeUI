#include "DashboardUI.h"
#include "imgui.h"
#include "implot.h"
#include "imgui_internal.h" 
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>

// Helper Functions
namespace {
    std::string FormatCurrency(double val) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << val;
        return ss.str();
    }

    void DrawCandlesticks(const char* label_id, const double* xs, const double* opens, const double* closes, const double* lows, const double* highs, int count, float width_sec) {
        ImDrawList* draw_list = ImPlot::GetPlotDrawList();
        
        // TradingView Colors
        ImU32 bullCol = IM_COL32(0, 194, 136, 255); // Green
        ImU32 bearCol = IM_COL32(234, 61, 61, 255); // Red
        ImU32 wickCol = IM_COL32(180, 180, 180, 255);

        double half_width = width_sec * 0.5;

        for (int i = 0; i < count; ++i) {
            double x = xs[i];
            double open = opens[i];
            double close = closes[i];
            double low = lows[i];
            double high = highs[i];

            ImVec2 wick_low  = ImPlot::PlotToPixels(x, low);
            ImVec2 wick_high = ImPlot::PlotToPixels(x, high);
            ImVec2 body_min  = ImPlot::PlotToPixels(x - half_width, open);
            ImVec2 body_max  = ImPlot::PlotToPixels(x + half_width, close);

            draw_list->AddLine(wick_low, wick_high, wickCol);
            
            ImU32 color = (open <= close) ? bullCol : bearCol;
            draw_list->AddRectFilled(body_min, body_max, color);
        }
    }
}

void DashboardUI::SetupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Binance/TradingView "Pro" Dark Theme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.13f, 0.14f, 0.17f, 1.00f); 
    colors[ImGuiCol_ChildBg]                = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_Border]                 = ImVec4(0.25f, 0.25f, 0.27f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.25f, 0.26f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.30f, 0.31f, 0.34f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.00f, 0.78f, 0.32f, 1.00f); 
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.25f, 0.26f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.30f, 0.31f, 0.34f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.20f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.25f, 0.26f, 0.29f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.30f, 0.31f, 0.34f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.25f, 0.25f, 0.27f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.20f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);

    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(5, 3);
    style.ItemSpacing = ImVec2(6, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.WindowRounding = 4.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 3.0f;
    style.LogSliderDeadzone = 4.0f;
    style.TabRounding = 4.0f;
}

// Internal Rendering Functions
void RenderChart(TradingEngine& engine) {
    auto& state = engine.state;
    // Top Bar inside Chart
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    if (ImGui::Button(state.symbol)) { /* Symbol Search */ }
    ImGui::SameLine();
    ImGui::TextDisabled("|"); ImGui::SameLine();
    
    const char* tfs[] = { "1m", "5m", "15m", "1h", "4h", "D" };
    for (int i=0; i<6; ++i) {
        if (i > 0) ImGui::SameLine();
        bool selected = (state.timeframe_idx == i);
        if (selected) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        if (ImGui::Button(tfs[i])) state.timeframe_idx = i;
        if (selected) ImGui::PopStyleColor();
    }
    
    ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();
    ImGui::Button("Indicators");
    
    ImGui::SameLine();
    static bool auto_scroll = true;
    ImGui::Checkbox("Auto-Scroll", &auto_scroll);

    ImGui::SameLine();
    static bool auto_fit_y = true;
    ImGui::Checkbox("Auto-Fit Y", &auto_fit_y);

    ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();

    // Simulation Controls in Toolbar
    static const double intervals[] = {0.1, 0.5, 1.0, 3.0, 5.0, 10.0, 30.0, 60.0};
    static const char* interval_names[] = {"0.1s", "0.5s", "1s", "3s", "5s", "10s", "30s", "1m"};
    ImGui::SetNextItemWidth(80);
    if (ImGui::SliderInt("##Speed", &state.simulation_interval_idx, 0, IM_ARRAYSIZE(intervals) - 1, interval_names[state.simulation_interval_idx])) {
        state.simulation_update_interval_s = intervals[state.simulation_interval_idx];
    }
    
    ImGui::SameLine();
    if (ImGui::Button(state.is_paused ? " > " : " || ")) {
        state.is_paused = !state.is_paused;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(state.is_paused ? "Resume Simulation" : "Pause Simulation");
    
    ImGui::PopStyleVar();

    // Smart Auto-Scroll with Zoom Logic (Pre-Calculation)
    static double view_width = 3600.0; // Initial view: 1 hour
    
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            double zoom_factor = 0.1;
            view_width -= view_width * wheel * zoom_factor;
            if (view_width < 60.0) view_width = 60.0;
            if (view_width > 86400.0 * 7) view_width = 86400.0 * 7;
        }
    }

    // The Chart
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(10, 10));
    if (ImPlot::BeginPlot("##MainChart", ImVec2(-1, -1), ImPlotFlags_NoTitle)) {
        int count = (int)state.candles.size();
        std::vector<double> times(count), opens(count), highs(count), lows(count), closes(count);
        for(int i=0; i<count; ++i) {
            times[i] = state.candles[i].time;
            opens[i] = state.candles[i].open;
            highs[i] = state.candles[i].high;
            lows[i] = state.candles[i].low;
            closes[i] = state.candles[i].close;
        }

        ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoLabel);
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_Opposite);

        if (count > 0) {
            if (auto_scroll) {
                double t_max = times.back();
                double t_min = t_max - view_width;
                ImPlot::SetupAxisLimits(ImAxis_X1, t_min, t_max + (view_width * 0.05), ImGuiCond_Always);
            } else {
                ImPlot::SetupAxisLimits(ImAxis_X1, times.front(), times.back() + 300.0, ImGuiCond_FirstUseEver);
            }
            
            double min_y = *std::min_element(lows.begin(), lows.end());
            double max_y = *std::max_element(highs.begin(), highs.end());
            
            if (auto_fit_y) {
                 ImPlot::SetupAxisLimits(ImAxis_Y1, min_y * 0.999, max_y * 1.001, ImGuiCond_Always);
            } else {
                 ImPlot::SetupAxisLimits(ImAxis_Y1, min_y * 0.999, max_y * 1.001, ImGuiCond_FirstUseEver);
            }
        }

        DrawCandlesticks("BTC/USD", times.data(), opens.data(), closes.data(), lows.data(), highs.data(), count, 40.0f);
        
        if (count > 0) {
            double cur_p = closes.back();
            ImPlot::TagY(cur_p, ImVec4(1, 0, 0, 1), "%.2f", cur_p);
        }

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleVar();
}

void RenderOrderBook(TradingEngine& engine) {
    auto& state = engine.state;
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2, 1));
    if (ImGui::BeginTable("OrderBookTable", 3, ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Total", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (int i = (int)state.asks.size() - 1; i >= 0; --i) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); 
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%.2f", state.asks[i].price);
            ImGui::TableNextColumn(); ImGui::Text("%.4f", state.asks[i].volume);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", state.asks[i].price * state.asks[i].volume);
        }

        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(30, 30, 30, 255));
        ImGui::TableNextColumn(); 
        if (!state.asks.empty() && !state.bids.empty()) {
            double spread = state.asks[0].price - state.bids[0].price;
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%.2f", state.current_price);
            ImGui::SameLine();
            ImGui::TextDisabled("(Spread: %.2f)", spread);
        }

        for (const auto& bid : state.bids) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); 
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), "%.2f", bid.price);
            ImGui::TableNextColumn(); ImGui::Text("%.4f", bid.volume);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", bid.price * bid.volume);
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}

void RenderRecentTrades(TradingEngine& engine) {
    auto& state = engine.state;
    if (ImGui::BeginTable("TradesTable", 3, ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Price");
        ImGui::TableSetupColumn("Amount");
        ImGui::TableSetupColumn("Time");
        ImGui::TableHeadersRow();

        for (const auto& t : state.trade_history) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextColored(t.is_buy ? ImVec4(0, 0.8f, 0.4f, 1) : ImVec4(1, 0.3f, 0.3f, 1), "%.2f", t.price);
            ImGui::TableNextColumn();
            ImGui::Text("%.4f", t.amount);
            ImGui::TableNextColumn();
            time_t rawtime = (time_t)t.time;
            struct tm * timeinfo = localtime(&rawtime);
            char buffer[80];
            strftime(buffer,80,"%H:%M:%S",timeinfo);
            ImGui::Text("%s", buffer);
        }
        ImGui::EndTable();
    }
}

void RenderOrderEntry(TradingEngine& engine) {
    auto& state = engine.state;
    ImGui::BeginTabBar("OrderType");
    if (ImGui::BeginTabItem("Limit")) { state.order_type = 0; ImGui::EndTabItem(); }
    if (ImGui::BeginTabItem("Market")) { state.order_type = 1; ImGui::EndTabItem(); }
    ImGui::EndTabBar();

    ImGui::Text("Equity: %.2f USD", state.equity);
    ImGui::Text("Avail:  %.2f USD", state.balance);
    ImGui::Separator();

    if (state.order_type == 0) { // Limit
        ImGui::InputFloat("Price", &state.order_price, 10.0f, 100.0f, "%.2f");
    } else {
        ImGui::TextDisabled("Price: Market (%.2f)", state.current_price);
    }
    
    ImGui::InputFloat("Amount", &state.order_amount, 0.01f, 0.1f, "%.4f");

    float estimated_price = (state.order_type == 0) ? state.order_price : state.current_price;
    float total = estimated_price * state.order_amount;
    
    ImGui::TextDisabled("Total: %.2f USD", total);
    ImGui::Separator();
    
    float w = ImGui::GetContentRegionAvail().x;
    // Green Buy Button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.4f, 1.0f));
    if (ImGui::Button("Buy / Long", ImVec2(w * 0.5f - 4, 40))) { 
        engine.PlaceOrder(true, state.order_type == 1, state.order_price, state.order_amount); 
    }
    ImGui::PopStyleColor(2);

    ImGui::SameLine();
    
    // Red Sell Button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    if (ImGui::Button("Sell / Short", ImVec2(w * 0.5f - 4, 40))) { 
        engine.PlaceOrder(false, state.order_type == 1, state.order_price, state.order_amount);
    }
    ImGui::PopStyleColor(2);
}

void RenderTerminal(TradingEngine& engine) {
    auto& state = engine.state;
    if (ImGui::BeginTabBar("TerminalTabs")) {
        // Open Orders
        char buf[32];
        sprintf(buf, "Open Orders (%zu)", state.open_orders.size());
        if (ImGui::BeginTabItem(buf)) {
            if (ImGui::BeginTable("OrdersTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                ImGui::TableSetupColumn("ID");
                ImGui::TableSetupColumn("Side");
                ImGui::TableSetupColumn("Price");
                ImGui::TableSetupColumn("Amount");
                ImGui::TableSetupColumn("Action");
                ImGui::TableHeadersRow();
                
                int to_delete = -1;
                for (int i=0; i<(int)state.open_orders.size(); ++i) {
                    const auto& o = state.open_orders[i];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("%d", o.id);
                    ImGui::TableNextColumn(); ImGui::TextColored(o.is_buy ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), o.is_buy ? "Buy" : "Sell");
                    ImGui::TableNextColumn(); ImGui::Text("%.2f", o.price);
                    ImGui::TableNextColumn(); ImGui::Text("%.4f", o.amount);
                    ImGui::TableNextColumn(); 
                    ImGui::PushID(i);
                    if (ImGui::Button("Cancel")) to_delete = i;
                    ImGui::PopID();
                }
                
                if (to_delete != -1) engine.CancelOrder(to_delete);
                
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
        
        // Positions
        if (ImGui::BeginTabItem("Positions")) {
             if (state.position.amount == 0.0) {
                 ImGui::TextDisabled("No active positions");
             } else {
                 if (ImGui::BeginTable("PosTable", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                    ImGui::TableSetupColumn("Size");
                    ImGui::TableSetupColumn("Entry Price");
                    ImGui::TableSetupColumn("Mark Price");
                    ImGui::TableSetupColumn("PnL (Unrealized)");
                    ImGui::TableSetupColumn("RoE %");
                    ImGui::TableSetupColumn("Action");
                    ImGui::TableHeadersRow();
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); 
                    ImGui::TextColored(state.position.amount > 0 ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), "%.4f", state.position.amount);
                    ImGui::TableNextColumn(); ImGui::Text("%.2f", state.position.entry_price);
                    ImGui::TableNextColumn(); ImGui::Text("%.2f", state.current_price);
                    
                    ImGui::TableNextColumn(); 
                    ImGui::TextColored(state.position.unrealized_pnl >= 0 ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), "%.2f USD", state.position.unrealized_pnl);
                    
                    ImGui::TableNextColumn();
                    double roe = 0.0;
                    if (state.position.entry_price > 0) 
                        roe = (state.position.unrealized_pnl / (std::abs(state.position.amount) * state.position.entry_price)) * 100.0;
                    ImGui::TextColored(roe >= 0 ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), "%.2f%%", roe);

                    ImGui::TableNextColumn();
                    if (ImGui::Button("Close")) {
                        engine.ClosePosition();
                    }
                    
                    ImGui::EndTable();
                 }
             }
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Trade History")) {
            if (ImGui::BeginTable("HistTable", 4, ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Side");
                ImGui::TableSetupColumn("Price");
                ImGui::TableSetupColumn("Amount");
                ImGui::TableSetupColumn("Time"); 
                ImGui::TableHeadersRow();
                for (const auto& o : state.order_history) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::TextColored(o.is_buy ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), o.is_buy ? "Buy" : "Sell");
                    ImGui::TableNextColumn(); ImGui::Text("%.2f", o.price);
                    ImGui::TableNextColumn(); ImGui::Text("%.4f", o.amount);
                    ImGui::TableNextColumn(); ImGui::Text("Just now"); 
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void DashboardUI::Render(TradingEngine& engine) {
    static bool first_frame = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; 

    // 1. DockSpace Setup
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpaceHost", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    // 2. Default Layout Construction
    if (first_frame) {
        first_frame = false;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodePos(dockspace_id, viewport->WorkPos);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_right, dock_bottom, dock_center; 

        ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, &dock_right, &dock_center);
        ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Down, 0.25f, &dock_bottom, &dock_center);
        
        ImGuiID dock_right_top, dock_right_mid, dock_right_bot;
        ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.66f, &dock_right_mid, &dock_right_top); 
        ImGui::DockBuilderSplitNode(dock_right_mid, ImGuiDir_Down, 0.5f, &dock_right_bot, &dock_right_mid);

        ImGui::DockBuilderDockWindow("Chart", dock_center);
        ImGui::DockBuilderDockWindow("Order Book", dock_right_top);
        ImGui::DockBuilderDockWindow("Recent Trades", dock_right_mid);
        ImGui::DockBuilderDockWindow("Order Entry", dock_right_bot);
        ImGui::DockBuilderDockWindow("Terminal", dock_bottom);

        ImGui::DockBuilderFinish(dockspace_id);
    }
    ImGui::End();

    // 3. Render Windows

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8,0));
    ImGui::Begin("Chart", nullptr, ImGuiWindowFlags_NoScrollbar);
    RenderChart(engine);
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Begin("Order Book");
    RenderOrderBook(engine);
    ImGui::End();

    ImGui::Begin("Recent Trades");
    RenderRecentTrades(engine);
    ImGui::End();

    ImGui::Begin("Order Entry");
    RenderOrderEntry(engine);
    ImGui::End();

    ImGui::Begin("Terminal");
    RenderTerminal(engine);
    ImGui::End();
}
