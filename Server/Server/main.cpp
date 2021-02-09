#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <functional>


#pragma comment (lib, "Ws2_32.lib")

#define IP_ADDRESS "192.168.56.1"
#define DEFAULT_PORT "3504"
#define DEFAULT_BUFLEN 512

struct client_type
{
	int id;
	SOCKET socket;
	std::string username;
	int score;//score of a client
	bool is_solved;// check if client solved the problem or not
};

const char OPTION_VALUE = 1;
const int MAX_CLIENTS = 3;
bool END_GAME = false;
//--------------------FUNCTION PROTOTYPES-----------------------------------
int process_client(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread,
	std::vector<int> A, std::vector<int> B, std::vector<int> C, int num_dish);
bool checkDuplicateName(std::vector<client_type> v, std::string name, int id);
int generateDish(std::vector<int>& A, std::vector<int>&  B, std::vector<int>&  C, int& num_dish);
std::string convertMsg(std::vector<int> A, std::vector<int>  B, std::vector<int>  C);
//------------FUNCTION IMPLEMENTATION------------------------
int generateDish(std::vector<int>& A, std::vector<int>&  B, std::vector<int>&  C, int& num_dish)
{
	std::srand(time(0));
	num_dish;//the number of dishes, from 3 to 6
	num_dish = rand() % 4 + 3;//random the number of dishes
	std::cout << "So luong dia la: " << num_dish << std::endl;
	int numA, numB, numC;
	int remain;
	//random the number of dishes of each column(A,B,C)
	numA = rand() % num_dish + 1;
	remain = num_dish - numA;
	
	if (remain != 0)
		numB = rand() % (remain + 1);
	else
		numB = 0;
	numC = remain - numB;
	

	//
	bool dish[7] = { false };//check if a dish is in use or not
	int tmp;
	for (int i = 0; i < numA; i++)
	{
		tmp = rand() % num_dish + 1;
		if (dish[tmp] == false)//is not visited
		{
			A.push_back(tmp);
			dish[tmp] = true;//set dish[tmp] = true, visited
		}
		else
			i--;
		
	}
	for (int i = 0; i < numB; i++)
	{
		tmp = rand() % num_dish + 1;
		if (dish[tmp] == false)//is not visited
		{
			B.push_back(tmp);
			dish[tmp] = true;//set dish[tmp] = true, visited
		}
		else
			i--;
	}
	for (int i = 0; i < numC; i++)
	{
		tmp = rand() % num_dish + 1;
		if (dish[tmp] == false)//is not visited
		{
			C.push_back(tmp);
			dish[tmp] = true;//set dish[tmp] = true, visited
		}
		else
			i--;
	}
	
	std::sort(A.begin(), A.end(), std::greater<int>());
	std::sort(B.begin(), B.end(), std::greater<int>());
	std::sort(C.begin(), C.end(), std::greater<int>());
	return 0;
}
std::string convertMsg(std::vector<int> A, std::vector<int>  B, std::vector<int>  C)// to convert infomation about columns and dishes
{
	std::string msg = "Thap Ha Noi co hinh dang: \n";
	msg += "A: ";
	for (int i = 0; i < A.size(); i++)
	{
		msg += std::to_string(A[i]) + " ";
	}

	msg += "\nB: ";
	for (int i = 0; i < B.size(); i++)
	{
		msg += std::to_string(B[i]) + " ";
	}
	msg += "\nC: ";
	for (int i = 0; i < C.size(); i++)
	{
		msg += std::to_string(C[i]) + " ";
	}
	
	return msg;
}
bool checkDuplicateName(std::vector<client_type> v, std::string name, int id)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (i != id && v[i].socket != INVALID_SOCKET)
		{
			//compare name
			if (name == v[i].username)
			{
				return false;// duplicated
			}
		}
	}
	return true;
}
int process_client(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread,
	std::vector<int> A, std::vector<int> B, std::vector<int> C, int num_dish)
{
	std::string msg = "";
	char tempmsg[DEFAULT_BUFLEN] = "";
	

	//Session
	while (END_GAME == false)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);

		if (new_client.socket != 0)
		{
			int iResult = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);
			//variable for check for each client's turn
			bool is_above = false;
			bool can_move = false;
			
			
			if (iResult != SOCKET_ERROR)
			{
				// demand to end game
				if (strcmp("Ket thuc", tempmsg) == 0)
				{
					//set end game, break while loop
					END_GAME = true;
				}
				if (strcmp("", tempmsg))
				{
					msg = "Client " + new_client.username + ": " + tempmsg + "\n";
					//tempmsg currently contains information about client's turn.
					int dish = int(tempmsg[0]) - 48;
					int column = toupper(tempmsg[2]) - 65;//convert to uppercase and subtract to 65
					int old_column;
					// check if the dish which is moved is above all the others or lying among them.	
					if (A.size() != 0 && A[A.size() - 1] == dish)
					{
						old_column = 0;
						is_above = true;
					}
					if (B.size() != 0 && B[B.size() - 1] == dish)
					{
						old_column = 1;
						is_above = true;
					}
					if (C.size() != 0 && C[C.size() - 1] == dish)
					{
						old_column = 2;
						is_above = true;
					}
					
					if (is_above)
					{
						if (column == 0)//A
						{
							if (A.size() == 0 || A[A.size() - 1] >= dish)//check size of current dish and next dish
							{
								if (old_column == 0)
								{
									A.pop_back();
								}
								if (old_column == 1)
								{
									B.pop_back();
								}
								if (old_column == 2)
								{
									C.pop_back();
								}
								can_move = true;
								A.push_back(dish);// move to A
							}
						}
						else if (column == 1)//B
						{
							if (B.size() == 0 || B[B.size() - 1] >= dish)//check size of current dish and next dish
							{
								if (old_column == 0)
								{
									A.pop_back();
								}
								if (old_column == 1)
								{
									B.pop_back();
								}
								if (old_column == 2)
								{
									C.pop_back();
								}
								can_move = true;
								B.push_back(dish);// move to B
							}
						}
						else//if column == 'C'
						{
							if (C.size() == 0 || C[C.size() - 1] >= dish)//check size of current dish and next dish
							{
								//erase from old column
								if (old_column == 0)
								{
									A.pop_back();
								}
								if (old_column == 1)
								{
									B.pop_back();
								}
								if (old_column == 2)
								{
									C.pop_back();
								}
								can_move = true;
								C.push_back(dish);// move to C
							}
						}
					}
				}
				if (can_move == false)
				{
					msg += "Dia vua chon khong hop le. Vui long chon lai.\n";
				}
				else
					client_array[new_client.id].score++;
				msg += convertMsg(A, B, C);
				std::cout << msg.c_str() << std::endl;
				if (C.size() == num_dish)
				{
					//set this client is solved Hanoi Tower
					client_array[new_client.id].is_solved = true;
					msg += new_client.username + " Da giai duoc thap ha noi\n";
					bool is_end_game = true;
					for (int i = 0; i < MAX_CLIENTS; i++)
					{
						if (client_array[i].socket != INVALID_SOCKET)
							if (client_array[i].is_solved == false)// remain a client that have not solved the problem
								is_end_game = false;
					}
					if (is_end_game == true)
					{
						//set end game, break while loop
						END_GAME = true;
					}
				}
				//send the form of Hanoi Tower to client
				if (client_array[new_client.id].socket != INVALID_SOCKET)
						iResult = send(client_array[new_client.id].socket, msg.c_str(), strlen(msg.c_str()), 0);
			}
			else
			{
				msg = "Client #" + new_client.username + " Disconnected";

				std::cout << msg << std::endl;

				closesocket(new_client.socket);
				closesocket(client_array[new_client.id].socket);
				client_array[new_client.id].socket = INVALID_SOCKET;

				//Broadcast the disconnection message to the other clients
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_array[i].socket != INVALID_SOCKET)
						iResult = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
				}

				break;
			}
		}
		if (END_GAME)
		{
			// inform that client is the winner and share it with the others.
			std::string win_msg = "Tro choi ket thuc.\n";
			int min = 1e9;
			int index = 0;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_array[i].socket != INVALID_SOCKET)
				{
					win_msg += "Player " + client_array[i].username + " Score: " +
						std::to_string(client_array[i].score) + "\n";
					if (min > client_array[i].score)
					{
						min = client_array[i].score;
						index = i;
					}
				}
			}
			win_msg += "Player " + client_array[index].username + " Score: " +
				std::to_string(client_array[index].score) + " la nguoi chien thang\n";

			std::cout << win_msg << std::endl;
			//Broadcast the disconnection message to the other clients
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_array[i].socket != INVALID_SOCKET)
					send(client_array[i].socket, win_msg.c_str(), strlen(win_msg.c_str()), 0);
			}
			//close all the connects
			closesocket(new_client.socket);
			//Close all clients.
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				
				closesocket(client_array[i].socket);

				client_array[i].socket = INVALID_SOCKET;
			}
		}
	} //end while

	thread.detach();

	return 0;
}

int main()
{
	WSADATA wsaData;
	struct addrinfo hints;
	struct addrinfo *server = NULL;
	SOCKET server_socket = INVALID_SOCKET;
	std::string msg = "";
	std::vector<client_type> client(MAX_CLIENTS);
	int num_clients = 0;
	int temp_id = -1;
	std::thread my_thread[MAX_CLIENTS];

	//Initialize Winsock
	std::cout << "Intializing Winsock..." << std::endl;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//Setup hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//Setup Server
	std::cout << "Setting up server..." << std::endl;
	getaddrinfo((IP_ADDRESS), DEFAULT_PORT, &hints, &server);

	//Create a listening socket for connecting to server
	std::cout << "Creating server socket..." << std::endl;
	server_socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

	//Setup socket options
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int)); //Make it possible to re-bind to a port that was used within the last 2 minutes
	setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //Used for interactive programs

																					 //Assign an address to the server socket.
	std::cout << "Binding socket..." << std::endl;
	bind(server_socket, server->ai_addr, (int)server->ai_addrlen);

	//Listen for incoming connections.
	std::cout << "Listening..." << std::endl;
	listen(server_socket, SOMAXCONN);

	//Initialize the client list
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		client[i] = { -1, INVALID_SOCKET };
	}

	while (1)
	{

		SOCKET incoming = INVALID_SOCKET;
		incoming = accept(server_socket, NULL, NULL);

		if (incoming == INVALID_SOCKET) continue;

		//Reset the number of clients
		num_clients = -1;

		//Create a temporary id for the next client
		temp_id = -1;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (client[i].socket == INVALID_SOCKET && temp_id == -1)
			{
				client[i].socket = incoming;
				client[i].id = i;
				temp_id = i;
			}

			if (client[i].socket != INVALID_SOCKET)
				num_clients++;

			//std::cout << client[i].socket << std::endl;
		}

		if (temp_id != -1)
		{
			
			//Send the notification to register username  to that client
			msg = "Nhap vao ten dang ki: ";
			//msg = std::to_string(client[temp_id].id);
			send(client[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);
			bool check_duplicate = false;
			do
			{
				//receive username of client
				char* tempmsg = new char[DEFAULT_BUFLEN];
				memset(tempmsg, 0, DEFAULT_BUFLEN);
				recv(client[temp_id].socket, tempmsg, DEFAULT_BUFLEN, 0);

				std::string tmp_name(tempmsg);
				check_duplicate = checkDuplicateName(client, tempmsg, client[temp_id].id);
				if (check_duplicate == false)//duplicated
				{
					std::string duplicate_msg = "Ten da bi trung. Vui long dang ki lai.\n";
					send(client[temp_id].socket, duplicate_msg.c_str(), strlen(duplicate_msg.c_str()), 0);
				}
				//copy name to client's name
				client[temp_id].username.assign(tempmsg);
				client[temp_id].score = 0;//set score = 0
				client[temp_id].is_solved = false;//set is_solved = false
			} while (check_duplicate == false);

			std::cout << "Client #" << client[temp_id].id << ": Username: " << client[temp_id].username
				<< " Accepted" << std::endl;
			
			std::cout << "So luong slot con trong la: " << 3 - num_clients - 1 << std::endl;
			
			if (3 - num_clients - 1 == 0)
			{
				//Initialize Game
				std::vector<int> A;//contains dishes of column A
				std::vector<int> B;//contains dishes of column B
				std::vector<int> C;//contains dishes of column C
				int num_dish;
				generateDish(A, B, C, num_dish);
				std::string tower_info = convertMsg(A, B, C);
				//Create a thread process for client connected
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					std::string succeed_msg = "Dang ki thanh cong.";
					send(client[i].socket, succeed_msg.c_str(), succeed_msg.length(), 0);
					//send information about Hanoi Tower
					send(client[i].socket, tower_info.c_str(), tower_info.length(), 0);

					my_thread[i] = std::thread(process_client, std::ref(client[i]),
						std::ref(client), std::ref(my_thread[i]), (A), (B), (C), (num_dish));
				}
			}
		}
		else//full server, start to announce to clients that the game is started.
		{
			msg = "Server is full";
			send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
			std::cout << msg << std::endl;
		}
	} //end while


	  //Close listening socket
	closesocket(server_socket);

	//Close client socket
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		my_thread[i].detach();
		closesocket(client[i].socket);
	}

	//Clean up Winsock
	WSACleanup();
	std::cout << "Program has ended successfully" << std::endl;

	system("pause");
	return 0;
}