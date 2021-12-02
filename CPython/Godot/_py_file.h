#ifndef _PY_FILE_H
#define _PY_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h> // off_t

#if defined(_MSC_VER)
#include <basetsd.h> // ssize_t
typedef SSIZE_T ssize_t;
#endif

#ifdef __GNUC__
#define _PRINTF_FORMAT_ATTRIBUTE_2_0 __attribute__((format(printf, 2, 0)))
#define _PRINTF_FORMAT_ATTRIBUTE_2_3 __attribute__((format(printf, 2, 3)))
#else
#define _PRINTF_FORMAT_ATTRIBUTE_2_0
#define _PRINTF_FORMAT_ATTRIBUTE_2_3
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PYFILE PYFILE;

int _gd_chdir(const char *dir);
char *_gd_getcwd(char *buf, int size);
int _gd_unlink(const char* path);

int _gd_open(const char *name, int flags, ...);
int _gd_close(int fd);
int _gd_lseek(int fd, off_t offset, int whence);
off_t _gd_ltell(int fd);
ssize_t _gd_read(int fd, void* buf, size_t len);
ssize_t _gd_write(int fd, const void *buf, size_t len);
ssize_t _gd_filesize(int fd);

PYFILE *_gd_fopen(const char *name, const char *mode);
PYFILE *_gd_wfopen(const wchar_t *name, const wchar_t *mode);
PYFILE *_gd_fdopen(const int fd, const char *mode);
int _gd_fstat(PYFILE *f, struct stat *buf);
int _gd_stat(const char *path, struct stat *buf);
int _gd_fclose(PYFILE *f);
int _gd_fseek(PYFILE *f, off_t offset, int whence);
off_t _gd_ftell(PYFILE *f);
void _gd_rewind(PYFILE *f);
ssize_t _gd_fread(void* buf, size_t len, size_t cnt, PYFILE *f);
ssize_t _gd_fwrite(const void* buf, size_t len, size_t cnt, PYFILE *f);
void _gd_fprintf(PYFILE *f, const char *format, ...) _PRINTF_FORMAT_ATTRIBUTE_2_3;
void _gd_vfprintf(PYFILE *f, const char *format, va_list ap) _PRINTF_FORMAT_ATTRIBUTE_2_0;
char *_gd_fgets(char* buf, size_t len, PYFILE *f);
int _gd_fputs(const char* buf, PYFILE *f);
int _gd_getc(PYFILE *f);
int _gd_putc(int ch, PYFILE *f);
int _gd_ungetc(int c, PYFILE *f);
PYFILE *_gd_tmpfile();
int _gd_fflush(PYFILE *f);
int _gd_fileno(PYFILE *f);
int _gd_feof(PYFILE *f);
int _gd_ferror(PYFILE *f);
void _gd_clearerr(PYFILE *f);
ssize_t _gd_ffilesize(PYFILE *f);
PYFILE *_gd_stderr();
PYFILE *_gd_stdin();
PYFILE *_gd_stdout();

#define pychdir    _gd_chdir
#define pygetcwd   _gd_getcwd
#define pyunlink   _gd_unlink

#define pyopen     _gd_open
#define pyclose    _gd_close
#define pylseek    _gd_lseek
#define pyltell    _gd_ltell
#define pyread     _gd_read
#define pywrite    _gd_write
#define pyfilesize _gd_filesize

#define pystderr    _gd_stderr()
#define pystdin     _gd_stdin()
#define pystdout    _gd_stdout()
#define pystat      _gd_stat
#define pyfstat     _gd_fstat
#define pyfopen     _gd_fopen
#define pyfdopen    _gd_fdopen
#define pywfopen    _gd_wfopen
#define pyfclose    _gd_fclose
#define pyfseek     _gd_fseek
#define pyftell     _gd_ftell
#define pyfread     _gd_fread
#define pyfwrite    _gd_fwrite
#define pyfgets     _gd_fgets
#define pytmpfile   _gd_tmpfile
#define pyfputs     _gd_fputs
#define pyputc      _gd_putc
#define pyfputc     _gd_putc
#define pygetc      _gd_getc
#define pyungetc    _gd_ungetc
#define pyfprintf   _gd_fprintf
#define pyvfprintf  _gd_vfprintf
#define pyrewind    _gd_rewind
#define pyfflush    _gd_fflush
#define pyfeof      _gd_feof
#define pyfileno    _gd_fileno
#define pyferror    _gd_ferror
#define pyclearerr  _gd_clearerr
#define pyffilesize _gd_ffilesize

#if 0

typedef FILE PYFILE;

#define pychdir    chdir
#define pygetcwd   getcwd
#define pyunlink   unlink

#define pyopen     open
#define pyclose    close
#define pylseek    lseek
#define pyltell    ltell
#define pyread     read
#define pywrite    write

#define pyfile(F)  F
#define pystderr   stderr
#define pystdin    stdin
#define pystdout   stdout
#define pyfopen    fopen
#define pyfdopen   fdopen
#define pypopen    popen
#define pyfclose   fclose
#define pystat     stat
#define pyfstat(F,B) fstat(fileno(F), B)
#define pyfseek    fseek
#define pyftell    ftell
#define pyfread    fread
#define pyfwrite   fwrite
#define pyfgets    fgets
#define pyfputs    fputs
#define pytmpfile  tmpfile
#define pygetc     getc
#define pyputc     putc
#define pyfputc    fputc
#define pyungetc   ungetc
#define pyfprintf  fprintf
#define pyvfprintf vfprintf
#define pyfflush   fflush
#define pyfeof     feof
#define pyfileno   fileno
#define pyferror   ferror
#define pyclearerr clearerr
#define pyrewind   rewind

#endif

#ifdef __cplusplus
}
#endif

#endif // _PY_FILE_H
