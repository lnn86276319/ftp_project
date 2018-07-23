#include"factory.h"

int login_check(pnode_t pcur)
{
	int ret;
	char* username,* encrypted;
	char password[256]={0};
	char spassword[256]={0};
	char Salt[32]={0};
	long lnmax;
	lnmax=sysconf(_SC_LOGIN_NAME_MAX);
	if(-1==lnmax)
	{
		lnmax=256;
	}
    username=(char*)calloc(lnmax,sizeof(char));  /*注意free*/
	if(NULL==username)
	{
		printf("calloc fail\n");
        return -1;		
	}
	send(pcur->new_sfd,"Login as:",10,0);
	while(1)
	{
		ret=recv(pcur->new_sfd,username,lnmax,0);
		if(0==ret)
		{
			printf("lost connect\n");
			return -1;
        }
		if(mysql_login_check(username,spassword,Salt))
		{
			send(pcur->new_sfd,"user not exist,try again:",26,0);
			bzero(username,lnmax);
		}
		else
		{
			send(pcur->new_sfd,"username_right",15,0);  /*用于激活密码输入*/
		    break;
		}
	}
	strcpy(pcur->username,username);
	mysql_set_root(pcur);  /*在这里寻找该用户的根目录的node值*/
	while(1)
	{
		ret=recv(pcur->new_sfd,password,256,0);
		if(0==ret)
		{
			printf("lost connect\n");
			return -1;
		}
        encrypted=crypt(password,Salt);
		bzero(password,256);
		if(NULL==encrypted)
		{
			printf("encrypted error\n");
		}
		if(strcmp(encrypted,spassword)==0)
		{
			send(pcur->new_sfd,"Login Success",14,0);
			break;
		}
		send(pcur->new_sfd,"password error,try again",26,0);
        send(pcur->new_sfd,"password_error",15,0); /*这句是用来激活继续输入密码的*/
	}
	free(username);
	return 0;
}
