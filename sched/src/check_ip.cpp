//static char *cvs_id="@(#) $Id: check_ip.C,v 1.2 2003/03/26 19:06:26 cjc Exp $";
#include "check_ip.H"

const char *check_ip(int val)
{
    static char val_string[10];
    switch(val)
    {
        case EACCES : return("EACCES");
        case EBADF  : return("EBADF");
        case EDESTADDRREQ : return("EDESTADREQ\n");
        case EFAULT : return("EFAULT");
        case EFBIG  : return("EFBIG");
        case EIDRM  : return("EIDRM");
        case EINTR  : return("EINTR");
        case EINVAL : return("EINVAL");
        case EMSGSIZE : return("EMSGSIZE");
        case EMFILE : return("EMFILE");
        case ENOMEM : return("ENOMEM");
        case ENOSPC : return("ENOSPC");
        case ENOTCONN : return("ENOTCONN");
        case ENOTSOCK : return("ENOTSOCK");
        case EPERM  : return("EPERM");
        case EPIPE  : return("EPIPE");
        case ERANGE : return("ERANGE");
        case E2BIG  : return("E2BIG");
        case EWOULDBLOCK : return("EWOULDBLOCK");
        default     :
                printf("GOT TO DEFAULT\n");
                sprintf(val_string,"%i",val);
                return(val_string);
    }
}/* end check_ip */

