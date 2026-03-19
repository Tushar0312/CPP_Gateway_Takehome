#include <RiskManager.hpp>

RiskManager::RiskManager(const std::string& filename){
    std::ifstream file(filename);  // your CSV file name

    if (!file.is_open()) {
        std::cerr << "Error opening " << filename << std::endl;
        return ;
    }

    std::string line;

    // Skip header
    std::getline(file, line);

    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string value, symbol;

        RiskValues* m_risk_values = new RiskValues();

        // symbol
        getline(ss, value, ',');
        symbol = value;

        // tick_size
        getline(ss, value, ',');
        m_risk_values->m_tick_size = int32_t(std::stod(value) * 100);

        // max_open_qty
        getline(ss, value, ',');
        m_risk_values->m_max_open_qty = std::stoi(value);

        // max_notional_per_order
        getline(ss, value, ',');
        m_risk_values->m_max_notional_per_order = int32_t(std::stod(value) * 100);

        m_rms_limits[symbol] = m_risk_values;
    }

    file.close();
}

RiskErrorCodes RiskManager::OrderValidation(int64_t ts_us, const std::string& strategy, const std::string& symbol, const std::string& client_order_id, int32_t price, int32_t quantity){

    auto itr = m_rms_limits.find(symbol);
    if(itr == m_rms_limits.end()){
        //symbol is not found, so don't fire order
        std::cout << "REJECT," << ts_us << "," << strategy << "," << client_order_id << ",UNKNOWN_SYMBOL"  << std::endl;
        return RiskErrorCodes::SYMBOL_NOT_FOUND;
    }

    RiskValues* risk_values = itr->second;

    if(price <= 0){
        std::cout << "REJECT," << ts_us << "," << strategy << "," << client_order_id << ",INVALID_PRICE"  << std::endl;
        return RiskErrorCodes::PRICE_INTEGRITY_FAIL;
    }

    if(quantity <= 0){
        std::cout << "REJECT," << ts_us << "," << strategy << "," << client_order_id << ",INVALID_QUANTITY"  << std::endl;
        return RiskErrorCodes::QUANTITY_INTEGRITY_FAIL;
    }

    if(price % risk_values->m_tick_size != 0){
        //not a multiple of tick size, don't fire order
        std::cout << "REJECT," << ts_us << "," << strategy << "," << client_order_id << ",INVALID_TICK"  << std::endl;
        return RiskErrorCodes::PRICE_INTEGRITY_FAIL;
    }

    
    if(m_open_qty_map.find(strategy + symbol) == m_open_qty_map.end()){
        m_open_qty_map[strategy + symbol] = 0;
    }

    if((price *1LL* quantity) > risk_values->m_max_notional_per_order){
        //max notional value exceeded, so don't fire order
        std::cout << "REJECT," << ts_us << "," << strategy << "," << client_order_id << ",MAX_NOTIONAL_LIMIT_FAIL"  << std::endl;
        return RiskErrorCodes::MAX_NOTIONAL_LIMIT_FAIL;
    }

    if(m_open_qty_map[strategy + symbol] + quantity > risk_values->m_max_open_qty){
        //max open qty exceeded, so don't fire order
        std::cout << "REJECT," << ts_us << "," << strategy << "," << client_order_id << ",MAX_OPEN_QTY_EXCEEDED"  << std::endl;
        return RiskErrorCodes::MAX_OPEN_QTY_EXCEEDED;
    } 

    return RiskErrorCodes::RISK_CHECK_PASS;
}


void RiskManager::AddOpenQty(const std::string& strategy, const std::string& symbol, int32_t quantity){
    m_open_qty_map[strategy + symbol] += quantity;
}
void RiskManager::SubtractOpenQty(const std::string& strategy, const std::string& symbol, int32_t quantity){
    m_open_qty_map[strategy + symbol] -= quantity;
}