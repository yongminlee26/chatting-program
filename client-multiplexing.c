// ��Ƽ�÷��� ��� ���� ���� ä�� ���α׷�
// Ŭ���̾�Ʈ ���� ���	: ����� ��ƾ�� ���ҵ� TCP ��Ƽ���μ��� Ŭ���̾�Ʈ
// �θ����μ��� : read ���
// �ڽ����μ��� : write ���

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
char sendingData[BUF_SIZE] = "["; // ������ ������ ������ = username + �Է��� ������

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
    
    // ������ ������ �޼���(sendingData)�� username ���� �߰�
    strcat(sendingData, argv[3]);  // sendingData = "[username"
    strcat(sendingData, "] ");   // sendingData = "[username] "

    //���ϻ���
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    // �����ּ������� ����ü�� �ʱ�ȭ
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    //�����û
    if (connect(sock, (struct sockaddr*) & serv_adr, sizeof(serv_adr)) == -1) // �������� ���� �����ϸ� connect()�� ��ȯ��
        error_handling("connect() error!");

    // ��Ƽ���μ������ ����·�ƾ ����
    pid = fork();
    if (pid == 0)   // �ڽ����μ��� : write ���
        write_routine(sock, buf);
    else            // �θ����μ��� : read ���
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
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")) { // Ŭ���̾�Ʈ ���� ���� (q, Q) �Է½�
            shutdown(sock, SHUT_WR); // half close : ��½�Ʈ�� close -> �������� ������ �����ʹ� ��� ���Ű���
            return;
        }
        int idx = strlen(sendingData);  // ���������� ��, �Էµ����͸� ���� ���� �Էµ����� ÷���� ���۵������� ��������
        strcat(sendingData, buf);       // ���۵�����(sendingData)�� �Էµ�����(buf) ÷�� -> sendingData = "[username]: buf"
        write(sock, sendingData, strlen(sendingData));  // ������ ������ ����
        sendingData[idx] = 0;   // ���۵����Ϳ��� �Էµ����� ���� -> sendingData = "[username] : "
    }
}

void read_routine(int sock, char* buf) {
    while (1) {
        // ������ ���Ź� ���
        int str_len = read(sock, buf, BUF_SIZE);    //����
        if (str_len == 0)
            return;
        buf[str_len] = 0;
        printf("%s", buf);  //���
    }
}