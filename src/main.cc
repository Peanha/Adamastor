#include "capture.h"
#include "parse-options.h"
#include "util.h"
#include <cstdio>

constexpr int ADAMASTOR_VERSION[3] = { 0, 0, 1 };
constexpr std::string_view HELP_MENU =
    " Adamastor HFT usage:\n"
    " --gen <in.jsonl> <out.bin>\n"
    " --read <capture.bin>\n"
    " --version\n";

int main(int argc, char **argv)
{
    bool gen = false;
    bool read = false;
    bool version = false;
    int i;

    const option options[] = {
        OPT_BOOL('g', "gen",  &gen),
        OPT_BOOL('r', "read", &read),
        OPT_BOOL('V', "version", &version),
    };

    i = parse_options(argc, argv, options);
    if (int(gen) + int(read) + int(version) > 1)
        die("--gen, --read and --version are mutually exclusive");

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

    if (version) {
        printf("Adamastor: %d.%d.%d\n", ADAMASTOR_VERSION[0],
               ADAMASTOR_VERSION[1], ADAMASTOR_VERSION[2]);
        return 0;
    }

    fprintf(stderr, "%s", HELP_MENU.data());
    return 1;
}
