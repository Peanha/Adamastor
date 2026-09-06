#include "capture.h"
#include "parse-options.h"

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
        if (argc - i != 2) {
            fprintf(stderr, "usage: %s --gen <in.jsonl> <out.bin>\n", argv[0]);
            return 1;
        }
        generate_capture(argv[i], argv[i + 1]);
        return 0;
    }

    if (read) {
        if (argc - i != 1) {
            fprintf(stderr, "usage: %s --read <capture.bin>\n", argv[0]);
            return 1;
        }
        read_capture(argv[i]);
        return 0;
    }

    fprintf(stderr, "usage: %s --gen <in.jsonl> <out.bin> | --read <capture.bin>\n", argv[0]);
    return 1;
}
