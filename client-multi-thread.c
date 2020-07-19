// Implementation of chatting program client using multi-thread

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);

char name[NAME_SIZE] = "\0";
char msg[BUF_SIZE];

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void* thread_return;

	if (argc != 4) {
		printf("Usage : %s <IP> <port> <username>\n", argv[0]);
		exit(1);
	}

	strcpy(name, argv[3]);
	sock = socket(PF_INET, SOCK_STREAM, 0); // make socket

    // initialize serv_addr
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

	// send client's name to server when connection is made
	write(sock, name, strlen(name));

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);  // thread1 - send msg
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);  // thread2 - receive msg
	pthread_join(snd_thread, &thread_return);   // wait thread1's job to be done
	pthread_join(rcv_thread, &thread_return);   // wait thread2's job to be done
	close(sock);
	return 0;
}

void* send_msg(void* arg)   // send thread main
{
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	while (1)
	{
		fgets(msg, BUF_SIZE, stdin); // "@toWhom messageData"
		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			close(sock);
			exit(0);
		}
		sprintf(name_msg, "%s %s", name, msg);  // name_msg = "name @toWhom messageData"
		write(sock, name_msg, strlen(name_msg)); // send name_msg		
	}
	return NULL;
}

void* recv_msg(void* arg)   // read thread main
{
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	int str_len;
	while (1)
	{
		str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1); // receive msg
		if (str_len == -1)
			return (void*)-1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);    // print msg
	}
	return NULL;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}