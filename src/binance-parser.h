#ifndef BINANCE_PARSER_H
#define BINANCE_PARSER_H

#include "event.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

/*
 * Possible errors returned while parsing a Binance depth update message.
 */
enum class ParseError : uint8_t {
     /*
     * The message was parsed successfully.
     */
    NONE = 0,
    /*
     * The message does not have the expected Binance depthUpdate format.
     */
    INVALID_MESSAGE,
    /*
     * The output buffer does not have enough space for all parsed events.
     */
    OUTPUT_FULL,
    /*
     * A numeric value is too large to fit in its uint64_t representation.
     */
    NUMBER_OVERFLOW,
};

/*
 * Result returned after parsing a Binance depth update message.
 */
struct ParseResult {
     /*
     * Number of event_atom entries written into the output buffer.
     */
    std::size_t count;
    /*
     * Parsing status.
     */
    ParseError error;
};

/*
 * Parses a Binance depthUpdate message into event_atom entries.
 *
 * The payload is read directly without copying it and the parsed events are
 * written into the memory provided by output.
 *
 * No dynamic memory allocation is performed by the parser.
 *
 * received_time is the local timestamp when the raw exchange message was
 * received.
 *
 * Returns the number of generated events and the parsing status.
 */
ParseResult parse_binance_depth(std::string_view payload,
                                uint64_t received_time,
                                std::span<event_atom> output);

#endif