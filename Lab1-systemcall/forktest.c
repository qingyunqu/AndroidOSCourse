#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<sched.h>
int main(int argc,char **argv)
{
	if(argc<4)
	{
		printf("Need more arguments!\n");
		return 0;
	}	
	sleep(atoi(argv[1]));
	printf("sleep for %s seconds!\n",argv[1]);
	if(!strcmp(argv[2],"fork"))
	{
		pid_t pid;
		if((pid=fork())==0)
		{			
			if(execvp(argv[3],argv+3)<0)
				printf("%s not found!",argv[3]);
			return 0;
		}
	}
	else
	{
		if(!strcmp(argv[2],"vfork"))
		{
			pid_t pid;
			if((pid=vfork())==0)
			{
				if(execvp(argv[3],argv+3)<0)
					printf("%s not found!",argv[3]);
				return 0;
			}
		}
		else
		{
			if(!strcmp(argv[2],"clone"))
			{
				pid_t pid;
				if((pid=clone())==0)
				{
					if(execvp(argv[3],argv+3)<0)
						printf("%s not found!",argv[3]);
					return 0;
				}
			}
		}
	}
	return 0;
}
