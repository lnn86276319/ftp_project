#include<head.h>
//进行网络连接的封装

/*服务器端网络初始化*/

int InetListen(char* IPv4,char* Port,int backlog)
{
	/*数据结构*/

	int sfd,ret,optval;
	struct sockaddr_in ser;

	/*连接初始化*/

	bzero(&ser,sizeof(struct sockaddr));   /*初始化ser*/
	ser.sin_family=AF_INET;                /*IPv4*/
	ser.sin_port=htons(atoi(Port));        /*端口*/
	ser.sin_addr.s_addr=inet_addr(IPv4);   /*地址*/
	sfd=socket(AF_INET,SOCK_STREAM,0);     /*选择IPv4与TCP*/
	ERROR_CHECK(-1,sfd,"socket");
	optval=1;

	/*如果在已经处于 ESTABLISHED 状态下的 socket(一般由端口号和标志符区分）调用 closesocket（一般不会立即关闭而经历 TIME_WAIT 的过程）后想继续重用该 socket*/

	ret=setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,(const char*)&optval,sizeof(int));
	ERROR_CHECK(-1,ret,"setsockopt");
	ret=bind(sfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
	ERROR_CHECK(-1,ret,"bind");
	ret=listen(sfd,backlog);
	ERROR_CHECK(-1,ret,"listen");
	return sfd;
}

/*客户端网络初始化*/

int InetConnect(char* IPv4,char* Port)
{
	/*数据结构*/

	int sfd,ret;
	struct sockaddr_in ser;

	/*连接初始化*/

	bzero(&ser,sizeof(struct sockaddr));   /*初始化ser*/
	ser.sin_family=AF_INET;                /*IPv4*/
	ser.sin_port=htons(atoi(Port));        /*端口*/
	ser.sin_addr.s_addr=inet_addr(IPv4);   /*地址*/
	sfd=socket(AF_INET,SOCK_STREAM,0);     /*选择IPv4与TCP*/
	ERROR_CHECK(-1,sfd,"socket");
    ret=connect(sfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
	ERROR_CHECK(-1,ret,"connect");
	return sfd;
}
