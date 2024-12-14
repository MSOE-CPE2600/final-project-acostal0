# Lab 13 Final Project

## Overview
This lab implements a voting machine system with a client-server application that is meant to handle electronic voting. User's can submit votes for candidates and get elecion results.

### usage
To vote:
* ./client --v <id> <candidate> <server_ip>
For results:
* ./client --r <server_ip>
For server:
* ./server

### server.c
- handle_client
This method is for handling the client. Each client has its own thread.
Wait for a command from the client.
Parse the command line.
If they want to vote, then record the vote.
If they want to see the results, display the results to the client.
Use semaphores to make sure everything works properly when multiple clients interact simultaneously.

- process_vote
Method records a vote for a candidate and makes sure no voter votes twice. 

- send_results
This function sends the current election results to the client. Goes through the list of candidates and formats it. Sends formatted results to the client.

- has_voted
This method checks whether the voter id is already in the list of recorded voters. It goes through voters array to check if the id matches any other one.

- record_vote
This method records a vote for a specific candidate and logs the voter's id. Goes through the cadidate array to find the candidate name. Adds 1 to the vote count for the candidate and then adds the voter's id to the voters array.

### client.c
- send_vote
This method sends a vote for a specifc candidate to the server. Creates a socket and connects to the server using the server ip. Formats the vote requested and sends it to the server and waits for the server's response and displays it.

- request_result 
This method requests the current election results from the server. Create a socket and connects to the server using the server ip. Sends a result command to the server. displays the server response.