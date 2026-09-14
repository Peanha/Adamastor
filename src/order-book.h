#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "event.h"
#include <cstdint>
#include <map>

/*
 * Order book for one instrument.
 *
 * Stores BID and ASK price levels.
 * quantity == 0 -> remove level
 * quantity > 0  -> add or update level
 */
class order_book {
public:

    // Maps price to quantity.
    using price_map = std::map<uint64_t, uint64_t>;

    // Loads the initial order book.
    void load_snapshot(uint64_t last_update_id,
                       const price_map &bids,
                       const price_map &asks);

    // Applies a new update.
    bool apply(const event_atom &e,
               uint64_t first_update_id,
               uint64_t final_update_id);

    // Returns the highest BID price.
    uint64_t best_bid() const;

    // Returns the lowest ASK price.
    uint64_t best_ask() const;

    // Gives read-only access to the order book.
    const price_map &get_bids() const { return bids; }
    const price_map &get_asks() const { return asks; }

private:

    /* Buy orders. */
    price_map bids;

    // Sell orders.
    price_map asks;

    // ID of the last processed update.
    uint64_t last_update_id = 0;

    // True after loading the snapshot.
    bool initialized = false;

    // Applies one update to the correct side.
    void apply_atom(const event_atom &e);
};

#endif