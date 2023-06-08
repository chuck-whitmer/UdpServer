#include <iostream>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

#define BUFLEN 512

bool breakReceived = false;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT) breakReceived = true;
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Two arguments are expected: IP address and port\n");
        return 1;
    }
    int port = atoi(argv[2]);
    char* ipString = argv[1];
    printf("Sending to %s:%d\n", ipString, port);

    unsigned long ulAddr = inet_addr(ipString);
    if (ulAddr == INADDR_NONE) {
        printf("inet_addr failed and returned INADDR_NONE\n");
        return 1;
    }

    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        printf("Could not set control handler.\n");
        return 1;
    }
    printf("ulAdder = %08x\n", ulAddr);

    sockaddr_in server, client;

    client.sin_family = AF_INET;
    client.sin_addr.S_un.S_addr = ulAddr;
    client.sin_port = htons(port);


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
    server.sin_port = htons(port);

    // bind
    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    char message[BUFLEN];

    while (!breakReceived)
    {
        std::cin.getline(message, BUFLEN);
        if (sendto(server_socket, message, strlen(message), 0, (sockaddr*)&client, sizeof(sockaddr_in)) == SOCKET_ERROR)
        {
            printf("sendto() failed with error code: %d", WSAGetLastError());
            return 3;
        }
    }
    closesocket(server_socket);
    WSACleanup();
}

