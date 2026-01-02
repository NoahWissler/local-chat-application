#include <iostream>
#include <WinSock2.h>


int main() {
	std::cout << "Client Application" << std::endl;
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
	SOCKET clientSocket = INVALID_SOCKET;
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cout << "Socket not initialized. Error Code : " << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
	}
	else {
		std::cout << "Socket initialized" << std::endl;
	}
	SOCKADDR_IN clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	clientService.sin_port = htons(55555);
	if (connect(clientSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		std::cout << "Could not connect error code : " << WSAGetLastError() << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}
	else {
		std::cout << "connected" << std::endl;
	}
	char buffer[200];
	std::cout << "enter message for server" << std::endl;
	std::cin.getline(buffer, 200);
	if (send(clientSocket, buffer, 200, 0) == 0) {
		std::cout << "nothing sent :(" << std::endl;
	}
	else {
		std::cout << "sent message :)" << std::endl;
	}


	std::cout << "closing socket, exiting programm" << std::endl;
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}