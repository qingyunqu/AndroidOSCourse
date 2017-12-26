#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

//#define EINVAL 1
#define EACCESS 23

#define BUFSIZE 1000

enum clfs_type {
		CLFS_PUT,
		CLFS_GET,
		CLFS_RM
};

struct clfs_req {
	enum clfs_type type;
	int inode;
	int size;
};

enum clfs_status{
	CLFS_OK = 0,
	CLFS_INVAL = EINVAL,
	CLFS_ACCESS = EACCESS,
	CLFS_ERROR
};

int main(){
	int sockfd;
	//struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;
	int sin_size;

	memset(&remote_addr,0,sizeof(remote_addr));
	
	remote_addr.sin_family=AF_INET;
	remote_addr.sin_addr.s_addr=inet_addr("10.0.2.2");
	remote_addr.sin_port=htons(8888);
	
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		printf("error : socket()\n");
		return 1;
	}
	if(connect(sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0){
		printf("error : connect()\n");
		return 1;
	}
	printf("connect to server\n");
	
	char buf[BUFSIZE];
	int len=0;
	struct clfs_req req;
	req.type=CLFS_PUT;
	req.inode=100;
	req.size=11;
	send(sockfd,(char *)&req,sizeof(struct clfs_req),0);
	len = recv(sockfd,buf,sizeof(enum clfs_status),0);
	enum clfs_status *state=(enum clfs_status *)buf;
	if(*state!=CLFS_OK){
		printf("CLFS_PUT1 error\n");
		return 1;
	}
	len = send(sockfd,"helloworld\n",11,0);
	len = recv(sockfd,buf,sizeof(enum clfs_status),0);
	if(*state!=CLFS_OK){
		printf("CLFS_PUT2 error\n");
		return 1;
	}
	
	req.type=CLFS_GET;
	send(sockfd,(char *)&req,sizeof(struct clfs_req),0);
	len = recv(sockfd,buf,sizeof(enum clfs_status),0);
	state=(enum clfs_status *)buf;
	if(*state!=CLFS_OK){
		printf("CLFS_PUT1 error\n");
		return 1;
	}
	len=recv(sockfd,buf,11,0);
	buf[11]='\0';
	printf("%s",buf);

	close(sockfd);
	return 0;
}
