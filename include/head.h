#ifndef __HEAD_H__
#define __HEAD_H__
#include<openssl/md5.h>
#include<mysql/mysql.h>
#include<sys/sendfile.h>
#include<sys/uio.h>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<sys/select.h>
#include<sys/time.h>
#include<sys/mman.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/msg.h>
#include<unistd.h>
#include<dirent.h>
#include<stdio.h>
#include<string.h>
#include<strings.h>
#include<stdlib.h>
#include<fcntl.h>
#include<syslog.h>
#include<time.h>
#include<ctype.h>
#include<signal.h>
#include<pthread.h>
#include<pwd.h>
#include<grp.h>
#include<shadow.h>
#include<crypt.h>
#include<errno.h>

#define ARGS_CHECK(a,b) {if(a!=b){printf("ERROR ARGS\n"); return -1;}}
#define ERROR_CHECK(r_num,ret,name){if(r_num==ret){perror(name);return -1;}}
#define PTHREAD_ERROR_CHECK(ret,name){if(ret){printf("%s error %d\n",name,ret);exit(0);}}

typedef struct trans
{
	int len;
	char buf[PATH_MAX];
}trans_t,*ptrans_t;

typedef struct tag_node
{
	int new_sfd;
	int dir;
	char username[16];
	struct tag_node* pNext;
}node_t,*pnode_t;

int InetListen(char*,char*,int);

int InetConnect(char*,char*);

int Compute_file_md5(const char*,char*);

int mysql_login_log(struct sockaddr_in*);

int mysql_login_check(char*,char*,char*);

int mysql_set_root(pnode_t);

int mysql_operate_log(pnode_t,char*);

int mysql_pwd(pnode_t,char*);

int mysql_ls(pnode_t);

int mysql_find_predir(pnode_t);

int mysql_find_nextdir(pnode_t,char*);

int mysql_create_dir(pnode_t,char*);

int mysql_find_remove(pnode_t,char*);

int mysql_check_remove(pnode_t);

int mysql_remove(pnode_t);

int mysql_check_upload(pnode_t,char*);

int mysql_insert_upload(pnode_t,char*,off_t,char*);

int mysql_check_download(pnode_t,char*);

#endif
