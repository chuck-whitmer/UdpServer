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
        printf("Two arguments are expected: port and filename\n");
        return 1;
    }
    int port = atoi(argv[1]);
    char* filename = argv[2];
    printf("Logging port %d to file %s\n", port, filename);

    FILE* file = fopen(filename, "at");
    if (file == NULL)
    {
        printf("Can't open file for writing.\n");
        return 1;
    }

    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        printf("Could not set control handler.\n");
        return 1;
    }

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
        // try to receive some data without blocking.
        // (Blocking inhibits the ^C capture.)
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
            message_len = recvfrom(server_socket, message, BUFLEN - 1, 0, (sockaddr*)&client, &slen);
            // We assume no \n at the end.
            if (message_len > 0 && message[message_len - 1] == '\n') message_len--;
            message[message_len] = '\0';
            char outbuffer[512];
            sprintf(outbuffer, "[%s:%d] %s", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);
            puts(outbuffer);
            fputs(outbuffer, file); fputs("\n", file);
        }
    }
    fclose(file);
    closesocket(server_socket);
    WSACleanup();
}

