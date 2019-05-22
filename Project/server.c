/**
* Based on code found at https://github.com/mafintosh/echo-servers.c (Copyright (c) 2014 Mathias Buus)
* Copyright 2019 Nicholas Pritchard, Ryan Bunney
**/

/**
 * @brief A simple example of a network server written in C implementing a basic echo
 * 
 * This is a good starting point for observing C-based network code but is by no means complete.
 * We encourage you to use this as a starting point for your project if you're not sure where to start.
 * Food for thought:
 *   - Can we wrap the action of sending ALL of out data and receiving ALL of the data?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define BUFFER_SIZE 1024

//check if read was successful
int read_check(read){
    if (read < 0){
        fprintf(stderr,"Client read failed\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

//check if server have successfully send packet to client
int send_check(send){
    if (send < 0){
        fprintf(stderr,"Message send fail.\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

//send packets to client after determined the outcome of the guess
int send_mess(char* buf, int client_fd, char* outcome){
    buf[0] = '\0';
    sprintf(buf, "1,%s",outcome);
    int err = send(client_fd, buf, strlen(buf), 0);
    send_check(err);
    return 0;
}

//validate player moves
int validate_moves(char* buf, int client_fd,int* player_lives){

    int lives = *player_lives;
    if(strstr(buf, "MOV,EVEN") != NULL){
        //roll the dice
        int dice1 = rand()%6+1;
        int dice2 = rand()%6+1;
        if((dice1+dice2)%2==0){
            char* outcome = "PASS";
            send_mess(buf,client_fd,outcome);
        }
        else{
            --(*player_lives);
            --lives;
            if(lives==0){
                char* outcome = "ELIM";
                send_mess(buf,client_fd,outcome);
            }
            else{
                char* outcome = "FAIL";
                send_mess(buf,client_fd,outcome);
            }
        }
    }
    else if(strstr(buf, "MOV,ODD") != NULL){
        int dice1 = rand()%6+1;
        int dice2 = rand()%6+1;
        if((dice1+dice2)%2==1){
            char* outcome = "PASS";
            send_mess(buf,client_fd,outcome);
        }
        else{
            --(*player_lives);
            --lives;
            if(lives==0){
                char* outcome = "ELIM";
                send_mess(buf,client_fd,outcome);
            }
            else{
                char* outcome = "FAIL";
                send_mess(buf,client_fd,outcome); 
            }
        }
    }
    else if(strstr(buf, "MOV,DOUB") != NULL){
        int dice1 = rand()%6+1;
        int dice2 = rand()%6+1;
        if(dice1==dice2){
            char* outcome = "PASS";
            send_mess(buf,client_fd,outcome);
        }
        else{
            --(*player_lives);
            --lives;
            if(lives==0){
                char* outcome = "ELIM";
                send_mess(buf,client_fd,outcome);
            }
            else{
                char* outcome = "FAIL";
                send_mess(buf,client_fd,outcome);
            }
        }
    }
     else if(strstr(buf, "MOV,CON") != NULL){
        int dice1 = rand()%6+1;
        int dice2 = rand()%6+1;
        char* token;
        token = strtok(buf,",");

        for (int i = 0; i < 3; ++i)
        {
            token = strtok(NULL,",");
        }
        if(atoi(token)==dice1||atoi(token)==dice2){
            char* outcome = "PASS";
            send_mess(buf,client_fd,outcome);
        }
        else{
            --(*player_lives);
            --lives;
            if(lives==0){
                char* outcome = "ELIM";
                send_mess(buf,client_fd,outcome);
            }
            else{
                char* outcome = "FAIL";
                send_mess(buf,client_fd,outcome);
            }
        }
    }
    return 0;
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [port]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    int server_fd;
    struct sockaddr_in server;

    int client_fd;
    struct sockaddr_in client;

    int err, opt_val, pid;
    char *buf;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0){
        fprintf(stderr,"Error in connection\n");
        exit(EXIT_FAILURE);
    }


    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    if (err < 0){
        fprintf(stderr,"Could not bind socket\n");
        exit(EXIT_FAILURE);
    } 

    err = listen(server_fd, 128);
    if (err == 0){
        printf("Server is listening on %d\n", port);
    }
    else{
        fprintf(stderr,"Could not listen on socket\n");
        exit(EXIT_FAILURE);
    }

    //initializing user_id and number of players
    int user_id = 0;
    int player_num = 0;
    while (1) {
        socklen_t client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd < 0) {
            fprintf(stderr,"Could not establish new connection\n");
            exit(EXIT_FAILURE);
        } 
        else{
            printf("Connection accepted from %s:%d\n", inet_ntoa(client.sin_addr),ntohs(client.sin_port));
            //if the connection successful increment the player_id and number of players.
            user_id++;
            player_num++;
        }

        while (1) {  
            buf = calloc(BUFFER_SIZE, sizeof(char));
            int read = recv(client_fd, buf, BUFFER_SIZE, 0);
            read_check(read);
            int player_lives = 4;
            //read packets, if contain "INIT" inside packets then initialize the game.
            if (strstr(buf, "INIT") != NULL){
                buf[0] = '\0';
                sprintf(buf, "WELCOME,%d",user_id);
                err = send(client_fd, buf, strlen(buf), 0); 
                //wait for 1 second to send welcome and start separately
                sleep(1);
                buf[0] = '\0';
                sprintf(buf,"START,%d,%d",player_num,player_lives);
                err = send(client_fd, buf, strlen(buf), 0);
                if(send_check(err)==0){
                    while(1){
                        if(player_lives>0){
                            struct pollfd fd;
                            int ret;

                            fd.fd = client_fd;
                            fd.events = POLLIN;
                            //server timeout 5 seconds waiting for the client response
                            ret = poll(&fd,1,5000);

                            switch(ret){
                                case -1:
                                    break;
                                case 0:
                                    --player_lives;
                                    if(player_lives==0){
                                        char* outcome = "ELIM";
                                        send_mess(buf,client_fd,outcome);
                                    }
                                    else{
                                        char* outcome = "FAIL";
                                        send_mess(buf,client_fd,outcome);
                                    }
                                    break;
                                default:
                                    buf = calloc(BUFFER_SIZE, sizeof(char));
                                    int read2 = recv(client_fd, buf, BUFFER_SIZE, 0);
                                    read_check(read2);
                                    validate_moves(buf,client_fd,&player_lives);
                            }
                        }
                        else{
                            free(buf);
                            printf("Connection disconnected from %s:%d\n", inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                            close(client_fd);
                            close(server_fd);
                            exit(EXIT_SUCCESS);
                        }

                    }
                }
            }
        }
        close(client_fd);
    }
    return 0;
}