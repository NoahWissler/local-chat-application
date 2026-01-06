#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <WinSock2.h>

std::mutex acceptMutex;

void sendfunc(SOCKET sock, char buffer[200], int clientNr) {
	std::string prefix = "client " + std::to_string(clientNr) + ": ";
	std::string message = prefix + buffer;
		send(sock, message.c_str(), 200, 0);
}

void receiveThread(SOCKET acceptSocket, std::vector <SOCKET> &acceptSockets) {
	char buffer[200];
	while (true) {
		if (recv(acceptSocket, buffer, 200, 0) != 0) {
			std::cout << "client : " << buffer << std::endl;
			std::vector<SOCKET> copy;
			{
				std::lock_guard<std::mutex> g(acceptMutex);
				copy = acceptSockets;
			}
			for (int i{}; i < copy.size(); ++i) {
				if (copy[i] != acceptSocket) {
					sendfunc(copy[i], buffer, i+1);
				}
			}
		}
	}
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
		acceptSockets.emplace_back(acceptSocket);
        recvThreads.emplace_back(std::thread(receiveThread, acceptSocket, std::ref(acceptSockets)));
		
	}
}



	std::cout << "closing socket, exiting programm" << std::endl;
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}