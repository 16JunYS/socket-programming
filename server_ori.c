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

int clnt_socks[MAX_CLNT];
int clnt_cnt = 0;
pthread_mutex_t mutex;

void send_msg(char* msg, int len, int sock);
void* handle_clnt(void* arg);
void error_handling(char* message);
int main(int argc, char* argv[])
{
	int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	pthread_t t_id;

	socklen_t clnt_addr_size;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);	
		exit(1);
	}
	pthread_mutex_init(&mutex, NULL);

	if ((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) { 
		error_handling("socket() error");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) 
		error_handling("bind() error");

	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while(1) {
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		
		pthread_mutex_lock(&mutex);
		clnt_socks[clnt_cnt++] = clnt_sock; // 새로 연결된 클라이언트 관리
		pthread_mutex_unlock(&mutex);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock); //클라이언트 관리 쓰레드 생성
		pthread_detach(t_id);
	}
	close(serv_sock);
	return 0;
}

void* handle_clnt(void* arg)
{
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	char msg[NAME_SIZE+BUF_SIZE];

	while((str_len = read(clnt_sock, msg, NAME_SIZE+BUF_SIZE-1)) != 0) { //클라이언트로부터 메시지 읽음
		fputs(msg, stdout);	
		send_msg(msg, str_len, clnt_sock); //연결된 다른 클라이언트에게 전송
	}

	pthread_mutex_lock(&mutex);
	for(i=0; i<clnt_cnt; i++) {
		if (clnt_socks[i] == clnt_sock) {
			while(i<clnt_cnt-1) {
				clnt_socks[i] = clnt_socks[i+1];
				i++;
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutex);
	close(clnt_sock);

	return NULL;
}

void send_msg(char* msg, int len, int sock)
{
	int i;

	pthread_mutex_lock(&mutex); // 전역 변수 clnt_socks의 동기화
	for(i=0; i<clnt_cnt; i++) {
		if (clnt_socks[i] == sock) { // 메세지를 보낸 자기 자신 클라이언트는 건너뜀
			continue;
		}
		write(clnt_socks[i], msg, len); // 다른 클라이언트에게 메세지 전송
	}
	pthread_mutex_unlock(&mutex);
}

void error_handling(char* message) 
{
	fputs(message, stderr);
	fputc('c', stderr);
	exit(1);
}
