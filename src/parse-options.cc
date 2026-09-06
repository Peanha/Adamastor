#include "util.h"
#include "parse-options.h"

#include <cstdlib>
#include <span>
#include <algorithm>

int parse_options(int argc, char **argv, std::span<const option> options)
{
    int i;

    for (i = 1; i < argc; i++) {
        auto it = options.end();

        if (*argv[i] != '-' || argv[i][1] == '\0' || (argv[i][1] == '-' && argv[i][2] == '\0'))
            break;

        if (argv[i][1] == '-') {
            std::string_view name = argv[i] + 2;

            it = std::ranges::find_if(options, [&name](const option &o) {
                return !o.name.empty() && name == o.name;
            });
        } else if (argv[i][2] == '\0') {
            char abbrev = argv[i][1];

            it = std::ranges::find_if(options, [abbrev](const option& o) {
                return o.abbrev && o.abbrev == abbrev;
            });
        }

        if (it == options.end())
            die("unknown option '%s'\n", argv[i]);

        switch (it->type) {
        case Option_type::BOOL:
            *static_cast<bool*>(it->val) = true;
            break;
        }
    }

    return i;
}
