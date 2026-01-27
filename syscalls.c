/* Low level system calls, which may be called by C standard library API */
#include <sys/stat.h>
#include "peripherals/usart.h"

int _write(int fd, char *ptr, int len) { 
    if (fd == 1) uart_write_buf(USART2, ptr, (size_t) len);
    return -1;
}

int _read(int fd, char *buf, unsigned int count) {
    (void) fd, (void) buf, (void) count;
    return -1;
}

int _close(int fd) {
    (void) fd;
    return -1;
}

int _lseek(int fd, int offset, int whence) {
    (void) fd, (void) offset, (void) whence;
    return 0;
}

int _isatty(int fd) {
    (void) fd;
    return 1;
}

int _fstat(int filedes, struct stat *buf) {
    (void) filedes, (void) buf;
    return -1;
}

void *_sbrk(int incr) { 
    extern char _end;
    static unsigned char *heap = NULL;
    unsigned char *prev_heap;
    if (heap == NULL) heap = (unsigned char *) &_end;
    prev_heap = heap; 
    heap += incr;
    return prev_heap;
}
