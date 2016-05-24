#include "netlib.h"



size_t
mystrlcat (char *dst, const char *src,size_t size){
     const char *ps;
     char *pd,*pde;
     size_t dlen,lest;

     for (pd = dst,lest = size; *pd != '\0' && lest != 0; pd++,lest--);
     dlen = pd -dst;
     if (size - dlen == 0){
          return (dlen + strlen(src));
          
     }
     pde = dst + size - 1;
     for (ps = src; *ps != '\0' && pd < pde; pd++,ps++){
          *pd = *ps;
          
     }

     for (; pd <= pde; pd++){
          *pd = '\0';
     }

     while (*ps++);
     return (dlen + (ps - src - 1));
     
}


//ssize_t						/* Write "n" bytes to a descriptor. */
void
sending(int fd, const void *vptr, size_t n,int flags)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
       if ( (nwritten = send(fd, ptr, nleft,flags)) <= 0) {
            if (nwritten < 0 && errno == EINTR){
                 nwritten = 0;		/* and call write() again */
            } else{
                 /* error */
                 err_print("send error");
                 break;
            }
       }

       nleft -= nwritten;
       ptr   += nwritten;
	}
}


//ssize_t
void
Send(int fd, const void *ptr, size_t nbytes,int flags)
{
     sending(fd,ptr,nbytes,flags);
     
}


