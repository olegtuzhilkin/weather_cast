#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>     //getopt_long

#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr

#include <netdb.h>      //getaddrinfo

#define CITY_LONG   256

char* get_request(char *city)
{
    int socket_desc;
    struct sockaddr_in server;
    char *message, server_reply[2000];

    struct hostent *hp;

    //create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1){
        printf("Could not create socket\n");
        return NULL;
    }

    if((hp = gethostbyname("ya.ru")) == NULL){
            herror("gethostbyname");
            exit(1);
        }

    memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
   // server.sin_addr.s_addr = inet_addr("google.com");
    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    //Connect to remote server
    if (connect(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0){
        printf("Connection error\n");
        return NULL;
    }
    printf("Connected\n");

    //Send some data
    message = "GET / HTTP/1.1\r\n\r\n";
    if( send(socket_desc , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return NULL;
    }
    puts("Data Send\n");

    //Receive a reply from the server
    if( recv(socket_desc, server_reply , 2000 , 0) < 0)
    {
        puts("recv failed");
    }
    puts("Reply received\n");
    puts(server_reply);
}

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
                        get_request(city);
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
        }//switch
    }//while



    return 0;
}
