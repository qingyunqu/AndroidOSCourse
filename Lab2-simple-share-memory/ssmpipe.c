#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/syscall.h>
#include<sys/sem.h>
#include<sys/types.h>

/*int read_cnt=0;
int read_mutex=1;
int mutex=1;

void P(int semid)
{
	struct sembuf sem_p;
	sem_p.sem_num=0;
	sem_p.sem_op=-1;
	sem_p.sem_flg=SEM_UNDO;
	if(semop(semid,&sem_p,1)==-1){
		printf("P op failed/n");
		exit(1);
	}
}
void V(int semid){
	struct sembuf sem_p;
	sem_p.sem_num=0;
	sem_p.sem_op=1;
	sem_p.sem_flg=SEM_UNDO;
	if(semop(semid,&sem_p,1)==-1){
		printf("V op failed/n");
		exit(1);
	}
}*/
void ssmpipe(char*name,int ssmem_id,char*type){
	char *ssmem=NULL;
	ssmem=(char*)syscall(320,ssmem_id,7,4*1024);
	char tmp[100];
	if((strcmp(type,"writer")==0)){	
		scanf("%s",tmp);
		sprintf(ssmem,"writer%s says %s",name,tmp);
	}
	else{
		printf("reader%s: %s\n",name,ssmem);
	}
	sleep(10);
	syscall(321,ssmem);
}
int main(int argc,char** argv)
{	
	ssmpipe(argv[1],atoi(argv[2]),argv[3]);	
	
	return 0;
}
