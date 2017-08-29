#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

static void usage(FILE *fp, int argc, char **argv)
{
    fprintf(fp,
            "Usage: %s [options]\n"
            "\n"
            "Print out a classic greeting\n"
            "\n"
            "Options:\n"
            "  -n, --name NAME     Name to use for the greeting\n"
            "  -c, --casual        Use casual greeting\n"
            "  -h, --help          Show this help message\n"
            "      --verbose       Print a verbose greeting\n"
            "      --terse         Print a terse greeting (default)\n"
            "      --foo           Option without argument\n"
            "      --bar BAR       Option with argument\n"
            "",
            argv[0]);
}

static const char short_options[] = "n:ch";

static int verbose_flag = 0;

static const struct option long_options[] = {
    { "name",       required_argument, NULL, 'n' },
    { "casual",     no_argument,       NULL, 'c' },
    { "help",       no_argument,       NULL, 'h' },
    { "verbose",    no_argument,       &verbose_flag, 1 },
    { "terse",      no_argument,       &verbose_flag, 0 },
    { "foo",        no_argument,       NULL, 0 },
    { "bar",        required_argument, NULL, 0 },
    { 0, 0, 0, 0 }
};

int main(int argc, char **argv)
{
    char *name = NULL;
    int casual = 0;

    /* Parse the command line arguments */
    for (;;) {
        int option_index;
        int c;

        c = getopt_long(argc, argv,
                        short_options, long_options, &option_index);

        if (-1 == c)
            break;

        switch (c) {
        case 0:
            /* Stop here if this option is setting a flag */
            if (long_options[option_index].flag != 0)
                break;
            /* For options that are not flags display the option
             * and if its argument (when applicable).
             */
            printf ("option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;

        case 'n':
            name = optarg;
            break;

        case 'c':
            casual = 1;
            break;

        case 'h':
            usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);

        default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    if (casual) {
        printf("Hey there ");
    } else {
        printf("Hello ");
    }
    if (name) {
        printf("%s", name);
    } else {
        printf("world");
    }
    if (verbose_flag) {
        printf(", how are you?\n");
    } else {
        printf(".\n");
    }

    if (optind < argc) {
        printf("extra arguments:\n");
        while(optind < argc) {
            printf("  %s\n", argv[optind++]);
        }
    }
    exit(EXIT_SUCCESS);
}
