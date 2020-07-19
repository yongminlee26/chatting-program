// Implementation of chatting program server using multi-thread

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256
#define NAME_SIZE 20

void* handle_clnt(void* arg);
void send_msg(char* msg, int len);
void error_handling(char* msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
char clnt_names[MAX_CLNT][NAME_SIZE]; 

pthread_mutex_t mutx;

int main(int argc, char* argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);	// create mutex
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*) & serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while (1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*) & clnt_adr, &clnt_adr_sz);	
		
		// client sends its name when connection is made
		char clnt_name[NAME_SIZE] = "\0";
		int name_len = 0;
		name_len = read(clnt_sock, clnt_name, sizeof(clnt_name));
		printf("User @%s has entered the chat\n", clnt_name);

		pthread_mutex_lock(&mutx);
		// critical section
		strcpy(clnt_names[clnt_cnt], clnt_name); // register client's name in name array
		clnt_socks[clnt_cnt++] = clnt_sock; // register client socket fd in client socket array
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);	// thread - send and reveive data with client
		pthread_detach(t_id);	// non-block waiting thread's job to be done 
	}
	close(serv_sock);
	return 0;
}

void* handle_clnt(void* arg)
{
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	char dataFromClnt[BUF_SIZE + NAME_SIZE] = "\0";

	while ((str_len = read(clnt_sock, dataFromClnt, sizeof(dataFromClnt))) != 0) {
		char fromWho[NAME_SIZE] = "\0";
		char toWho[NAME_SIZE] = "\0";
		char message[BUF_SIZE] = "\0";
		
		// seperate dataFromClnt into fromWho, toWho, message
		char* str = strtok(dataFromClnt, " ");

		for (int k = 0; k < 3; k++) {

			if (k == 0) { // get fromWho information 
				strcpy(fromWho, str);
				str = strtok(NULL, " ");
			}
			else if (k == 1) { // get toWho information
				strcpy(toWho, str);
				// remove '@' in toWho
				int toWhom_len = strlen(toWho);
				for (int j = 0; j < toWhom_len-1; j++) {
					toWho[j] = toWho[j + 1];
				}
				toWho[toWhom_len - 1] = '\0';
				str = strtok(NULL, " ");
			}
			else { // get message information
				strcpy(message, str);	
				while (str != NULL) {
					str = strtok(NULL, " ");
					if(str!=NULL)
						sprintf(message, "%s %s", message, str);
				}
			}	
		}
		
		// send the message to the appropriate receiver
		if (!strcmp(toWho, "all")) {
			char send_data[BUF_SIZE + NAME_SIZE] =  "\0";
			char name[NAME_SIZE] = "\0";
			sprintf(name, "[%s]", fromWho);
			sprintf(send_data, "%s %s", name, message);
			send_msg(send_data, strlen(send_data)); // send message to all the clients
		}
		else {
			int k = 0;
			for (k = 0; k < MAX_CLNT; k++) { // find the appropriate client as a receiver
				if (!strcmp(toWho, clnt_names[k])) {
					char send_data[BUF_SIZE + NAME_SIZE] = "\0";
					char name[NAME_SIZE] = "\0";
					sprintf(name, "[%s]", fromWho);
					sprintf(send_data, "%s %s", name, message);
					write(clnt_socks[k], send_data, strlen(send_data)); // send message
					break;
				}
			}
			if (k == MAX_CLNT) { // if 'toWho' is not registered in server
				for (int j = 0; j < MAX_CLNT; j++) {
					if (!strcmp(fromWho, clnt_names[j])) {
						char send_data[BUF_SIZE + NAME_SIZE] = "\0";
						strcpy(send_data, "Target User not found\n\0");
						write(clnt_socks[j], send_data, strlen(send_data));
						break;
					}
				}
			}	
		}

		// clear dataFromClnt array
		for (int idx = 0; idx < BUF_SIZE + NAME_SIZE; idx++) {
			dataFromClnt[idx] = '\0';
		}
	}
	
	pthread_mutex_lock(&mutx);
	// critical section
	for (i = 0; i < MAX_CLNT; i++)   // remove disconnected client
	{	
		if (clnt_sock == clnt_socks[i])
		{
			printf("User @%s has left the chat\n" ,clnt_names[i]);
			while (i < MAX_CLNT - 1) {
				clnt_socks[i] = clnt_socks[i + 1];
				strcpy(clnt_names[i], clnt_names[i + 1]);
				i++;
			}
			break;
		}
	}

	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}
void send_msg(char* msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	// critical section
	for (i = 0; i < clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
