# Order Gateway Simulation (C++)

## Overview

This project implements a simplified internal order gateway in C++.
It simulates how a trading system processes order requests, applies validation, tracks state, and emits events.

The system processes:

-> New Orders
-> Cancel Requests
-> Fill Notifications

It validates orders using:

-> Field-level checks
-> Exchange rules (tick size)
-> Risk checks (max notional, max open quantity)

---

## Build Instructions

```bash
g++ .\demo.cpp .\src\OMS.cpp .\src\RiskManager.cpp -o demo -I .\include\
```

---

## Run Instructions

```bash
./demo symbols.csv events.txt
```

---

## Design Decisions

### 1. Price Modification

Price is taken in paisa to avoid the floating point errors or precision errors.

### 2. Order Identification

Orders are uniquely identified by:

```
client_order_id
```

This avoids collisions across strategies.

---

### 3. Order Lifecycle

```
LIVE → FILLED
LIVE → CANCELED
```

-> Once an order is FILLED or CANCELED, it is terminal.
-> Terminal orders do not contribute to open quantity.

---

### 4. Open Quantity Tracking

I maintain:

```
open_qty[strategy+symbol]
```

This stores the _sum of remaining quantity_ of all live orders.

Benefits:

-> O(1) risk validation
-> No need to iterate over all orders

---

### 5. Duplicate Order IDs

Not Allowed: Even if order is

- Cancelled
- Full fill

---

### 6. Parsing Strategy

-> Input is split using `stringstream`
-> Invalid formats → `PARSE_REJECT`

---

## Edge Cases Handled

-> Cancel before new → CANCEL_REJECT
-> Fill before new → FILL_REJECT
-> Duplicate ID (live) → REJECT
-> Partial fill then cancel → handled correctly
-> Overfill → FILL_REJECT
-> Fill after cancel → FILL_REJECT
-> Unknown symbol → REJECT
-> Invalid tick → REJECT
-> Malformed input → PARSE_REJECT

---

## External Libraries

None used. Only standard C++17 library.

---

## What I Would Improve in Production

-> I would add the order modification part in production. I cannot always do the cancel and fire the new order. This will loose my time priority in the order book. And this would also chock my message line as instead of sending 1 order, I am sending 2 orders.

-> I would also add the order acknowledgement from the exchange. Wheather the order is accepted or rejected by the exchange.

---

## Bugs I Guarded Against

-> Floating-point precision errors in tick validation, so replaced the floating-point price to integer
-> Overfill scenarios
-> Invalid side values
-> Incorrect open quantity tracking

---
