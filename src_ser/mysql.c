#include"factory.h"

#define SERVER   "localhost"
#define USER     "root"
#define PASSWORD "951124"
#define DATABASE "ftp"

/*日志记录*/

int mysql_login_log(struct sockaddr_in* pcli)
{
	MYSQL* conn=NULL;
	char IPv4[20]={0};
	inet_ntop(AF_INET,&pcli->sin_addr,IPv4,16);
	char query[512]="insert into LoginLog(IP) values";
	sprintf(query,"%s('%s')",query,IPv4);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	mysql_close(conn);
	return 0;
}

/*登录验证*/

int mysql_login_check(char* username,char* spassword,char* Salt)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW  row;
	int numrows;
	char query[512]="select Password,Salt from UserData where UserName=";
	sprintf(query,"%s'%s'",query,username);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
    res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		if(0==numrows)
		{
			mysql_free_result(res);
			mysql_close(conn);
			return -1;
		}
		else
		{
			row=mysql_fetch_row(res);
            strcpy(spassword,row[0]);
			strcpy(Salt,row[1]);
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
	}
	else
	{
		printf("query error\n");
		mysql_close(conn);
		return -1;
	}
}

/*在已有用户名的信息条件下取得该用户根目录结点node值*/

int mysql_set_root(pnode_t pcur)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int numrows;
	char query[512]="select node from FilePool where ";
	sprintf(query,"%sprenode=0 and UserName='%s'",query,pcur->username);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
    res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		if(0==numrows)
		{
			mysql_free_result(res);
			mysql_close(conn);
			return -1;
		}
		else
		{
			row=mysql_fetch_row(res);
            pcur->dir=atoi(row[0]);
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
	}
	else
	{
		printf("mysql_store_result error\n");
		mysql_close(conn);
		return -1;
	}
}

/*对登陆用户进行操作记录*/

int mysql_operate_log(pnode_t pcur,char* cmd_buf)
{
	MYSQL* conn;
	char query[512]="insert into OperateLog(UserName,Operate) values";
	sprintf(query,"%s('%s','%s')",query,pcur->username,cmd_buf);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	mysql_close(conn);
	return 0;
}

/*在已知用户名和当前目录node的情况下查找上层目录并视情况更新*/

int mysql_find_predir(pnode_t pcur)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int prenode,numrows;
	char query[512]="select prenode from FilePool where ";
	sprintf(query,"%snode=%d",query,pcur->dir);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
    res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		if(0==numrows)
		{
			mysql_free_result(res);
			mysql_close(conn);
			return -1;
		}
		else
		{
			row=mysql_fetch_row(res);
            prenode=atoi(row[0]);
			if(0==prenode) /*当前目录为root不能cd*/
			{
				send(pcur->new_sfd,"It is root,can not cd",24,0);
			}
			else        /*普通目录更新为上一级*/
			{
				send(pcur->new_sfd,"cd success",11,0);
				pcur->dir=prenode;
			}
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
	}
	else
	{
		printf("mysql_store_result error\n");
		return -1;
	}
}

/*进入下层目录，如果目录不存在，询问是否则创建目录，是则创建并更新node，否返回信息*/

int mysql_find_nextdir(pnode_t pcur,char* dir_name)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int numrows;
	char chose[8]={0};
	char query[512]="select node from FilePool where ";
	sprintf(query,"%sprenode=%d and FileName='%s' and FileType='d'",query,pcur->dir,dir_name);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
    res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		if(0==numrows) /*未找到相关目录，询问是否创建但不进入*/
		{
			mysql_free_result(res);
			send(pcur->new_sfd,"No such directory,do you want to create?[y/n]",46,0);
			int ret;
			while(1)
			{
				bzero(chose,8);
				ret=recv(pcur->new_sfd,chose,7,0);
				if(-1==ret)
				{
					printf("lost connect\n");
					mysql_close(conn);
					return -1;
				}
				if(strcmp(chose,"y")==0||strcmp(chose,"Y")==0)
				{
					mysql_create_dir(pcur,dir_name);
					mysql_close(conn);
					return 0;
				}
				else if(strcmp(chose,"n")==0||strcmp(chose,"N")==0)
				{
					mysql_close(conn);
					return 0;
				}
				else
				{
					send(pcur->new_sfd,"error input,try again:[y/n]",28,0);
				}
			}
		}
		else
		{
			row=mysql_fetch_row(res);
			pcur->dir=atoi(row[0]);
			send(pcur->new_sfd,"cd success",11,0);
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
	}
	else
	{
		printf("mysql_store_result error\n");
		mysql_close(conn);
		return -1;
	}
}

/*创建目录*/

int mysql_create_dir(pnode_t pcur,char* dir_name)
{
	MYSQL* conn;
	char query[512]="insert into FilePool(prenode,UserName,FileName,FileType) ";
	sprintf(query,"%svalues(%d,'%s','%s','d')",query,pcur->dir,pcur->username,dir_name);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	send(pcur->new_sfd,"create success",15,0);
	mysql_close(conn);
	return 0;
}

/*显示当前目录所有文件*/

int mysql_ls(pnode_t pcur)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int numrows;
	char query[512]="select FileType,FileName,FileSize from FilePool where prenode=";
	sprintf(query,"%s%d",query,pcur->dir);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
    res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		if(0==numrows)
		{
			send(pcur->new_sfd,"empty",6,0);
			mysql_free_result(res);
			mysql_close(conn);
			return 0; 
		}
		else
		{
			while((row=mysql_fetch_row(res))!=NULL)
			{
				bzero(query,512);
				for(int i=0;i<3;++i)
				{
					sprintf(query,"%s%s      ",query,row[i]);
				}
				send(pcur->new_sfd,query,strlen(query),0);
			}
			mysql_close(conn);
			return 0;
		}
	}
	else
	{
		printf("mysql_store_result error\n");
		mysql_close(conn);
		return -1;
	}
}

/*查看当前虚拟路径*/

int mysql_pwd(pnode_t pcur,char* path)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int numrows;
	char new_path[PATH_MAX]={0};
	strcpy(new_path,path);
	char query[512]="select prenode,FileName from FilePool where node=";
    sprintf(query,"%s%d",query,pcur->dir);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
    res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		if(0==numrows)
		{
			mysql_free_result(res);
			mysql_close(conn);
			return -1;
		}
		else
		{
			row=mysql_fetch_row(res);
            pcur->dir=atoi(row[0]);
			sprintf(path,"%s/%s",(pcur->dir==0?"":row[1]),new_path);
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
	}
	else
	{
		printf("mysql_store_result error\n");
		mysql_close(conn);
		return -1;
	}
}

/*先要找到目标，这个目标可能是目录也可能是文件,进入该函数前保存当前目录结点值*/

int mysql_find_remove(pnode_t pcur,char* file_name)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int numrows;
	char query[512]="select node,FileType from FilePool where ";
	sprintf(query,"%sprenode=%d and FileName='%s'",query,pcur->dir,file_name); 
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -2;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -2;
	}
    res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		if(0==numrows)   /*要删除的文件不在当前目录*/
		{
			send(pcur->new_sfd,"No such file or directory",26,0);
			mysql_free_result(res);
			mysql_close(conn);
			return -1;
		}
		else
		{
			row=mysql_fetch_row(res);
			pcur->dir=atoi(row[0]);   /*注意备份，操作完成需回到原来结点*/
			if(strcmp(row[1],"d")==0)
			{
				mysql_free_result(res);
				mysql_close(conn);
				return 0;
			}
			mysql_remove(pcur);
			mysql_free_result(res);
			mysql_close(conn);
			return 1;
		}
	}
	else
	{
		printf("mysql_store_result error\n");
		mysql_close(conn);
		return -2;
	}
}

/*若删除的是目录，需要进行二次判断，空的才能删*/

int mysql_check_remove(pnode_t pcur)
{
	MYSQL* conn;
	MYSQL_RES* res;
	int numrows;
	char query[512]="select * from FilePool where prenode=";
	sprintf(query,"%s%d",query,pcur->dir);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		if(0==numrows) /*这个目录为空，可删*/
		{
			mysql_remove(pcur);
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
		else
		{
			send(pcur->new_sfd,"The directory is not empty",27,0);
			mysql_free_result(res);
			mysql_close(conn);
			return -1;
		}
    }
	else
	{
		printf("mysql_store_result error\n");
		mysql_close(conn);
		return -1;
	}
}

/*删除文件或空目录，不接受绝对路径*/

int mysql_remove(pnode_t pcur)
{
	MYSQL* conn;
	char query[512]="delete from FilePool where node=";
	sprintf(query,"%s%d",query,pcur->dir);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	send(pcur->new_sfd,"remove success",15,0);
	mysql_close(conn);
	return 0;
}

/*服务器在接收客户端上传文件时需要检查该用户此目录是否已存在该文件，不允许重名*/

int mysql_check_upload(pnode_t pcur,char* file_name)
{
	MYSQL* conn;
	MYSQL_RES* res;
	int numrows;
	char query[512]="select * from FilePool where ";
	sprintf(query,"%sprenode=%d and FileName='%s'",query,pcur->dir,file_name);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		mysql_free_result(res);
		if(0==numrows)  /*目录没有重名文件*/
		{
			mysql_close(conn);
			return 0;
		}
		else
		{
			mysql_close(conn);
			return -1;
		}
	}
	else
	{
		printf("mysql_store_result error\n");
		mysql_close(conn);
		return -1;
	}
}

/*插入文件信息*/

int mysql_insert_upload(pnode_t pcur,char* file_name,off_t file_size,char* MD5)
{
	MYSQL* conn;
	char query[512]="insert into FilePool(prenode,UserName,FileName,FileType,FileSize,MD5) ";
	sprintf(query,"%svalues(%d,'%s','%s','-',%ld,'%s')",query,pcur->dir,pcur->username,file_name,file_size,MD5);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	mysql_close(conn);
	return 0;
}

/*下载前检查是否存在文件以及是否是目录*/

int mysql_check_download(pnode_t pcur,char* file_name)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int numrows;
	char query[512]="select FileType,MD5 from FilePool where ";
	sprintf(query,"%sprenode=%d and FileName='%s'",query,pcur->dir,file_name);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,SERVER,USER,PASSWORD,DATABASE,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	if(mysql_real_query(conn,query,strlen(query)))
	{
		printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	res=mysql_store_result(conn);
	if(res)
	{
		numrows=mysql_num_rows(res);
		if(0==numrows) /*当前目录没有这个文件*/
		{
			mysql_free_result(res);
			mysql_close(conn);
			return -1;
		}
		else
		{
			row=mysql_fetch_row(res);
			if(strcmp(row[0],"d")==0) /*要下载的是目录*/
			{
				mysql_free_result(res);
				mysql_close(conn);
				return 1;
			}
			else   /*存在文件且非目录*/
			{
				bzero(file_name,PATH_MAX);
				strcpy(file_name,row[1]);
				mysql_free_result(res);
				mysql_close(conn);
				return 0;
			}
		}
	}
	else
	{
		printf("mysql_store_result error\n");
		return -1;
	}
}
