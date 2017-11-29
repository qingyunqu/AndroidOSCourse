#include "sched.h"

#include <linux/list.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/rwsem.h>
#include<linux/delay.h>

//extern unsigned long volatile netlock_nearest;
//extern struct rw_semaphore net_nearest;

void init_edf_rq(struct edf_rq *edf)
{
	INIT_LIST_HEAD(&edf->head);
	edf->edf_nr_running=0;
	//down_write(&net_nearest);
	edf->nearest=(unsigned long)(0-1);
	//up_write(&net_nearest);
}
static void enqueue_task_edf(struct rq *rq, struct task_struct *p, int flags)
{
	struct edf_rq *edf_rq=&rq->edf;
	struct sched_edf_entity *edf=&p->edf;	
	list_add(&edf->run_list,&edf_rq->head);
	//down_write(&net_nearest);
	if(edf_rq->nearest > edf->deadline)
		edf_rq->nearest = edf->deadline;
	//up_write(&net_nearest);
	edf_rq->edf_nr_running++;
/*int i;for(i = 0;i < 5;i++)
	printk("enqueue_task_edf netlock_nearest %lx =%lx\n",&netlock_nearest,netlock_nearest);
ssleep(1);*/
}

static void dequeue_task_edf(struct rq *rq, struct task_struct *p, int flags)
{
	list_del(&p->edf.run_list);
	rq->edf.edf_nr_running--;
	//down_write(&net_nearest);
	if(!list_empty(&rq->edf.head)){
		struct list_head *pos;
		struct list_head *head=&(rq->edf.head);
		struct sched_edf_entity *edf;
		edf=list_first_entry(head,struct sched_edf_entity,run_list);
		
		rq->edf.nearest=edf->deadline;
		list_for_each(pos,head){
			edf=list_entry(pos,struct sched_edf_entity,run_list);
			if(edf->deadline < rq->edf.nearest)
				rq->edf.nearest = edf->deadline;
		}
			
	}
	else
		rq->edf.nearest=(unsigned long)(0-1);
	//up_write(&net_nearest);
}

static void yield_task_edf(struct rq *rq)
{

}

static void check_preempt_curr_edf(struct rq*rq, struct task_struct *p, int flags)
{

}

static struct task_struct *pick_next_task_edf(struct rq*rq)
{
	if(list_empty(&rq->edf.head))
		return NULL;
	struct list_head *head=&rq->edf.head;
	struct list_head *pos;
	struct sched_edf_entity *edf;
	list_for_each(pos,head){
		edf=list_entry(pos,struct sched_edf_entity,run_list);
		if(edf->deadline==rq->edf.nearest)
			return edf->p;
	}		
}

static void put_prev_task_edf(struct rq *rq, struct task_struct *prev)
{

}

static void task_tick_edf(struct rq *rq, struct task_struct *curr, int queued)
{
	
}
static void set_curr_task_edf(struct rq*rq)
{

}
const struct sched_class edf_sched_class = {
	.next			=&fair_sched_class,
	.enqueue_task		= enqueue_task_edf,
	.dequeue_task		= dequeue_task_edf,
	.yield_task		= yield_task_edf,

	.check_preempt_curr	= check_preempt_curr_edf,

	.pick_next_task		= pick_next_task_edf,
	.put_prev_task		= put_prev_task_edf,

/*#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_edf,

	.set_cpus_allowed       = set_cpus_allowed_edf,
	.rq_online              = rq_online_edf,
	.rq_offline             = rq_offline_edf,
	.pre_schedule		= pre_schedule_edf,
	.post_schedule		= post_schedule_edf,
	.task_woken		= task_woken_edf,
	.switched_from		= switched_from_edf,
#endif
*/
	.set_curr_task          = set_curr_task_edf,
	.task_tick		= task_tick_edf,

/*	.get_rr_interval	= get_rr_interval_edf,

	.prio_changed		= prio_changed_edf,
	.switched_to		= switched_to_edf,*/
};

extern int sched_setscheduler_nocheck(struct task_struct *p, int policy, 
					const struct sched_param *param); 

void init_netlock_use(unsigned long timeout){
	current->edf.deadline=jiffies+timeout*100;
	current->edf.p=current;
	current->sched_class->dequeue_task(task_rq(current),current,0);
	current->sched_class=&edf_sched_class;
	current->sched_class->enqueue_task(task_rq(current),current,0);
	//printk("init_netlock_use complete\n");
} 


void fini_netlock(){
	current->sched_class->dequeue_task(task_rq(current),current,0);
	current->sched_class=&fair_sched_class;
	current->sched_class->enqueue_task(task_rq(current),current,0);
} 

