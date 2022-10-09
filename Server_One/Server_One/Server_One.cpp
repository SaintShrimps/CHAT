// чтобы <Windows.h> и  <WinSock2.h> не конфликтовали
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")//Нужно для доступа ко всем функциям
#pragma warning(disable: 4996) // что бы не вылетала ошибка 

#define IP_HOST_1 "127.0.0.1" // local host
#define HOST_PORT_1 15000 // port


SOCKET Connection[100]; //Хранит подключенных клиентов
int CountUser = 0; //Считает клиентов

void sentColorAndKayb(std::string &strOut) {
	//Цвета
	std::stringstream ss;
	std::string line[] = {	"Color of window text : " , 
							"Color of desktop : ", 
							"Color of window : "};
	int aiElements[] = {	COLOR_WINDOWTEXT, //цвет текста
							COLOR_DESKTOP, //цвет декстопа
							COLOR_WINDOW //цвет бэкграунда 
	};
	DWORD Color1 = GetSysColor(aiElements[0]);
	DWORD Color2 = GetSysColor(aiElements[1]);
	DWORD Color3 = GetSysColor(aiElements[2]);
	DWORD nColors[] = { Color1, Color2, Color3 };  

	//Раскладка 
	HKL hk = GetKeyboardLayout(0);
	int lang = LOWORD(hk);

	char str1[128], str2[128], str3[128], buf[128];

	sprintf(str1, "%s RGB(0x%x, 0x%x, 0x%x) \n", line[0],GetRValue(nColors[0]),
		GetGValue(nColors[0]), GetBValue(nColors[0]));
	sprintf(str2, "%s RGB(0x%x, 0x%x, 0x%x) \n", line[1], GetRValue(nColors[1]),
		GetGValue(nColors[1]), GetBValue(nColors[1]));
	sprintf(str3, "%s RGB(0x%x, 0x%x, 0x%x) \n", line[2], GetRValue(nColors[2]),
		GetGValue(nColors[2]), GetBValue(nColors[2]));
	//sprintf(buf, "Keyboard layot code: %d\n", line[2], GetRValue(nColors[2]),
		//GetGValue(nColors[2]), GetBValue(nColors[2]));

	sprintf(buf, "SYS Color:\r\n%s\r\n%s\r\n%s\r\n\r\nKeyboard layot code: %d\r\n",
		str1,str2,str3,lang);
	printf(buf);

	ss << str1 << str2 << str3 << "Keyboard layot code:" << lang;
	strOut = ss.str();
	printf(strOut.c_str());
}


int main()
{
	//Step 0: Проверка на то, что такой сервер ОДИН
	HANDLE Mut = CreateMutex(NULL, FALSE, L"SERVER_1");
	DWORD result = WaitForSingleObject(Mut, NULL);
	if (result != WAIT_OBJECT_0) {
		printf("This server is already running.");
		Sleep(3000);
		return FALSE;
	}

	//Step 1: иницилизация WinSock
	WSADATA wsaData; //нужная структура
	if (WSAStartup(MAKEWORD(2, 1), &wsaData)){
		printf("Failed with %d.\n", GetLastError());
		return 1;
	}
	
	//Step 2: Информация об сокете
	SOCKADDR_IN addr; //Храним адрес 
	addr.sin_addr.S_un.S_addr = inet_addr(IP_HOST_1);//хранит айпи
	addr.sin_port = htons(HOST_PORT_1); //Порт хоста
	addr.sin_family = AF_INET; // тип семейства интернет протоколов IPv4

	int sizeofaddr = sizeof(addr);//чтобы некоторые функции не ругались

	//Step 3: Запуск прослушивания( создание сокета сокета)
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
																 
	//привязка адрес к сокету
	bind(sListen, (SOCKADDR*)&addr, sizeofaddr);

	//Step 4: Прослушивания порта в ожидание с соединением со стороны клиента
	if (listen(sListen, SOMAXCONN)) {
		printf("Error: listen with %d \n", GetLastError());
		system("pause");
		return 1;
	}
	printf("PRIVET\n");
	

	//************
	//Обозначаем, что сокет для прослушивания
	fd_set master; //структура винсокета, которая может пригадится, к примеру для select(синзр ввода/вывода)
	FD_ZERO(&master);

	FD_SET(sListen, &master);

	bool running = true;

	while (running)
	{	
		//Копируем "прослушивание", чтобы не было проблем, если мы захотим несколько клиентов
		fd_set copy = master;
		//"Смотрим кто с нами рагаваривает"
		int socketCount = select(NULL, &copy, nullptr, nullptr, nullptr);

		//Цикл для для соединений/для возможных соединений
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];//для этого и копировали
			if (sock == sListen)
			{
				SOCKET client = accept(sListen, nullptr, nullptr);//Принимаем новое соединени
				FD_SET(client, &master);// Добавляем новое соединение в лист соединеных слиентов

				std::string welMES = "Hello my dear friend";
				send(client, welMES.c_str(), welMES.size()+1, NULL);
			}
			else
			{
				char buf[32];
				ZeroMemory(buf, 32);

				int bytesIn = recv(sock, buf, 32, 0);
				if (bytesIn <= 0)
				{
					//Дропаем клиента
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else {
					std::string strOut;
					sentColorAndKayb(strOut);
					send(sock, strOut.c_str(), strOut.size() + 1, NULL);
				}
			}
		}

	}
	
	//Удаляем прослушивание 
	FD_CLR(sListen, &master);
	closesocket(sListen);

	std::string msg = "Server is shutting down. POKA \r\n";

	while (master.fd_count > 0)
	{
		//Перечисляем клиентов для закрытия
		SOCKET sock = master.fd_array[0];

		send(sock, msg.c_str(), msg.size() + 1, 0);

		// Удаляем клиентов и закрываем их
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	////Для удержки связи с клиентом 
	//SOCKET newConnection;
	//for (int i = 0; i < 100; i++)
	//{
	//	newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);//возвращает дискриптор сокета
	//	//Если сервер устанавливает связь с клиентом, то функция accept возвращает новый сокет-дескриптор, через который и происходит общение клиента с сервером.
	//	
	//	if (newConnection == 0)
	//		printf("Failed with %d.\n", GetLastError());
	//	else {
	//		printf("Client Connected.\n");
	//		Connection[i] = newConnection;//Записываем нового клиен та
	//		CountUser++;

	//		//CreateThread(NULL, NULL, ClientHandler, (LPVOID)i, NULL, NULL);
	//		sentColorAndKayb(i);
	//	}

	//}

	WSACleanup();//Очищаем сокет 
	printf("POKA\n");
	system("pause");
	//closesocket(sListen);
}
