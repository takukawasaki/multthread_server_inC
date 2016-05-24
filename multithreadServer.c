#include "netlib.h"


int
main(int argc, char *argv[])
{
     int listenfd;
     listenfd = Makesocket(argv[1],argv[2]);
     err_print("server ready\n");
     

     pthread_accept_loop(listenfd);

     Close(listenfd);
     return 0;
}

