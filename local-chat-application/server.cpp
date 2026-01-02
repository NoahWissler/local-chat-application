#include <iostream>
#include <WinSock2.h>

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
	if (bind(serverSocket,(SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
		std::cout << "Socket could not bind. Error Code : " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return -1;
	}
	else {
		std::cout << "Socket bound!" << std::endl;
	}

	if (listen(serverSocket, 1) == SOCKET_ERROR) {
		std::cout << "Could not listen. Error Code : " << WSAGetLastError() << std::endl;
	}
	else {
		std::cout << "Socket now listening for requests" << std::endl;
	}
	SOCKET acceptSocket = INVALID_SOCKET;
	acceptSocket = accept(serverSocket, NULL, NULL);
	if (acceptSocket == INVALID_SOCKET) {
		std::cout << "accept failed Error Code : " << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
	}
	char buffer[200];
	if (recv(acceptSocket, buffer, 200, 0) == 0) {
		std::cout << "we didnt receive shit" << std::endl;
	}
	else {
		std::cout << "YOU GOT MAIL! : " << buffer << std::endl;
	}



	std::cout << "closing socket, exiting programm" << std::endl;
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}