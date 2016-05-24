#include "netlib.h"


#define MAXFD 64

int
daemonize(int nochdir, int noclose)
{
     int i, fd;
     pid_t pid;

     if ((pid = fork()) < 0) {
          return (-1);
     }else if(pid != 0){//parent process exit
          _exit(0);
     }

     (void) setsid(); //session leader
     (void) signal(SIGHUP, SIG_IGN);

     if ((pid = fork()) != 0) {
          _exit(0); //child process exit
     }

     if (nochdir == 0) {
          (void) chdir("/");
          
     }
     if (noclose == 0) {
          //all diskripter close
          for (i = 0; i < MAXFD; i++) {
               (void) close(i);
          }
          if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
               (void) dup2(fd,0);
               (void) dup2(fd, 1);
               (void) dup2(fd, 2);
               if (fd>2) {
                    (void) close(fd);
               }
          }

     }
          
     return 0;
}


int
main(int argc, char *argv[])
{
     char buf[MAXLINE];
     /* deamonize */
     (void) daemonize(0,0);
     (void) fprintf(stderr, "stderr\n");
     syslog(LOG_USER|LOG_NOTICE, "daemon:pwd=%s\n",getcwd(buf, sizeof(buf)));
     return 0;
}
