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
#define MAXSIZE 512

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

loadpage(int newsd,char *page)
{
    int fsize,i=0,c;
    char out_buf[MAXSIZE];
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
     fp = fopen(page,"r");
     bzero(out_buf,MAXSIZE);
     while((c = getc(fp)) != EOF)
     {
     if(i==511)
     {
     out_buf[i]=c;
     write(newsd, out_buf, strlen(out_buf));
     printf("%c",c);
     bzero(out_buf,MAXSIZE);
     i=-1;
     }
     else
     {
     out_buf[i]=c;
     printf("%c",c);
     }
     i++;
     }
     if(c==EOF&&i!=0)
     {
     write(newsd, out_buf, strlen(out_buf));
     }
     fclose(fp);
}

/* CHILD PROCEDURE, WHICH ACTUALLY DOES THE FILE TRANSFER */
doftp(int newsd)
  {
    char fname[MAXLINE];

    FILE *fp;

     bzero(fname,MAXLINE);
     strcpy(fname,"index.html");

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
     write(newsd, "HTTP/1.1 200 OK\n", 16);
     write(newsd, "Content-Type: text/html\n", 24);
     }
     loadpage(newsd,fname);

   printf("server: FILE TRANSFER COMPLETE on socket %d\n",newsd);
   fclose(fp);
   close(newsd);
  }


int main(int argc, char *argv[])
{
    time_t mytime;
    int pid;

     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
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
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);

     /*bzero(buffer,256);
     strcpy(buffer,"Current Time: ");
      mytime = time(NULL);
     strcat(buffer,ctime(&mytime));*/
     /*n = write(newsockfd,buffer ,strlen(buffer));
     if (n < 0) error("ERROR writing to socket");*/

     doftp(newsockfd);

     close(newsockfd);
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
