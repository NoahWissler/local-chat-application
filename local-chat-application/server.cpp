#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <WinSock2.h>

std::mutex acceptMutex;

void sendfunc(SOCKET sock, char buffer[200], const std::string &acceptSocketName) {
    std::string prefix = "client " + acceptSocketName + ": ";
    std::string message = prefix + std::string(buffer);
    send(sock, message.c_str(), static_cast<int>(message.size()), 0);
}

void receiveThread(SOCKET acceptSocket, std::vector<SOCKET>& acceptSockets, std::vector<std::string>& acceptSocketsNames) {
    char buffer[200];

    // Erstes Paket als Name lesen
    int ret = recv(acceptSocket, buffer, sizeof(buffer), 0);
    if (ret <= 0) {
        closesocket(acceptSocket);
        return;
    }
    std::string name(buffer, ret);

    // Namen in die gemeinsame Liste unter Schutz einfügen
    {
        std::lock_guard<std::mutex> g(acceptMutex);
        acceptSocketsNames.emplace_back(name);
    }

    while (true) {
        ret = recv(acceptSocket, buffer, sizeof(buffer), 0);
        if (ret <= 0) break;

        // Kurz die gemeinsamen Listen kopieren (unter Mutex), dann ohne Mutex senden
        std::vector<SOCKET> copyAcceptSockets;
        std::vector<std::string> copyAcceptSocketsNames;
        {
            std::lock_guard<std::mutex> g(acceptMutex);
            copyAcceptSockets = acceptSockets;
            copyAcceptSocketsNames = acceptSocketsNames;
        }

        // Index des sendenden Sockets finden
        auto it = std::find(copyAcceptSockets.begin(), copyAcceptSockets.end(), acceptSocket);
        if (it == copyAcceptSockets.end()) continue;
        size_t index = static_cast<size_t>(std::distance(copyAcceptSockets.begin(), it));
        std::string senderName = (index < copyAcceptSocketsNames.size()) ? copyAcceptSocketsNames[index] : name;

        std::cout << "client " << senderName << ": " << std::string(buffer, ret) << std::endl;

        for (size_t i = 0; i < copyAcceptSockets.size(); ++i) {
            if (copyAcceptSockets[i] != acceptSocket) {
                sendfunc(copyAcceptSockets[i], buffer, senderName);
            }
        }
    }

    // Aufräumen: Socket und Name aus den gemeinsamen Listen entfernen
    {
        std::lock_guard<std::mutex> g(acceptMutex);
        auto it = std::find(acceptSockets.begin(), acceptSockets.end(), acceptSocket);
        if (it != acceptSockets.end()) {
            size_t idx = static_cast<size_t>(std::distance(acceptSockets.begin(), it));
            acceptSockets.erase(acceptSockets.begin() + idx);
            if (idx < acceptSocketsNames.size()) acceptSocketsNames.erase(acceptSocketsNames.begin() + idx);
        }
    }

    closesocket(acceptSocket);
}

int main() {
    std::cout << "Server Application" << std::endl;
    WSAData WSAData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int startUpResult = WSAStartup(wVersionRequested, &WSAData);
    if (startUpResult == 0) {
        std::cout << "WSAPI started up!" << std::endl;
    }
    else {
        std::cout << "WSAStartup not happening lil bro Error Code : " << startUpResult << std::endl;
        return -1;
    }
    SOCKET serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Socket not initialized. Error Code : " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }
    else {
        std::cout << "Socket initialized" << std::endl;
    }
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    service.sin_port = htons(55555);
    if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "Socket could not bind. Error Code : " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }
    else {
        std::cout << "Socket bound!" << std::endl;
    }

    if (listen(serverSocket, 30) == SOCKET_ERROR) {
        std::cout << "Could not listen. Error Code : " << WSAGetLastError() << std::endl;
    }
    else {
        std::cout << "Socket now listening for requests" << std::endl;
    }
    std::vector<std::thread> recvThreads;
    std::vector<SOCKET> acceptSockets;
    std::vector<std::string> acceptSocketsNames;
    for (int i{}; i < 30; ++i) {
        SOCKET acceptSocket = INVALID_SOCKET;
        acceptSocket = accept(serverSocket, NULL, NULL);
        if (acceptSocket == INVALID_SOCKET) {
            std::cout << "accept failed Error Code : " << WSAGetLastError() << std::endl;
            WSACleanup();
            return -1;
        }
        else {
            std::cout << "accepted connection" << std::endl;
            {
                std::lock_guard<std::mutex> g(acceptMutex);
                acceptSockets.emplace_back(acceptSocket);
            }
            // WICHTIG: beide shared-Container als Referenz übergeben
            recvThreads.emplace_back(std::thread(receiveThread, acceptSocket, std::ref(acceptSockets), std::ref(acceptSocketsNames)));
        }
    }

    // Threads joinen, damit shared container nicht zerstört werden, während Threads laufen
    for (auto &t : recvThreads) {
        if (t.joinable()) t.join();
    }

    std::cout << "closing socket, exiting programm" << std::endl;
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}