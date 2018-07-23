#include"factory.h"

int handle_task(pnode_t pcur)
{
	int ret;
	char cmd_buf[PATH_MAX]={0};
	char* cmd;
	while(1)
	{
		bzero(cmd_buf,PATH_MAX);
		ret=recv(pcur->new_sfd,cmd_buf,PATH_MAX,0);
		if(0==ret)
		{
			printf("client lost connect\n");
			return -1;
		}
		mysql_operate_log(pcur,cmd_buf);
		if(ret>0)
		{
			if(strcmp(cmd_buf,"pwd")==0)
			{
				cmd_pwd(pcur);
			}
			else if(strcmp(cmd_buf,"ls")==0)
			{
				cmd_ls(pcur);
			}
			else
			{
				cmd=analyse_cmd(cmd_buf);
				if(strcmp(cmd,"cd")==0)
				{
					cmd_cd(pcur,cmd_buf);
				}
				else if(strcmp(cmd,"remove")==0)
				{
					cmd_remove(pcur,cmd_buf);
				}
				else if(strcmp(cmd,"puts")==0)
				{
					cmd_puts(pcur,cmd_buf);
				}
				else if(strcmp(cmd,"gets")==0)
				{
					cmd_gets(pcur,cmd_buf);
				}
				else
				{
					send(pcur->new_sfd,"error cmd",10,0);
				}
				free(cmd);
			}
		}
	}
	return 0;
}

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

/*pwd*/

void cmd_pwd(pnode_t pcur)
{
	int dir_node=pcur->dir;  /*暂存当前所在目录*/
	char path[PATH_MAX]={0};
	while(pcur->dir!=0)
	{
		mysql_pwd(pcur,path);
	}
	pcur->dir=dir_node;    /*还原*/
	send(pcur->new_sfd,path,strlen(path),0);
}

/*ls*/

void cmd_ls(pnode_t pcur)
{
	mysql_ls(pcur);
}

/*cd:只响应cd .. 与 cd 文件名*/

void cmd_cd(pnode_t pcur,char* buf)
{
	if(strlen(buf)<3)
	{
		send(pcur->new_sfd,"error cmd",10,0);
		return ;
	}
	int i=0;
	char file_name[PATH_MAX]={0};
	while(buf[2+i]!=0&&buf[2+i]==' ')    /*跳过所有空格*/
	{
		++i;
	}
	strncpy(file_name,buf+2+i,strlen(buf)-(2+i));
	if(*file_name==0)
	{
		send(pcur->new_sfd,"error cmd",10,0);
		return ;
	}
	if(strcmp(file_name,"..")==0)
	{

		/*在数据库查找上层目录,视情况更新*/

		mysql_find_predir(pcur);
		return ;
	}
	else
	{
		i=0;
		while(file_name[i]!=0) /*不接受绝对路径和连续跳跃*/
		{
			if(file_name[i]=='/'||file_name[i]=='.')
			{
				send(pcur->new_sfd,"error cmd",10,0);
				return ;
			}
			++i;
		}
		mysql_find_nextdir(pcur,file_name);   /*查找下层目录*/
	}
}

/*remove 只能删除当前目录下文件，不接受绝对路径*/

void cmd_remove(pnode_t pcur,char* buf)
{
	if(strlen(buf)<7)
	{
		send(pcur->new_sfd,"error cmd",10,0);
		return ;
	}
	int i=0,ret,node;
	char file_name[PATH_MAX]={0};
	while(buf[6+i]!=0&&buf[6+i]==' ')    /*跳过所有空格*/
	{
		++i;
	}
	strncpy(file_name,buf+6+i,strlen(buf)-(6+i));
	if(*file_name==0)
	{
		send(pcur->new_sfd,"error cmd",10,0);
		return ;
	}
	node=pcur->dir;
	ret=mysql_find_remove(pcur,file_name);
	if(0==ret) /*要删除的是目录*/
	{
		mysql_check_remove(pcur);
	}
	pcur->dir=node;
}

/*puts*/

void cmd_puts(pnode_t pcur,char* buf)
{
	if(strlen(buf)<5)
	{
    	send(pcur->new_sfd,"error cmd",10,0);
        return ;
	}
	int i=0;
	load_t Up;
	bzero(&Up,sizeof(load_t));
	Up.pcur=pcur;
	while(buf[4+i]!=0&&buf[4+i]==' ')
	{
		++i;
	}
	strncpy(Up.file_name,buf+4+i,strlen(buf)-(4+i));
	if(*Up.file_name==0)
	{
    	send(pcur->new_sfd,"error cmd",10,0);
		return ;
	}
	pthread_t pth_id;
	pthread_create(&pth_id,NULL,thread_ser_upload,(void*)&Up);
	pthread_join(pth_id,NULL);
}

/*gets*/

void cmd_gets(pnode_t pcur,char* buf)
{
	if(strlen(buf)<5)
	{
    	send(pcur->new_sfd,"error cmd",10,0);
        return ;
	}
	int i=0;
	load_t Down;
	bzero(&Down,sizeof(load_t));
	Down.pcur=pcur;
	while(buf[4+i]!=0&&buf[4+i]==' ')
	{
		++i;
	}
	strncpy(Down.file_name,buf+4+i,strlen(buf)-(4+i));
	if(*Down.file_name==0)
	{
    	send(pcur->new_sfd,"error cmd",10,0);
		return ;
	}
	pthread_t pth_id;
	pthread_create(&pth_id,NULL,thread_ser_download,(void*)&Down);
	pthread_join(pth_id,NULL);
}
