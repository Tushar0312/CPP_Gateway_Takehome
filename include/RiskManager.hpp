#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include <Types.hpp>

class RiskManager{
    public:
    std::unordered_map<std::string, RiskValues*> m_rms_limits; //mapping the symbol to RmsLimits 
    std::unordered_map<std::string, int32_t> m_open_qty_map; //mapping strategy + order key to open qty

    RiskManager(const std::string& filename);

    RiskErrorCodes OrderValidation(int64_t ts_us, const std::string& strategy, const std::string& symbol, const std::string& client_order_id, int32_t price, int32_t quantity);

    void AddOpenQty(const std::string& strategy, const std::string& client_order_id, int32_t quantity);
    void SubtractOpenQty(const std::string& strategy, const std::string& client_order_id, int32_t quantity);
};