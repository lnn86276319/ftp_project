#include"factory.h"

char* analyse_cmd(char* buf)
{
	int i,j;
	char* cmd=(char*)calloc(10,sizeof(char));
	for(i=0,j=0;i<strlen(buf);++i)
	{
		if(buf[i]!=' ')
		{
			cmd[j++]=buf[i];
		}
		else
		{
			break;
		}
	}
	return cmd;

}

int main(int argc,char** argv)
{
	ARGS_CHECK(3,argc);

	/*数据结构*/

	char* cmd,* password;
	int ret,sfd,epfd,nfds,i;
	char buf[PATH_MAX]={0};
	struct epoll_event eve,evs[2];

	/*tcp连接初始化*/

	sfd=InetConnect(argv[1],argv[2]);
	ERROR_CHECK(-1,sfd,"InetConnect");

	/*epoll多路复用*/

	bzero(&eve,sizeof(struct epoll_event));
	epfd=epoll_create(1);
	eve.events=EPOLLIN;
	eve.data.fd=sfd;
	ret=epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&eve);
	ERROR_CHECK(-1,ret,"epoll_ctl");
	eve.data.fd=STDIN_FILENO;
	ret=epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&eve);
	ERROR_CHECK(-1,ret,"epoll_ctl");
	while(1)
	{
		nfds=epoll_wait(epfd,evs,2,-1);
		for(i=0;i<nfds;++i)
		{
			if(evs[i].data.fd==sfd)
			{
				bzero(buf,PATH_MAX);
				ret=recv(sfd,buf,PATH_MAX,0);
				if(0==ret)
				{
					printf("lose connect\n");
					return -1;
				}
				if(strcmp(buf,"username_right")==0||strcmp(buf,"password_error")==0)
				{
					password=getpass("Password:");
					send(sfd,password,strlen(password),0);
					bzero(password,strlen(password));
				}
				else if(ret>0)
				{
					puts(buf);
				}
			}
			if(evs[i].data.fd==STDIN_FILENO)
			{
				bzero(buf,PATH_MAX);
				ret=read(STDIN_FILENO,buf,PATH_MAX);
				buf[ret-1]=0;
				ret=send(sfd,buf,strlen(buf),0);
				cmd=analyse_cmd(buf);
				if(strcmp(cmd,"puts")==0)
				{
					int i=0;
					load_t Up;
					bzero(&Up,sizeof(load_t));
					Up.sfd=sfd;
					while(buf[4+i]!=0&&buf[4+i]==' ')
					{
						++i;
					}
					strncpy(Up.file_name,buf+4+i,strlen(buf)-(4+i));
					if(*Up.file_name==0)
					{
						break;
					}
					pthread_t pth_id;
					pthread_create(&pth_id,NULL,thread_cli_upload,(void*)&Up);
					pthread_join(pth_id,NULL);
				}
				else if(strcmp(cmd,"gets")==0)
				{
					int i=0;
					load_t Down;
					Down.sfd=sfd;
					while(buf[4+i]!=0&&buf[4+i]==' ')
					{
						++i;
					}
					strncpy(Down.file_name,buf+4+i,strlen(buf)-(4+i));
					if(*Down.file_name==0)
					{
						break;
					}
					pthread_t pth_id;
					pthread_create(&pth_id,NULL,thread_cli_download,(void*)&Down);
					pthread_join(pth_id,NULL);
				}
				free(cmd);
			}
		}
	}
	close(sfd);
	return 0;
}
