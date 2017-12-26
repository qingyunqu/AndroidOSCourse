#include <linux/fs.h>
#include <linux/file.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/blk_types.h>

#include "ext2.h"
#include "xattr.h"
#include "../mount.h"

#include <linux/pagemap.h>
#include <linux/cache.h>

/* kernel socket */
#include<linux/in.h>  
#include<linux/inet.h>  
#include<linux/socket.h>  
#include<net/sock.h>

//#define EINVAL 22
#define EACCESS 23

#define BUFSIZE 1024

enum clfs_type {
		CLFS_PUT = 0,
		CLFS_GET = 1,
		CLFS_RM = 2
};

struct clfs_req {
	enum clfs_type type;
	int inode;
	int size;
};

enum clfs_status {
	CLFS_OK = 0,
	CLFS_INVAL = EINVAL,
	CLFS_ACCESS = EACCESS,
	CLFS_ERROR
};

struct s_i_data {
	loff_t			i_size;
	struct timespec		i_atime;
	struct timespec		i_mtime;
	struct timespec		i_ctime;
	unsigned short          i_bytes;
	unsigned int		i_blkbits;
	blkcnt_t		i_blocks;
	unsigned long		dirtied_when;
};

char server_ip[20] = "10.0.2.2" ;
unsigned short server_port = 8888 ;
int high_watermark = 95 ;
int low_watermark = 85 ;
int evict_target = 70 ;

static char buf[BUFSIZE];
static struct s_i_data s_i;
//static int file_len = 100;
static char filename[PAGE_SIZE]="/mnt/sdcard/s.txt\0";

extern void for_each_inode(
	struct super_block *super, void (*fun)(struct inode*, void*), void *arg);

static void set_wh(int wh)
{
	
	high_watermark = wh;
	printk("the high_watermark is : %d\n",wh);
}

static void set_wl(int wl)
{
	low_watermark = wl;
	printk("the low_watermark is : %d\n",wl);
}

static unsigned int atou(const char *str)
{
    unsigned res = 0;
    int i = 0;
    for (; str[i]; ++i)
    {
        if (str[i] < 48 || str[i] > 57)
        {
            break;
        }
        res = (res * 10) + str[i] - 48;
    }
    return res;
}

static void set_port(int port)
{
	server_port=(unsigned short)port;
	printk("the server_port is : %d\n",port);
}

static void set_srv(char *ip)
{
	int i=0;	
	while(*ip != '\0'&&*ip!=':'){
		server_ip[i]=*ip;
		ip++;
		i++;
	}
	server_ip[i]='\0';
	printk("the server_ip is : %s\n",server_ip);
	if(*ip==':'){
		ip++;
		set_port(atou(ip));
	}
}

static void set_evict(int evict)
{
	evict_target = evict;
	printk("the evcit_target is : %d\n",evict);
}


int clfs_parse_options(char *p)
{
	if(p[0]=='w'&&p[1]=='h'){
		set_wh(atou(p+3));
	}
	else if(p[0]=='w'&&p[1]=='l'){
		set_wl(atou(p+3));
	}
	if(p[0]=='s'&&p[1]=='r'&&p[2]=='v'){
		set_srv(p+4);
	}
	else if(p[0]=='e'){
		set_evict(atou(p+6));
	}
	return 0;	
}

/*static void copy_to_buf(char*p,int len)
{
	int i;	
	for (i=0;i<len;i++)
		buf[i]=p[i];
}*/

/*static int socket_connect(struct socket *socket,const char *server_ip,unsigned short port)
{
	printk("socket_connect!\n");	
	struct sockaddr_in sockaddr;
	int ret;
	
	memset(&sockaddr, 0,sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = in_aton(server_ip);
	
	ret = sock_create_kern(AF_INET, SOCK_STREAM, 0, &socket);
	if(unlikely(ret<0)){
		return ret;
	}
	
	ret = socket->ops->connect(socket, (struct sockaddr *)(&sockaddr), sizeof(sockaddr), 0);
	if(unlikely(ret <0)){
		printk("socket_connect ret <0\n");
		return ret;
	}
	
	return 0;
}*/

/*static int socket_send(struct socket *socket, char *buf, loff_t size)
{
	printk("socket_send!\n");	
	//struct kvec vec;
	struct msghdr msg;
	struct iovec iov;
	int ret;

	//memset(&vec,0,sizeof(vec));
	//vec.iov_base = buf;
	//vec.iov_len = size;
	//memset(&msg,0,sizeof(msg));
	
	iov.iov_base = buf;
	iov.iov_len = size;
	
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_name = 0;
	msg.msg_namelen = 0;

	printk("before kernel_sendmsg!\n");
	printk("socket*= 0x%lx\n",socket);	
	ret = kernel_sendmsg(socket, &msg, &iov, 1, size);
	printk("kernel_sendmsg ret = %d\n",ret);
	if(unlikely(ret<0)){
		return ret;
	}
	
	return ret;
}*/
/*static int socket_send(struct socket *sock,char *buf, int len) 
{
	printk("socket_send!\n");
	struct msghdr msg;
	struct iovec iov;
	int size;
	
	//struct sockaddr addr;
	//int addrlen;
	//printk("before kernel_getsockname!\n");
	printk("ssocket*= 0x%lx\n",sock);
	//kernel_getsockname(sock,&addr,&addrlen);
	//printk("after kernel_getsockname!\n");
	mm_segment_t oldfs;

	iov.iov_base = buf;
	iov.iov_len = len;

	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_name = 0;
	msg.msg_namelen = 0;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	printk("before sock_sendmsg!\n");
	//size = kernel_sendmsg(sock, &msg, &iov,1,len);
	size = sock_sendmsg(sock,&msg,len);	
	set_fs(oldfs);
    printk("send: message size: %d\n", size);

	return size;
}*/
/*static int socket_recv(struct socket *sock, unsigned char *buf, int len) 
{
	struct msghdr msg;
	struct iovec iov;
	mm_segment_t oldfs;
	int size = 0;
    int ite = 0;

	if (sock->sk == NULL) 
    {
        return 0;
    }

	oldfs = get_fs();
	set_fs(KERNEL_DS);

    while (ite < len)
    {
        int remain = len - ite;
        iov.iov_base = buf + ite;
        iov.iov_len = remain;

        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_flags = 0;
        msg.msg_name = 0;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        size = sock_recvmsg(sock, &msg, remain, msg.msg_flags);
        ite += size;
    }
    set_fs(oldfs);
    printk("recv: message size is : %d\n", ite);

	return ite;
}*/

/*static int socket_recv(struct socket *socket, char *buf,loff_t size)
{
	printk("socket_recv!\n");	
	struct kvec vec;
	struct msghdr msg;
	int ret;

	memset(&vec,0,sizeof(vec));
	memset(&msg,0,sizeof(msg));
	vec.iov_base = buf;
	vec.iov_len = size;
	ret = kernel_recvmsg(socket,&msg,&vec,1,size,0);
	if(unlikely(ret<0)){
		return ret;
	}
	return ret;
}*/

/*static void socket_close(struct socket *socket)
{
	sock_release(socket);
}*/

static int clfs_put(char *filename , unsigned long i_num, loff_t size)
{
	printk("clfs_put!\n");	
	int sockfd;
	struct sockaddr_in remote_addr;
	//int sin_size;
	memset(&remote_addr,0,sizeof(remote_addr));
	
	remote_addr.sin_family=AF_INET;
	remote_addr.sin_addr.s_addr=in_aton(server_ip);
	remote_addr.sin_port=htons(server_port);
	
	if((sockfd=sys_socket(AF_INET,SOCK_STREAM,0))<0){
		printk("error : sys_socket()\n");
		return -1;
	}
	if(sys_connect(sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0){
		printk("error : sys_connect()\n");
		goto close_sockfd;
	}
	printk("connect to server success\n");
	
	int len=0;
	struct clfs_req req;
	enum clfs_status state;
	req.type=CLFS_PUT;
	req.inode=i_num;
	req.size=size;
	sys_send(sockfd,(char *)&req,sizeof(struct clfs_req),0);
	len = sys_recv(sockfd,(char*)&state,sizeof(enum clfs_status),0);
	
	if(state!=CLFS_OK){
		printk("CLFS_PUT request error\n");
		goto close_sockfd;
	}
	int fd;
	fd = sys_open(filename,O_RDONLY,0777);
	if(unlikely(fd<0)){
		printk("open file error!\n");
		goto close_sockfd;
	}
	len = sys_read(fd,buf,size);
	printk("read %d bytes from file\n",len);
	len = sys_send(sockfd,buf,len,0);
	len = sys_recv(sockfd,(char*)&state,sizeof(enum clfs_status),0);
	if(state!=CLFS_OK){
		printk("CLFS_PUT send error\n");
		goto close_fd;
	}
	sys_close(fd);
	sys_close(sockfd);
	return len;
close_fd:
	sys_close(fd);
close_sockfd:
	sys_close(sockfd);
	return -1;
}
/*static int clfs_put(char *filename, unsigned long i_num, loff_t size)
{
	printk("clfs_put!\n");	
	struct socket *socket=NULL;
	int fd;
	int ret;

	socket = (struct socket *)kmalloc(sizeof(struct socket), GFP_KERNEL);
	if(unlikely(socket == NULL)){
		return -1;
	}
	
	fd = sys_open(filename, O_RDONLY, 0777);
	if(unlikely(fd<0)){
		ret = fd;
		goto free_socket;
	}
	
	ret = socket_connect(socket, server_ip,server_port);
	printk("socket_connect ret = %d\n",ret);
	if(unlikely(ret<0)){
		goto close_file;
	}
	
	struct clfs_req*req=(struct clfs_req *)kmalloc(sizeof(struct clfs_req),GFP_KERNEL);
	req->type=CLFS_PUT;
	req->inode = i_num;
	req->size = size;
	
	//copy_to_buf(&req,sizeof(req));
	printk("sizeof(struct clfs_req) : %d\n",sizeof(struct clfs_req));
	printk("req's address is 0x%lx\n",req);
	printk("socket * = %lx\n",socket);
	socket_send(socket,(char*)req,12);
	
	enum clfs_status state;
	socket_recv(socket,(char*)&state,sizeof(enum clfs_status));
	//state=(enum clfs_status *)buf;
	if(unlikely(state != CLFS_OK)){
		ret = -1;
		goto release_socket;
	}
	
	ret = sys_read(fd, buf, size);
	if(unlikely(ret<0)){
		goto release_socket;
	}
	
	socket_send(socket, buf , ret);
	
	socket_recv(socket, (char*)&state,sizeof(enum clfs_status));
	if(unlikely(state != CLFS_OK)){
		ret = -1;
		goto release_socket;
	}
	ret = 0;
	
release_socket:
	socket_close(socket);
	goto out;
close_file:
	sys_close(fd);
free_socket:
	kfree(socket);
	return ret;
out:
	sys_close(fd);
	return ret;
}*/

static int clfs_get(char *filename, unsigned long i_num, loff_t size)
{
	printk("clfs_get!\n");	
	int sockfd;
	struct sockaddr_in remote_addr;
	
	memset(&remote_addr,0,sizeof(remote_addr));
	
	remote_addr.sin_family=AF_INET;
	remote_addr.sin_addr.s_addr=in_aton(server_ip);
	remote_addr.sin_port=htons(server_port);
	
	if((sockfd=sys_socket(AF_INET,SOCK_STREAM,0))<0){
		printk("error : sys_socket()\n");
		return -1;
	}
	if(sys_connect(sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0){
		printk("error : sys_connect()\n");
		goto close_sockfd;
	}
	printk("connect to server success\n");
	
	int len=0;
	struct clfs_req req;
	enum clfs_status state;
	req.type=CLFS_GET;
	req.inode=i_num;
	req.size=size;
	sys_send(sockfd,(char *)&req,sizeof(struct clfs_req),0);
	len = sys_recv(sockfd,(char*)&state,sizeof(enum clfs_status),0);
	
	if(state!=CLFS_OK){
		printk("CLFS_GET request error\n");
		goto close_sockfd;
	}
	len = sys_recv(sockfd,buf,size,0);
	printk("recv %d bytes from server\n");
	
	int fd;
	fd = sys_open(filename,O_WRONLY | O_TRUNC,0777);
	if(unlikely(fd<0)){
		printk("open file error!\n");
		goto close_sockfd;
	}
	sys_write(fd,buf,len);
	
	sys_close(fd);
	sys_close(sockfd);
	
	return len;
close_fd:
	sys_close(fd);
close_sockfd:
	sys_close(sockfd);
	
	return -1;
}
/*static int clfs_get(char *filename,unsigned long i_num, loff_t size)
{
	printk("clfs_get!\n");	
	struct socket *socket;
	int fd;
	int ret;

	socket = (struct socket *)kmalloc(sizeof(struct socket), GFP_KERNEL);
	if(unlikely(socket == NULL)){
		return -1;
	}

	fd = sys_open(filename, O_WRONLY | O_TRUNC, 0777);
	if(unlikely(fd<0)){
		ret = fd;
		goto free_socket;
	}
	
	ret = socket_connect(socket, server_ip,server_port);
	if(unlikely(ret<0)){
		goto close_file;
	}

	struct clfs_req req;
	req.type=CLFS_GET;
	req.inode = i_num;
	req.size = size;
	
	socket_send(socket,(char*)&req,sizeof(req));
	
	enum clfs_status state;
	socket_recv(socket,(char*)&state,sizeof(enum clfs_status));
	if(unlikely(state != CLFS_OK)){
		ret = -1;
		goto release_socket;
	}
	
	ret = socket_recv(socket,buf,size);
	
	ret = sys_write(fd,buf,ret);
	
	ret = 0;

release_socket:
	socket_close(socket);
	goto out;
close_file:
	sys_close(fd);
free_socket:
	kfree(socket);
	return ret;
out:
	sys_close(fd);
	return ret;
	return 0;
}*/

static int save_i_data(struct inode *inode)
{
	s_i.i_size = inode->i_size;
	s_i.i_atime = inode->i_atime;
	s_i.i_mtime = inode->i_mtime;
	s_i.i_ctime = inode->i_ctime;
	s_i.i_blkbits = inode->i_blkbits;
	s_i.i_blocks = inode->i_blocks;
	s_i.i_bytes = inode->i_bytes;
	s_i.dirtied_when = inode->dirtied_when;
	return 0;
}

static int recover_i_data(struct inode *inode)
{
	inode->i_size = s_i.i_size;
	inode->i_atime = s_i.i_atime;
	inode->i_mtime = s_i.i_mtime;
	inode->i_ctime = s_i.i_ctime;
	inode->i_blkbits = s_i.i_blkbits;
	inode->i_blocks = s_i.i_blocks;
	inode->i_bytes = s_i.i_bytes;
	inode->dirtied_when = s_i.dirtied_when;
	return 0;
}
	
static int rm_diskfile(char *file)
{
	printk("rm_diskfile!\n");	
	int ret;
	ret = sys_open(file, O_WRONLY | O_TRUNC , 0777);
	if(unlikely(ret < 0)){
		return ret;
	}
	
	ret = sys_close(ret);
	if(unlikely(ret < 0)){
		return ret;
	}
	
	return 0;
}

extern struct ext2_dir_entry_2 *ext2_find_entry_ino (struct inode * dir,
			ino_t ino, struct page ** res_page);
extern char *d_absolute_path(const struct path *path, char *buf, int buflen);

static inline void ext2_put_page(struct page *page)
{
	kunmap(page);
	page_cache_release(page);
}

static inline int is_root_inode(struct inode *inode)
{
    return inode->i_ino == 2;
}

static int get_filename_by_inode(struct inode *inode,char *buf,int buflen)
{
	printk("get_filename_by_inode!\n");
	char *ret;
    struct list_head *mnt_ite;
    struct path path;
    int pos;
    int x;
    struct inode *dir, *file;
    int ppos = buflen;

    list_for_each(mnt_ite, &(current->nsproxy->mnt_ns->list))
    {
        struct mount *mount = container_of(mnt_ite, struct mount, mnt_list);
        
        if (mount->mnt.mnt_sb->s_op->evict_fs != NULL)
        {
            path.mnt = &mount->mnt;
        }
    }

    path.dentry = inode->i_sb->s_root;
    if (unlikely(IS_ERR(ret = d_absolute_path(&path, buf, buflen))))
    {
        return -1;
    }

    pos = strlen(ret);
    memcpy(buf, ret, pos);

    ext2_xattr_get(inode, EXT2_XATTR_INDEX_TRUSTED, "parent_ino", &x, 4);

    file = ext2_iget(inode->i_sb, inode->i_ino);
    dir = ext2_iget(inode->i_sb, x);

    while (!is_root_inode(file))
    {
        struct ext2_dir_entry_2 *ed;
        struct page *page;

        ed = ext2_find_entry_ino(dir, file->i_ino, &page);
        buf[ppos -= (ed->name_len + 1)] = '/';
        //ed->name[ed->name_len] = 0;
        memcpy(buf + ppos + 1, ed->name, ed->name_len);

        ext2_put_page(page);

        iput(file);
        file = dir;
        ed = ext2_dotdot(dir, &page);
        ext2_put_page(page);
        dir = ext2_iget(inode->i_sb, ed->inode);
    }

    for (; ppos < buflen; ++ppos)
    {
        buf[pos++] = buf[ppos];
    }
    buf[pos] = 0;

    iput(dir);
    iput(file);
    printk("path %s\n", buf);
    return 0;
}

static inline int set_filename(struct inode *inode)
{
    filename[0] = '\0';
    return get_filename_by_inode(inode, filename, PAGE_SIZE);
}

static int inode_using(struct inode *inode)
{
	struct hlist_node *ite = (inode->i_dentry).first;
	while (ite != NULL){
		struct dentry *dentry = container_of(ite, struct dentry, d_alias);

		if (dentry->d_count != 0){
        		return 1;
        	}

        	ite = ite->next;
    	}
    	return 0;
}

static int set_inode_evicted(struct inode* inode,int e)
{
	int ret;
	int evicted = e;
	ret = ext2_xattr_set(inode, EXT2_XATTR_INDEX_TRUSTED,"evicted",&evicted,4,XATTR_REPLACE);
	if(ret < 0){
		int eevicted = e;
		ret = ext2_xattr_set(inode,EXT2_XATTR_INDEX_TRUSTED,"evicted",&eevicted,4,XATTR_CREATE);
		if(ret<0){
			return ret;
		}
	}
	return 0;
}

int inode_evicted(struct inode *inode)
{
	int ret;
	int evicted = 0;
	size_t size = 0;

	ret = ext2_xattr_get(inode,EXT2_XATTR_INDEX_TRUSTED,"evicted",(void*)&evicted,size);
	if( ret < 0 ){
		set_inode_evicted(inode,0);
		return 0;
	}
	
	return evicted;
}

int ext2_get_usage(struct super_block *super);
int ext2_evict ( struct inode * i_node );
void ext2_judge_evict(struct inode *inode,void *arg)
{
	printk("ext2_judge_evict!\n");	
	int value;
	int ret;
	if (inode->i_mode&S_IFDIR){
		printk("inode : %d is a dir\n",inode->i_ino);        	
		return;
    	}
	if (inode_using(inode)){
		printk("inode : %d is in using\n",inode->i_ino);
        	return;
    	}
	ret = ext2_get_usage(inode->i_sb);
	if( ret < evict_target){
		printk("usage : %d < evict_target : %d\n",ret,evict_target);
		return;
	}
	ext2_evict(inode);
}	

static int clock_hand(struct super_block *super)
{
	printk("clock_hand!\n");	
	for_each_inode(super,ext2_judge_evict,NULL);
	return 0;
}

int ext2_get_usage(struct super_block *super)
{
	struct ext2_sb_info *sbi = EXT2_SB(super);
	struct ext2_super_block *es = sbi->s_es;

	unsigned long total = es->s_blocks_count;
	unsigned long used = total - ext2_count_free_blocks(super);

	return (100 * (used)) / total;
	
	//return 0;
}

int ext2_evict_fs(struct super_block *super)
{
	printk("ext2_evict_fs!\n");
	int ret;
	ret = ext2_get_usage(super);
	printk("ext2_get_usage : %d\n",ret);
	
	if(ret >= low_watermark){
		clock_hand(super);
	}

	return 0;
}


 
/* The call should return when the file is evicted . Besides
 * the file data pointers , no other metadata , e . g . , access time ,
 * size , etc . should be changed . Appropriate errors should
 * be returned . In particular , the operation should fail if the
 * inode currently maps to an open file . Lock the inode
 * appropriately to prevent a file open operation on it while
 * it is being evicted .
 */
int ext2_evict ( struct inode * i_node )
{
	printk("ext2_evict!\n");	
	unsigned long i_num;
	loff_t size;
	int ret;
	int evicted;
	mm_segment_t oldfs;

	i_num = i_node -> i_ino;
	size = i_node -> i_size;

	if(size == 0){
		printk("inode : %d 's size = 0 \n",i_node->i_ino);
		return -1;
	}
	
	evicted = inode_evicted(i_node);
	if(unlikely(evicted < 0)){
		return evicted;
	}
	if(evicted == 1){
		printk("inode : %d is evicted!\n");
		return -1;
	}
	
	oldfs=get_fs();
	set_fs(KERNEL_DS);
	
	//printk("get_filename...\n");
	//set_filename(i_node);
	//printk("filename : %s\n",filename);
	save_i_data(i_node);

	ret = clfs_put(filename,i_num,size);
	if(unlikely(ret < 0)){
		printk("ext2_evict failed!\n");
	}
	
	rm_diskfile(filename);
	recover_i_data(i_node);
	
	evicted = 1;
	set_inode_evicted(i_node,evicted);
	
	set_fs(oldfs);
	
	return 0;
}

/* Fetch the file specified by i_node from the cloud server .
 * The function should allocate space for the file on the local
 * filesystem . No other metadata of the file should be changed .
 * Lock the inode appropriately to prevent concurrent fetch
 * operations on the same inode , and return appropriate errors .
 */
int ext2_fetch ( struct inode * i_node )
{
	unsigned long i_num;
	loff_t size;
	int ret;
	int evicted;

	i_num = i_node -> i_ino;
	size = i_node -> i_size;

	//set_filename(i_node);

	save_i_data(i_node);
	
	ret = clfs_get(filename,i_num, size);
	if(unlikely(ret < 0)){
		printk("clfs_get failed!\n");
	}

	recover_i_data(i_node);
	
	evicted = 0;
	set_inode_evicted(i_node,evicted);

	return 0;	
}
