#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <string>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define DEFAULT_IP "192.168.1.68"

class Client
{
    private:
        WSADATA wsaData;
        SOCKET ConnectSocket = INVALID_SOCKET;
        struct addrinfo *result = NULL,
                        *ptr = NULL,
                        hints;
        //Char array of what is being sent
        char *sendbuf;
        //Char array of what is being recieved
        char recvbuf[DEFAULT_BUFLEN];
        //Integer of the length of recvbuf
        int iResult;
        int recvbuflen = DEFAULT_BUFLEN;
    
    public:
        Client(){}

        int init();

        int sendData();   

        int close();
};

int __cdecl main(void) 
{
    Client c = Client();
    c.init();
    c.sendData();
    c.close();
}

int Client::init()
{
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 0;
    }
    else
    {
        printf("WSAStartup success\n");
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(DEFAULT_IP, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 0;
    }
    else
    {
        printf("getaddrinfo success\n");
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 0;
        }
        else
        {
            printf("socket success\n");
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 0;
    }
    else
    {
        return 1;   
    }
}

int Client::sendData()
{
    std::string line;
    char e;
    while(getline(std::cin, line))
    {
        std::cout << "Waiting for input: ";

        sendbuf = &line[0];

        // Send an initial buffer
        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 0;
        }
    }
    
    return 1;
}

int Client::close()
{
        printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
        {
            printf("Bytes received: %d\n", iResult); 
            printf(recvbuf);
        }
        else if ( iResult == 0 )
        {
            printf("Connection closed\n");
        }
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
        }
            

    } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}