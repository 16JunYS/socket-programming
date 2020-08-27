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
	
	printf("Connected.....\n");
	printf("how to quit : !quit\n");
	//printf("send file : !filesend");

	printf("Enter username : ");
	fgets(name, NAME_SIZE, stdin);
	name[strlen(name)-1] = 0;

	pthread_create(&send_tid, NULL, send_msg, (void*)&sock);
	pthread_create(&recv_tid, NULL, recv_msg, (void*)&sock);
	pthread_join(send_tid, &t_stat);
	pthread_join(recv_tid, &t_stat);

	close(sock);
	return 0;
}

void* send_msg(void* arg)
{
	char message[BUF_SIZE];
	char snd_message[NAME_SIZE+BUF_SIZE];
	int sock = *((int*)arg);

	while(1) {
		memset(message, 0, sizeof(message));
		memset(snd_message, 0, sizeof(snd_message));
		
		fgets(message, BUF_SIZE, stdin);
		if(!strcmp(message, "!quit\n"))
			break;
	
		sprintf(snd_message, "[%s] %s", name, message);
		write(sock, snd_message, strlen(snd_message));
	}
	close(sock);
	return NULL;
}

void* recv_msg(void* arg)
{
	char message[BUF_SIZE];
	int sock = *((int*)arg);
	int str_len = 0;
	
	while(1) {
		if ((str_len = read(sock, message, NAME_SIZE+BUF_SIZE-1)) == -1) {
			close(sock);
			error_handling("error receiving message");
		}
		message[str_len] = 0;
		fputs(message, stdout);
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
