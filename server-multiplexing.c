// 멀티플렉싱 기반 다중 접속 채팅 프로그램
// 서버 구현 방법	: 멀티플렉싱 기반 다중 접속 서버

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

	int serv_sock, clnt_sock;	// 서버소켓fd, 클라이언트소켓fd
	struct sockaddr_in serv_adr, clnt_adr;	// 서버소켓, 클라이언트소켓
	struct timeval timeout;	// select()의 timeout 설정
	fd_set reads, cpy_reads; // fd들을 저장하는 fd_set
	socklen_t adr_sz;
	int fd_max, str_len, fd_num; // fd들중 최댓값, 수신데이터길이, 변화가 생긴 fd 값
	char buf[BUF_SIZE];
	int i; // 반복문에서 변화가 생긴 fd를 가리키는 역활

	if (argc != 2) {
		printf("usage : %s <port>\n", argv[0]);
		exit(1);
	}

	// 서버소켓 생성 및 초기화
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);	// TCP
	memset(&serv_adr, 0, sizeof(serv_adr));         // 구조체의 모든 멤버 0으로 초기화
	serv_adr.sin_family = AF_INET;                  // 주소체계지정
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);   // 문자열 기반의 ip주소 초기화 : INADDR_ANY =  현재실행중인 컴퓨터의 ip를 소켓에부여
	serv_adr.sin_port = htons(atoi(argv[1]));       // 문자열 기반의 port번호 초기화
	
	// 주소할당
	if (bind(serv_sock, (struct sockaddr*) & serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	// 연결대기
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	FD_ZERO(&reads);			// fd_set 초기화
	FD_SET(serv_sock, &reads);	// 서버소켓 등록 : connection request가 왔는지 관찰
	fd_max = serv_sock;


	while (1) {
		// 타임아웃 초기화
		cpy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1) // 오류발생
			break;
		if (fd_num == 0) 	// timeout
			continue;
			
		for (i = 0; i < fd_max+1; i++) {

			if (FD_ISSET(i, &cpy_reads)) {	// fd(i)에 변화가 있으면 

				if (i == serv_sock) { // 리스닝 소켓에 변화가 있다 = connection request 수신
					adr_sz = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (struct sockaddr*) & clnt_adr, &adr_sz); // 연결수락
					FD_SET(clnt_sock, &reads);	// 클라이언트 소켓 등록 : 데이터가 수신됐는지 관찰
					if (fd_max < clnt_sock) 
						fd_max = clnt_sock;
					printf("connected client : %d \n", clnt_sock);
				}
				else {	// clinet socket에 변화가 있다 = 데이터 수신 -> read message
					str_len = read(i, buf, BUF_SIZE);

					if (str_len == 0) {
						FD_CLR(i, &reads); // 해당 fd 삭제
						close(i);
						printf("closed client : %d \n", i);
					}
					else {
						for(int j=4; j<fd_max+1; j++)	// fd 0,1,2는 stdin, stdout, stderr이고 3는 serv_sock이므로 4부터 clnt_sock
							write(j, buf, str_len); // 모든 클라이언트에게 메세지 전달
					}
				}
			}
			
		}
	}

	close(serv_sock);
	return 0;
}
