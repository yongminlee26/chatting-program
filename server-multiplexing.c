// ��Ƽ�÷��� ��� ���� ���� ä�� ���α׷�
// ���� ���� ���	: ��Ƽ�÷��� ��� ���� ���� ����

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#define BUF_SIZE 100

void error_handling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char* argv[]) {

	int serv_sock, clnt_sock;	// ��������fd, Ŭ���̾�Ʈ����fd
	struct sockaddr_in serv_adr, clnt_adr;	// ��������, Ŭ���̾�Ʈ����
	struct timeval timeout;	// select()�� timeout ����
	fd_set reads, cpy_reads; // fd���� �����ϴ� fd_set
	socklen_t adr_sz;
	int fd_max, str_len, fd_num; // fd���� �ִ�, ���ŵ����ͱ���, ��ȭ�� ���� fd ��
	char buf[BUF_SIZE];
	int i; // �ݺ������� ��ȭ�� ���� fd�� ����Ű�� ��Ȱ

	if (argc != 2) {
		printf("usage : %s <port>\n", argv[0]);
		exit(1);
	}

	// �������� ���� �� �ʱ�ȭ
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);	// TCP
	memset(&serv_adr, 0, sizeof(serv_adr));         // ����ü�� ��� ��� 0���� �ʱ�ȭ
	serv_adr.sin_family = AF_INET;                  // �ּ�ü������
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);   // ���ڿ� ����� ip�ּ� �ʱ�ȭ : INADDR_ANY =  ����������� ��ǻ���� ip�� ���Ͽ��ο�
	serv_adr.sin_port = htons(atoi(argv[1]));       // ���ڿ� ����� port��ȣ �ʱ�ȭ
	
	// �ּ��Ҵ�
	if (bind(serv_sock, (struct sockaddr*) & serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	// ������
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	FD_ZERO(&reads);			// fd_set �ʱ�ȭ
	FD_SET(serv_sock, &reads);	// �������� ��� : connection request�� �Դ��� ����
	fd_max = serv_sock;


	while (1) {
		// Ÿ�Ӿƿ� �ʱ�ȭ
		cpy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1) // �����߻�
			break;
		if (fd_num == 0) 	// timeout
			continue;
			
		for (i = 0; i < fd_max+1; i++) {

			if (FD_ISSET(i, &cpy_reads)) {	// fd(i)�� ��ȭ�� ������ 

				if (i == serv_sock) { // ������ ���Ͽ� ��ȭ�� �ִ� = connection request ����
					adr_sz = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (struct sockaddr*) & clnt_adr, &adr_sz); // �������
					FD_SET(clnt_sock, &reads);	// Ŭ���̾�Ʈ ���� ��� : �����Ͱ� ���ŵƴ��� ����
					if (fd_max < clnt_sock) 
						fd_max = clnt_sock;
					printf("connected client : %d \n", clnt_sock);
				}
				else {	// clinet socket�� ��ȭ�� �ִ� = ������ ���� -> read message
					str_len = read(i, buf, BUF_SIZE);

					if (str_len == 0) {
						FD_CLR(i, &reads); // �ش� fd ����
						close(i);
						printf("closed client : %d \n", i);
					}
					else {
						for(int j=4; j<fd_max+1; j++)	// fd 0,1,2�� stdin, stdout, stderr�̰� 3�� serv_sock�̹Ƿ� 4���� clnt_sock
							write(j, buf, str_len); // ��� Ŭ���̾�Ʈ���� �޼��� ����
					}
				}
			}
			
		}
	}

	close(serv_sock);
	return 0;
}
