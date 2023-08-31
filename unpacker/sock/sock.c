#include "sock.h"


void HandleClient(SOCKET client)
{
    SYSTEMTIME current_time;
    GetLocalTime(&current_time);

    char filename[50];
    snprintf(filename, sizeof(filename), "pack_%04d-%02d-%02d_%02d-%02d-%02d",
        current_time.wYear, current_time.wMonth, current_time.wDay,
        current_time.wHour, current_time.wMinute, current_time.wSecond);

    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        ADR_LOG("Error creating file (%s)", filename);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_received = 0;
    while (1)
    {
        bytes_received = recv(client, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0)
        {
            break;
        }

        DWORD bytes_written = 0;
        if (!WriteFile(hFile, buffer, bytes_received, &bytes_written, NULL) || bytes_written != bytes_received)
        {
            ADR_LOG("Error writing to file (%s)", filename);
            break;
        }
    }

    CloseHandle(hFile);
    closesocket(client);
}


bool StartServer()
{
    ADR_LOG("Server Mode");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ADR_LOG("WSAStartup failed");
        return false;
    }

    SOCKET server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == INVALID_SOCKET)
    {
        ADR_LOG("Socket creation failed");
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(ADR_PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        ADR_LOG("Socket binding failed");
        closesocket(server_socket);
        return false;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR)
    {
        ADR_LOG("Listen failed");
        closesocket(server_socket);
        return false;
    }

    ADR_LOG("Server listening on port %d", ADR_PORT);

    while (1) {
        SOCKET client_socket;
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET)
        {
            ADR_LOG("Accept failed");
            continue;
        }

        ADR_LOG("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HandleClient, (LPVOID)client_socket, 0, NULL);
        if (thread == NULL)
        {
            ADR_LOG("Thread creation failed");
            closesocket(client_socket);
            continue;
        }

        CloseHandle(thread);
    }

    closesocket(server_socket);
    WSACleanup();

    return true;
}
