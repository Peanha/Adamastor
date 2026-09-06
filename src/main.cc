#include "capture.h"
#include "parse-options.h"
#include "util.h"

int main(int argc, char **argv)
{
    bool gen = false;
    bool read = false;

    const option options[] = {
        OPT_BOOL('g', "gen",  &gen),
        OPT_BOOL('r', "read", &read),
    };

    int i = parse_options(argc, argv, options);

    if (gen) {
        if (argc - i != 2)
            die("usage: %s --gen <in.jsonl> <out.bin>", argv[0]);

        generate_capture(argv[i], argv[i + 1]);
        return 0;
    }

    if (read) {
        if (argc - i != 1)
            die("usage: %s --read <capture.bin>", argv[0]);

        read_capture(argv[i]);
        return 0;
    }

    die("usage: %s --gen <in.jsonl> <out.bin> | --read <capture.bin>", argv[0]);
    return 1;
}
