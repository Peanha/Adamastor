#include "parse-options.h"

int main(int argc, char **argv)
{
    bool gen = false;

    const option options[] = {
        OPT_BOOL('g', "gen", &gen),
    };

    parse_options(argc, argv, options);

    printf("test = %d\n", gen);
    return 0;
}
