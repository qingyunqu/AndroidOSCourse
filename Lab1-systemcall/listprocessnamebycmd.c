#include<stdio.h>
#include<sys/syscall.h>
#include<dirent.h>
#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#define BUF_SIZE 1024
#define N 10
int main()
{
	DIR *dir;
	//pid_t pid=0;
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
				printf("%s\n",cur_task_name);
				//if(!strcmp(task_name,cur_task_name))
				//	pid=atoi(ptr->d_name);
				fclose(fp);
			}
		}
		close(dir);
	}
	return 0;
}
