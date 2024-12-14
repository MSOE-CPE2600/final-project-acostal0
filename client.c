/*********************************************
 * Filename: client.c
 * Assignment: Lab 13 voting machine
 * Section: 121
 * Description: Client for voting machine system
 * Author: Lizbeth Acosta
 * Date: 12/05/24
 * Note: compile with
 * $ make
 * $ ./client --v 1242 Lizbeth 127.0.0.1
 * $ ./client --r 127.0.0.1
 * server_ip = 127.0.0.1
 *********************************************/

#include <stdio.h>      
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>     
#include <arpa/inet.h>  //networking functions

#define SERVER_PORT 8080   //port on which the server is listening
#define BUFFER_SIZE 1024   //buffer size for sending and receiving data


void send_vote(const char *id, const char *candidate, const char *server_ip);
void request_result(const char *server_ip);

int main(int argc, char *argv[]) {
    
    if (argc < 3) {
        //check if the user provided enough arguments

        fprintf(stderr, "Usage:\n"); //print usage instructions
        fprintf(stderr, " %s --v <id> <candidate> <server_ip>\n", argv[0]); //for voting
        fprintf(stderr, " %s --r <server_ip>\n", argv[0]); //for requesting results
        return 1; 
    }

    const char *server_ip = argv[argc - 1]; //the server IP is always the last argument

   
    if (strcmp(argv[1], "--v") == 0) {
        //check if the user wants to vote

        if (argc != 5) { //if correct number of arguments for voting
            fprintf(stderr, "Usage: %s --v <id> <candidate> <server_ip>\n", argv[0]);
            return 1; 
        }
        send_vote(argv[2], argv[3], server_ip); //call the voting function
    } else if (strcmp(argv[1], "--r") == 0) {
        //check if the user wants to request results

        request_result(server_ip); //call the results function
    } else {
        //handle unknown commands

        fprintf(stderr, "Unknown command: %s\n", argv[1]); 
        return 1;
    }
    return 0; 
}


void send_vote(const char *id, const char *candidate, const char *server_ip) {
    //send a vote to the server

    int sock; //socket descriptor
    struct sockaddr_in server_addr; //server address structure
    char buffer[BUFFER_SIZE]; //buffer for sending and receiving messages

    //create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        //if socket fails print error
        perror("Socket creation error"); 
        exit(EXIT_FAILURE); 
    }

    //set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    //convert server IP address from text to binary
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported"); 
        exit(EXIT_FAILURE);
    }

    //connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE); 
    }

    //format vote request
    snprintf(buffer, BUFFER_SIZE, "VOTE %s %s", id, candidate);
    send(sock, buffer, strlen(buffer), 0); //send vote request to the server

    //receive server's response
    memset(buffer, 0, BUFFER_SIZE); //clear buffer
    recv(sock, buffer, BUFFER_SIZE, 0); //receive data from server
    printf("Server response: %s\n", buffer);

    close(sock);
}

void request_result(const char *server_ip) {
    //function to request election results from server

    int sock; //socket descriptor
    struct sockaddr_in server_addr; //server address structure
    char buffer[BUFFER_SIZE]; //buffer for sending and receiving messages

    //create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error"); 
        exit(EXIT_FAILURE); 
    }

    //set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    //convert server IP address from text to binary
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    //connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    //send request for election results
    snprintf(buffer, BUFFER_SIZE, "RESULT");
    send(sock, buffer, strlen(buffer), 0); //send request to server

    //receive the server's response
    memset(buffer, 0, BUFFER_SIZE); //clear buffer
    recv(sock, buffer, BUFFER_SIZE, 0); //receive data from server
    printf("Server response: %s\n", buffer);

    close(sock);
}
