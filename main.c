#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define CITY_LONG   256

int main(int argc, char *argv[])
{
    char city[CITY_LONG];
    const char* short_options = "c:";

    extern char* optarg;
    extern int optind;
    int opchar = 0;
    int opindex = 0;
    struct option long_options[] = {
        {"city", required_argument, NULL, 'c'},
        {NULL, 0, NULL, 0}
    };

    while ((opchar = getopt_long(argc, argv, short_options, long_options, &opindex)) != -1){
        switch (opchar){
            case 'c':
                    printf("long opt is %s\n", long_options[opindex].name);
                    printf("short opt is %c\n", long_options[opindex].val);
                    if (optarg != NULL){
                        sprintf(city, "%s", optarg);
                        printf("arg is %s\n", city);
                    }
                    else{
                        printf("You didn't enter the city\n");
                        exit(0);
                    }
                    break;
            case '?':
                    printf("opt is none\n");
                    exit(0);
                    break;
        }
    }

    return 0;
}
