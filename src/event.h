#ifndef EVENT_H
#define EVENT_H

#include <cstddef>
#include <cstdint>

/*
 * Binance returns the prices up to 8 decimals, to store it as an int we have to
 * multiply by 10^8 to remove the decimal part.
 */
inline constexpr uint64_t PRICE_SCALE = 100000000;
/*
 * Worst case scenario (minimum size) for a cache line to be.
 */
inline constexpr std::size_t MIN_ACCESS_SIZE = 64;

enum class Side : uint8_t {
    BID = 0,
    ASK,
};

/*
 * Each entry of the bids/asks of the exchange response.
 */
struct event_atom {
    /*
     * Time when WE get the data in ns.
     */
    uint64_t received_time;
    /*
     * E:
     * Time when the exchange sends us the data parsed to ns.
     */
    uint64_t exchange_time;
    /*
     * u:
     * Last updated Id.
     */
    uint64_t update_id;
    /*
     * Book key, the amount of money it is wanted to pay.
     */
    uint64_t price;
    /*
     * How much for a price is wanted.
     * Not a delta, overwrites the previous value. Zero means it has to be removed
     */
    uint64_t quantity;
    /*
     * If it is a BID or ASK.
     */
    Side side;
};

/*
 * Keep event_atom under MIN_ACCESS_SIZE bytes to avoid having it as two reads.
 */
static_assert(sizeof(event_atom) <= MIN_ACCESS_SIZE);

#endif
