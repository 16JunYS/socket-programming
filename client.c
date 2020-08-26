#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define NAME_SIZE 1024
#define MAX_CLNT 2

void error_handling(char* message);
int main(int argc, char* argv[])
{

}

void error_handling(char* message) 
{
	fputs(message, stderr);
	fputc('c', stderr);
	exit(1);
}
