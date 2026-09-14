#include "order_book.h"

// Loads the initial state of the order book.
void order_book::load_snapshot(uint64_t snapshot_id,
                               const price_map &snapshot_bids,
                               const price_map &snapshot_asks) {

    // Store the snapshot update ID.
    last_update_id = snapshot_id;

    //initial BID and ASK orders.
    bids = snapshot_bids;
    asks = snapshot_asks;

    // The order book is ready for updates.
    initialized = true;
}

// Applies a new update to the order book.
bool order_book::apply(const event_atom &e,
                       uint64_t first_update_id,
                       uint64_t final_update_id) {

    // Ignore updates before the snapshot is loaded.
    if (!initialized)
        return false;

    // Ignore old or already processed updates.
    if (final_update_id <= last_update_id)
        return false;

    // Detect missing updates.
    if (first_update_id > last_update_id + 1)
        return false;

    // Store the latest processed update ID.
    last_update_id = final_update_id;

    // Apply the price and quantity change.
    apply_atom(e);

    return true;
}

// Adds, updates or removes a price level.
void order_book::apply_atom(const event_atom &e) {

    // Select the BID or ASK map.
    auto &side = (e.side == Side::BID) ? bids : asks;

    // Quantity 0 removes the price level.
    if (e.quantity == 0)
        side.erase(e.price);
    else
        side[e.price] = e.quantity;
}

// Returns the highest BID price.
uint64_t order_book::best_bid() const {

    if (bids.empty())
        return 0;

    // The highest price is the last element.
    return bids.rbegin()->first;
}

// Returns the lowest ASK price.
uint64_t order_book::best_ask() const {

    if (asks.empty())
        return 0;

    // The lowest price is the first element.
    return asks.begin()->first;
}