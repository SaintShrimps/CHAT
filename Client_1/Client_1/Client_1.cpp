// чтобы <Windows.h> и  <WinSock2.h> не конфликтовали
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")//Нужно для доступа ко всем функциям
#pragma warning(disable: 4996) // что бы не вылетала ошибка 

#define IP_HOST_1 "127.0.0.1" // local host
#define HOST_PORT_1 15000 // port

int main()
{
	//Step 1: иницилизация WinSock
	WSADATA wsaData; //нужная структура
	if (WSAStartup(MAKEWORD(2, 1), &wsaData)) {
		printf("Failed with %d.\n", GetLastError());
		Sleep(5000);
		return 1;
	}

	//Step 2: Сокет для соединения с сервером
	SOCKET sock = socket(AF_INET, SOCK_STREAM, NULL);

	//Step 3: Информация об сокете 1 
	SOCKADDR_IN addr; //Храним адрес 
	addr.sin_addr.S_un.S_addr = inet_addr(IP_HOST_1);//хранит айпи
	addr.sin_port = htons(HOST_PORT_1); //Порт хоста
	addr.sin_family = AF_INET; // тип семейства интернет протоколов IPv4

	int sizeofaddr = sizeof(addr);//чтобы некоторые функции не ругались

	//Step 4: соединение
	if (connect(sock, (SOCKADDR*)&addr, sizeofaddr)) {
		printf("Failed connection to server with %d.\n",GetLastError());
		Sleep(5000);
		return FALSE;
	}
	printf("Connected");

	char buf[4096];
	std::string usInp;

	do
	{
		std::cout << "\n>";
		getline(std::cin, usInp);

		if (usInp.size() > 0)
		{
			int sendResult = send(sock, usInp.c_str(), usInp.size() + 1, NULL);
			
			if (sendResult != SOCKET_ERROR) {
				ZeroMemory(buf, 4096);
				int bytesRecv = recv(sock, buf, 4096, NULL);
				printf("\n no GOOD \r\n");
				if (bytesRecv > 0)
					std::cout << "SERVER>" << std::string(buf, 0, bytesRecv)<<std::endl;
					//printf("SERVER>\n %s", std::string(buf, 0, bytesRecv));
			}
			else
				printf("SOCKET ERROR with %d. \n", GetLastError());
		}

	} while (usInp.size() > 0);

	closesocket(sock);
	WSACleanup();
	printf("POKA\n");
	Sleep(5000);
}