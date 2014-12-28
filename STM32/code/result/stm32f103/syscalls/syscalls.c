/**************************************************************************//*****
 * @file     stdio.c
 * @brief    Implementation of newlib syscall
 ********************************************************************************/

#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include <stm32f10x.h>
#include "stm32f10x_usart.h"

#define STDOUT_USART USART1
#define STDERR_USART USART1
#define STDIN_USART USART1

#undef errno
extern int errno;
extern int  _end;

__attribute__ ((used))
caddr_t _sbrk ( int incr )
{
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;

  if (heap == NULL) {
    heap = (unsigned char *)&_end;
  }
  prev_heap = heap;

  heap += incr;

  return (caddr_t) prev_heap;
}

__attribute__ ((used))
int _link(char *old, char *new) {
return -1;
}

__attribute__ ((used))
int _close(int file)
{
  return -1;
}

__attribute__ ((used))
int _fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

__attribute__ ((used))
int _isatty(int file)
{
  return 1;
}

__attribute__ ((used))
int _lseek(int file, int ptr, int dir)
{
  return 0;
}
__attribute__ ((used))
int _read(int file, char *ptr, int len)
{
  return 0;
}
__attribute__ ((used))
/*int _write(int file, char *ptr, int len)
{
  return len;
}*/

int _write(int file, char *ptr, int len)
{
    int n;
    switch (file)
    {
    case STDOUT_FILENO: /*stdout*/
        for (n = 0; n < len; n++)
        {
            while (USART_GetFlagStatus(STDOUT_USART, USART_FLAG_TC) == RESET);
            USART_SendData(STDOUT_USART, (uint8_t) * ptr++);
        }
        break;
    case STDERR_FILENO: /* stderr */
        for (n = 0; n < len; n++)
        {
            while (USART_GetFlagStatus(STDERR_USART, USART_FLAG_TC) == RESET);
            USART_SendData(STDERR_USART, (uint8_t) * ptr++);
        }
        break;
    default:
        errno = EBADF;
        return -1;
    }
    return len;
}

__attribute__ ((used))
void abort(void)
{
  /* Abort called */
  while(1);
}

/* --------------------------------- End Of File ------------------------------ */
