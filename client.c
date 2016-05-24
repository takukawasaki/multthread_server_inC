#include "netlib.h"


int
client_socket(const char *host, const char *service)
{
     int sockfd;
     sockfd = Tcp_connect(host, service);
     return (sockfd);
     
}


/* using select */
void
client_snd_recv_loop(int fd)
{
     char buf[BUFFSIZE];
     struct timeval timeout;
     int end, width;
     ssize_t len;

     fd_set mask, ready;

     FD_ZERO(&mask);
     FD_SET(fd,&mask);
     FD_SET(0,&mask);           /* set stdin */


     width = fd + 1;

     for (end = 0;  ;  ) {
          ready = mask;
          timeout.tv_sec = 1;
          timeout.tv_usec = 0;
          switch (select(width, (fd_set *)&ready, NULL, NULL, &timeout)) {
          case -1: {
               err_print("select");
               break;
          }
          case 0: {
               break;
          }
          default:
               if (FD_ISSET(fd, &ready)) {
                    if ((len = recv(fd, buf, sizeof(buf), 0)) == -1) {
                         err_print("recv");
                         end = 1;
                         break;
                    }
                    if (len == 0) {
                         err_print("recv:EOF\n");
                         end = 1;
                         break;
                    }

                    buf[len] = '\0';
                    printf("> %s",buf);
                    
                    
               }
               if (FD_ISSET(0,&ready)) {
                    Fgets(buf, sizeof(buf), stdin);
                    if ((feof(stdin))) {
                         end = 1;
                         break;
                         
                    }
                    if ((len = send(fd, buf, strlen(buf), 0)) == -1) {
                         err_print("send");
                         end = 1;
                         break;
                    }
                    
               }
               break;
               

          }
          if (end) {
               break;
          }
     }
     
}


int
main(int argc, char *argv[])
{
    int soc;
    /* 引数にホスト名、ポート番号が指定されているか？ */
    if (argc <= 2) {
         err_sys("usage: client <host addr> <port#>\n");
    }
    /* サーバにソケット接続 */
    if ((soc = client_socket(argv[1], argv[2])) == -1) {
         err_sys("client_socket(): error\n");
    }
    /* 送受信処理 */
    client_snd_recv_loop(soc);
    /* ソケットクローズ */
    Close(soc);

}


