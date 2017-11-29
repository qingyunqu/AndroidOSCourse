#include<stdio.h>
#include<sys/syscall.h>


enum __netlock_t {
         NET_LOCK_USE,
         NET_LOCK_SLEEP
     };
typedef enum __netlock_t netlock_t;

int main(){
	netlock_t type=NET_LOCK_SLEEP;
	while(1){

		//printf("sleeper enter sys_net_lock!\n");
		syscall(322,type,0);
		printf("sleeper get the netlock!\n");

		syscall(324);
		
		syscall(323);
		
		printf("sleeper release the netlock!\n");
		sleep(5);
	}
	return 0;
}
