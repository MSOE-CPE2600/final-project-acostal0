/*********************************************
 * Filename: server.c
 * Assignment: Lab 13 voting machine
 * Section: 121
 * Description: Server for a distributed voting machine system.
 * Author: Lizbeth Acosta
 * Date: 12/05/24
 * Note: compile with
 * $ make
 * $ ./server
 *********************************************/

#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <pthread.h>    
#include <semaphore.h> 
#include <arpa/inet.h>

#define MAX_PEOPLE 5    //max number of candidates
#define MAX_VOTERS 100  //max number of voters
#define BUFFER_SIZE 256 //buffer size for messages
#define PORT 8080       //port the server listens on

//struct representing candidate
typedef struct {
    char name[50];      //candidate's name
    int votes;          //number of votes a candidate has received
} Candidate;

//struct representing voter
typedef struct {
    char id[50];        //unique voter ID
} Voter;

//arrays for candidates and voters
Candidate candidate[MAX_PEOPLE] = {
    {"Lizbeth", 0},     //candidate 1
    {"Justin", 0},      //candidate 2
    {"Aidan", 0},       //candidate 3
    {"Spencer", 0}      //candidate 4
};
int num_people = 4;     //number of candidates there are
Voter voters[MAX_VOTERS];   //array to track voters
int num_voters = 0;     //current num of recorded voters

sem_t vote_sem;         //semaphore

void *handle_client(void *arg);      
void process_vote(char *id, char *name, int client_socket);
void send_results(int client_socket);   
int has_voted(char *id);                
void record_vote(char *id, char *name); 

int main() {
    int server_socket, client_socket;    //server and client socket descriptors
    struct sockaddr_in server_addr, client_addr; //address structures for server and client
    socklen_t client_len = sizeof(client_addr);  //size of the client address structure

    //make semaphore have a value of 1
    sem_init(&vote_sem, 0, 1);

    //create the server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        //if socket creation fails print error 
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE);            
    }

    //configure server address structure
    server_addr.sin_family = AF_INET;        
    server_addr.sin_addr.s_addr = INADDR_ANY; //accept connections on any IP
    server_addr.sin_port = htons(PORT);     

    //bind server socket to specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        //if binding fails print error 
        perror("Bind failed");   
        close(server_socket);    //close server socket
        exit(EXIT_FAILURE);
    }

    //listen for incoming client connections
    if (listen(server_socket, 5) == -1) {
        //if listening fails print error 
        perror("Listen failed"); 
        close(server_socket);    //close server socket
        exit(EXIT_FAILURE);     
    }

    printf("Server running on port %d\n", PORT); //server is running

    while (1) {
        //accept a client connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1) {
            //if accepting connection fails print error
            perror("Accept failed"); 
            continue;                
        }

        printf("Client connected\n"); //client has connected

        //new thread to handle client
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, (void *)&client_socket) != 0) {
            //if thread creation fails print error 
            perror("Thread creation failed"); 
            close(client_socket);           
        }
    }

     
    close(server_socket); //close server socket
    sem_destroy(&vote_sem); //destroy semaphore
    return 0; 
}


void *handle_client(void *arg) {
    //handle client communication in different thread

    int client_socket = *(int *)arg;  //get client socket descriptor
    char buffer[BUFFER_SIZE];        //buffer to store messages from client

    while (1) {
        memset(buffer, 0, BUFFER_SIZE); //clear buffer
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0); //receive data

        if (bytes_read <= 0) {         
            //check if client has disconnected
            printf("Client disconnected\n");
            break;                    
        }

        char *command = strtok(buffer, " "); //get first word of the command

        if (strcmp(command, "VOTE") == 0) {
            //check if the command is "VOTE"

            char *id = strtok(NULL, " ");    //get voter ID
            char *name = strtok(NULL, " ");  //get candidate name

            if (id && name) {                
                process_vote(id, name, client_socket); //process vote
            } else {
                //print error
                send(client_socket, "ERROR Invalid vote format\n", strlen("ERROR Invalid vote format\n"), 0);
            }
        } else if (strcmp(command, "RESULT") == 0) { 
            //if command is "RESULT" send election results
            send_results(client_socket); 
        } else {
            send(client_socket, "ERROR Unknown command\n", strlen("ERROR Unknown command\n"), 0); //handle unknown command
        }
    }

    close(client_socket); 
    pthread_exit(NULL); 
}


void process_vote(char *id, char *name, int client_socket) {
    //process a vote request

    sem_wait(&vote_sem);  //lock semaphore

    if (has_voted(id)) {  
        //check if voter has voted

        send(client_socket, "ERROR Voter has already voted\n", strlen("ERROR Voter has already voted\n"), 0);
    } else {
        record_vote(id, name); //record vote
        send(client_socket, "Vote recorded\n", strlen("Vote recorded\n"), 0);
    }

    sem_post(&vote_sem);  //release semaphore
}


void send_results(int client_socket) {
    //sends election results to client

    char buffer[BUFFER_SIZE]; //buffer to store results
    memset(buffer, 0, BUFFER_SIZE); //clear buffer
    strcat(buffer, "Election results:\n"); //add header to buffer

    sem_wait(&vote_sem); //lock semaphore

    for (int i = 0; i < num_people; i++) { 
        //loop through all candidates

        char line[100];
        sprintf(line, " %s: %d votes\n", candidate[i].name, candidate[i].votes); //format candidate results
        strcat(buffer, line); //append results to buffer
    }

    sem_post(&vote_sem); //release semaphore

    send(client_socket, buffer, strlen(buffer), 0); //send results to client
}

int has_voted(char *id) {
    //check if a voter has voted

    for (int i = 0; i < num_voters; i++) { 
        //loop through all recorded voters

        if (strcmp(voters[i].id, id) == 0) { 
            //check if ID matches
            return 1; //if voter has already voted return true 
        }
    }
    return 0; //else return false
}

void record_vote(char *id, char *name) {
    //record a vote for a candidate

    for (int i = 0; i < num_people; i++) { 
        //loop through all candidates

        if (strcmp(candidate[i].name, name) == 0) { 
            //check if candidate name matches

            candidate[i].votes++; //add 1 to the candidate's vote count
            strcpy(voters[num_voters++].id, id); //record voter ID
            return;
        }
    }
}
