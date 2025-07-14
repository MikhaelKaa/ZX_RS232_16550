/* errno.h - минимальная реализация для Z80 */
#ifndef _ERRNO_H
#define _ERRNO_H

#define EPERM   1   /* Operation not permitted */
#define ENOENT  2   /* No such file or directory */
#define EINTR   4   /* Interrupted system call */
#define EIO     5   /* I/O error */
#define E2BIG   7   /* Argument list too long */
#define ENOEXEC 8   /* Exec format error */
#define EBADF   9   /* Bad file number */
#define EAGAIN  11  /* Try again */
#define ENOMEM  12  /* Out of memory */
#define EACCES  13  /* Permission denied */
#define EFAULT  14  /* Bad address */
#define EBUSY   16  /* Device or resource busy */
#define EEXIST  17  /* File exists */
#define ENODEV  19  /* No such device */
#define EINVAL  22  /* Invalid argument */
#define ENOTTY  25  /* Not a typewriter */
#define EPIPE   32  /* Broken pipe */
#define ERANGE  34  /* Math result not representable */

extern int errno;

#endif /* _ERRNO_H */