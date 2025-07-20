#include <winsock2.h>
#include <ws2tcpip.h>
#include <locale.h>
#include <string>
#include <sstream>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

const int SERVER_ERROR = 0;
const int SUCCESS = 1;
const int EXIT = 2;

struct CLIENT {
	SOCKET socket;
	SOCKADDR_IN socket_addr;
};

CLIENT client;

std::string input() {
	std::string summands;
	std::getline(std::cin, summands);
	return summands;
}

int make_client(std::string server_ip, int port) {
	int ret_val;

	WSAData wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
		std::cout << "WSAStartup failed" << std::endl;
		return SERVER_ERROR;
	}

	client.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (client.socket == INVALID_SOCKET) {
		std::cout << "Socket creation failed with error: " << WSAGetLastError() << "\n\n";
		WSACleanup();
		return SERVER_ERROR;
	}

	client.socket_addr.sin_family = AF_INET;
	client.socket_addr.sin_port = htons(port);
	inet_pton(AF_INET, (PCSTR)server_ip.data(), &client.socket_addr.sin_addr);

	return SUCCESS;

}

int connect_server(std::string server_ip, int port) {
	int ret_val; 

	ret_val = connect(client.socket, (sockaddr*)&client.socket_addr, sizeof(client.socket_addr));

	if (ret_val == SOCKET_ERROR) {
		std::cout << "Connecting failed with error: " << WSAGetLastError() << "\n\n";
		closesocket(client.socket);
		WSACleanup();
		return SERVER_ERROR;
	}
	return SUCCESS;
}

int sum(std::string summands, std::string* res) {
	int ret_val;

	ret_val = send(client.socket, summands.data(), summands.size(), 0);

	if (ret_val == SOCKET_ERROR) {
		std::cout << "Sending failed with error: " << WSAGetLastError() << "\n\n";
		closesocket(client.socket);
		WSACleanup();
		return SERVER_ERROR;
	}

	if (summands.compare("stop") == 0) {
		closesocket(client.socket);
		WSACleanup();
		return EXIT;
	}

	res->resize(1024);
	
	ret_val = recv(client.socket, (char*)res->data(), 1024, 0);

	if (ret_val == SOCKET_ERROR) {
		std::cout << "Receiving failed with error: " << WSAGetLastError() << "\n\n";
		closesocket(client.socket);
		WSACleanup();
		return SERVER_ERROR;
	}

	closesocket(client.socket);
	return SUCCESS;

}

int menu() {
	int ret_val = -1;
	while (true) {
		std::cout << "Input server IP and port space-separated or \"stop\" for exit" << std::endl;

		std::string server_ip;
		int port;

		std::cin >> server_ip;
		if (server_ip.compare("stop") == 0)
			return 0;

		std::cin >> port;
		std::cin.ignore();

		ret_val = make_client(server_ip, port);

		if (ret_val == SERVER_ERROR)
			continue;

		ret_val = connect_server(server_ip, port);

		if (ret_val == SERVER_ERROR)
			continue;

		bool cont_sum = true;
		while (cont_sum) {
			std::cout << "Input 2 summands space separated or \"stop\" for exit" << std::endl;
			std::string res;

			int ret_val = sum(input(), &res);

			switch (ret_val) {
			case(SUCCESS):
				std::cout << "Сумма: " << res << "\n\n";
				make_client(server_ip, port);
				connect_server(server_ip, port);
				break;
			case(EXIT):
				return 0;
			case(SERVER_ERROR):
				cont_sum = false;
				break;
			}
		}
	}
}

int main() {
	setlocale(LC_ALL, "rus");
	menu();
}