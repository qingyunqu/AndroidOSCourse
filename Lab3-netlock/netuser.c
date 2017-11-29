#include<stdio.h>
#include<sys/syscall.h>

enum __netlock_t {
         NET_LOCK_USE,
         NET_LOCK_SLEEP
     };
typedef enum __netlock_t netlock_t;

int main(){
	netlock_t type = NET_LOCK_USE;
	while(1){
		printf("user enter sys_net_lock!\n");
		syscall(322,type,5);
		printf("user get the netlock!\n");
		
		syscall(323,type);
		printf("user release the netlock!\n");
		sleep(2);	
	}
	return 0;
}
