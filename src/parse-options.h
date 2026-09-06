#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

enum class Option_type : uint8_t {
    /*
     * Sets the value variable to a boolean value.
     */
    BOOL = 0,
};

struct option {
    /*
     * Abbreviated form for the command example: "-n" for "--dry-run".
     */
    char abbrev;
    /*
     * Full command name.
     */
    std::string_view name;
    /*
     * Type of the command to know what it is expected of it.
     */
    Option_type type;
    /*
     * Pointer to a variable that will contain the value of that given opt after
     * parsing the args.
     */
    void *val;
};

#define OPT_BOOL(abbrev, name, var) { abbrev, name, Option_type::BOOL, var }

void parse_options(int argc, char **argv, std::span<const option> options);

#endif
