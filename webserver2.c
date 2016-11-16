#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define MAXLINE 256
#define MAXSIZE 102400

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

loadpage(int newsd,char *page)
{
    int fsize,i=0,c;
    char out_buf[MAXSIZE];
    int fd,msg_ok,fail,fail1,req,ack;
    int no_read ,num_blks , num_blks1,num_last_blk,tmp;

fsize = 0;
FILE *fp;
fp=fopen(page,"r");
     while ((c = getc(fp)) != EOF) {fsize++;}


     write(newsd, "Content-length: ", 16);
     bzero(out_buf,MAXSIZE);
     sprintf(out_buf,"%d",fsize);
     write(newsd, out_buf, strlen(out_buf));
     write(newsd, "\n\n", 2);

     printf("%d",fsize);
     rewind(fp);
     bzero(out_buf,MAXSIZE);

     num_blks = fsize / MAXSIZE;
    num_blks1 = htons(num_blks);
    num_last_blk = fsize % MAXSIZE;

     for(i= 0; i < num_blks; i ++) {
      no_read = fread(out_buf,sizeof(char),MAXSIZE,fp);
      if (no_read == 0) {printf("server: file read error\n");exit(0);}
      if (no_read != MAXSIZE)
              {printf("server: file read error : no_read is less\n");exit(0);}
      if((write(newsd,out_buf,MAXSIZE)) < 0)
                 {printf("server: error sending block:%d\n",errno);exit(0);}
      printf(" %d...",i);
      }

   if (num_last_blk > 0) {
      printf("%d\n",num_blks);
      no_read = fread(out_buf,sizeof(char),num_last_blk,fp);
      if (no_read == 0) {printf("server: file read error\n");exit(0);}
      if (no_read != num_last_blk)
            {printf("server: file read error : no_read is less 2\n");exit(0);}
      if((write(newsd,out_buf,num_last_blk)) < 0)
                 {printf("server: file transfer error %d\n",errno);exit(0);}
      }
    else printf("\n");
     fclose(fp);
}

char* getmime(char *filename)
{
FILE *fp;
  char *mime=malloc(sizeof(char)*40);
    strcpy(mime,"file --mime-type -b ");
    strcat(mime,filename);
  fp = popen(mime, "r");
  if (fp == NULL) {
      printf("Failed to run command\n" );
      exit -1;
  }

  while (fgets(mime, 40, fp) != NULL) {
     // printf("%s", mime);
  }

  pclose(fp);
  return mime;

}

/* CHILD PROCEDURE, WHICH ACTUALLY DOES THE FILE TRANSFER */
doftp(int newsd,char *filename)
  {
    char fname[MAXLINE];

    FILE *fp;

     bzero(fname,MAXLINE);
     strcpy(fname,filename);


     /* IF SERVER CANT OPEN FILE THEN INFORM CLIENT OF THIS AND TERMINATE */
     //printf("%s",fname);
     if((fp = fopen(fname,"r")) == NULL) /*cant open file*/
     {
     write(newsd, "HTTP/1.1 404 NOT FOUND\n", 23);
     write(newsd, "Content-Type: application/x-php\n", 24);
     strcpy(fname,"404.php");
     }
     else
     {
     char *mimetype=getmime(filename);
     printf(mimetype);
     write(newsd, "HTTP/1.1 200 OK\n", 16);
     write(newsd, "Content-Type: ", 14);
     write(newsd,mimetype,strlen(mimetype));
     //write(newsd,"\n",1);
     }
     loadpage(newsd,fname);

   printf("server: FILE TRANSFER COMPLETE on socket %d\n",newsd);
   fclose(fp);
   close(newsd);
  }

char* getfilename(char *request)
{
char *url,*filename;
url=strtok(request," ");
url=strtok(NULL," ");
filename=strrchr(url,'/');
return filename;
}


int main(int argc, char *argv[])
{
    time_t mytime;
    int pid;

     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     char *filename;
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
while(1){
     newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");
     bzero(buffer,256);
     pid=fork();
     if(pid==0){
     printf("Connection Established--------\n");
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);

     /*bzero(buffer,256);
     strcpy(buffer,"Current Time: ");
      mytime = time(NULL);
     strcat(buffer,ctime(&mytime));*/
     /*n = write(newsockfd,buffer ,strlen(buffer));
     if (n < 0) error("ERROR writing to socket");*/
     filename=getfilename(buffer);
     filename++;
     doftp(newsockfd,filename);
printf("Page Transfered-------\n");
     close(newsockfd);
     close(sockfd);
     exit(0);
     }
     else
     {
     close(newsockfd);
     }
}
     close(sockfd);
     return 0;
}
