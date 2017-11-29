#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/syscalls.h>
#include<linux/linkage.h>
#include<linux/errno.h>
#include<linux/sched.h>
#include<linux/rwsem.h>
#include<linux/delay.h>
#include<linux/list.h>

enum __netlock_t {
         NET_LOCK_USE,
         NET_LOCK_SLEEP
     };
typedef enum __netlock_t netlock_t;

DECLARE_RWSEM(netlock);

extern void init_netlock_use(unsigned long timeout);/*{
	current->edf.deadline=jiffies+timeout*100;
	current->edf.p=current;
	
} */
extern void fini_netlock();
/* 	Acquire netlock. type indicates
        whether a use or sleep lock is needed, and the timeout_val indicates
        how long a user is willing to wait in seconds. It's value is
        ignored for sleepers. Returns 0 on success and -1 on failure.  
      */

//unsigned long volatile netlock_nearest;
//DECLARE_RWSEM(net_nearest);
//EXPORT_SYMBOL(netlock_nearest);
//sys_net_lock 322
SYSCALL_DEFINE2(net_lock, netlock_t, type, u_int16_t, timeout_val)
{
	if(type==NET_LOCK_USE){
		//printk("netuser enter the SYS_netlock\n");
		init_netlock_use((unsigned long)timeout_val);
		//printk("in netuser nearest = %lx\n",netlock_nearest);	
		down_read(&netlock);
		//printk("netuser get the netlock\n");
	}
	else{
		
		down_write(&netlock);
		//printk("sleeper get the netlock\n");
	}
	return 0;

}

/*      Release netlock.Return 0
        on success and -1 on failure.  
      */
//sys_net_unlock 323
SYSCALL_DEFINE1(net_unlock,netlock_t,type)
{
	if(type==NET_LOCK_USE){
		up_read(&netlock);
		fini_netlock();
	}
	else{
		up_write(&netlock);
	}
	return 0;
}

/*      Wait for user timeout. Return 0 on a successful
        timeout, and -<Corresponding ERRNO> on failure.  
      */

//sys_net_lock_wait_timeout 324

SYSCALL_DEFINE0(net_lock_wait_timeout)
{
	//unsigned long time=netlock_nearest;
	/*down_read(&net_nearest);
	while(netlock_nearest > jiffies){
		printk("nearest %lx =%lx > jiffies=%lx\n",&netlock_nearest,netlock_nearest,jiffies);
		up_read(&net_nearest);
		ssleep(1);
		down_read(&net_nearest);
		//time =netlock_nearest;
	}
	up_read(&net_nearest);*/
	ssleep(5);
	return 0;
}
