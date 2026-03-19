#include <iostream>

enum class RiskErrorCodes : int16_t {
    SYMBOL_NOT_FOUND = -1,
    CLIENT_ORDER_ID_NOT_FOUND = -2,
    PRICE_INTEGRITY_FAIL = -3,
    QUANTITY_INTEGRITY_FAIL = -4,
    MAX_NOTIONAL_LIMIT_FAIL = -5,
    MAX_OPEN_QTY_EXCEEDED = -6,
    ORDER_NOT_OPEN = -7,
    DUPLICATE_ORDER_ID = -8,
    RISK_CHECK_PASS = 1
};

enum class Side:int8_t { 
    BUY = 1, 
    SELL = 2,
    INVALID = -1
};


enum class Status:int16_t { 
    LIVE = 1, 
    CANCELED = 2, 
    FILLED = 3 
};

struct OrderDescriptor{
    int64_t m_ts_us;
    int64_t m_exchange_order_sequence;
    std::string m_symbol;
    std::string m_client_order_id;
    int32_t m_price;
    int32_t m_qty;
    int32_t m_remaining_qty;
    int32_t m_modified_price;
    int32_t m_modified_qty;
    Side m_side;

    Status m_status; // This will tell the status of the order
};

struct RiskValues{
    int32_t m_tick_size;
    int32_t m_max_open_qty;
    int64_t m_max_notional_per_order;
    int32_t m_used_open_qty;

    RiskValues(){
        m_tick_size = 0;
        m_max_open_qty = 0;
        m_max_notional_per_order = 0;
        m_used_open_qty = 0;
    }
};

