#include<stdio.h>
#include<sys/syscall.h>
#include<dirent.h>
#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#define BUF_SIZE 1024
#define N 10
int getpidbyname(char*task_name)
{
	DIR *dir;
	pid_t pid=0;
	struct dirent *ptr;
	FILE *fp;
	char filepath[50];
	char cur_task_name[50];
	char buf[BUF_SIZE];
	dir=opendir("/proc");
	if(dir!=NULL)
	{
		while((ptr=readdir(dir))!=NULL)
		{
			if((strcmp(ptr->d_name,".")==0)||(strcmp(ptr->d_name,"..")==0))
				continue;
			if(DT_DIR!=ptr->d_type)
				continue;
			sprintf(filepath,"/proc/%s/cmdline",ptr->d_name);
			fp=fopen(filepath,"r");
			if(NULL!=fp)
			{
				if(fgets(buf,BUF_SIZE-1,fp)==NULL)
				{
					fclose(fp);
					continue;
				}
				sscanf(buf,"%s",cur_task_name);
				if(!strcmp(task_name,cur_task_name))
				{
					pid=atoi(ptr->d_name);
					break;
				}
				fclose(fp);
			}
			sprintf(filepath,"/proc/%s/status",ptr->d_name);
			fp=fopen(filepath,"r");
			if(NULL!=fp)
			{
				if(fgets(buf,BUF_SIZE-1,fp)==NULL)
				{
					fclose(fp);
					continue;
				}
				sscanf(buf,"%*s %s",cur_task_name);
				if(!strcmp(task_name,cur_task_name))
				{
					pid=atoi(ptr->d_name);
					break;
				}
				fclose(fp);
			}
		}
		close(dir);
	}
	return pid;
}
int main(int argc,char** argv)
{
	if(argc==1)
	{
		printf("need more arguments!\n");
	}
	else
	{
		int i;
		pid_t pids[N];
		int colors[N];
		int retval[N];
		int j=0;
		for(i=1;i<argc;)
		{
			pid_t pid;
			pid=getpidbyname(argv[i]);
			if(pid==0)
			{
				printf("wrong process name: %s!\n",argv[i]);
				return 0;
			}
			else
			{
				pids[j]=pid;
				//colors[j]=atoi(argv[i+1]);
				j++;
			}
			i+=1;
		}
		int ret=syscall(319,j,pids,colors,retval);
		if(ret!=0)
			printf("error in system calls !\n");
		for(i=1;i<=j;i++)
			printf("%s's pid & color : %d %d\n",argv[i],pids[i-1],colors[i-1]);
	}
	return 0;
}
