#include <OMS.hpp>

OMS::OMS(const std::string &filename)
{
    m_risk_manager = new RiskManager(filename);
}

RiskErrorCodes OMS::PlaceNewOrder(int64_t ts_us, const std::string &strategy, const std::string &symbol, const std::string &client_order_id, Side side, int32_t qty, int32_t price)
{

    auto itr = m_active_orders.find(client_order_id);

    if (itr != m_active_orders.end())
    {
        // client order id found
        OrderDescriptor *order_desc = itr->second;
        std::cout << "REJECT," << ts_us << "," << strategy << "," << client_order_id << ",DUPLICATE_ORDER_ID" << std::endl;

        return RiskErrorCodes::DUPLICATE_ORDER_ID;
    }

    RiskErrorCodes ret = m_risk_manager->OrderValidation(ts_us, strategy, symbol, client_order_id, price, qty);

    if (ret == RiskErrorCodes::RISK_CHECK_PASS)
    {
        OrderDescriptor *m_order_desc = new OrderDescriptor();

        m_order_desc->m_ts_us = ts_us;
        m_order_desc->m_symbol = symbol;
        m_order_desc->m_client_order_id = client_order_id;
        m_order_desc->m_price = price;
        m_order_desc->m_qty = qty;
        m_order_desc->m_remaining_qty = qty;
        m_order_desc->m_side = side;
        m_order_desc->m_status = Status::LIVE;

        m_active_orders[client_order_id] = m_order_desc;

        m_risk_manager->AddOpenQty(strategy, symbol, qty);

        std::cout << "ACK," << ts_us << "," << strategy << "," << client_order_id << std::endl;
    }

    return ret;
}

RiskErrorCodes OMS::PlaceCancelOrder(int64_t ts_us, const std::string &strategy, const std::string &client_order_id)
{

    auto itr = m_active_orders.find(client_order_id);

    if (itr == m_active_orders.end())
    {
        // client order id not found, so can not cancel the order
        std::cout << "CANCEL_REJECT," << ts_us << "," << strategy << "," << client_order_id << ",UNKNOWN_ORDER" << std::endl;
        return RiskErrorCodes::CLIENT_ORDER_ID_NOT_FOUND;
    }

    OrderDescriptor *order_desc = itr->second;

    // check if the order is not open, then already done
    if (order_desc->m_status != Status::LIVE)
    {
        std::cout << "CANCEL_REJECT," << ts_us << "," << strategy << "," << client_order_id << ",ALREADY_DONE" << std::endl;
        return RiskErrorCodes::ORDER_NOT_OPEN;
    }

    m_risk_manager->SubtractOpenQty(strategy, order_desc->m_symbol, order_desc->m_remaining_qty);
    // Cancel Order (remove open quantity)

    std::cout << "CANCELLED," << ts_us << "," << strategy << "," << client_order_id << std::endl;
    order_desc->m_status = Status::CANCELED;

    return RiskErrorCodes::RISK_CHECK_PASS;
}

void OMS::onTradeConfirmation(int64_t ts_us, const std::string &strategy, const std::string &client_order_id, int32_t fill_qty)
{
    if(fill_qty <= 0){
        std::cout << "FILL_REJECT," << ts_us << "," << strategy << "," << client_order_id << ",INVALID_FILL_QTY" << std::endl;
        return;
    }

    auto itr = m_active_orders.find(client_order_id);
    if (itr == m_active_orders.end())
    {
        // Some error as no active order corresponds to the trade
        std::cout << "FILL_REJECT," << ts_us << "," << strategy << "," << client_order_id << ",UNKNOWN_ORDER" << std::endl;
        return;
    }

    OrderDescriptor *order_desc = itr->second;

    // check the status of the order
    if (order_desc->m_status != Status::LIVE)
    {
        // order has been filled or cancelled
        if (order_desc->m_status == Status::CANCELED)
        {
            std::cout << "FILL_REJECT," << ts_us << "," << strategy << "," << client_order_id << ",ORDER_CANCELLED" << std::endl;
        }

        if (order_desc->m_status == Status::FILLED)
        {
            std::cout << "FILL_REJECT," << ts_us << "," << strategy << "," << client_order_id << ",ALREADY_DONE" << std::endl;
        }
        return;
    }

    // handling the overfill case
    if(order_desc->m_remaining_qty - fill_qty < 0){
        std::cout << "FILL_REJECT," << ts_us << "," << strategy << "," << client_order_id << ",OVERFILL" << std::endl;
        return;
    }

    // make the order status as complete and remove the open qty of that order
    order_desc->m_remaining_qty -= fill_qty;
    m_risk_manager->SubtractOpenQty(strategy, client_order_id, fill_qty);

    if (order_desc->m_remaining_qty == 0)
    {
        order_desc->m_status = Status::FILLED;
    }

    std::cout << "FILL," << ts_us << "," << strategy << "," << client_order_id << "," << fill_qty << "," << order_desc->m_qty - order_desc->m_remaining_qty << "," << order_desc->m_remaining_qty << std::endl;
}

Side OMS::parse_side(const std::string &side)
{
    if (side == "B")
        return Side::BUY;
    if (side == "S")
        return Side::SELL;

    return Side::INVALID;
}