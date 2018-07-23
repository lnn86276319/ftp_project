#include"factory.h"

void* thread_ser_echo(void* p)
{
	int ret;
	pnode_t pcur;
	pque_t pq;
	pfac_t pf;
	pf=(pfac_t)p;
	pq=&pf->que;
	while(1)
	{
		pthread_mutex_lock(&pq->que_mutex);
		if(0==pq->que_size)
		{
			pthread_cond_wait(&pq->que_cond,&pq->que_mutex);
		}
		que_get(pq,&pcur);
		pthread_mutex_unlock(&pq->que_mutex);
		if(pcur)
		{
			ret=login_check(pcur);   /*登录验证函数*/
			if(0==ret)
			{
				handle_task(pcur);  /*登录成功后响应客户端请求*/
			}
			else
			{
				send(pcur->new_sfd,"Login fail",11,0);
			}
			close(pcur->new_sfd);
			free(pcur);
		}
	}
	pthread_exit((void*)0);
}

