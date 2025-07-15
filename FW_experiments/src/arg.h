#ifndef __ARG_H__
#define __ARG_H__

typedef unsigned char *va_list;
#define va_start(ap, last) (ap = ((va_list)&(last)) + sizeof(last))
#define va_arg(ap, type)   (*(type *)((ap += sizeof(type)) - sizeof(type)))
#define va_end(ap)         ((void)0)

#define NULL ((void*)0) // 


#endif // __ARG_H__