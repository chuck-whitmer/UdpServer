#include <iostream>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

#define BUFLEN 512
#define PORT 8888

int main()
{
    system("title UDP Server");

    sockaddr_in server, client;

    // initialise winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Error initializing winsock: %d\n", WSAGetLastError());
        exit(0);
    }

    // create a socket
    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Error creating socket: %d\n", WSAGetLastError());
        exit(0);
    }

    // prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // bind
    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    char message[BUFLEN];

    while (true)
    {
        // try to receive some data, but don't block.
        int slen = sizeof(sockaddr_in); 
        int message_len = recvfrom(server_socket, message, BUFLEN-1, MSG_PEEK, (sockaddr*)&client, &slen);
        if (message_len == SOCKET_ERROR)
        {
            printf("recvfrom() failed: %d\n", WSAGetLastError());
            exit(0);
        }
        if (message_len != 0)
        {
            // If the peek showed there was data, then we need to remove it from the queue
            // with a second call to recvfrom.
            message_len = recvfrom(server_socket, message, BUFLEN-1, 0, (sockaddr*)&client, &slen);
            message[message_len] = '\0';
            printf("[%s:%d] %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);
        }

        /*
        std::cin.getline(message, BUFLEN);
        // reply the client with 2the same data
        if (sendto(server_socket, message, strlen(message), 0, (sockaddr*)&client, sizeof(sockaddr_in)) == SOCKET_ERROR)
        {
            printf("sendto() failed with error code: %d", WSAGetLastError());
            return 3;
        }
        */
    }

    closesocket(server_socket);
    WSACleanup();
}