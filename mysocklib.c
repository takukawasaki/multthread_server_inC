#include "netlib.h"


int
server_socket(const char *host,const char *portnum){
     int listenfd;
     socklen_t addrlen;
     
     listenfd = Tcp_listen(host, portnum, &addrlen);
     return (listenfd);
}


void
accept_loop(int soc){
     pid_t childpid;
     char hbuf[NI_MAXHOST],sbuf[NI_MAXSERV];
     struct sockaddr_storage from;
     int connfd;
     socklen_t len;
     
     for (;;){
          len = (socklen_t) sizeof(from);
          /* accept connection */
          if ((connfd = Accept(soc,(struct sockaddr *) &from ,&len)) == -1){
               if (errno != EINTR){
                    continue;
               }else{
                    err_sys("accept error");
               }
               
          }
          if((childpid = Fork()) == 0) {//子プロセス
               (void) getnameinfo((struct sockaddr *) &from ,len,
                                  hbuf,sizeof(hbuf),
                                  sbuf,sizeof(sbuf),
                                  NI_NUMERICHOST | NI_NUMERICSERV);

               err_print("accept:%s:%s\n",hbuf,sbuf);

               Close(soc);//リスニングソケットをクローズ
               /*send and receive loop*/
               send_recv_loop(connfd);
               /*accept socket close */
               exit(0);
          }
          Close(connfd);//親プロセスでは接続済みソケットをクローズ。
     }
}



void
select_accept_loop(int listenfd)
{
     char line[MAXLINE];
     int i, maxi, maxfd,  connfd, sockfd;
     int nready, client[FD_SETSIZE];
     ssize_t n;
     fd_set rset, allset;
     socklen_t clilen;
     struct sockaddr_in cliaddr;

     maxfd = listenfd;          /* 初期化 */

     maxi = -1;                 /* client[] の添字 */
     

     for (i = 0; i < FD_SETSIZE; i++) {
          client[i] = -1;       /* -1 は利用可能なエントリを示す */
          
     }
     FD_ZERO(&allset);
     FD_SET(listenfd, &allset);

     for (;;) {
          rset  = allset;       /* 構造体の代入 */
          nready = Select(maxfd + 1, &rset, NULL,NULL,NULL);
          if (FD_ISSET(listenfd, &rset)) { /* 新規クライアント */
               clilen = sizeof(cliaddr);
               connfd = Accept(listenfd, (SA *)&cliaddr, &clilen);


               Getpeername(connfd, (SA *)&cliaddr, &clilen);
               err_print("connect from: [%s]\n", Sock_ntop((SA *)&cliaddr, clilen));
               
               for (i = 0; i < FD_SETSIZE; i++) {
                    if (client[i] < 0) {
                         client[i] = connfd; /* ディスクリプタの保存 */
                         break;
                    }
               }

               if (i == FD_SETSIZE) {
                    err_quit("too many clients");
               }

               FD_SET(connfd, &allset); /* 新しいディスクリプタを集合に加える */
               if (connfd > maxfd) {
                    maxfd = connfd; /* select 用 */
               }
               if (i > maxi) {
                    maxi = i;   /* client 配列の最大添字 */
               }
               if (-nready <= 0) {
                    continue;   /* 読み出し可能ディスクリプタはない */
                    
               }

          }

          for (i = 0; i <= maxi; i++) { /* client からのデータを検査 */
               if ((sockfd = client[i]) < 0) {
                    continue;
               }
               if (FD_ISSET(sockfd, &rset)) {
                    if ((n = Readline(sockfd, line, MAXLINE)) == 0) {
                         /* client がクローズした */
                         Close(sockfd);
                         FD_CLR(sockfd, &allset);
                         client[i] = -1;
                    }else{
                         Writen(sockfd,line, n);
                    }
                    if (-nready <= 0) {
                         break; /* 読み出し可能ディスクリプタなし */
                    }
               }
          }
     }

}


void
poll_accept_loop(int listenfd)
{
#define OPEN_MAX 256
     char line[MAXLINE];
     int i, maxi,   connfd, sockfd;
     int nready;
     ssize_t n;
     socklen_t clilen;
     struct pollfd client[OPEN_MAX];
     struct sockaddr_in cliaddr;

     client[0].fd = listenfd;
     client[0].events = POLLRDNORM;

     for (i = 1; i < OPEN_MAX; i++) {
          client[i].fd = -1;
     }

     maxi = 0;
     
     for (;;) {
          nready = Poll(client, maxi + 1, INFTIM);
          if (client[0].revents & POLLRDNORM) {
               clilen = sizeof(cliaddr);
               connfd = Accept(listenfd, (SA *)&cliaddr,&clilen );

               Getpeername(connfd, (SA *)&cliaddr, &clilen);
               err_print("connect from: [%s]\n", Sock_ntop((SA *)&cliaddr, clilen));               
               for (i = 1; i < OPEN_MAX; i++) {
                    if (client[i].fd < 0) {
                         client[i].fd = connfd;
                         break;
                    }
               }
               if (i == OPEN_MAX) {
                    err_quit("too many clients");
               }
               client[i].events = POLLRDNORM;
               if (i > maxi) {
                    maxi = i;
               }
               if (-nready <= 0) {
                    continue;
               }
          }
          for (i = 0; i <= maxi; i++){
               if ((sockfd = client[i].fd) < 0) {
                    continue;
               }
               if (client[i].revents & (POLLRDNORM | POLLERR)) {
                    if ((n = Readline(sockfd, line,MAXLINE)) < 0) {
                         if (errno == ECONNRESET) {
                              Close(sockfd);
                              client[i].fd = -1;
                         }else{
                              err_sys("readline error");
                         }
                         
                    }else if(n == 0){
                         Close(sockfd);
                         client[i].fd = -1;
                         
                    }else{
                         Writen(sockfd, line,n);
                              
                    }
                    if (-nready <= 0) {
                         break;
                    }
               }
          }

     }

}



void
pthread_accept_loop(int soc)
{
     char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
     struct sockaddr_storage from;
     int acc;
     socklen_t len;
     pthread_t thread_id;
     void *send_recv_thread(void *arg);

     for (;;) {
          len = (socklen_t) sizeof(from);
          /* 接続受付 */
          if ((acc = Accept(soc, (struct sockaddr *) &from, &len)) == -1) {
               if(errno!=EINTR){
                    continue;
               }else{
                    err_sys("accept error");
               }
          } else {
               (void) getnameinfo((struct sockaddr *) &from, len,
                                  hbuf, sizeof(hbuf),
                                  sbuf, sizeof(sbuf),
                                  NI_NUMERICHOST | NI_NUMERICSERV);

               err_print("accept:%s:%s\n",hbuf, sbuf);

               /* スレッド生成 */
               if (pthread_create(&thread_id, NULL, send_recv_thread, (void *) acc)
                   != 0) {
                    err_sys("pthread create");
               } else {

                    err_print("pthread_create:create:thread_id=%d\n", (int)thread_id);
                    
               }
          }
     }
}

/* receive and sent loop */

void
send_recv_loop(int acc){
     char buf[512],*ptr;
     ssize_t len;
     for (;;){
          /*receive*/
          if ((len = recv(acc,buf,sizeof(buf),0)) == -1){
               /*erro */
               //perror("recv");
               err_print("recv");
               break;
          }
          if (len == 0){
               /*end of file */
               //(void) fprintf(stderr,"recv:EOF\n");
               err_print("recv:EOF\n");
               break;
          }
          /* strings make */
          buf[len] = '\0';
          if ((ptr = strpbrk(buf,"\r\n")) != NULL) {
               *ptr = '\0';
          }
          
          //(void) fprintf(stderr,"[client]%s\n",buf);
          err_print("[client]%s\n",buf);

          
          /* making response strings */
          (void)mystrlcat(buf,":Received\r\n",sizeof(buf));
          len = (ssize_t) strlen(buf);
          
          Send(acc,buf,(size_t)len,0);
          
          /* if ((len = send(acc,buf,(size_t) len,0)) == -1){ */
          /*      /\* error *\/ */
          /*      err_print("send"); */
          /*      break; */
          /* } */

     }
}

