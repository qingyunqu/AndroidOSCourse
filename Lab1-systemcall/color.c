#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/syscalls.h>
#include<linux/linkage.h>
#include<linux/errno.h>
#include<linux/types.h>
#include<linux/uaccess.h>

/* nr_pids contains the number of entries in
   the pids, colors, and the retval arrays. The colors array contains the
   color to assign to each pid from the corresponding positioin of
   the pids array. Returns 0 if all set color request
   succeed. Otherwise, the array retval contains per-request
   error codes -EINVAL for an invalid pid, or 0 on success.
*/
//sys_318
asmlinkage long sys_setcolors(int nr_pids, pid_t *pids, u_int16_t *colors, int *retval) 
{
	if(!access_ok(VERIFY_READ,(void __user*)pids,nr_pids*sizeof(pid_t)))
		return -EFAULT;
	if(!access_ok(VERIFY_READ,(void __user*)colors,nr_pids*sizeof(u_int16_t)))
		return -EFAULT;
	if(!access_ok(VERIFY_WRITE,(void __user*)retval,nr_pids*sizeof(int)))
		return -EFAULT;	
	kuid_t uid=current_uid();
	if(uid)
		return -EACCES;
	struct task_struct *task;
	int i;
	int flag=0;
	for(i=0;i<nr_pids;i++)
	{
		task=find_task_by_vpid(pids[i]);
		if(task!=NULL)
		{
			task->color=colors[i];
			retval[i]=0;
		}
		else
		{
			retval[i]=-EINVAL;
			flag++;
		}
	}
	return flag;
}

/* Gets the colors of the processes
   contained in the pids array. Returns 0 if all get color requests
   succeed. Otherwise, an error code is returned. The array
   retval contains per-request error codes: -EINVAL for an
   invalid pid, or 0 on success
*/
//sys_319
asmlinkage long sys_getcolors(int nr_pids, pid_t *pids, u_int16_t *colors, int *retval) 
{
	if(!access_ok(VERIFY_READ,(void __user*)pids,nr_pids*sizeof(pid_t)))
		return -EFAULT;
	if(!access_ok(VERIFY_WRITE,(void __user*)colors,nr_pids*sizeof(u_int16_t)))
		return -EFAULT;
	if(!access_ok(VERIFY_WRITE,(void __user*)retval,nr_pids*sizeof(int)))
		return -EFAULT;	
	struct task_struct *task;
	int i;
	int flag=0;
	for(i=0;i<nr_pids;i++)
	{
		task=find_task_by_vpid(pids[i]);
		if(task!=NULL)
		{
			colors[i]=task->color;
			retval[i]=0;
		}
		else
		{
			retval[i]=-EINVAL;
			flag++;
		}
	}
	return flag;
}
