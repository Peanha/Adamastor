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

struct main_commands {
    bool generate;
    bool read;
    bool version;
};

#define MAIN_COMMANDS_INIT { 0 }

int main(int argc, char **argv)
{
    struct main_commands flags = MAIN_COMMANDS_INIT;
    int i;

    const option options[] = {
        OPT_BOOL('g', "gen",  &flags.generate),
        OPT_BOOL('r', "read", &flags.read),
        OPT_BOOL('V', "version", &flags.version),
    };

    i = parse_options(argc, argv, options);
    if (int(flags.generate) + int(flags.read) + int(flags.version) > 1)
        die("--gen, --read and --version are mutually exclusive");

    if (flags.generate) {
        if (argc - i != 2)
            die("usage: %s --gen <in.jsonl> <out.bin>", argv[0]);

        generate_capture(argv[i], argv[i + 1]);
        return 0;
    }

    if (flags.read) {
        if (argc - i != 1)
            die("usage: %s --read <capture.bin>", argv[0]);

        read_capture(argv[i]);
        return 0;
    }

    if (flags.version) {
        printf("Adamastor: %d.%d.%d\n", ADAMASTOR_VERSION[0],
               ADAMASTOR_VERSION[1], ADAMASTOR_VERSION[2]);
        return 0;
    }

    fprintf(stderr, "%s", HELP_MENU.data());
    return 1;
}
