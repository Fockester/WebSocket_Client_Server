
#pragma region INCLUDES

#include<stdio.h>
#include<winsock2.h>
#include<time.h>

#pragma endregion 

#pragma comment(lib,"ws2_32.lib") 
#pragma warning(disable : 4996)

#pragma region GLOBAL_VARIABLES

const BUFFLENGTH = 1024;
char ipServer[25];
char location[50];
char fileName[25];
const IP_PORT = 68000;

#pragma endregion

//IP -> 192.168.1.127

int flag = 0;

SOCKET InitializeSocket();
SOCKET ConnectToServer(SOCKET socket);
void HandleMessages(SOCKET socketClient);
int handleOptions(SOCKET socketServer, char option);
void fileNameExits(SOCKET socketServer);

int main(int argc, char* argv[])
{
	SOCKET socketServer;	

	int size = 0;
	
	char msgMenu[1024] = "";
	char buffer[1024] = "";
	char* output = "";
	
	socketServer = InitializeSocket();
	socketServer = ConnectToServer(socketServer);

	HandleMessages(socketServer);

	if ((size = recv(socketServer, buffer, sizeof(buffer), 0)) == SOCKET_ERROR)
	{
		printf("\n\n\n Ocorreu um erro a receber o Welcome\n\n\n");
	}

	printf("\n\n\n");
	puts(buffer);

	// Close the socket
	closesocket(socketServer);

	//Cleanup winsock
	WSACleanup();

	return 0;
}

SOCKET InitializeSocket(){
	
	WSADATA wsa;
	SOCKET _socket;
	
	printf("\nA incializar a Winsock...");
	
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("\nFalhou. Codigo de Erro : %d\n", WSAGetLastError());
		return 1;
	}

	printf("\nWinsock inicializada.\n");

	//Create a socket
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	if (_socket == INVALID_SOCKET)
	{
		printf("\nNao foi possivel criar a socket : %d\n", WSAGetLastError());
		return 1;
	}

	printf("\nSocket creada.\n");
	
	return _socket;
}

SOCKET ConnectToServer(SOCKET socket){
	
	int ws_result = 0;
	struct sockaddr_in server;
	
	printf("\nIntroduza o endereco IP do servidor: ");
	scanf("%s", ipServer);

	printf("\nIntroduza a sua localizacao: ");
	scanf("%s", location);
	
	printf("\nIntroduza para o ficheiro: ");
	scanf("%s", fileName);

	// create the socket  address (ip address and port)
	server.sin_addr.s_addr = inet_addr(ipServer);
	server.sin_family = AF_INET;
	server.sin_port = htons(IP_PORT);

	//Connect to remote server
	ws_result = connect(socket, (struct sockaddr*)&server, sizeof(server));

	if (ws_result < 0)
	{
		puts("\nErro de connexao\n");
		return 1;
	}

	printf("\nConnectado a %s : %d\n", ipServer, IP_PORT);

	return socket;
}

void HandleMessages(SOCKET socketServer)
{
	char buffer[1024];
	char option[2] ="";
	int output = 0;
	int size = 0;

	ZeroMemory(buffer, 1024);

	//----------------------------------------------------------------------------Receber o Welcome
	if ((size = recv(socketServer, buffer, sizeof(buffer), 0)) == SOCKET_ERROR)
	{
		printf("\n\n\n Ocorreu um erro a receber o Welcome\n\n\n");
	}else {

		//Print welcome Recieved
		puts(buffer);
	
		//Clear Variable
		buffer[0] = '\0';
	}

	//----------------------------------------------------------------------------Enviar Localização
	if (send(socketServer, location, sizeof(location), 0) < 0)
	{
		printf("\n\nErro a enviar location\n\n");
	}

	//----------------------------------------------------------------------------Enviar nome file
	if (send(socketServer, fileName, sizeof(fileName), 0) < 0)
	{
		printf("\n\nErro a enviar location\n\n");
	}

	while (TRUE)
	{
		//----------------------------------------------------------------------------Receber Menu
		if ((size = recv(socketServer, buffer, BUFFLENGTH, 0)) == SOCKET_ERROR)
		{
			printf("\n\n\n Ocorreu um erro a receber o MENU\n\n\n");
		}

		//----------------------------------------------------------------------------Enviar Opcao Menu
		do{
			printf("\n\n");

			puts(buffer);

			scanf("%s", option);
			if (send(socketServer, option, sizeof(option), 0) < 0)
			{
				printf("\n\nErro a enviar opcao escolhida\n\n");
			}

			output = handleOptions(socketServer, toupper(option[0]));
		
		} while (output != -1);
		
		break;
	}
}

int handleOptions(SOCKET socketServer, char option)
{
	char buffer[1024];
	char choice[4] = "";
	char conf[10] = "Ok";
	char totalBoughtTickets[20] = "";
	char totalSuggTheaters[20] = "";

	int size = 0;
	int ID = 0;
	
	if (option == 'A')
	{
		do
		{
			//Recebe Lista de Teatros
			if ((size = recv(socketServer, buffer, sizeof(buffer), 0)) == SOCKET_ERROR)
			{
				printf("\n\nErro a receber a lista de teatros completa\n\n");
			}
			
			printf("\n---------Lista Teatros---------\n");

			puts(buffer);
			buffer[0] = '\0';

			printf("\n Escreva o ID do bilhete que pretende comprar ou 0 para mostrar mais: ");
			scanf("%s", choice);

			//Envia a resposta para o servidor 
			if (send(socketServer, choice, sizeof(choice), 0) < 0) {
				printf("\n\nOcorreu um erro a enviar a opcao..\n\n");
			}
		
		} while (choice[0] == '0');

		//Exit Route, the client as chosen a ticket
		if (choice[0] != '0') {
			
			//Recive from server 'thank you for buying'
			if ((size = recv(socketServer, buffer, sizeof(buffer), 0)) == SOCKET_ERROR)
			{
				printf("\n\nErro a receber despedida\n\n");
			}

			//Verificacao se o nome do ficheiro existe
			fileNameExits(socketServer);
			
			//Return value for exit
			return 0;
		}
	}
	else if(option == 'B')
	{
		do {
			//Recebe a lista de Teatros com base na localizacao dele
			if ((size = recv(socketServer, buffer, sizeof(buffer), 0)) == SOCKET_ERROR)
			{
				printf("\n\nErro a receber a lista de teatros\n\n");
			}
			
			puts(buffer);
			buffer[0] = '\0';

			printf("\n Escreva o ID do bilhete que pretende comprar ou 0 para mostrar mais: ");
			scanf("%s", choice);

			//Envia a resposta para o servidor 
			if (send(socketServer, choice, sizeof(choice), 0) < 0) {
				printf("\n\nOcorreu um erro a enviar a opcao escolhida..\n\n");
			}

		} while (choice[0] == '0'); 

		//Exit Route, the client as chosen a ticket
		if (choice[0] != '0') {
			
			//Recive from server 'thank you for buying'
			if ((size = recv(socketServer, buffer, sizeof(buffer), 0)) == SOCKET_ERROR)
			{
				printf("\n\nErro a receber despedida\n\n");
			}
			puts(buffer);

			//Verificacao do nome do ficheiro
			fileNameExits(socketServer);

			//Return value for exit
			return 0;
		}
	}
	else if (option == 'C') {
		printf("\n\n Os Teatros levaram Reset\n\n");
		return 0;
	}
	else if(option == 'D') {
		
		//Receber mensagem Numero de teatros apresentados
		if ((size = recv(socketServer, buffer, sizeof(buffer), 0)) == SOCKET_ERROR)
		{
			printf("\n\nErro a receber mensagem de teatros apresentados\n\n");
		}
		//Escrever o numero de teatros apresentado
		puts(buffer);
		
		//Enviar uma mensagem de confirmacao de recepcao
		//Para manter um fluxo de send seguido de um recv em ambos os lados
		if (send(socketServer, conf, sizeof(conf), 0) < 0) {
			printf("\n\nOcorreu um erro a enviar mensagem de controlo\n\n");
		}
		
		if ((size = recv(socketServer, totalSuggTheaters, sizeof(totalSuggTheaters), 0)) == SOCKET_ERROR)
		{
			printf("\n\nErro a receber mensagem com info do numero de teatros sugeridos\n\n");
		}

		puts(totalSuggTheaters);

		buffer[0] = '\0';

		if (send(socketServer, conf, sizeof(conf), 0) < 0) {
			printf("\n\nOcorreu um erro a enviar mensagem de controlo\n\n");
		}

		//Receber mensagem Numero de bilhetes comprados
		if ((size = recv(socketServer, buffer, sizeof(buffer), 0)) == SOCKET_ERROR)
		{
			printf("\n\nErro a receber despedida\n\n");
		}
		puts(buffer);
		
		if (send(socketServer, conf, sizeof(conf), 0) < 0) {
			printf("\n\nOcorreu um erro a enviar mensagem de controlo\n\n");
		}

		//Receber o numero total de bilhetes comprados
		if ((size = recv(socketServer, totalBoughtTickets, sizeof(totalBoughtTickets), 0)) == SOCKET_ERROR)
		{
			printf("\n\nErro a receber mensagem com info do numero de teatros sugeridos\n\n");
		}
		puts(totalBoughtTickets);

		//Return value for exit
		return 0;
	}
	else if(option == 'E') {
			//Return value for exit
			return -1;
	}
	else {
		printf("\n\nA opcao escolhida nao existe..\n");
		return 0;
	}
}

void fileNameExits(SOCKET socketServer)
{
	char recvConf[50] = "";
	char conf[50] = "Existe";
	int size = 0;
	
	//Se ele estiver a verificar sempre isto vai acontecer que o ficheiro existe sempre
	//Visto que esta funcao ]e chamada mais do que uma vez e eu nao consigo prever quando vai ser a primeira vez que ele a vai chamar
	if(flag == 0)
	{
		if ((size = recv(socketServer, recvConf, sizeof(recvConf), 0)) == SOCKET_ERROR)
		{
			printf("\n\nErro a receber se o ficheiro existe\n\n");
		}

		//Significa que existe
		if (strcmp(recvConf, conf) == 0)
		{
			printf("\n\nO nome de ficheiro que introduziu inicialmente ja existe introduza um novo: ");
			scanf("%s", fileName);

			if (send(socketServer, fileName, sizeof(fileName), 0) < 0) {
				printf("\n\nOcorreu um erro a enviar fileName\n\n");
			}
		}
		flag = 1;
	}
}