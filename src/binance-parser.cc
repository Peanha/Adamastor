#include "binance-parser.h"

#include <limits>

namespace {

constexpr uint64_t DECIMAL_SCALE = 100000000;
constexpr uint64_t MILLIS_TO_NANOS = 1000000;

void skip_whitespace(std::string_view input, std::size_t &position) {
    while (position < input.size()) {
        const char c = input[position];

        if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
            return;

        ++position;
    }
}

bool consume(std::string_view input, std::size_t &position, char expected) {
    skip_whitespace(input, position);

    if (position >= input.size() || input[position] != expected)
        return false;

    ++position;
    return true;
}

bool find_key(std::string_view input,
              std::string_view key,
              std::size_t &value_position) {
    std::size_t search_position = 0;

    while (search_position < input.size()) {
        const std::size_t quote = input.find('"', search_position);

        if (quote == std::string_view::npos)
            return false;

        const std::size_t key_start = quote + 1;

        if (key_start + key.size() < input.size() &&
            input.compare(key_start, key.size(), key) == 0 &&
            input[key_start + key.size()] == '"') {

            std::size_t position = key_start + key.size() + 1;

            skip_whitespace(input, position);

            if (position < input.size() && input[position] == ':') {
                value_position = position + 1;

                skip_whitespace(input, value_position);

                return true;
            }
        }

        search_position = quote + 1;
    }

    return false;
}

ParseError parse_uint64(std::string_view input,
                        std::size_t &position,
                        uint64_t &value) {
    skip_whitespace(input, position);

    if (position >= input.size() ||
        input[position] < '0' ||
        input[position] > '9') {
        return ParseError::INVALID_MESSAGE;
    }

    uint64_t result = 0;

    while (position < input.size() &&
           input[position] >= '0' &&
           input[position] <= '9') {

        const uint64_t digit =
            static_cast<uint64_t>(input[position] - '0');

        if (result >
            (std::numeric_limits<uint64_t>::max() - digit) / 10) {
            return ParseError::NUMBER_OVERFLOW;
        }

        result = result * 10 + digit;

        ++position;
    }

    value = result;

    return ParseError::NONE;
}

ParseError parse_scaled_decimal(std::string_view input,
                                std::size_t &position,
                                uint64_t &value) {
    if (!consume(input, position, '"'))
        return ParseError::INVALID_MESSAGE;

    if (position >= input.size() ||
        input[position] < '0' ||
        input[position] > '9') {
        return ParseError::INVALID_MESSAGE;
    }

    uint64_t whole = 0;

    while (position < input.size() &&
           input[position] >= '0' &&
           input[position] <= '9') {

        const uint64_t digit =
            static_cast<uint64_t>(input[position] - '0');

        if (whole >
            (std::numeric_limits<uint64_t>::max() - digit) / 10) {
            return ParseError::NUMBER_OVERFLOW;
        }

        whole = whole * 10 + digit;

        ++position;
    }

    uint64_t fraction = 0;
    unsigned int fraction_digits = 0;

    if (position < input.size() && input[position] == '.') {
        ++position;

        while (position < input.size() &&
               input[position] >= '0' &&
               input[position] <= '9') {

            if (fraction_digits >= 8)
                return ParseError::INVALID_MESSAGE;

            fraction =
                fraction * 10 +
                static_cast<uint64_t>(input[position] - '0');

            ++fraction_digits;
            ++position;
        }
    }

    if (position >= input.size() || input[position] != '"')
        return ParseError::INVALID_MESSAGE;

    ++position;

    while (fraction_digits < 8) {
        fraction *= 10;
        ++fraction_digits;
    }

    if (whole >
        (std::numeric_limits<uint64_t>::max() - fraction) /
            DECIMAL_SCALE) {
        return ParseError::NUMBER_OVERFLOW;
    }

    value = whole * DECIMAL_SCALE + fraction;

    return ParseError::NONE;
}

ParseError parse_event_type(std::string_view input) {
    std::size_t position = 0;

    if (!find_key(input, "e", position))
        return ParseError::INVALID_MESSAGE;

    if (!consume(input, position, '"'))
        return ParseError::INVALID_MESSAGE;

    constexpr std::string_view EXPECTED = "depthUpdate";

    if (position + EXPECTED.size() > input.size())
        return ParseError::INVALID_MESSAGE;

    if (input.compare(position, EXPECTED.size(), EXPECTED) != 0)
        return ParseError::INVALID_MESSAGE;

    position += EXPECTED.size();

    if (position >= input.size() || input[position] != '"')
        return ParseError::INVALID_MESSAGE;

    return ParseError::NONE;
}

ParseError parse_side(std::string_view input,
                      std::size_t &position,
                      uint64_t received_time,
                      uint64_t exchange_time,
                      uint64_t update_id,
                      Side side,
                      std::span<event_atom> output,
                      std::size_t &count) {
    if (!consume(input, position, '['))
        return ParseError::INVALID_MESSAGE;

    skip_whitespace(input, position);

    /*
     * Empty Binance side:
     *
     * "b":[]
     *
     * or
     *
     * "a":[]
     */
    if (position < input.size() && input[position] == ']') {
        ++position;

        return ParseError::NONE;
    }

    while (position < input.size()) {

        /*
         * We have no more preallocated event_atom slots.
         */
        if (count >= output.size())
            return ParseError::OUTPUT_FULL;

        /*
         * Each book update looks like:
         *
         * ["60000.00000000","1.50000000"]
         */
        if (!consume(input, position, '['))
            return ParseError::INVALID_MESSAGE;

        uint64_t price = 0;
        uint64_t quantity = 0;

        ParseError error =
            parse_scaled_decimal(input, position, price);

        if (error != ParseError::NONE)
            return error;

        if (!consume(input, position, ','))
            return ParseError::INVALID_MESSAGE;

        error =
            parse_scaled_decimal(input, position, quantity);

        if (error != ParseError::NONE)
            return error;

        if (!consume(input, position, ']'))
            return ParseError::INVALID_MESSAGE;

        /*
         * Write directly into memory supplied by the caller.
         *
         * No vector.
         * No push_back.
         * No malloc.
         */
        output[count] = event_atom{
            .received_time = received_time,
            .exchange_time = exchange_time,
            .update_id = update_id,
            .price = price,
            .quantity = quantity,
            .side = side,
        };

        ++count;

        skip_whitespace(input, position);

        if (position >= input.size())
            return ParseError::INVALID_MESSAGE;

        /*
         * End of bids/asks array.
         */
        if (input[position] == ']') {
            ++position;

            return ParseError::NONE;
        }

        /*
         * Otherwise another price level must follow.
         */
        if (input[position] != ',')
            return ParseError::INVALID_MESSAGE;

        ++position;
    }

    return ParseError::INVALID_MESSAGE;
}

} // namespace

ParseResult parse_binance_depth(std::string_view payload,
                                uint64_t received_time,
                                std::span<event_atom> output) {
    ParseResult result{
        .count = 0,
        .error = ParseError::NONE,
    };

    /*
     * Check that Binance actually sent a depthUpdate message.
     */
    result.error = parse_event_type(payload);

    if (result.error != ParseError::NONE)
        return result;

    /*
     * Read Binance exchange timestamp:
     *
     * "E":1672515782136
     */
    std::size_t position = 0;

    uint64_t exchange_time_ms = 0;

    if (!find_key(payload, "E", position)) {
        result.error = ParseError::INVALID_MESSAGE;

        return result;
    }

    result.error =
        parse_uint64(payload, position, exchange_time_ms);

    if (result.error != ParseError::NONE)
        return result;

    /*
     * event.h stores exchange_time in nanoseconds.
     * Binance E is milliseconds.
     */
    if (exchange_time_ms >
        std::numeric_limits<uint64_t>::max() /
            MILLIS_TO_NANOS) {

        result.error = ParseError::NUMBER_OVERFLOW;

        return result;
    }

    const uint64_t exchange_time =
        exchange_time_ms * MILLIS_TO_NANOS;

    /*
     * Read Binance final update ID:
     *
     * "u":160
     */
    uint64_t update_id = 0;

    if (!find_key(payload, "u", position)) {
        result.error = ParseError::INVALID_MESSAGE;

        return result;
    }

    result.error =
        parse_uint64(payload, position, update_id);

    if (result.error != ParseError::NONE)
        return result;

    /*
     * Parse bids:
     *
     * "b":[
     *     ["60000.00000000","1.50000000"],
     *     ...
     * ]
     */
    if (!find_key(payload, "b", position)) {
        result.error = ParseError::INVALID_MESSAGE;

        return result;
    }

    result.error =
        parse_side(payload,
                   position,
                   received_time,
                   exchange_time,
                   update_id,
                   Side::BID,
                   output,
                   result.count);

    if (result.error != ParseError::NONE)
        return result;

    /*
     * Parse asks:
     *
     * "a":[
     *     ["60010.00000000","0.80000000"],
     *     ...
     * ]
     */
    if (!find_key(payload, "a", position)) {
        result.error = ParseError::INVALID_MESSAGE;

        return result;
    }

    result.error =
        parse_side(payload,
                   position,
                   received_time,
                   exchange_time,
                   update_id,
                   Side::ASK,
                   output,
                   result.count);

    return result;
}