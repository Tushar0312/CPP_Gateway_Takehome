#include <iostream>
#include <unordered_map>
#include <RiskManager.hpp>

class OMS{
private:

    RiskManager* m_risk_manager;
    std::unordered_map<std::string, OrderDescriptor*> m_active_orders; //mapping the client_order_id with the active order

public:
    OMS(const std::string& filename);
    Side parse_side(const std::string& side);

    RiskErrorCodes PlaceNewOrder(int64_t ts_us, const std::string& strategy_name, const std::string& symbol, const std::string& client_order_id, Side side, int32_t qty, int32_t price);
    RiskErrorCodes PlaceCancelOrder(int64_t ts_us, const std::string& strategy_name, const std::string& client_order_id);
    void onTradeConfirmation(int64_t ts_us, const std::string& strategy_name, const std::string& client_order_id, int32_t fill_qty);

};