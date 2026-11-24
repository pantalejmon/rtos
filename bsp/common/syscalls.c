/*
 * syscalls.c — minimalne wywołania systemowe dla newlib-nano.
 *
 * printf() z newliba kończy na _write(), malloc() na _sbrk(). Reszta to
 * zaślepki, żeby linker nie dokładał stubów z nosys (które generują
 * ostrzeżenia „is not implemented and will always fail”).
 *
 * Uwaga na później (etap 2+): newlib nie jest wielowątkowy. Po wprowadzeniu
 * zadań printf/malloc z kilku zadań naraz trzeba będzie chronić mutexem
 * (albo zaimplementować __malloc_lock/__malloc_unlock).
 */
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>

#include "bsp.h"

#undef errno
extern int errno;

/* Symbole z linker scriptu (bsp/common/sections.ld). */
extern uint8_t _heap_start[];
extern uint8_t _heap_end[];

int _write(int fd, const char *buf, int len)
{
    if (fd != 1 && fd != 2) {
        errno = EBADF;
        return -1;
    }
    for (int i = 0; i < len; i++) {
        if (buf[i] == '\n')
            bsp_console_putc('\r');     /* terminale oczekują CRLF */
        bsp_console_putc(buf[i]);
    }
    return len;
}

int _read(int fd, char *buf, int len)
{
    if (fd != 0) {
        errno = EBADF;
        return -1;
    }
    int n = 0;
    while (n < len) {
        int c = bsp_console_getc();
        if (c < 0) {
            if (n > 0)
                break;
            continue;                   /* blokujemy do pierwszego znaku */
        }
        buf[n++] = (char)c;
        if (c == '\r' || c == '\n')
            break;
    }
    return n;
}

void *_sbrk(ptrdiff_t incr)
{
    static uint8_t *brk = _heap_start;
    if (brk + incr > _heap_end || brk + incr < _heap_start) {
        errno = ENOMEM;
        return (void *)-1;
    }
    uint8_t *prev = brk;
    brk += incr;
    return prev;
}

int _close(int fd)                       { (void)fd; return -1; }
int _isatty(int fd)                      { return fd >= 0 && fd <= 2; }
int _lseek(int fd, int off, int whence)  { (void)fd; (void)off; (void)whence; return 0; }
int _getpid(void)                        { return 1; }

int _fstat(int fd, struct stat *st)
{
    (void)fd;
    st->st_mode = S_IFCHR;
    return 0;
}

int _kill(int pid, int sig)
{
    (void)pid; (void)sig;
    errno = EINVAL;
    return -1;
}

void _exit(int status)
{
    (void)status;
    for (;;) { }                         /* tu kończy abort() i powrót z main() */
}
