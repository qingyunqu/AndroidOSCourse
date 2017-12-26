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


#define EINVAL 22
#define EACCESS 23

#define BUFSIZE 1024

enum clfs_type {
		CLFS_PUT = 0,
		CLFS_GET = 1,
		CLFS_RM = 2
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

int is_dir(const char *path)
{
    struct stat statbuf;
    if(lstat(path, &statbuf) ==0)
    {
        return S_ISDIR(statbuf.st_mode) != 0;
    }
    return 0;
}

int is_file(const char *path)
{
    struct stat statbuf;
    if(lstat(path, &statbuf) ==0)
        return S_ISREG(statbuf.st_mode) != 0;
    return 0;
}
void delete_file(const char *path)
{
	if(is_file(path))
		remove(path);
	return ;
}

int main(){
	int server_sockfd;
	int client_sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;
	int sin_size;	
	
	memset(&my_addr,0,sizeof(my_addr));

	my_addr.sin_family=AF_INET;
	my_addr.sin_addr.s_addr=inet_addr("0.0.0.0");//allow connecting from all local address
	my_addr.sin_port=htons(8888);

	
	if((server_sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		printf("error : socket()\n");
		return 1;
	}
	
	if(bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0){
		printf("error : bind()\n");
		return 1;
	}

	listen(server_sockfd,5);
	
	sin_size=sizeof(struct sockaddr_in);
	
	while(1){  //connect
		if((client_sockfd=accept(server_sockfd,(struct sockaddr *)&remote_addr,&sin_size))<0){
			printf("error : accept()\n");
			continue;
		}
		printf("accept a client connecting!\n");
		//thread???
		
		char buf[BUFSIZE];
		int len=0;
		while((len=recv(client_sockfd,buf,BUFSIZ,0))>0){  //request
			struct clfs_req *req=(struct clfs_req *)buf;
			int inode=req->inode;
			enum clfs_type type=req->type;
			char file[100];
			sprintf(file,"/home/lyq/course_file/androidcourse_zyw/test_for_lab4/clfs_store/%d.dat\0",inode);
			//if(is_dir("/home/lyq/course_file/androidcourse_zyw/test_for_lab4/clfs_store"))
			//	printf("hahahaha\n");
			int size=req->size;
			enum clfs_status state;
			int fd;
			if(type==CLFS_PUT){
				state=CLFS_OK;
				send(client_sockfd,(char *)&state,sizeof(enum clfs_status),0);
				len=recv(client_sockfd,buf,size,0);
				printf("CLFS_PUT : get %s\n",buf);
				fd=open(file,O_RDWR|O_CREAT|O_APPEND,0666);//APPEND or TRUNC ?
				if(fd<0){
					printf("CLFS_PUT error : can not open file\n");
					perror("ERROR");
					state=CLFS_ACCESS;
					send(client_sockfd,(char *)&state, sizeof(enum clfs_status),0);
				}
				else{
					write(fd,buf,len);

					send(client_sockfd,(char *)&state, sizeof(enum clfs_status),0);
				}
				close(fd);
				printf("CLFS_PUT : %d.dat\n",inode);
			}
			if(type == CLFS_GET){
				state=CLFS_OK;
				fd=open(file,O_RDONLY);
				if(fd<0){
					state=CLFS_INVAL;
					send(client_sockfd,(char *)&state, sizeof(enum clfs_status),0);
					printf("CLFS_GET error : no such file\n");
				}
				else{
					send(client_sockfd,(char *)&state, sizeof(enum clfs_status),0);	
					len=read(fd,buf,size);
					send(client_sockfd,buf,len,0);
					
					close(fd);
					printf("CLFS_GET : %d.dat\n",inode);
				}
			}
			if(type==CLFS_RM){
				state=CLFS_OK;
				fd=open(file,O_RDONLY);
				if(fd<0){
					state=CLFS_INVAL;
					send(client_sockfd,(char *)&state, sizeof(enum clfs_status),0);
				}
				else{
					send(client_sockfd,(char *)&state, sizeof(enum clfs_status),0);
					
					close(fd);
					delete_file(file);
					printf("CLFS_RM : %d.dat\n",inode);
				}
			}
		}
					
	}
	
	return 0;
}
