#include "capture.h"
#include "client.h"
#include "parse-options.h"
#include "util.h"
#include <cstdio>

constexpr int ADAMASTOR_VERSION[3] = {0, 1, 0};
constexpr char HELP_MENU[] = " Adamastor HFT usage:\n"
                             " --connect \n"
                             " --gen <in.jsonl> <out.bin>\n"
                             " --read <capture.bin>\n"
                             " --version\n";

struct main_commands {
    bool generate;
    bool read;
    bool version;
    bool connect;
};

#define MAIN_COMMANDS_INIT {0}

int main(int argc, char **argv) {
    struct main_commands flags = MAIN_COMMANDS_INIT;
    int i;

    /* Keep abc order */
    const option options[] = {
        OPT_BOOL('c', "connect", &flags.connect),
        OPT_BOOL('g', "gen", &flags.generate),
        OPT_BOOL('r', "read", &flags.read),
        OPT_BOOL('V', "version", &flags.version),
    };

    i = parse_options(argc, argv, options);
    if (int(flags.generate) + int(flags.read) + int(flags.version) > 1)
        die("--gen, --read and --version are mutually exclusive");

    if (flags.connect) {
        if (argc - i != 0)
            die("usage: %s --connect", argv[0]);

        connect_binance();
        return 0;
    }

    /*
     * TODO: @ingest
     * This should be changed to generate a capture file from real data at the ingest.
     */
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
        printf("Adamastor: %d.%d.%d\n", ADAMASTOR_VERSION[0], ADAMASTOR_VERSION[1],
               ADAMASTOR_VERSION[2]);
        return 0;
    }

    fprintf(stderr, "%s", HELP_MENU);
    return 1;
}
