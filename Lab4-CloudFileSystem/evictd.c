#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/types.h>
#include <linux/quotaops.h>
#include <linux/module.h>

extern void iterate_supers(void (*)(struct super_block *, void *), void *);
extern ssize_t sel_write_enforce_val(int new_value);
struct task_struct *task_evictd = NULL;

static void try_evictd(struct super_block *super,void *data)
{
	//printk("try_evictd!\n");
	if(super->s_op->evict_fs != NULL)
	{
		super->s_op->evict_fs(super);
	}
}

static int do_evictd(void *data)
{
	//struct file_system_type *p;	
	while(!kthread_should_stop()){
		printk("evictd begin!\n");
				
		/*for(p=file_systems; p ; p=p->next){
				
		}*/
		sel_write_enforce_val(0);
		iterate_supers(try_evictd, NULL);		
		
		printk("evictd sleep!\n");		
		ssleep(15);
	}
	
	return 0;
}

static int __init init_evictd(void)
{
	printk("init_evictd!\n");	
	task_evictd = kthread_run(do_evictd,NULL,"kfs_evictd");
	return 0;
}

static void __exit exit_evictd(void)
{
	if(task_evictd){
		printk("exit_evictd!\n");
		kthread_stop(task_evictd);
		task_evictd = NULL;
	}
}

fs_initcall(init_evictd);
fs_exitcall(exit_evictd);
