#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512            
#define IP_ADDRESS "192.168.56.1"
#define DEFAULT_PORT "3504"
char* duplicate_msg = "Ten da bi trung. Vui long dang ki lai.\n";
char* end_game_message = "Tro choi ket thuc.\n";
struct client_type
{
	SOCKET socket;
	int id;
	char received_message[DEFAULT_BUFLEN];
	bool end_game;
};

int process_client(client_type &new_client);
bool checkValidName(string name);
bool checkSendMessage(std::string msg);

bool checkValidName(string name)
{
	int len = name.length();
	if (len > 10)
		return false;
	for (int i = 0; i < len; i++)
	{
		if ((name[i] >= 65 && name[i] <= 90
			|| name[i] >= 48 && name[i] <= 57
			|| name[i] >= 97 && name[i] <= 122) == false)
		{
			return false;
		}
	}
	return true;
}
bool checkSendMessage(std::string msg)
{
	//msg must have format as number-index_column
	//Ex: 6-A(6-a), 1-B(1-b) ,1-A(1-a),....
	int len = msg.length();
	if (len == 3)
	{
		if (msg[0] >= 49 && msg[0] <= 54)//from 1-6
		{
			if (msg[2] == 'A' || msg[2] == 'B' || msg[2] == 'C' || msg[2] == 'a' || msg[2] == 'b' || msg[2] == 'c')
				return true;
		}
	}
	return false;
}
int process_client(client_type &new_client)
{
	while (1)
	{
		memset(new_client.received_message, 0, DEFAULT_BUFLEN);

		if (new_client.socket != 0)
		{
			int iResult = recv(new_client.socket, new_client.received_message, DEFAULT_BUFLEN, 0);

			if (iResult != SOCKET_ERROR)
			{
				bool end = true;
				for (int i = 0; end_game_message[i] != '\0'; i++)
				{
					if (new_client.received_message[i] != end_game_message[i])
					{
						end = false;
						break;
					}
				}
				if (end)
				{
					closesocket(new_client.socket);
				}
				cout << new_client.received_message << endl;
			}
			else
			{
				cout << "recv() failed: " << WSAGetLastError() << endl;
				break;
			}
		}
	}

	if (WSAGetLastError() == WSAECONNRESET)
		cout << "The server has disconnected" << endl;

	return 0;
}

int main()
{
	WSAData wsa_data;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	string sent_message = "";
	client_type client = { INVALID_SOCKET, -1, "", false };
	
	int iResult = 0;
	string message;

	cout << "Starting Client...\n";

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (iResult != 0) {
		cout << "WSAStartup() failed with error: " << iResult << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	cout << "Connecting...\n";

	// Resolve the server address and port
	iResult = getaddrinfo((IP_ADDRESS), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo() failed with error: " << iResult << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		client.socket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (client.socket == INVALID_SOCKET) {
			cout << "socket() failed with error: " << WSAGetLastError() << endl;
			WSACleanup();
			system("pause");
			return 1;
		}

		// Connect to server.
		iResult = connect(client.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(client.socket);
			client.socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (client.socket == INVALID_SOCKET) {
		cout << "Unable to connect to server!" << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	cout << "Successfully Connected" << endl;

	//Obtain message announcing to register name from server for this client;
	recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
	message = client.received_message;

	

	if (message != "Server is full")
	{
		cout << message << endl;
		//client.id = atoi(client.received_message);

		//send username to server
		bool check_valid_name = false;
		do
		{
			getline(cin, sent_message);
			check_valid_name = checkValidName(sent_message);
			if (check_valid_name == false)
			{
				std::cout << "Ten dang ki khong hop le. Vui long dang ki lai.\n";
			}
		} while (check_valid_name == false);
		
		bool check_duplicate = false;
		memset(client.received_message, 0, DEFAULT_BUFLEN);//reset received message
		do
		{//send username to server and wait for checking at server
			send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);
			//
			
			recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
			
			if (strcmp(client.received_message, duplicate_msg) == 0)
			{
				check_duplicate = true;
				std::cout << duplicate_msg;
				getline(cin, sent_message);
				memset(client.received_message, 0, DEFAULT_BUFLEN);//reset received message
			}
			else
			{
				check_duplicate = false;
			}
		} while (check_duplicate == true);

		//receive success message
		std::cout << client.received_message << std::endl;
		
		//play game
		thread my_thread(process_client, client);

		while (1)
		{
			
			bool c;
			do
			{
				getline(cin, sent_message);// the format of sent_message is number-index_column(6-A, 1-B)
				c = checkSendMessage(sent_message);
				if (sent_message == "Ket thuc")
				{
					c = true;
				}
				if (c == false)
				{
					std::cout << "Dinh dang khong hop le.\n";
				}
			} while (c == false);
			iResult = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);

			if (iResult <= 0)
			{
				cout << "send() failed: " << WSAGetLastError() << endl;
				break;
			}
		}

		//Shutdown the connection since no more data will be sent
		my_thread.detach();
	}
	else
		cout << client.received_message << endl;

	cout << "Shutting down socket..." << endl;
	iResult = shutdown(client.socket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
		closesocket(client.socket);
		WSACleanup();
		system("pause");
		return 1;
	}

	closesocket(client.socket);
	WSACleanup();
	system("pause");
	return 0;
}