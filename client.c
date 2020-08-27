#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define NAME_SIZE 10
#define MAX_CLNT 2

char name[NAME_SIZE];

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* message);
int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t send_tid;
	pthread_t recv_tid;
	int str_len;
	void* t_stat;
	
	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		error_handling("socket() error");	
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
	
	printf("Type <!quit> to quit\n");
	//printf("send file : !filesend");

	printf("Enter username : ");
	fgets(name, NAME_SIZE, stdin); // 사용자 이름 입력
	name[strlen(name)-1] = 0;

	pthread_create(&send_tid, NULL, send_msg, (void*)&sock); // 메세지 전송에 대한 쓰레드 생성
	pthread_create(&recv_tid, NULL, recv_msg, (void*)&sock); // 메세지를 받기 위한 쓰레드 생성
	pthread_join(send_tid, &t_stat);
	pthread_join(recv_tid, &t_stat);

	close(sock);
	return 0;
}

void* send_msg(void* arg)
{
	char message[BUF_SIZE];
	char file_name[NAME_SIZE];
	char snd_message[NAME_SIZE+BUF_SIZE];
	int sock = *((int*)arg);
	int fd = 0;

	while(1) {
		memset(message, 0, sizeof(message));
		memset(snd_message, 0, sizeof(snd_message));
		
		fgets(message, BUF_SIZE, stdin); // 메세지 입력

		if(!strncmp(message, "!sendf", 6)) {
			fgets(file_name, NAME_SIZE, stdin);
			sprintf(snd_message, "!s[%s] sent you %s", name, file_name);
		}

		else if (!strcmp(message, "!quit\n")) { // 대화 종료 메세지
			write(sock, message, strlen(message)); // 대화 종료 메세지 서버로 전달
				memset(message, 0, sizeof(message));	
			read(sock, message, sizeof(message));
			fputs(message, stdout);
			exit(0);	
		}
		
		else {
			sprintf(snd_message, "[%s] %s", name, message); // 메세지 형식 지정: [유저이름] 메세지
		}

		write(sock, snd_message, strlen(snd_message)); // 메세지 서버로 전달
	}
	close(sock);
	return NULL;
}

void* recv_msg(void* arg)
{
	char message[BUF_SIZE];
	char file_name[NAME_SIZE];
	char file_msg[BUF_SIZE];
	int sock = *((int*)arg);
	FILE* fp;	
	int str_len = 0;

	while(1) {
		if ((str_len = read(sock, message, NAME_SIZE+BUF_SIZE-1)) == -1) { // 서버로부터 메세지 전달 받음
			close(sock);
			error_handling("error receiving message");
		}
		message[str_len] = 0;
		if (!strncmp(message, "!sf", 3)) {
		}
		fputs(message, stdout); // 다른 사용자가 보낸 메세지 출력
	}
	close(sock);
	return NULL;
}
void error_handling(char* message) 
{
	fputs(message, stderr);
	fputc('c', stderr);
	exit(1);
}
