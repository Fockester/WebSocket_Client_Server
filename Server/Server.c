/*
Simple winsock Server
*/

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>

#define TRUE 1
#define DS_TEST_PORT 68000

#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable : 4996)

typedef struct LIST_NODE
{
	void* data;

	//Criar aqui o que me vai indicar se o teatro foi visto ou nao
	struct LIST_NODE* next;
}List_Node;

typedef List_Node* LIST;

typedef struct TEATRO
{
	char ID[10];
	char name[100];
	char city[100];
	char name_Play[100];
	char date[100];
	char numb_Seats[100];
	char price[20];

	int viewed;
}TEATRO;

#define DATA(node) ((node)->data)
#define NEXT(node) ((node)->next)

//Variaveis globais
char fileName[25];
char excelName[] = "ListaTeatros.csv";
int totalBoughtTickets = 0;
int totalsuggTheaters = 0;
int flag = 0;

//List handlers
List_Node* initList(LIST* list);
List_Node* newNode(void* data);
bool emptyList(List_Node* list);
List_Node* insertEnd(List_Node* list, void* data);
List_Node* getNewTeatro(List_Node* list, TEATRO* teatro);
void add_to_List(List_Node* list);
void printTEATRO(List_Node* teatro);
void deleteLastNode(List_Node* lista);
void printViewd(List_Node* list);

// function to handle the incoming connection
//	param: the socket of the calling client
DWORD WINAPI handleconnection(LPVOID lpParam);
SOCKET initializeSocket();
SOCKET listiningForConnection(SOCKET socket);
void dist_OfClient(SOCKET socket);
int sendData(SOCKET clientSocket, List_Node* list);
int displayOptions(SOCKET clientSocket, List_Node* list, const char* location);
int searchByLocation(SOCKET clientSocket, List_Node* list, const char* location);
void logInformation(SOCKET clientSocket, List_Node* list, char* ID, int mode);
void renewList(List_Node* list);
void infoTheatersSeen(SOCKET clientSocket);

int main()
{
	SOCKET socket;

	/*Inicialização das sockets*/
	socket = initializeSocket();
	socket = listiningForConnection(socket);

	//This fucntion will distribute the connections made to the server
	//and attribute them to a specific thread
	dist_OfClient(socket);

	// Close listening socket
	closesocket(socket);

	//Cleanup winsock
	WSACleanup();
}

#pragma region LIST_CODE
List_Node* initList(List_Node* list)
{
	if ((list = (List_Node*)malloc(sizeof(List_Node))) != NULL) {
		list->next = NULL;
		list->data = NULL;
	}
	else {
		printf("Erro a criar inicializar a LIST com os parametros a NULL\n\n");
	}
	return list;
}

List_Node* newNode(void* data)
{
	List_Node* newNode;

	if ((newNode = (List_Node*)malloc(sizeof(List_Node))) != NULL) {
		DATA(newNode) = data;
		NEXT(newNode) = NULL;
	}
	else {
		printf("Erro a criar um novo Nodulo\n\n");
	}
	return newNode;
}

bool emptyList(List_Node* list)
{
	if (list->next == NULL && list->data == NULL)
	{
		return true;
	}
	else {
		return false;
	}
}

List_Node* insertEnd(List_Node* list, void* data)
{
	List_Node* Node, * aux;
	aux = list;

	if ((Node = newNode(data)) != NULL)
	{
		if (emptyList(list) == true)
		{
			return list = Node;
		}
		else {
			while (NEXT(aux) != NULL)
			{
				aux = NEXT(aux);
			}
			NEXT(aux) = Node;

			return list;
		}
		return NULL;
	}
	else {
		printf("Ocorreu um erro a inserir a nova lista no final da lista anterior..\n\n");
		return NULL;
	}
}

List_Node* getNewTeatro(List_Node* list, TEATRO* teatro)
{
	int len = 0;
	int i = 0;
	int j = 0;

	FILE* listaTeatros;

	if ((listaTeatros = fopen(excelName, "rt")) != 0)
	{
		while (!feof(listaTeatros))
		{
			//Vamos alocar o espaco de memoria para uma estrutura do tipo TEATRO e guardar o inicio do end em teatro
			if ((teatro = (TEATRO*)malloc(sizeof(struct TEATRO))) == NULL)
			{
				printf("Erro alocar memoria em getNewTeatro..\n\n");
			}
			else {

				fscanf(listaTeatros, "%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%s", teatro->ID, teatro->name,
					teatro->city, teatro->name_Play, teatro->date, teatro->numb_Seats, teatro->price);

				len = strlen(teatro->ID);
				for (i = 0; i < len; i++)
				{
					if (teatro->ID[i] == '\n')
					{
						for (j = i; j < len; j++)
						{
							teatro->ID[j] = teatro->ID[j + 1];
						}
						len--;
						i--;
					}
				}
				teatro->viewed = 0;

				//Vamos agora inserir a lista no final, para as encadiarmos dinamicamente
				if ((list = insertEnd(list, teatro)) && (list->data == NULL))
				{
					printf("Ocorreu um erro a ligar as listas..\n");
				}
			}
		}
	}
	else {
		printf("Ocorreu um erro ao abrir o ficheiro..\n\n\n");
		return 0;
	}
	//Fechar a lista
	fclose(listaTeatros);
	return list;
}

//Apagar o último nodo da lista
void deleteLastNode(List_Node* lista)
{
	List_Node* aux, * aux2;
	int i = 0;
	aux = lista;
	aux2 = lista;

	if (emptyList(lista) == true)
	{
		printf("\nA lista esta vazia, logo nao a podes apagar\n");
	}

	while (NEXT(aux) != NULL) {
		aux = NEXT(aux);
		if (i >= 1) {
			aux2 = NEXT(aux2);
		}
		i++;
	}
	NEXT(aux2) = NULL;
}

void add_to_List(List_Node* list)
{
	List_Node* aux;

	aux = list;

	while (aux != NULL)
	{
		strcat(((TEATRO*)DATA(aux))->numb_Seats, " Lugares");
		strcat(((TEATRO*)DATA(aux))->price, " Euros");

		aux = NEXT(aux);
	}
}

void printTEATRO(List_Node* list)
{
	while (list != NULL)
	{
		strcat(((TEATRO*)DATA(list))->numb_Seats, " Lugares");
		strcat(((TEATRO*)DATA(list))->price, " Euros");

		printf("%s  %s  %s  %s  %s  %s  %s ",
			((TEATRO*)DATA(list))->ID, ((TEATRO*)DATA(list))->name, ((TEATRO*)DATA(list))->city, ((TEATRO*)DATA(list))->name_Play,
			((TEATRO*)DATA(list))->date, ((TEATRO*)DATA(list))->numb_Seats, ((TEATRO*)DATA(list))->price);

		list = NEXT(list);
	}
}

void printViewd(List_Node* list)
{
	while (list != NULL)
	{
		printf("\n%d", ((TEATRO*)DATA(list))->viewed);
		list = NEXT(list);
	}
}
#pragma endregion

SOCKET initializeSocket()
{
	// Initialise winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	printf("\nInicializacao da Winsock...");
	int wsResult = WSAStartup(ver, &wsData);
	if (wsResult != 0) {
		fprintf(stderr, "\nOcorreu um erro a dar SetUp da Winsocket! Error Code : %d\n", WSAGetLastError());
		return 1;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		fprintf(stderr, "\nErro a criar a Socket! Error Code : %d\n", WSAGetLastError());
		return 1;
	}

	printf("\nSocket criada.");

	return listening;
}

SOCKET listiningForConnection(SOCKET socket)
{
	// Bind the socket (ip address and port)
	struct sockaddr_in hint;

	hint.sin_family = AF_INET;
	hint.sin_port = htons(DS_TEST_PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(socket, (struct sockaddr*)&hint, sizeof(hint));
	printf("\nSocket binded.");

	// Setup the socket for listening
	listen(socket, SOMAXCONN);
	printf("\nServer listening.");

	return socket;
}

//This function as the purpose of accepting a request to connect to the server
//and it will distribute the connections made, to its own threads
void dist_OfClient(SOCKET socket)
{
	struct sockaddr_in client;
	int clientSize;
	int conresult = 0;

	SOCKET clientSocket;
	SOCKET* ptclientSocket;
	DWORD dwThreadId;
	HANDLE  hThread;

	while (TRUE)
	{
		clientSize = sizeof(client);
		clientSocket = accept(socket, (struct sockaddr*)&client, &clientSize);

		//Ele guarda a posiçao da memoria onde esta guardada a socket
		ptclientSocket = &clientSocket;

		printf("\nHandling de uma nova ligacao.");

		// Handle the communication with the client 
		hThread = CreateThread(
			NULL,					// default security attributes
			0,					   // use default stack size  
			handleconnection,	  // thread function name
			ptclientSocket,		 // argument to thread function 
			0,					// use default creation flags 
			&dwThreadId);	   // returns the thread identifier 

		// Check the return value for success.
		// If CreateThread fails, terminate execution. 
		if (hThread == NULL)
		{
			printf("\nErro a criar Thread.");
			ExitProcess(3);
		}
	}
	//Close the socket
	closesocket(clientSocket);
}

//Função responsável por dar handle da conexão clinte->servidor
//Will recieve as argument '&clientSocket'
DWORD WINAPI handleconnection(LPVOID lpParam)
{
	List_Node* listaTeatros = NULL;
	TEATRO* teatro = NULL;
	SOCKET clientSocket;
	SOCKET* ptCs;
	char strMsg[1024];
	char strRec[50];
	int i = 1;
	int error = 0;
	int aux = 0;


	ptCs = (SOCKET*)lpParam;
	clientSocket = *ptCs;

	//----------------------------------------------------------------------------Enviar Welcome
	strcpy(strMsg, "\nOla! Bem vindo ao servidor!...\n");
	send(clientSocket, strMsg, strlen(strMsg) + 1, 0);

	//----------------------------------------------------------------------------Receber localização
	int bytesReceived = recv(clientSocket, strRec, sizeof(strRec), 0);

	if (bytesReceived == SOCKET_ERROR) {
		printf("\nErro a receber localizacao!\n");

	}
	if (bytesReceived == 0) {
		printf("\nClient disconectado!\n");
	}

	//----------------------------------------------------------------------------Receber nome ficheiro
	if (bytesReceived = recv(clientSocket, fileName, sizeof(fileName), 0) == SOCKET_ERROR)
	{
		printf("\nErro a receber o nome do ficheiro do utilizador!\n");
	}

	strcat(fileName, ".txt");

	/*Manipulação das listas*/
	listaTeatros = initList(listaTeatros);
	listaTeatros = getNewTeatro(listaTeatros, teatro);
	deleteLastNode(listaTeatros);
	add_to_List(listaTeatros);

	strcpy(strMsg, " Escolha uma das seguintes opcoes:\nA- Lista de todos os Teatros\nB- Pesquisa por localizacao\nC- Renovar Lista\nD- Mostrar Informacao do numero de teatros vistos e comprados\nE- Sair\n");

	//----------------------------------------------------------------------------Enviar Menu
	if (send(clientSocket, strMsg, sizeof(strMsg), 0) < 0)
	{
		printf("\n\nOcorreu um erro a enviar a mensagem 'Menu'\n");
		return 1;
	}

	error = displayOptions(clientSocket, listaTeatros, strRec);

	strcpy(strMsg, "\n\nFoste Disconectado! Adeus!\n");
	send(clientSocket, strMsg, strlen(strMsg), 0);

	closesocket(clientSocket);
	return 0;
}

//It is going to display the MENU OPTIONS
//Return 1 for an ERROR
//Return 0 for no ERROR
//Return -1 for EXIT
int displayOptions(SOCKET clientSocket, List_Node* list, const char* location)
{
	char option[2] = "";
	char* aux = "";
	int size = 0;

	while (true)
	{
		//Retirei daqui o envio do menu, para poupar recursos. Assim ele só envia uma vez o menu

		//----------------------------------------------------------------------------Receber escolha
		if ((size = recv(clientSocket, option, sizeof(option), 0)) == SOCKET_ERROR)
		{
			printf("\n\nErro a receber resposta do utilizador sobre a escolha do menu\n");
			return 1;
		}

		//Options Menu
		switch (toupper(option[0]))
		{
			//Option in which the user wants to display the Theaters
		case 'A':
			sendData(clientSocket, list);
			break;

		case 'B':
			//Procurar por localizacao
			searchByLocation(clientSocket, list, location);
			break;

		case 'C':
			//Renovar lista
			renewList(list);
			break;

		case 'D':
			//Mostrar Info
			infoTheatersSeen(clientSocket);
			break;

			//Saida
		case 'E':
			//Vamos guardar todos os teatros vistos pelo utilizador
			logInformation(clientSocket, list, aux, 0);
			return 1;
			break;

		default:
			printf("\n\nIsso nao e uma opcao possivel..\n\n\n");
			break;
		}
	}
}
//Function responsible for sending the DATA from the FILE to the USER
int sendData(SOCKET clientSocket, List_Node* list)
{
	char buffer[1024] = "";
	char option[4] = "";
	char ID[10] = "";
	int size = 0;
	int i = 0;

	List_Node* aux;

	aux = list;

	do {

		option[0] = '\0';
		memset(buffer, 0, strlen(buffer));

		if (aux != NULL)
		{
			strcat(buffer, "\nID | Nome | Cidade | Nome da Peca | Data | Num. Lugares | Preco \n\n");
		}

		while (aux != NULL && i != 9)
		{
			if (((TEATRO*)aux->data)->viewed == 0)
			{
				//Putting all our data listed in the List to the Buffer to send
				strcat(buffer, ((TEATRO*)DATA(aux))->ID);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->name);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->city);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->name_Play);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->date);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->numb_Seats);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->price);
				strcat(buffer, "\n");

				((TEATRO*)DATA(aux))->viewed = 1;

				i++;
				totalsuggTheaters++;
			}
			aux = NEXT(aux);
		}

		if (i == 0)
		{
			strcpy(buffer, "\n\nNao existem mais teatros..\nVolta para o menu (enter E) e recarrega os teatros la\n ");

			if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
				printf("\n\nOcorreu a enviar a mensage\n\n");
			}

			if ((size = recv(clientSocket, option, sizeof(option), 0)) == SOCKET_ERROR) {
				printf("Ocorreu um erro a receber a opcao do utilizador..\n\n");
			}

			if (toupper(option[0]) == 'E')
			{
				strcpy(buffer, "\n\nVais agora voltar para o menu!\n ");

				if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
					printf("\n\nOcorreu a enviar a mensagem\n\n");
				}

				return 1;
			}
		}
		else {
			//Enviar Resultados dos Teatros encontrados
			if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
				printf("\n\nOcorreu um erro a enviar a lista de teatros\n\n");
			}
		}
		i = 0;

		if ((size = recv(clientSocket, option, sizeof(option), 0) == SOCKET_ERROR)) {
			printf("\n\nOcorreu um erro a receber o teatro escolhido pelo utilizador..\n\n");
		}

		if (option[0] == '0' && aux == NULL) {

			//Vamos renovar a lista, porque o utilizador deve ter passado o Teatro que pretendia sem querer
			renewList(list);
			
			//Voltar a colocar o aux na cabeça da lista
			aux = list;
		}

		//Para evitar que ele faca a verificacao sem estar a ser comprado um bilhete
		if (option[0] != '0')
		{
			aux = list;

			//Verificacao de que o utilizador nao esta a comprar um bilhete invalido
			while (aux != NULL)
			{
				//Encontrar o ticket que o utilizador pretende comprar
				if (strcmp(((TEATRO*)aux->data)->ID, option) == 0)
				{
					if (((TEATRO*)aux->data)->viewed == 1)
					{
						strcpy(buffer, "\n\nO teu bilhete foi comprado, obrigado pela tua compra!\n");

						if (send(clientSocket, buffer, sizeof(buffer), 0) < 0)
						{
							printf("\n\nOcorreu um erro a enviar a mensagem de compra\n");
						}

						logInformation(clientSocket, list, option, 1);
						return 1;
					}
					else {
						printf("\n\nO bilhete %s %s %s \nnao foi possivel ser comprado pelo utilizador com o nome de ficheiro: %s, because it is not an option\n\n",
							((TEATRO*)aux->data)->ID, ((TEATRO*)aux->data)->name, ((TEATRO*)aux->data)->name_Play, fileName);

						strcpy(buffer, "\n Desculpa mas esse bilhete nao existe ou nao esta a ser mostrado\n");
						if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
							printf("\n\nThere was an error sending our list info\n\n");
						}
						return 1;
					}
				}
				aux = NEXT(aux);
			}
		}
	} while (option[0] == '0' && aux != NULL);
}

int searchByLocation(SOCKET clientSocket, List_Node* list, const char* location)
{
	List_Node* aux;

	char buffer[1024] = "";
	char option[4] = "";
	int size = 0;
	int clientSize = 0;
	int i = 0;

	aux = list;

	//Contador para o número de resultados que vai enviar
	do
	{
		option[0] = '\0';

		if (aux != NULL)
		{
			strcat(buffer, "\nID | Nome | Cidade | Nome da Peca | Data | Num. Lugares | Preco \n\n");
		}

		//Ou seja o ciclo pode acabar por ter atingido o fim da lista
		//Contudo independentemente disso vai mandar 9 resultados, se os tiver,
		//caso n tenha vai chegar ao final da lista e devolver aquilo que tem
		while (aux != NULL && i != 9)
		{
			if (strcmp(((TEATRO*)aux->data)->city, location) == 0 && ((TEATRO*)aux->data)->viewed == 0)
			{
				//Putting all our data listed in the List to the Buffer to send
				strcat(buffer, ((TEATRO*)DATA(aux))->ID);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->name);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->city);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->name_Play);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->date);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->numb_Seats);
				strcat(buffer, " | ");
				strcat(buffer, ((TEATRO*)DATA(aux))->price);
				strcat(buffer, "\n");

				//Colocar os teatros enviados como lidos
				((TEATRO*)DATA(aux))->viewed = 1;

				i++;
				totalsuggTheaters++;
			}
			aux = NEXT(aux);
		}

		//Significa que nenhum teatro foi mostrado, ou seja o utilizador ja viu todos os resultados possiveis
		if (i == 0)
		{
			strcpy(buffer, "\n\nNao existem mais Teatros a serem mostrados com base na tua localizacao..\nVolta para o menu (E) e la podes dar reset aos teatros!\n Se esta mensagem apareceu primeiro do que o resultado de algum Teatro, nao existem Teatros na nossa base de dados para esta localizacao\n");

			if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
				printf("\n\nOcorreu um erro a mandar a mensagem de erro\n\n");
			}

			if ((size = recv(clientSocket, option, sizeof(option), 0)) == SOCKET_ERROR) {
				printf("Ocorreu um erro a receber a opcao do utilizador..\n\n");
			}

			if (toupper(option[0]) == 'E')
			{
				strcpy(buffer, "\n\nVais agora voltar para o menu!\n ");

				if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
					printf("\n\nOcorreu um erro a enviar a mensagem\n\n");
				}

				return 1;
			}
		}
		else {
			//Enviar Resultados dos Teatros encontrados
			if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
				printf("\n\nOcorreu um erro a enviar os resultados dos teatros encontrados\n\n");
			}
		}

		i = 0;
		memset(buffer, 0, strlen(buffer));

		//Receber se pretendeu comprar um Teatro ou se pretendeu dar print de mais
		if ((size = recv(clientSocket, option, sizeof(option), 0)) == SOCKET_ERROR) {
			printf("Ocorreu um erro a receber a opcao do utilizador..\n\n");
		}

		//Para evitar que ele faca a verificacao sem estar a ser comprado um bilhete
		if (strcmp(option, "0") != 0)
		{
			aux = list;
			//Verificacao de que o utilizador nao esta a comprar um bilhete invalido
			while (aux != NULL) {

				if (strcmp(((TEATRO*)aux->data)->ID, option) == 0)
				{
					if (strcmp(((TEATRO*)aux->data)->city, location) == 0)
					{
						strcpy(buffer, "\n\nO teu bilhete foi comprado com sucesso. Obrigado pela compra!\n");

						if (send(clientSocket, buffer, sizeof(buffer), 0) < 0)
						{
							printf("\n\nOcorreu um erro a enviar a mensagem para o utlizador\n");
						}

						logInformation(clientSocket, list, option, 1);
						return 1;
					}
					else {
						printf("\n\nO bilhete %s %s %s \nnao foi possivel de ser comprado pelo utilizador com o nome de ficheiro: %s, because it is not an option\n\n",
							((TEATRO*)aux->data)->ID, ((TEATRO*)aux->data)->name, ((TEATRO*)aux->data)->name_Play, fileName);

						strcpy(buffer, "\n Desculpa mas esse bilhete nao existe ou nao esta a ser mostrado\n");
						if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
							printf("\n\nOcorreu um erro a enviar a mensagem de erro\n\n");
						}
						return 1;
					}
				}
				aux = NEXT(aux);
			}
		}
	} while (option[0] == '0');
}

void logInformation(SOCKET clientSocket, List_Node* list, char* ID, int mode)
{
	List_Node* aux;

	FILE* logFile;
	FILE* test;

	char buffer[50] = "";
	aux = list;
	int size = 0;
	int allow = 0;

	//Isto so precisa de ser verificado uma vez
	if (flag == 0)
	{
		if ((test = fopen(fileName, "r")) != 0)
		{
			strcpy(buffer, "Existe");
			if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
				printf("\n\nOcorreu um erro a enviar a informacao\n\n");
			}

			if (recv(clientSocket, fileName, sizeof(fileName), 0) < 0) {
				printf("\n\nErro a receber o nome do ficheiro..\n\n");
			}

			strcat(fileName, ".txt");
			allow = 1;

			fclose(test);
		}

		if (allow == 0)
		{
			strcpy(buffer, "Nao Existe");
			if (send(clientSocket, buffer, sizeof(buffer), 0) < 0) {
				printf("\n\nOcorreu um erro a enviar a informacao\n\n");
			}
		}
		flag = 1;
	}
	//Giving it the name of the socketnumber of the client
	//So that you can save multiple files, with no worries
	//Opening the txt file in append mode
	if ((logFile = fopen(fileName, "a")) != 0)
	{
		if (mode == 1)
		{
			//Lets first find the ticket the user wants to buy
			while (strcmp(((TEATRO*)aux->data)->ID, ID) != 0)
			{
				aux = NEXT(aux);
			}

			//Saving on the text file the DATA
			fprintf(logFile, "-------Bilhete comprado pelo utilizador-------\n");
			fprintf(logFile, "%s | %s | %s | %s | %s | %s | %s \n",
				((TEATRO*)DATA(aux))->ID,
				((TEATRO*)DATA(aux))->name,
				((TEATRO*)DATA(aux))->city,
				((TEATRO*)DATA(aux))->name_Play,
				((TEATRO*)DATA(aux))->date,
				((TEATRO*)DATA(aux))->numb_Seats,
				((TEATRO*)DATA(aux))->price);
			fprintf(logFile, "\n-------------------------------------\n");

			printf("\nTicket bought! ");
			totalBoughtTickets++;
		}
		else if (mode == 0)
		{
			fprintf(logFile, "\n-------Teatros Vistos pelo utilizador-------\n");
			while (aux != NULL)
			{
				if (((TEATRO*)aux->data)->viewed == 1)
				{
					fprintf(logFile, "%s | %s | %s | %s | %s | %s | %s \n",
						((TEATRO*)DATA(aux))->ID,
						((TEATRO*)DATA(aux))->name,
						((TEATRO*)DATA(aux))->city,
						((TEATRO*)DATA(aux))->name_Play,
						((TEATRO*)DATA(aux))->date,
						((TEATRO*)DATA(aux))->numb_Seats,
						((TEATRO*)DATA(aux))->price);
				}
				aux = NEXT(aux);
			}
			fprintf(logFile, "\n-------------------------------------\n");

			printf("\n %s's guardado com sucesso! ", fileName);
		}

		fclose(logFile);
	}
	else {
		printf("\n\nErro abrir ficheiro para guardar a informacao dos teatros comprados e vistos pelo utilizador\n\n");
	}
}

void renewList(List_Node* list)
{
	List_Node* aux;

	aux = list;

	while (aux != NULL)
	{
		((TEATRO*)DATA(aux))->viewed = 0;
		aux = NEXT(aux);
	}

	printf("\nLista renovada com sucesso para o utilizador com o nome de ficheiro: %s", fileName);
}

void infoTheatersSeen(SOCKET clientSocket)
{
	char buffer[1024] = "";
	char recvMsg[10] = "";
	char tst[20] = "";
	int size = 0;

	strcpy(buffer, "\nNumero de Teatros apresentados: ");
	if (send(clientSocket, buffer, sizeof(buffer), 0) < 0)
	{
		printf("Ocorreu um erro a enviar mensagem Numero de Teatros Apresentados..\n");
	}

	//Tem o unico proposito de manter um fluxo de comunicacao
	if ((size = recv(clientSocket, recvMsg, sizeof(recvMsg), 0)) == SOCKET_ERROR)
	{
		printf("Ocurreu um erro a receber ok message..\n");
	}

	//Vai guardar dentro da nossa string o valor presente em totalsuggTheaters
	sprintf(tst, "%d", totalsuggTheaters);

	if (send(clientSocket, tst, sizeof(int), 0) < 0)
	{
		printf("Ocorreu um erro a enviar..\n");
	}

	if ((size = recv(clientSocket, recvMsg, sizeof(recvMsg), 0)) == SOCKET_ERROR)
	{
		printf("Ocurreu um erro a receber ok message..\n");
	}

	tst[0] = '\0';

	strcpy(buffer, "\nNumero de bilhetes comprados: ");
	if (send(clientSocket, buffer, sizeof(buffer), 0) < 0)
	{
		printf("Ocorreu um erro a enviar mensagem Numero de bilhetes comprado..\n");
	}

	if ((size = recv(clientSocket, recvMsg, sizeof(recvMsg), 0)) == SOCKET_ERROR)
	{
		printf("Ocurreu um erro a receber ok message..\n");
	}

	//Vai guardar dentro da nossa string o valor presente em totalBoughtTickets
	sprintf(tst, "%d", totalBoughtTickets);

	if (send(clientSocket, tst, sizeof(int), 0) < 0)
	{
		printf("Ocorreu um erro a enviar o numero de bilhetes comprado..\n");
	}
}