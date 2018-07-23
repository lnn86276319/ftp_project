#ifndef __FACTORY_H__
#define __FACTORY_H__
#include"head.h"
#include"work_que.h"

typedef void* (*thread_func_t)(void*);

typedef struct factory
{
	pthread_t* pthread_id;  //用于存储线程id，使用时需分配空间
    que_t que;              //任务队列
	int thread_num;
	thread_func_t pthread_func;
}fac_t,*pfac_t;

int factory_init(pfac_t,thread_func_t,int,int);

void* thread_ser_echo(void*);

void* thread_ser_upload(void*);

void* thread_ser_download(void*);

void* thread_cli_upload(void*);

void* thread_cli_download(void*);

int login_check(pnode_t);

int handle_task(pnode_t);

char* analyse_cmd(char*);

void cmd_pwd(pnode_t);

void cmd_ls(pnode_t);

void cmd_cd(pnode_t,char*);

void cmd_remove(pnode_t,char*);

void cmd_puts(pnode_t,char*);

void cmd_gets(pnode_t,char*);
#endif
