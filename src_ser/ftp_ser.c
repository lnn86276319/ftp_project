#include"factory.h"

//服务器端

int main(int argc,char** argv)
{
	ARGS_CHECK(5,argc);  /*检查传入参数是否正确*/

	/*数据结构*/

	int fds[2];
	int ret,sfd,epfd,nfds,i,new_sfd;
	int thread_num=atoi(argv[3]);
	int capacity=atoi(argv[4]);
	pnode_t pnew;
	pque_t pq;
	pfac_t pf;           /*定义工厂变量*/
	struct sockaddr_in cli;
	socklen_t addrlen=sizeof(struct sockaddr);
	struct epoll_event eve,evs[2];

	/*创建进程*/

	pipe(fds);
	close(fds[1]);   /*子进程保留读*/
	if(fork())
	{
		close(fds[0]); /*父进程保留写*/
		//退出机制
		wait(NULL);
		return 0;
	}

	/*初始化线程池*/

	pf=(pfac_t)calloc(1,sizeof(fac_t));
	factory_init(pf,thread_ser_echo,thread_num,capacity);
	pq=&pf->que;

	/*建立tcp连接*/

	sfd=InetListen(argv[1],argv[2],capacity);
	ERROR_CHECK(-1,sfd,"InetListen");

	/*epoll多路复用*/
	bzero(&eve,sizeof(struct epoll_event));
	epfd=epoll_create(1);
	ERROR_CHECK(-1,epfd,"epoll_create");
	eve.events=EPOLLIN;
	eve.data.fd=sfd;
	ret=epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&eve);
	ERROR_CHECK(-1,ret,"epoll_ctl");
	eve.data.fd=fds[0];
	ret=epoll_ctl(epfd,EPOLL_CTL_ADD,fds[0],&eve);
	ERROR_CHECK(-1,ret,"epoll_ctl");
	while(1)
	{
		nfds=epoll_wait(epfd,evs,2,-1);
		for(i=0;i<nfds;++i)
		{
			if(evs[i].data.fd==sfd)
			{
				new_sfd=accept(sfd,(struct sockaddr*)&cli,&addrlen);
				
				/*只要有连接，就记录到数据库ftp表LoginLog里*/

				mysql_login_log(&cli);

				if(pq->que_size<capacity)
				{
					pnew=(pnode_t)calloc(1,sizeof(node_t));
					pnew->new_sfd=new_sfd;
					pthread_mutex_lock(&pq->que_mutex);
					que_set(pq,pnew);
					pthread_mutex_unlock(&pq->que_mutex);
					pthread_cond_signal(&pq->que_cond);
				}
				else
				{
					printf("task full,request denied\n");
				}
			}
			if(evs[i].data.fd==fds[0])
			{}
		}
	}
	return 0;
}
