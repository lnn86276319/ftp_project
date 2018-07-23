#include"factory.h"

int factory_init(pfac_t pf,thread_func_t thread_func,int thread_num,int capacity)
{
	pf->pthread_id=(pthread_t*)calloc(thread_num,sizeof(pthread_t));
	for(int i=0;i<thread_num;++i)
	{
		pthread_create(pf->pthread_id+i,NULL,thread_func,(void*)pf);
	}
	pf->thread_num=thread_num;
	pf->pthread_func=thread_func;
	pf->que.que_capacity=capacity;
	pthread_mutex_init(&pf->que.que_mutex,NULL);
	pthread_cond_init(&pf->que.que_cond,NULL);
	return 0;
}

void que_set(pque_t pq,pnode_t pnew)
{
	if(!pq->que_head)
	{
		pq->que_head=pnew;
		pq->que_tail=pnew;
	}
	else
	{
		pq->que_tail->pNext=pnew;
		pq->que_tail=pnew;
	}
	++pq->que_size;
}

void que_get(pque_t pq,pnode_t* pcur)
{
	*pcur=pq->que_head;
	if(pq->que_head)
	{
		if(-1==(*pcur)->new_sfd)
		{
			pthread_mutex_unlock(&pq->que_mutex);
			pthread_exit(0);
		}
		pq->que_head=pq->que_head->pNext;
		--pq->que_size;
	}
}

