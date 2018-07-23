#ifndef __WORK_QUE_H__
#define __WORK_QUE_H__
#include"head.h"

typedef struct load
{
	int sfd;
	pnode_t pcur;
	char file_name[PATH_MAX];
}load_t,*pload_t;

typedef struct work_que
{
	pnode_t que_head,que_tail; //指向任务链表的头尾指针
	int que_capacity;          //可容纳的最大任务数
	int que_size;              //当前任务数
	pthread_mutex_t que_mutex; //所有线程都要通过这个队列进行数据拿取
	pthread_cond_t que_cond;   //条件变量当有任务时唤醒线程
}que_t,*pque_t;

void que_set(pque_t,pnode_t);   //添加新任务

void que_get(pque_t,pnode_t*);  //拿取任务

#endif
