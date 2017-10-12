# Lab1-系统调用报告
<br />

#### Part 1 环境搭建
<br/>

* 这部分按WriteUp所述，搭建起来即可
#### Part 2 添加系统调用
<br/>

* 首先在`include/linux/sched.h`的`task_struct`中添加color这一进程属性，然后在定义系统调用的文件`syscall_64.tbl`中添加`sys_setcolors`和`sys_getcolors`和两个系统调用，系统调用号分别为318和319

* 在kernel文件夹下新建`color.c`实现这两个系统调用，这里只对代码中关键的部分进行解释  

```c
if(!access_ok(VERIFY_READ,(void __user*)pids,nr_pids*sizeof(pid_t)))
		return -EFAULT;
if(!access_ok(VERIFY_READ,(void __user*)colors,nr_pids*sizeof(u_int16_t)))
		return -EFAULT;
if(!access_ok(VERIFY_WRITE,(void __user*)retval,nr_pids*sizeof(int)))
		return -EFAULT;	
``` 
* 此处检查传入的函数指针（`getcolors`中也有对应的部分，略有不同，主要为VERIFY_READ和VERIFY_WRITE的区别），查看`copy_from_user()`的源码，发现它其实调用的还是`access_ok(type,addr,size)`函数，所以此处直接调用该函数进行指针检查，主要是确认用户空间传入的指针是否指向可读或可写的内存地址。
```c
kuid_t uid=current_uid();
if(uid)
		return -EACCES;
```
* 此处通过得到进程的uid（用户id）来判断执行`setcolors`的用户是否是root用户，uid为0即为root用户，在内核中只有uid的区别，并没有用户名的存在。
```c
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
```
* 此处通过`find_task_by_vpid()`函数来得到进程对应的`task_struct`指针，然后修改（或得到）对应的color，如果有某一进程无法得到task指针，便将对应的return value设为`-EINVAL`,同时flag（初始为0）加一，最后返回flag的值，告诉进程一共有几个出错pid。
#### Part 3 修改fork.c binder.c
<br/>

* 首先是修改`fork.c`，由于c语言的struct无法默认初始化，而进程经常性是malloc出来的，无法保证color属性的初始值一定是零，而且要满足fork和clone时color属性继承父进程，vfork时color属性为0，故而要修改fork.c文件

* 阅读fork.c的源码，发现`fork()` `vfork()` `clone()`均是调用`do_fork()`,而`do_fork()`调用的是`copy_process()`，这两者之间传递的用来区分三者的是`clone_flags`，故而在`copy_process()`中的合适位置添加如下代码：
```c
	if(clone_flags&CLONE_VFORK)  /*init the color*/
	{
		p->color=0;
	}
	else
	{
		p->color=p->parent->color;
	}
```
* 阅读`binder.c`中的`binder_transaction()`代码，发现proc指向当前进程，故而`proc->tsk`指向当前进程的`task_struct`，而`target_proc->tsk`指向目标进程的`task_struct`，故而在代码中已经判断`target_proc->tsk`存在的代码之后添加自己的对于`color`属性的判断。再分析整个函数的错误处理，发现它均是在判断为有误之后，`goto`至结尾的错误处理部分，并设置相关的`return value`，我这里就直接采用之前的错误类型进行返回：
```c
	if((proc->tsk->color!=0)&&(target_proc->tsk->color!=0)&&(proc->tsk->color!=target_proc->tsk->color)){
			return_error=BR_DEAD_REPLY;		
			goto err_dead_binder;
	}
```
#### Part 4 测试程序
<br/>

* 首先是分析如何从进程名得到对应的pid，我的做法是遍历`/proc/N/`下的`status`和`cmdline`文件，由于`status`文件中对于进程名，只能保存后15个字符，而用户态的进程在`cmdline`文件中均有完整的进程名，故而我先遍历`cmdline`文件，没有匹配的进程名再遍历`status`文件，具体实现在`getpidbyname()`函数中。

* `setcolors`实现了给某进程赋颜色

* `getcolors`实现了获取某进程的颜色，具体实现均在代码中

* `forktest`实现了对命令的执行，我这里采用的是`execvp`函数，该函数的好处是它会自动从环境变量中寻找可执行的二进制文件，不需要提供完整的路径，具体实现在代码中。
