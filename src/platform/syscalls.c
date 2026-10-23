#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>

int _close(int fd)
{
    (void)fd;
    return -1;
}

int _fstat(int fd, struct stat *st)
{
    (void)fd;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd)
{
    (void)fd;
    return 1;
}

off_t _lseek(int fd, off_t offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    return (off_t)-1;
}

int _read(int fd, void *buf, size_t count)
{
    (void)fd;
    (void)buf;
    (void)count;
    return -1;
}

int _write(int fd, const void *buf, size_t count)
{
    (void)fd;
    (void)buf;
    return (int)count;
}

void *_sbrk(ptrdiff_t incr)
{
    extern char _end;
    static char *heap;
    char *prev;
    if (heap == 0) {
        heap = &_end;
    }
    prev = heap;
    heap += incr;
    return prev;
}

void _exit(int status)
{
    (void)status;
    for (;;) {
    }
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    return -1;
}

int _getpid(void)
{
    return 1;
}

void _init(void)
{
}

void _fini(void)
{
}
