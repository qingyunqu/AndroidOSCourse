#include<linux/syscalls.h>
#include<linux/linkage.h>
#include<linux/errno.h>
#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/slab.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/mman.h>
#include<linux/gfp.h>



#define SSMEM_FLAG_CREATE 0x1
#define SSMEM_FLAG_WRITE 0x2
#define SSMEM_FLAG_EXEC 0x4

#define SSMEM_ID_MAX 1025

#define SSMEM_PAGE_SIZE 4096
#define SSMEM_PAGES_MAX 10

#define SSMEM_PAGE_ALIGN(len) ((len - 1)/SSMEM_PAGE_SIZE + 1)*SSMEM_PAGE_SIZE

#define SSMEM_ATTACH_MAX 10

#define EADDRNOTAVALL 100


struct ssmem_data_struct {
	size_t length;
	int count;
	struct mm_struct *list_mm[SSMEM_ATTACH_MAX];
	struct vm_area_struct *list_vma[SSMEM_ATTACH_MAX];
	struct page *page;//[SSMEM_PAGES_MAX];
	//page的页数？？？
};

struct ssmem_data_struct *ssmem[SSMEM_ID_MAX];

static void ssmem_close(struct vm_area_struct *vma)
{
}

static int ssmem_fault(struct vm_area_struct *vma,
			struct vm_fault *vmf)
{
	
	struct page *page;
	if(vma->ssmem_id){
		if(ssmem[vma->ssmem_id]->page==NULL){
			page=alloc_page(GFP_USER);
			vmf->page=page;
			ssmem[vma->ssmem_id]->page=page;
					
		}
		else{
			vmf->page=ssmem[vma->ssmem_id]->page;
			
		}
	}
	else{
		return VM_FAULT_SIGBUS;
	}
	get_page(vmf->page);
	return 0;
}
static const struct vm_operations_struct  ssmem_ops = {
	.close = ssmem_close,
	.fault = ssmem_fault,
};


void init_ssmem_data(struct ssmem_data_struct *ssmem)
{
	int i;	
	for(i=0;i<SSMEM_ATTACH_MAX;i++){
		ssmem->list_mm[i]=NULL;
		ssmem->list_vma[i]=NULL;
		//ssmem->page[i]=NULL;
	}
	ssmem->page=NULL;	
}


extern unsigned long do_mmap_pgoff(struct file *file, unsigned long addr,
			unsigned long len, unsigned long prot,
			unsigned long flags, unsigned long pgoff,
			unsigned long *populate);
//sys_ssmem_attach 320
SYSCALL_DEFINE3(ssmem_attach,int,id,int,flags,size_t,length)
{
	if(id <= 0 || id >= SSMEM_ID_MAX){
		return -EINVAL;
	}
	if(length <= 0 || length > SSMEM_PAGE_SIZE*SSMEM_PAGES_MAX){
		return -EINVAL;
	}
	int flags_mmap=PROT_NONE|PROT_READ;
	if(flags&SSMEM_FLAG_WRITE)
		flags_mmap=flags_mmap|PROT_WRITE;
	if(flags&SSMEM_FLAG_EXEC)
		flags_mmap=flags_mmap|PROT_EXEC;
	size_t align_length;	
	if(flags&SSMEM_FLAG_CREATE){
		if(ssmem[id]==NULL){
			align_length=SSMEM_PAGE_ALIGN(length);
		}
		else{
			align_length=ssmem[id]->length;
		}
	}
	else{
		if(ssmem[id]==NULL){
			return 	-EADDRNOTAVALL;
		}
		else{
			align_length=ssmem[id]->length;
		}
	}
	unsigned long addr=0;
	struct vm_area_struct *vma=NULL;
	struct mm_struct *mm=current->mm;
	unsigned long populate;
	if(flags&SSMEM_FLAG_CREATE){
		addr=do_mmap_pgoff(NULL,addr,align_length,flags_mmap,/*MAP_PRIVATE*/MAP_SHARED|MAP_LOCKED,0,&populate);//vma存在合并问题//MAP_PRIVATE or MAP_SHARED
		if(addr>0){
			vma=find_vma(mm,addr);
			vma->vm_ops=&ssmem_ops;
			vma->ssmem_id=id;
			if(ssmem[id]==NULL){
				ssmem[id]=(struct ssmem_data_struct *)kzalloc(sizeof(struct ssmem_data_struct),GFP_KERNEL);		
				ssmem[id]->length=align_length;
				ssmem[id]->count=1;
				init_ssmem_data(ssmem[id]);
				ssmem[id]->list_mm[0]=mm;
				ssmem[id]->list_vma[0]=vma;
			}
			else{
				int count=ssmem[id]->count;			
				ssmem[id]->list_mm[count]=mm;
				ssmem[id]->list_vma[count]=vma;
				ssmem[id]->count++;
			}
			return addr;
		}
		else{
			return -ENOMEM;
		}
	}
	else{
		if(ssmem[id]==NULL){
			return -EADDRNOTAVALL;
		}
		else{
			addr=do_mmap_pgoff(NULL,addr,align_length,flags_mmap,/*MAP_PRIVATE*/MAP_SHARED|MAP_LOCKED,0,&populate);
			if(addr>0){
				vma=find_vma(mm,addr);
				vma->vm_ops=&ssmem_ops;
				vma->ssmem_id=id;
				int count=ssmem[id]->count;				
				ssmem[id]->list_mm[count]=mm;
				ssmem[id]->list_vma[count]=vma;
				ssmem[id]->count++;
				return addr;
			}
			else{
				return -ENOMEM;
			}
		}
	}
}	

int find_ssmem(int ssmem_id,struct mm_struct *mm){
	int i;
	for(i=0;i<SSMEM_ATTACH_MAX;i++){
		if(ssmem[ssmem_id]->list_mm[i]==NULL)
			continue;
		else if(ssmem[ssmem_id]->list_mm[i]==mm){
			return i;
		}
	}
	return -1;
}

//sys_ssmem_detach 321
SYSCALL_DEFINE1(ssmem_detach,void *,addr)
{
	struct mm_struct *mm=current->mm;
	struct vm_area_struct *vma=find_vma(mm,addr);
	if(vma==NULL){
		return -EFAULT;
	}
	else if(vma->ssmem_id==0){
		return -EFAULT;
	}
	else{
		int count;
		if(ssmem[vma->ssmem_id]->count==0)
			return -EFAULT;
		count=find_ssmem(vma->ssmem_id,mm);
		if(ssmem[vma->ssmem_id]->count==1){
			ssmem[vma->ssmem_id]->page=NULL;
		}
		do_munmap(mm,(unsigned long)ssmem[vma->ssmem_id]->list_vma[count],SSMEM_PAGE_SIZE);
		ssmem[vma->ssmem_id]->list_mm[count]=NULL;
		ssmem[vma->ssmem_id]->list_vma[count]=NULL;
		ssmem[vma->ssmem_id]->count--;
	}
	return 0;
}
