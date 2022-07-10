#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>     //getopt_long

#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr

#include <netdb.h>      //getaddrinfo

#include <errno.h>
#include <fcntl.h>

#include <ctype.h>

#define CITY_LONG   256
#define RQST_LONG   256
#define MSG_LONG    5096

#define PARAM_FLOAT     0
#define PARAM_STRING    1

struct t_location {
    char label[100];
    char latitude[10], longitude[10];
};

void setFdNonblock(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

char *get_request(char *city)
{
    int socket_desc;
    struct sockaddr_in server;
    char message[RQST_LONG], server_reply[MSG_LONG];
    static char whole_request[MSG_LONG];
    struct hostent *hp;

    //create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1){
        printf("error: Could not create socket\n");
        return NULL;
    }

    if((hp = gethostbyname("api.positionstack.com")) == NULL){
        printf("error: gethostbyname\n");
        return NULL;
    }

    if (memcpy(&server.sin_addr, hp->h_addr, hp->h_length) == NULL){
        printf("error: memcpy\n\rCouldn't copy memory\n");
        return NULL;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    //Connect to remote server
    if (connect(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0){
        printf("error: Connection error\n");
        return NULL;
    }
    printf("Connected\n");

    //Send some data
    sprintf(message, "GET /v1/forward?access_key=a1567a1c4e82698080c3de2445183e23&&query=%s HTTP/1.1\r\nHost: api.positionstack.com\r\nUser-Agent: weather_cast_1.0\r\nAccept: */*\r\n\r\n", city);
    if(send(socket_desc , message , strlen(message) , 0) < 0)
    {
        printf("error: Sending request is failed\n");
        return NULL;
    }
//    printf("Data Send\n");

    //Receive a reply from the server
//    setFdNonblock(socket_desc);
    char enter = 1;                         //condition to enter to while. switch enter to 0 after getting end_rqst
    int n = 0, whole_msg_pos = 0;           //n - count of received bytes, whole_msg_pos - shift osition in dest buffer - whole request
    char end_rqst[] = "]}\r\n";            //condition of success received message
    while (enter){
        //printf("errno=%d",errno);
        if((n = recv(socket_desc, server_reply , MSG_LONG, 0)) > 0){    //receive data from server
//            printf("received %d bytes\n", n);
            if (memcpy(whole_request+whole_msg_pos, server_reply, n) == NULL){  //copy data to dest buffer
                printf("error: memcpy\n\rCouldn't copy memory\n");
                return NULL;
            };
            whole_msg_pos += n;             //shift pointer at dest buffet
            if (strstr(server_reply, end_rqst) != NULL)     //if end condition is received - exit from receuve cycle
                enter = 0;
        }
    }

//    printf("Reply received\n");
//    printf("%s\n", whole_request);
    return whole_request;
}

char *copy_param(char *req, char *dest, char *param, int param_type)
{
    char *tmp;
    int length;
    tmp = NULL; length = 0;

    req = strstr(req, param);
    if (req != NULL){
        switch (param_type) {
        case PARAM_FLOAT:
            req += strlen(param);
            while (!isdigit((int)(*req)))
                req++;
            tmp = req;
            while ((isdigit((int)(*tmp)))||(*tmp == '.'))
                tmp++;

            length = tmp - req;
            strncpy(dest, req, length);
            break;
        case PARAM_STRING:
            req += strlen(param);
            while (!isalpha((int)(*req)))
                req++;
            tmp = req;
            while ((isalpha((int)(*tmp)))||(*tmp == ',')||(*tmp == ' '))
                tmp++;

            length = tmp - req;
            strncpy(dest, req, length);
            break;
        }
    }

    return req;
}

struct t_location* parse_coord(char *request, int *cntr)
{
    static struct t_location* result;
    int *counter = cntr;
    char *req = request,
         *tmp;

    int length;

    if (result != NULL)
        free(result);           //????
    //counter = *cntr;
    *counter = 0; length = 0;
    result = NULL;

    req = strstr(req, "data");
    if (req != NULL){

        tmp = strstr(req, "latitude");
        while (tmp != NULL){
            (*counter)++;
            result = realloc(result, (*counter)*sizeof (struct t_location));

            req = copy_param(req, result[(*counter)-1].latitude, "latitude", PARAM_FLOAT);
            req = copy_param(req, result[(*counter)-1].longitude, "longitude", PARAM_FLOAT);
            req = copy_param(req, result[(*counter)-1].label, "label", PARAM_STRING);

//                req = strstr(req, "label");
//                if (req != NULL){
//                    req = strchr(req, ':')+1;
//                    tmp = strchr(req, '}')-2;
//                    length = tmp - req;
//                    strncpy(result[counter-1].label, req+1, length);
//                    }


            tmp = strstr(req, "latitude");
        }

    }

    return result;
}

int main(int argc, char *argv[])
{
    char city[CITY_LONG], *pos_req = NULL;
    const char* short_options = "c:";

    struct t_location *cities_list;
    int counter, current_pos;
    int i;

    extern char* optarg;
    extern int optind;
    int opchar = 0;
    int opindex = 0;
    struct option long_options[] = {
        {"city", required_argument, NULL, 'c'},
        {NULL, 0, NULL, 0}
    };

    current_pos = 0;

    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    while ((opchar = getopt_long(argc, argv, short_options, long_options, &opindex)) != -1){
        switch (opchar){
            case 'c':
                    printf("long opt is %s\n", long_options[opindex].name);
                    printf("short opt is %c\n", long_options[opindex].val);
                    if (optarg != NULL){
                        sprintf(city, "%s", optarg);
                        printf("arg is %s\n", city);
                        pos_req = get_request(city);
                        if (pos_req != NULL){
                            printf("\n%s\n", pos_req);

                        }
                        else{
                            printf("bad request\n");
                            exit(0);
                        }

                        cities_list = parse_coord(pos_req, &counter);

                        if (counter > 1){
                            printf("Please enter the number of city which you need:\n");
                            for (i = 0; i < counter; i++){
                                printf("%d: %s\n", i, cities_list[i].label);
                            }
                            printf("-------------------------------------------------------\n");
                            scanf(" %d", &current_pos);
                            if (current_pos >= counter){
                                printf("error: you entered incorrect value\n");
                                exit(0);
                            }
                            //printf("%d\n", current_pos);
                        }

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
