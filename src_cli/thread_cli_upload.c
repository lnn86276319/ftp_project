#include"factory.h"

/*客户端上传文件*/

void* thread_cli_upload(void* p)
{
	pload_t pUp=(pload_t)p;
	int fd;
	struct stat fmsg;
	struct trans bag;

	/*打开文件*/

	fd=open(pUp->file_name,O_RDONLY);
	if(-1==fd)
	{
		printf("file open fail\n");
		pthread_exit((void*)-1);
	}

	/*发送文件大小*/
    
	bzero(&fmsg,sizeof(struct stat));
	fstat(fd,&fmsg);
	bzero(&bag,sizeof(struct trans));
	bag.len=sizeof(off_t);
	memcpy(bag.buf,&fmsg.st_size,sizeof(off_t));
	send(pUp->sfd,(char*)&bag,4+bag.len,0);

	/*计算MD5并发送*/

	bzero(&bag,sizeof(struct trans));
	Compute_file_md5(pUp->file_name,bag.buf);
	bag.len=strlen(bag.buf);
	send(pUp->sfd,(char*)&bag,4+bag.len,0);

	/*根据回复，再决定是否发送*/

	bzero(&bag,sizeof(struct trans));
	recv(pUp->sfd,(char*)&bag.len,4,0);
	recv(pUp->sfd,bag.buf,bag.len,0);
	if(strcmp(bag.buf,"filename repeat")==0||strcmp(bag.buf,"file exist")==0)
	{
		close(fd);
		bzero(&bag,sizeof(struct trans));
		recv(pUp->sfd,bag.buf,PATH_MAX,0);
		printf("%s\n",bag.buf);
		pthread_exit((void*)-1);
	}

	/*发送文件*/

	sendfile(pUp->sfd,fd,NULL,fmsg.st_size);
	close(fd);
	pthread_exit((void*)0);
}
