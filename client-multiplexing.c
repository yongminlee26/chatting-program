// 멀티플렉싱 기반 다중 접속 채팅 프로그램
// 클라이언트 구현 방법	: 입축력 루틴이 분할된 TCP 멀티프로세스 클라이언트
// 부모프로세스 : read 담당
// 자식프로세스 : write 담당

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 100

void error_handling(char* message);
void read_routine(int sock, char* buf);
void write_routine(int sock, char* buf);
char sendingData[BUF_SIZE] = "["; // 서버에 보내는 데이터 = username + 입력한 데이터

int main(int argc, char* argv[])
{
    int sock;
    pid_t pid;
    char buf[BUF_SIZE];
    struct sockaddr_in serv_adr;

    if (argc != 4) {
        printf("Usage : %s <IP> <port> <username>\n", argv[0]);
        exit(1);
    }
    
    // 서버에 보내는 메세지(sendingData)에 username 정보 추가
    strcat(sendingData, argv[3]);  // sendingData = "[username"
    strcat(sendingData, "] ");   // sendingData = "[username] "

    //소켓생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    // 서버주소정보를 구조체에 초기화
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    //연결요청
    if (connect(sock, (struct sockaddr*) & serv_adr, sizeof(serv_adr)) == -1) // 서버에서 연결 수락하면 connect()는 반환됨
        error_handling("connect() error!");

    // 멀티프로세스기반 입출력루틴 분할
    pid = fork();
    if (pid == 0)   // 자식프로세스 : write 담당
        write_routine(sock, buf);
    else            // 부모프로세스 : read 담당
        read_routine(sock, buf);

    close(sock);
    return 0;
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void write_routine(int sock, char* buf) {
    while (1) {
        fgets(buf, BUF_SIZE, stdin); 
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")) { // 클라이언트 종료 문자 (q, Q) 입력시
            shutdown(sock, SHUT_WR); // half close : 출력스트림 close -> 서버에서 보내는 데이터는 계속 수신가능
            return;
        }
        int idx = strlen(sendingData);  // 데이터전송 후, 입력데이터를 비우기 위해 입력데이터 첨부전 전송데이터의 길이저장
        strcat(sendingData, buf);       // 전송데이터(sendingData)에 입력데이터(buf) 첨부 -> sendingData = "[username]: buf"
        write(sock, sendingData, strlen(sendingData));  // 서버에 데이터 전송
        sendingData[idx] = 0;   // 전송데이터에서 입력데이터 비우기 -> sendingData = "[username] : "
    }
}

void read_routine(int sock, char* buf) {
    while (1) {
        // 데이터 수신및 출력
        int str_len = read(sock, buf, BUF_SIZE);    //수신
        if (str_len == 0)
            return;
        buf[str_len] = 0;
        printf("%s", buf);  //출력
    }
}