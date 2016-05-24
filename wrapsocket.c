#include "netlib.h"


int
Socket(int family, int type, int protocol)
{
     int n;
     if ((n = socket(family, type, protocol)) < 0) {
          err_sys("socket error");

     }
     return(n);     
}


void
Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
     if (bind(fd, sa, salen) < 0) {
          err_sys("bind error");

     }
}


void
Connect(int fd, const struct sockaddr  *sa, socklen_t salen)
{
     
     if (connect(fd, sa, salen) < 0) {
          err_sys("connect error");
     }
}

int
Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
     int		n;

again:
     if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
          if (errno == EPROTO || errno == ECONNABORTED)
#else
               if (errno == ECONNABORTED)
#endif
                    goto again;
               else
                    err_sys("accept error");
     }
     return(n);
}

/* include Listen */
void
Listen(int fd, int backlog)
{
     char	*ptr;

     /*4can override 2nd argument with environment variable */
     if ( (ptr = getenv("LISTENQ")) != NULL)
          backlog = atoi(ptr);

     if (listen(fd, backlog) < 0)
          err_sys("listen error");
}

/* end Listen */

void
Listen2(int fd, int backlog,struct addrinfo *res)
{
     char	*ptr;

     /*4can override 2nd argument with environment variable */
     if ( (ptr = getenv("LISTENQ")) != NULL)
          backlog = atoi(ptr);

     if (listen(fd, backlog) < 0){
          err_print("listen error");
          Close(fd);
          freeaddrinfo(res);
     }
  
}
/* end Listen */


void
Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
     if (getpeername(fd, sa, salenptr) < 0) {
          err_sys("getpeername error");
     }
}

void
Getsockname(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
     if (getsockname(fd,sa, salenptr) < 0) {
          err_sys("getsockname error");
     }
}

void
Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr)
{
     if (getsockopt(fd, level, optname, optval, optlenptr) < 0){
          err_sys("getsockopt error");
     }
}

#ifdef	HAVE_POLL
int
Poll(struct pollfd *fdarray, unsigned long nfds, int timeout)
{
     int		n;

     if ( (n = poll(fdarray, nfds, timeout)) < 0)
          err_sys("poll error");

     return(n);
}
#endif


ssize_t
Recv(int fd, void *ptr, size_t nbytes, int flags)
{
     ssize_t n;
     if ((n = recv(fd, ptr, nbytes, flags)) < 0) {
          err_sys("recv error");
     }
     return(n);
}


ssize_t
Recvfrom(int fd, void *ptr, size_t nbytes, int flags,
         struct sockaddr *sa, socklen_t *salenptr)
{
     ssize_t n;
     if ((n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0) {
          err_sys("recvfrom error");
     }
     return(n);
}


ssize_t
Recvmsg(int fd, struct msghdr *msg, int flags)
{
     ssize_t n;
     if ((n = recvmsg(fd, msg, flags)) < 0) {
          err_sys("recvmsg error");
     }
     return(n);
     
}

int
Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout)
{
     int		n;

     if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
          err_sys("select error");
     return(n);		/* can return 0 on timeout */
}




void
Sendto(int fd, const void *ptr, size_t nbytes, int flags,
       const struct sockaddr *sa, socklen_t salen)
{
     if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes)
          err_sys("sendto error");
}

void
Sendmsg(int fd, const struct msghdr *msg, int flags)
{
     unsigned int	i;
     ssize_t			nbytes;

     nbytes = 0;	/* must first figure out what return value should be */
     for (i = 0; i < msg->msg_iovlen; i++)
          nbytes += msg->msg_iov[i].iov_len;

     if (sendmsg(fd, msg, flags) != nbytes)
          err_sys("sendmsg error");
}

void
Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
     if (setsockopt(fd, level, optname, optval, optlen) < 0)
          err_sys("setsockopt error");
}

void
Shutdown(int fd, int how)
{
     if (shutdown(fd, how) < 0)
          err_sys("shutdown error");
}

int
Sockatmark(int fd)
{
     int		n;

     if ( (n = sockatmark(fd)) < 0)
          err_sys("sockatmark error");
     return(n);
}


void
Socketpair(int family, int type, int protocol, int *fd)
{
     int		n;

     if ( (n = socketpair(family, type, protocol, fd)) < 0)
          err_sys("socketpair error");
}


int Tcp_listen(const char *host, const char *service, socklen_t *addrlenp)
{
     char nbuf[NI_MAXHOST], sbuf[NI_MAXHOST];
     int listenfd, n;
     const int on = 1;
     struct addrinfo hints, *res, *ressave;
     bzero(&hints,sizeof(struct addrinfo));
     hints.ai_flags = AI_PASSIVE;
     hints.ai_family = AF_UNSPEC;
     hints.ai_socktype = SOCK_STREAM;

     if ((n = getaddrinfo(host, service, &hints, &res)) != 0) {
          err_quit("tcp_listen error for %s, %s: %s",
                   host, service,gai_strerror(n));
     }

     if ((n = getnameinfo(res->ai_addr,res->ai_addrlen,
                          nbuf,sizeof(nbuf),
                          sbuf,sizeof(sbuf),
                          NI_NUMERICHOST | NI_NUMERICSERV)) != 0){
          err_print("getnameinfo():%s\n",gai_strerror(n));
          freeaddrinfo(res);
          return (-1);
     }
     err_print("port=%s\n",sbuf);
     ressave = res;

     do
     {
          listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
          if (listenfd < 0) {
               err_print("listen");

               freeaddrinfo(res);
               continue;
          }
          Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

          if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0) {
               break;
          }
          Close(listenfd);
          freeaddrinfo(res);
          
     } while ((res = res->ai_next) != NULL);
     

     if (res == NULL) {
          err_sys("tcplisten error for %s, %s", host, service);
     }
     
     Listen(listenfd, LISTENQ);
     
     if (addrlenp) {
          *addrlenp = res->ai_addrlen;
     }
     
     freeaddrinfo(ressave);

     return(listenfd);
}




#ifdef	HAVE_SOCKADDR_DL_STRUCT
#include	<net/if_dl.h>
#endif

/* include sock_ntop */
char *
sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
     char		portstr[8];
     static char str[128];		/* Unix domain is largest */

     switch (sa->sa_family) {
     case AF_INET: {
          struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

          if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
               return(NULL);
          if (ntohs(sin->sin_port) != 0) {
               snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
               strcat(str, portstr);
          }
          return(str);
     }
/* end sock_ntop */

#ifdef	IPV6
     case AF_INET6: {
          struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

          str[0] = '[';
          if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL)
               return(NULL);
          if (ntohs(sin6->sin6_port) != 0) {
               snprintf(portstr, sizeof(portstr), "]:%d", ntohs(sin6->sin6_port));
               strcat(str, portstr);
               return(str);
          }
          return (str + 1);
     }
#endif

#ifdef	AF_UNIX
     case AF_UNIX: {
          struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

          /* OK to have no pathname bound to the socket: happens on
             every connect() unless client calls bind() first. */
          if (unp->sun_path[0] == 0)
               strcpy(str, "(no pathname bound)");
          else
               snprintf(str, sizeof(str), "%s", unp->sun_path);
          return(str);
     }
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
     case AF_LINK: {
          struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;

          if (sdl->sdl_nlen > 0)
               snprintf(str, sizeof(str), "%*s (index %d)",
                        sdl->sdl_nlen, &sdl->sdl_data[0], sdl->sdl_index);
          else
               snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
          return(str);
     }
#endif
     default:
          snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
                   sa->sa_family, salen);
          return(str);
     }
     return (NULL);
}

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
     char	*ptr;

     if ( (ptr = sock_ntop(sa, salen)) == NULL)
          err_sys("sock_ntop error");	/* inet_ntop() sets errno */
     return(ptr);
}



pid_t
Fork(void)
{
     pid_t	pid;

     if ( (pid = fork()) == -1)
          err_sys("fork error");
     return(pid);
}



Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}


Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
     Sigfunc	*sigfunc;

     if ( (sigfunc = signal(signo, func)) == SIG_ERR)
          err_sys("signal error");
     return(sigfunc);
}


void
sig_chld(int signo)
{
     pid_t pid;
     int stat;
     while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
          printf("child pid: %d terminated\n", pid);
     }
     return;
}



void
Close(int fd)
{
     if (close(fd) == -1)
          err_sys("close error");
}

void *
Malloc(size_t size)
{
     void	*ptr;

     if ( (ptr = malloc(size)) == NULL)
          err_sys("malloc error");
     return(ptr);
}




int
tcp_connect(const char *host, const char *service)
{
     char nbuf[NI_MAXHOST], sbuf[NI_MAXSERV]; //hostname and service name
     int				sockfd, n;
     struct addrinfo	hints, *res, *ressave;

     bzero(&hints, sizeof(struct addrinfo));
     hints.ai_family = AF_UNSPEC;
     hints.ai_socktype = SOCK_STREAM;

     if ( (n = getaddrinfo(host, service, &hints, &res)) != 0)
          err_quit("tcp_connect error for %s, %s: %s",
                   host, service, gai_strerror(n));

     if ((n = getnameinfo(res->ai_addr,res->ai_addrlen,
                          nbuf,sizeof(nbuf),
                          sbuf,sizeof(sbuf),
                          NI_NUMERICHOST | NI_NUMERICSERV)) != 0){
          err_print("getnameinfo():%s\n",gai_strerror(n));
          freeaddrinfo(res);
          return (-1);
     }
     err_print("addr=%s\n", nbuf);
     err_print("port=%s\n", sbuf);
     
     ressave = res;

     do {
          sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
          if (sockfd < 0){
               err_print("socket");
               freeaddrinfo(res);
               continue;	/* ignore this one */
          }
          if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
               break;		/* success */

          Close(sockfd);	/* ignore this one */
          freeaddrinfo(res);
          
     } while ( (res = res->ai_next) != NULL);

     if (res == NULL)	/* errno set from final connect() */
          err_sys("tcp_connect error for %s, %s", host, service);

     freeaddrinfo(ressave);
     return(sockfd);
}
/* end tcp_connect */


int
tcp_connect_with_timeout(const char *hostnm, const char *portnm,int timeout_sec)
{
    char nbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    struct addrinfo hints, *res0;
    struct timeval timeout;
    int soc, errcode, width, val;
    socklen_t len;
    fd_set mask, write_mask, read_mask;

    /* アドレス情報のヒントをゼロクリア */
    (void) memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    /* アドレス情報の決定 */
    if ((errcode = getaddrinfo(hostnm, portnm, &hints, &res0)) != 0) {
        (void) fprintf(stderr, "getaddrinfo():%s\n", gai_strerror(errcode));
        return (-1);
    }
    if ((errcode = getnameinfo(res0->ai_addr, res0->ai_addrlen,
                               nbuf, sizeof(nbuf),
                               sbuf, sizeof(sbuf),
                               NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
        (void) fprintf(stderr, "getnameinfo():%s\n", gai_strerror(errcode));
        freeaddrinfo(res0);
        return (-1);
    }
    (void) fprintf(stderr, "addr=%s\n", nbuf);
    (void) fprintf(stderr, "port=%s\n", sbuf);
    /* ソケットの生成 */
    if ((soc = socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol))
        == -1) {
        perror("socket");
        freeaddrinfo(res0);
        return (-1);
    }
    if (timeout_sec < 0) {
        /* タイムアウト無し */
        /* コネクト */
        if (connect(soc, res0->ai_addr, res0->ai_addrlen) == -1) {
            perror("connect");
            (void) close(soc);
            freeaddrinfo(res0);
            return (-1);
        }
        freeaddrinfo(res0);
        return(soc);
    } else {
        /* タイムアウト有り */
        /* ノンブロッキングモードに */
        (void) Set_block(soc, 0);
        /* コネクト */
        if (connect(soc, res0->ai_addr, res0->ai_addrlen) == -1) {
            if (errno != EINPROGRESS) {
                /* 進行中以外：エラー */
                perror("connect");
                (void) close(soc);
                freeaddrinfo(res0);
                return (-1);
            }
        } else {
            /* コネクト完了 */
            (void) Set_block(soc, 1);
            freeaddrinfo(res0);
            return(soc);
            /*NOT REACHED*/
        }
        /* コネクト結果待ち */
        width = 0;
        FD_ZERO(&mask);
        FD_SET(soc, &mask);
        width = soc + 1;
        timeout.tv_sec = timeout_sec;
        timeout.tv_usec = 0;
        for (;;) {
            write_mask = mask;
            read_mask = mask;
            switch (select(width, &read_mask, &write_mask, NULL, &timeout)) {
            case -1:
                if (errno != EINTR) {
                    /* selectエラー */
                    perror("select");
                    (void) close(soc);
                    freeaddrinfo(res0);
                    return (-1);
                }
                break;
            case 0:
                /* selectタイムアウト */
                (void) fprintf(stderr, "select:timeout\n");
                (void) close(soc);
                freeaddrinfo(res0);
                return (-1);
                /*NOT REACHED*/
                break;
            default:
                if (FD_ISSET(soc, &write_mask) || FD_ISSET(soc, &read_mask)) {
                    len = sizeof(len);
                    if (getsockopt(soc, SOL_SOCKET, SO_ERROR, &val, &len)!=-1) {
                        if (val == 0) {
                            /* connect成功 */
                            (void) Set_block(soc, 1);
                            freeaddrinfo(res0);
                            return(soc);
                        } else {
                            /* connect失敗 */
                            (void) fprintf(stderr,
			                   "getsockopt:%d:%s\n",
					   val,
					   strerror(val));
                            (void) close(soc);
                            freeaddrinfo(res0);
                            return (-1);
                        }
                    } else {
                        /* getsockoptエラー */
                        perror("getsockopt");
                        (void) close(soc);
                        freeaddrinfo(res0);
                        return (-1);
                    }
                }
                break;
            }
        }
    }
     
}



int
Tcp_connect(const char *host, const char *service)
{
     return(tcp_connect(host, service));
}




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

     for (;;) {
          len = (socklen_t) sizeof(from);
          /* 接続受付 */
          if ((acc = Accept(soc, (SA *) &from, &len)) == -1) {
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
               if (pthread_create(&thread_id, NULL, send_recv_thread,(void *)acc)
                   != 0) {
                    err_sys("pthread create");
               } else {

                    err_print("pthread_create:create:thread_id=%d\n", (int)thread_id);
                    
               }
          }
     }
}


int
Makesocket(const char *host , const char *port)
{

     int listenfd;
     const int on = 1;
     struct sockaddr_in  servaddr;
     listenfd = Socket(AF_INET, SOCK_STREAM, 0);

     bzero(&servaddr, sizeof(servaddr));
     servaddr.sin_family = AF_INET;
     servaddr.sin_addr.s_addr = htonl(atoi(host));
     servaddr.sin_port = htons(atoi(port));

     Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
     
     Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

     Listen(listenfd, LISTENQ);
     return (listenfd);
     
}



/* receive and sent loop */

void
send_recv_loop(int acc){
     char buf[512],*ptr;
     ssize_t len;
     for (;;){
          /*receive*/
          if ((len = recv(acc,buf,sizeof(buf),0)) == -1){
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
          (void)mystrlcat(buf,"::Server-Received\r\n",sizeof(buf));
          len = (ssize_t) strlen(buf);
          
          Send(acc,buf,(size_t)len,0);
          

     }
}


void *
send_recv_thread(void *arg)
{
     char buf[512], *ptr;
     ssize_t len;
     int acc;

     /* スレッドのデタッチ */
     (void) pthread_detach(pthread_self());
     /* 引数の取得 */
     acc =  (int)arg;
     for (;;) {
          /* 受信 */
          if ((len = recv(acc, buf, sizeof(buf), 0)) == -1) {
               /* エラー */
               err_print("recv");
               break;
          }
          if (len == 0) {
               /* エンド・オブ・ファイル */
               err_print("<%d>client says:EOF\n", (int) pthread_self());
               break;
          }
          /* 文字列化・表示 */
          buf[len]='\0';
          if ((ptr = strpbrk(buf, "\r\n")) != NULL) {
               *ptr = '\0';
          }
          
          err_print("<%d>[client]%s\n", (int) pthread_self(), buf);
          /* 応答文字列作成 */
          (void) mystrlcat(buf, ":received\r\n", sizeof(buf));
          
          len = (ssize_t) strlen(buf);
          /* 応答 */

          Send(acc,buf,(size_t)len,0);
          
     }
     (void) close(acc);
     pthread_exit((void *) 0);
     /*NOT REACHED*/
     return ((void *) 0);
}

const char *
Inet_ntop(int family, const void *addrptr, char *strptr, size_t len)
{
	const char	*ptr;

	if (strptr == NULL)		/* check for old code */
		err_quit("NULL 3rd argument to inet_ntop");
	if ( (ptr = inet_ntop(family, addrptr, strptr, len)) == NULL)
		err_sys("inet_ntop error");		/* sets errno */
	return(ptr);
}

void
Inet_pton(int family, const char *strptr, void *addrptr)
{
	int		n;

	if ( (n = inet_pton(family, strptr, addrptr)) < 0)
		err_sys("inet_pton error for %s", strptr);	/* errno set */
	else if (n == 0)
		err_quit("inet_pton error for %s", strptr);	/* errno not set */

	/* nothing to return */
  
}

int
Client_socket4(const char *hostnm, const char *portnm){
    char buf[MAXLINE];
    struct sockaddr_in server ;
    struct in_addr addr;
    int sock, portno;
    struct hostent *host;
    struct servent *se;

    (void) memset(&server, 0, sizeof(server)) ;
    server.sin_family = AF_INET ;
    if ( inet_pton(AF_INET, hostnm, &addr) == 0) {
         if ((host = gethostbyname2(hostnm, AF_INET)) == NULL) {
              err_sys("gethostbyname2():error\n");
              
         }
         memcpy(&addr, (struct in_addr *) *host->h_addr_list,
                sizeof(struct in_addr));

         
    }
    
    err_print("addr=%s\n",
              Inet_ntop(AF_INET, &addr, buf, sizeof(buf)));

    server.sin_addr = addr;
    if (isdigit(portnm[0])) {
         if ((portno = atoi(portnm)) <= 0) {
              err_sys("bad prot no");
         }
         server.sin_port = htons(portno);
    }else{
         if ((se = getservbyname(portnm, "tcp")) == NULL) {
              err_sys("getservbyname():error\n");
              
         }else{
              server.sin_port = se->s_port;
              
         }
    }

    err_print("port=%d\n",ntohs(server.sin_port));

    sock = Socket(PF_INET, SOCK_STREAM, 0);
    Connect(sock,(SA *)&server, sizeof(server));
    return (sock);

    
}



int
Client_socket6(const char *hostnm, const char *portnm){
    char buf[MAXLINE];
    struct sockaddr_in6 server ;
    struct in6_addr addr;
    int sock, portno;
    struct hostent *host;
    struct servent *se;

    (void) memset(&server, 0, sizeof(server)) ;
    server.sin6_family = AF_INET ;
    if ( inet_pton(AF_INET, hostnm, &addr) == 0) {
         if ((host = gethostbyname2(hostnm, AF_INET)) == NULL) {
              err_sys("gethostbyname2():error\n");
              
         }
         memcpy(&addr, (struct in6_addr *) *host->h_addr_list,
                sizeof(struct in6_addr));

         
    }
    
    err_print("addr=%s\n",
              Inet_ntop(AF_INET, &addr, buf, sizeof(buf)));

    server.sin6_addr = addr;
    if (isdigit(portnm[0])) {
         if ((portno = atoi(portnm)) <= 0) {
              err_sys("bad prot no");
         }
         server.sin6_port = htons(portno);
    }else{
         if ((se = getservbyname(portnm, "tcp")) == NULL) {
              err_sys("getservbyname():error\n");
              
         }else{
              server.sin6_port = se->s_port;
              
         }
    }

    err_print("port=%d\n",ntohs(server.sin6_port));

    sock = Socket(PF_INET6, SOCK_STREAM, 0);
    Connect(sock,(SA *)&server, sizeof(server));
    return (sock);

    
}

int
Set_block(int fd, int flag)
{
     int flags;

     flags = Fcntl(fd, F_GETFL, 0);
     if (flag == 0) {
          Fcntl(fd, F_SETFL, flags | O_NONBLOCK);
     }else if(flag == 1){
          Fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
     }
     return (0);
}


int
Fcntl(int fd, int cmd, int arg)
{
	int	n;

	if ( (n = fcntl(fd, cmd, arg)) == -1)
		err_sys("fcntl error");
	return(n);
}
