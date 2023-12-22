#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#define MAX_FILE_SIZE 1000000

//return a sockaddr_in struct
void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        // printf("Setting ipv4 settings");
        return &((struct sockaddr_in*)sa)->sin_addr;

    }
    else {
        // printf("Setting ipv6 settings");
        return &((struct sockaddr_in6*)sa)->sin6_addr;
    }
}

int main(void) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    int sockfd;
    struct addrinfo hints, * servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    int result = getaddrinfo(NULL, "3990", &hints, &servinfo);
    if (result != 0) {
        printf("[!] Error with getaddrinfo(), WSAError = %d\n", WSAGetLastError());
    }


    while (servinfo != NULL) {
        sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
            printf("[!] Error creating socket, WSAError = %d\n", WSAGetLastError());
            servinfo = servinfo->ai_next;
            continue;
        }
        if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) != SOCKET_ERROR) {
            printf("[+] We have bind!\n");
            break;
        }
        else {
            servinfo = servinfo->ai_next;
        }
    }

    freeaddrinfo(servinfo);
    result = listen(sockfd, 5);
    if (result == SOCKET_ERROR) {
        printf("[!] Error listen() , WSAError = %d\n", WSAGetLastError());
        return 1;
    }
    printf("[+] Binded on port 3990, waiting on connections!\n");
    while (1) {
        int accept_socket;
        struct sockaddr_storage thier_addr;
        int addr_len = sizeof(thier_addr);
        accept_socket = accept(sockfd, (struct sockaddr*)&thier_addr, &addr_len);
        if (accept_socket == INVALID_SOCKET) {
            // printf("[!] Error with accpet()! WSAGetLastError() = %d \n", WSAGetLastError());
            continue;
        }
        else {
            // printf("Accepted a socc");
            char s[INET6_ADDRSTRLEN];
            inet_ntop(thier_addr.ss_family, get_in_addr((struct sockaddr*)&thier_addr), s, INET6_ADDRSTRLEN);
            printf("[+] Accepted connection from %s !\n", s);
        }
        char hostName[256];
        gethostname(hostName, 256);
        char fullString[1024];
        sprintf_s(fullString, "Welcome, you have connected to %s, please input a file name to store:  \n", hostName);
        if (send(accept_socket, fullString, strlen(fullString), 0) == SOCKET_ERROR) {
            printf("[!] Error with send()! WSAGetLastError() = %d \n", WSAGetLastError());
            continue;
        }
        char fileName[126];
        int recv_bytes = recv(accept_socket, fileName, 126, 0);
        if (recv_bytes == SOCKET_ERROR) {
            close(accept_socket);
            printf("[!] Error with recv()! WSAGetLastError() = %d \n", WSAGetLastError());
        }

        fileName[recv_bytes] = '\0';
        printf("[+] recieved filename %s, will wait for incoming data next\n", fileName);

        int size_of_incoming_file = 0;
        char recv_file_size[10];
        recv_bytes = recv(accept_socket, recv_file_size, 10, 0);
        if (recv_bytes == SOCKET_ERROR) {
            close(accept_socket);
            printf("[!] Error with recv()! WSAGetLastError() = %d \n", WSAGetLastError());
        }
        recv_file_size[recv_bytes] = '\0';
        size_of_incoming_file = atoi(recv_file_size);
        printf("[+] Incoming file size of %d bytes\n", size_of_incoming_file);

        //char* recv_file = malloc(sizeof(char) * (size_of_incoming_file + 2));
        char recv_file[MAX_FILE_SIZE];
        HANDLE hFile = CreateFileA(fileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (!hFile) {
            printf("[-] Error with creating file, GetLastError() = %d\n");
            close(accept_socket);
            continue;
        }
        result = recv(accept_socket, recv_file, size_of_incoming_file + 2, 0);
        if (result == SOCKET_ERROR) {
            printf("[!] Error with recv()! WSAGetLastError() = %d \n", WSAGetLastError());
            close(accept_socket);
            
        }
        int lp_num_of_bytes_written = 0;
        if (!WriteFile(hFile, recv_file, size_of_incoming_file, &lp_num_of_bytes_written, NULL)) {
            printf("[-] Error with writing file, GetLastError() = %d\n");
            close(accept_socket);
            CloseHandle(hFile);
            continue;
        }
        printf("[+] Wrote to file %s!", fileName);
        CloseHandle(hFile);
        closesocket(accept_socket);

    }

    return 0;
}