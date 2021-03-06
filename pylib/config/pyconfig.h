#ifndef Py_PYCONFIG_H
#define Py_PYCONFIG_H

#ifdef __APPLE__
# include <TargetConditionals.h>
# include <AvailabilityMacros.h>
#endif

/* Limited posix features on small/homebrew consoles */
#if defined(__NX__) || defined(__3DS__) || defined(__psp2__) || defined(__psp__)
# define LIMITED_POSIX
#endif

#if defined(__psp2__) || defined(__psp__)
# define PSP
#endif

/* Visual Studio 2005 introduces deprecation warnings for
   "insecure" and POSIX functions. The insecure functions should
   be replaced by *_s versions (according to Microsoft); the
   POSIX functions by _* versions (which, according to Microsoft,
   would be ISO C conforming). Neither renaming is feasible, so
   we just silence the warnings. */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#endif

#ifdef MS_WINDOWS
#ifndef __cplusplus /* macros can cause compilation errors outside c */
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <process.h>
# include <io.h>
# define open _open
# define close _close
# define read _read
# define write _write
# define fileno _fileno
# define lseek _lseek
# define isatty _isatty
# define unlink _unlink
# define getcwd _getcwd
# define umask _umask
# define execv _execv
# define execve _execve
# define putenv _putenv
# define getpid _getpid
# define dup _dup
# define dup2 _dup2
# include <string.h>
# define strdup _strdup
# define copysign _copysign
# define stricmp _stricmp
# define hypot _hypot
#endif // __cplusplus
#endif /* MS_WINDOWS_MACROS */

#ifdef MS_WINDOWS
# include <float.h>
# define Py_IS_NAN _isnan
# define Py_IS_INFINITY(X) (!_finite(X) && !_isnan(X))
# define Py_IS_FINITE(X) _finite(X)
#endif

#ifdef _WIN64
# define MS_WIN64
#endif

#ifdef MS_WINDOWS
#define NT_THREADS
#ifndef NETSCAPE_PI
# define USE_SOCKET
#endif
#endif

#ifdef MS_WINDOWS
#ifdef __GNUC__
# define PY_LONG_LONG long long
# define PY_LLONG_MIN LLONG_MIN
# define PY_LLONG_MAX LLONG_MAX
# define PY_ULLONG_MAX ULLONG_MAX
#endif /* GNUC */
#ifndef PY_LONG_LONG
# define PY_LONG_LONG __int64
# define PY_LLONG_MAX _I64_MAX
# define PY_LLONG_MIN _I64_MIN
# define PY_ULLONG_MAX _UI64_MAX
#endif
#endif /* MS_WINDOWS */

#ifdef _DEBUG
 #define Py_DEBUG
#endif

/* define some ANSI types that are not defined in earlier Win headers */
#if defined(_MSC_VER) && _MSC_VER >= 1200
/* This file only exists in VC 6.0 or higher */
#include <basetsd.h>
#endif

/* Define if building universal (internal helper macro) */
#define AC_APPLE_UNIVERSAL_BUILD 1

/* Define for AIX if your compiler is a genuine IBM xlC/xlC_r and you want
 support for AIX C++ shared extension modules. */
/* #undef AIX_GENUINE_CPLUSPLUS */

/* Define this if you have AtheOS threads. */
/* #undef ATHEOS_THREADS */

/* Define this if you have BeOS threads. */
/* #undef BEOS_THREADS */

/* Define if you have the Mach cthreads package */
/* #undef C_THREADS */

/* Define if C doubles are 64-bit IEEE 754 binary format, stored in ARM
 mixed-endian order (byte order 45670123) */
/* #undef DOUBLE_IS_ARM_MIXED_ENDIAN_IEEE754 */

/* Define if C doubles are 64-bit IEEE 754 binary format, stored with the most
 significant byte first */
/* #undef DOUBLE_IS_BIG_ENDIAN_IEEE754 */

/* Define if C doubles are 64-bit IEEE 754 binary format, stored with the
 least significant byte first */
#define DOUBLE_IS_LITTLE_ENDIAN_IEEE754 1

/* Define if --enable-ipv6 is specified */
#ifndef LIMITED_POSIX
#define ENABLE_IPV6 1
#endif

/* Define if flock needs to be linked with bsd library. */
/* #undef FLOCK_NEEDS_LIBBSD */

/* Define if getpgrp() must be called as getpgrp(0). */
/* #undef GETPGRP_HAVE_ARG */

/* Define if gettimeofday() does not have second (timezone) argument This is
 the case on Motorola V4 (R40V4.2) */
/* #undef GETTIMEOFDAY_NO_TZ */

/* Define to 1 if you have the `acosh' function. */
#define HAVE_ACOSH 1

/* struct addrinfo (netdb.h) */
#define HAVE_ADDRINFO 1

/* Define to 1 if you have the `alarm' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_ALARM 1
#endif

/* Define this if your time.h defines altzone. */
/* #undef HAVE_ALTZONE */

/* Define to 1 if you have the `asinh' function. */
#define HAVE_ASINH 1

/* Define to 1 if you have the <asm/types.h> header file. */
/* #undef HAVE_ASM_TYPES_H */

/* Define to 1 if you have the `atanh' function. */
#define HAVE_ATANH 1

/* Define if GCC supports __attribute__((format(PyArg_ParseTuple, 2, 3))) */
/* #undef HAVE_ATTRIBUTE_FORMAT_PARSETUPLE */

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
/* #undef HAVE_BIND_TEXTDOMAIN_CODESET */

/* Define to 1 if you have the <bluetooth/bluetooth.h> header file. */
/* #undef HAVE_BLUETOOTH_BLUETOOTH_H */

/* Define to 1 if you have the <bluetooth.h> header file. */
/* #undef HAVE_BLUETOOTH_H */

/* Define if nice() returns success/failure instead of the new priority. */
/* #undef HAVE_BROKEN_NICE */

/* Define if the system reports an invalid PIPE_BUF value. */
/* #undef HAVE_BROKEN_PIPE_BUF */

/* Define if poll() sets errno on invalid file descriptors. */
/* #undef HAVE_BROKEN_POLL */

/* Define if the Posix semaphores do not work on your system */
/* #undef HAVE_BROKEN_POSIX_SEMAPHORES */

/* Define if pthread_sigmask() does not work on your system. */
/* #undef HAVE_BROKEN_PTHREAD_SIGMASK */

/* define to 1 if your sem_getvalue is broken. */
#define HAVE_BROKEN_SEM_GETVALUE 1

/* Define this if you have the type _Bool. */
#define HAVE_C99_BOOL 1

/* Define to 1 if you have the `chflags' function. */
/* #undef HAVE_CHFLAGS */

/* Define to 1 if you have the `chown' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_CHOWN 1
#endif

/* Define if you have the 'chroot' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_CHROOT 1
#endif

/* Define to 1 if you have the `clock' function. */
#define HAVE_CLOCK 1

/* Define to 1 if you have the `confstr' function. */
#if !defined(__ANDROID__) && !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_CONFSTR 1
#endif

/* Define to 1 if you have the <conio.h> header file. */
#ifdef MS_WINDOWS
#define HAVE_CONIO_H
#endif

/* Define to 1 if you have the `copysign' function. */
#define HAVE_COPYSIGN 1

/* Define to 1 if you have the `ctermid' function. */
#if !defined(__ANDROID__) && !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_CTERMID 1
#endif

/* Define if you have the 'ctermid_r' function. */
#ifdef __APPLE__
#define HAVE_CTERMID_R 1
#endif

/* Define to 1 if you have the <curses.h> header file. */
#define HAVE_CURSES_H 1

/* Define if you have the 'is_term_resized' function. */
#define HAVE_CURSES_IS_TERM_RESIZED 1

/* Define if you have the 'resizeterm' function. */
#define HAVE_CURSES_RESIZETERM 1

/* Define if you have the 'resize_term' function. */
#define HAVE_CURSES_RESIZE_TERM 1

/* Define to 1 if you have the declaration of `isfinite', and to 0 if you
 don't. */
#define HAVE_DECL_ISFINITE 1

/* Define to 1 if you have the declaration of `isinf', and to 0 if you don't.
 */
#define HAVE_DECL_ISINF 1

/* Define to 1 if you have the declaration of `isnan', and to 0 if you don't.
 */
#define HAVE_DECL_ISNAN 1

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
 */
/* #undef HAVE_DECL_TZNAME */

/* Define to 1 if you have the device macros. */
#if !defined(__ANDROID__) && !defined(MS_WINDOWS) && !defined(PSP)
#define HAVE_DEVICE_MACROS 1
#endif

/* Define if we have /dev/ptc. */
#if !defined(__APPLE__) && !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_DEV_PTC
#endif

/* Define if we have /dev/ptmx. */
#if !defined(__ANDROID__) && !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_DEV_PTMX 1
#endif

/* Define to 1 if you have the <direct.h> header file. */
#ifdef MS_WINDOWS
#define HAVE_DIRECT_H
#endif

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
 */
#ifndef MS_WINDOWS
#define HAVE_DIRENT_H 1
#endif

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #define HAVE_DLFCN_H 1 */

/* Define to 1 if you have the `dlopen' function. */
/* #define HAVE_DLOPEN 1 */

/* Define to 1 if you have the `dup2' function. */
#ifndef LIMITED_POSIX
#define HAVE_DUP2 1
#endif

/* Defined when any dynamic module loading is enabled. */
/* #define HAVE_DYNAMIC_LOADING 1 */

/* Define if you have the 'epoll' functions. */
/* #undef HAVE_EPOLL */

/* Define to 1 if you have the `erf' function. */
#define HAVE_ERF 1

/* Define to 1 if you have the `erfc' function. */
#define HAVE_ERFC 1

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the `execv' function. */
#ifndef LIMITED_POSIX
#define HAVE_EXECV 1
#endif

/* Define to 1 if you have the `expm1' function. */
#define HAVE_EXPM1 1

/* Define if you have the 'fchdir' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_FCHDIR 1
#endif

/* Define to 1 if you have the `fchmod' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_FCHMOD 1
#endif

/* Define to 1 if you have the `fchown' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_FCHOWN 1
#endif

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the fcntl function. */
#ifndef MS_WINDOWS
#define HAVE_FCNTL 1
#endif

/* Define if you have the 'fdatasync' function. */
/* #undef HAVE_FDATASYNC */

/* Define to 1 if you have the `finite' function. */
#define HAVE_FINITE 1

/* Define to 1 if you have the `flock' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_FLOCK 1
#endif

/* Define to 1 if you have the `fork' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_FORK 1
#endif

/* Define to 1 if you have the `forkpty' function. */
#ifdef __APPLE__
#ifndef forkpty
#define HAVE_FORKPTY 1
#endif
#endif

/* Define to 1 if you have the `fpathconf' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_FPATHCONF 1
#endif

/* Define to 1 if you have the `fseek64' function. */
/* #undef HAVE_FSEEK64 */

/* Define to 1 if you have the `fseeko' function. */
#define HAVE_FSEEKO 1

/* Define if you have the 'fsync' function. */
#ifndef LIMITED_POSIX
#define HAVE_FSYNC 1
#endif

/* Define to 1 if you have the `ftell64' function. */
/* #undef HAVE_FTELL64 */

/* Define to 1 if you have the `ftello' function. */
#define HAVE_FTELLO 1

/* Define to 1 if you have the `ftime' function. */
#ifndef __ANDROID__
#define HAVE_FTIME 1
#endif

/* Define to 1 if you have the `ftruncate' function. */
#ifndef MS_WINDOWS
#define HAVE_FTRUNCATE 1
#endif

/* Define to 1 if you have the `gai_strerror' function. */
#ifndef PSP
#define HAVE_GAI_STRERROR 1
#endif

/* Define to 1 if you have the `gamma' function. */
#define HAVE_GAMMA 1

/* Define if we can use gcc inline assembler to get and set x87 control word
 */
#ifndef _MSC_VER
# if defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
# define HAVE_GCC_ASM_FOR_X87 1
# endif
#endif

/* Define if you have the getaddrinfo function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define this if you have flockfile(), getc_unlocked(), and funlockfile() */
#define HAVE_GETC_UNLOCKED 1

/* Define to 1 if you have the `getgroups' function. */
#if !defined(MS_WINDOWS) && !defined(__NX__)
#define HAVE_GETGROUPS 1
#endif

/* Define to 1 if you have the `gethostbyname' function. */
#define HAVE_GETHOSTBYNAME 1

/* Define this if you have some version of gethostbyname_r() */
/* #undef HAVE_GETHOSTBYNAME_R */

/* Define this if you have the 3-arg version of gethostbyname_r(). */
/* #undef HAVE_GETHOSTBYNAME_R_3_ARG */

/* Define this if you have the 5-arg version of gethostbyname_r(). */
/* #undef HAVE_GETHOSTBYNAME_R_5_ARG */

/* Define this if you have the 6-arg version of gethostbyname_r(). */
/* #undef HAVE_GETHOSTBYNAME_R_6_ARG */

/* Define to 1 if you have the `getitimer' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_GETITIMER 1
#endif

/* Define to 1 if you have the `getloadavg' function. */
#if !defined(__ANDROID__) && !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_GETLOADAVG 1
#endif

/* Define to 1 if you have the `getlogin' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_GETLOGIN 1
#endif

/* Define to 1 if you have the `getnameinfo' function. */
#if !defined(PSP)
#define HAVE_GETNAMEINFO 1
#endif

/* Define if you have the 'getpagesize' function. */
#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `getpeername' function. */
#define HAVE_GETPEERNAME 1

/* Define to 1 if you have the `getpgid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_GETPGID 1
#endif

/* Define to 1 if you have the `getpgrp' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_GETPGRP 1
#endif

/* Define to 1 if you have the `getpid' function. */
#define HAVE_GETPID 1

/* Define to 1 if you have the `getpriority' function. */
#define HAVE_GETPRIORITY 1

/* Define to 1 if you have the `getpwent' function. */
#define HAVE_GETPWENT 1

/* Define to 1 if you have the `getresgid' function. */
/* #undef HAVE_GETRESGID */

/* Define to 1 if you have the `getresuid' function. */
/* #undef HAVE_GETRESUID */

/* Define to 1 if you have the `getsid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_GETSID 1
#endif

/* Define to 1 if you have the `getspent' function. */
/* #undef HAVE_GETSPENT */

/* Define to 1 if you have the `getspnam' function. */
/* #undef HAVE_GETSPNAM */

/* Define to 1 if you have the `gettimeofday' function. */
#ifndef MS_WINDOWS
#define HAVE_GETTIMEOFDAY 1
#endif

/* Define to 1 if you have the `getwd' function. */
#ifndef MS_WINDOWS
#define HAVE_GETWD 1
#endif

/* Define to 1 if you have the <grp.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_GRP_H 1
#endif

/* Define if you have the 'hstrerror' function. */
#if !defined(MS_WINDOWS) && !defined(PSP)
#define HAVE_HSTRERROR 1
#endif

/* Define to 1 if you have the `hypot' function. */
#define HAVE_HYPOT 1

/* Define to 1 if you have the <ieeefp.h> header file. */
/* #undef HAVE_IEEEFP_H */

/* Define if you have the 'inet_aton' function. */
#ifdef MS_WINDOWS
# if defined(_WIN32_WINNT) && _WIN32_WINNT > 0x0501
#  define inet_aton(a,b) inet_pton(AF_INET,a,b) // for Vista or higher
# else
#  define inet_aton(a,b) (*a = inet_addr(a)) // for XP
# endif
#endif
#define HAVE_INET_ATON 1

/* Define if you have the 'inet_pton' function. */
#define HAVE_INET_PTON 1

/* Define to 1 if you have the `initgroups' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_INITGROUPS 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <io.h> header file. */
#ifdef MS_WINDOWS
#define HAVE_IO_H
#endif

/* Define to 1 if you have the `kill' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_KILL 1
#endif

/* Define to 1 if you have the `killpg' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_KILLPG 1
#endif

/* Define if you have the 'kqueue' functions. */
#ifdef __APPLE__
#define HAVE_KQUEUE 1
#endif

/* Define to 1 if you have the <langinfo.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_LANGINFO_H 1
#endif

/* Define to 1 if you have the `nl_langinfo' function. */
#ifndef __ANDROID__
#define HAVE_NL_LANGINFO
#elif __ANDROID_API__ > 25
#define HAVE_NL_LANGINFO
#endif

/* Defined to enable large file support when an off_t is bigger than a long
 and long long is available and at least as big as an off_t. You may need to
 add some flags for configuration and compilation to enable this mode. (For
 Solaris and Linux, the necessary defines are already defined.) */
#if defined(__LP64__) || defined(__3DS__) || defined(PSP)
#undef HAVE_LARGEFILE_SUPPORT
#else
#define HAVE_LARGEFILE_SUPPORT 1
#endif

/* Define to 1 if you have the `lchflags' function. */
/* #undef HAVE_LCHFLAGS */

/* Define to 1 if you have the `lchmod' function. */
#if !defined(__ANDROID__) && !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_LCHMOD 1
#endif

/* Define to 1 if you have the `lgamma' function. */
#define HAVE_LGAMMA 1

/* Define to 1 if you have the `dl' library (-ldl). */
#define HAVE_LIBDL 1

/* Define to 1 if you have the `dld' library (-ldld). */
/* #undef HAVE_LIBDLD */

/* Define to 1 if you have the `ieee' library (-lieee). */
/* #undef HAVE_LIBIEEE */

/* Define to 1 if you have the <libintl.h> header file. */
/* #undef HAVE_LIBINTL_H */

/* Define if you have the readline library (-lreadline). */
#define HAVE_LIBREADLINE 1

/* Define to 1 if you have the `resolv' library (-lresolv). */
/* #undef HAVE_LIBRESOLV */

/* Define to 1 if you have the <libutil.h> header file. */
/* #define HAVE_LIBUTIL_H 1 */

/* Define if you have the 'link' function. */
#if !defined(MS_WINDOWS) && !defined(__NX__)
#define HAVE_LINK 1
#endif

/* Define to 1 if you have the <linux/netlink.h> header file. */
#if defined(__ANDROID__) && defined(__linux__)
#define HAVE_LINUX_NETLINK_H
#endif

/* Define to 1 if you have the <linux/tipc.h> header file. */
/* #undef HAVE_LINUX_TIPC_H */

/* Define to 1 if you have the `localeconv' function. */
#ifndef __ANDROID__
#define HAVE_LOCALECONV 1
#endif

/* Define to 1 if you have the `log1p' function. */
#define HAVE_LOG1P 1

/* Define this if you have the type long double. */
#define HAVE_LONG_DOUBLE 1

/* Define this if you have the type long long. */
/* 64 bit ints are usually spelt __int64 unless compiler has overridden */
#define HAVE_LONG_LONG 1

/* Define to 1 if you have the `lstat' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_LSTAT 1
#endif

/* Define this if you have the makedev macro. */
#if !defined(__ANDROID__) && !defined(MS_WINDOWS)
#define HAVE_MAKEDEV 1
#endif

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkfifo' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_MKFIFO 1
#endif

/* Define to 1 if you have the `mknod' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_MKNOD 1
#endif

/* Define to 1 if you have the `mktime' function. */
#define HAVE_MKTIME 1

/* Define to 1 if you have the `mremap' function. */
/* #undef HAVE_MREMAP */

/* Define to 1 if you have the <ncurses.h> header file. */
#define HAVE_NCURSES_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netpacket/packet.h> header file. */
/* #undef HAVE_NETPACKET_PACKET_H */

/* Define to 1 if you have the `nice' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_NICE 1
#endif

/* Define to 1 if you have the `openpty' function. */
#ifdef __APPLE__
#define HAVE_OPENPTY 1
#endif

/* Define if compiling using MacOS X 10.5 SDK or later. */
#define HAVE_OSX105_SDK 1

/* Define to 1 if you have the `pathconf' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_PATHCONF 1
#endif

/* Define to 1 if you have the `pause' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_PAUSE 1
#endif

/* Define to 1 if you have the `plock' function. */
/* #undef HAVE_PLOCK */

/* Define to 1 if you have the `poll' function. */
/* #undef HAVE_POLL */

/* Define to 1 if you have the <poll.h> header file. */
/* #undef HAVE_POLL_H */

/* Define to 1 if you have the <process.h> header file. */
#ifdef MS_WINDOWS
#define HAVE_PROCESS_H */
#endif

/* Define if your compiler supports function prototype */
#define HAVE_PROTOTYPES 1

/* Define if you have GNU PTH threads. */
/* #undef HAVE_PTH */

/* Defined for Solaris 2.6 bug in pthread header. */
/* #undef HAVE_PTHREAD_DESTRUCTOR */

/* Define to 1 if you have the <pthread.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_PTHREAD_H 1
#endif

/* Define to 1 if you have the `pthread_init' function. */
#define HAVE_PTHREAD_INIT 1

/* Define to 1 if you have the `pthread_sigmask' function. */
#define HAVE_PTHREAD_SIGMASK 1

/* Define to 1 if you have the <pty.h> header file. */
/* #undef HAVE_PTY_H */

/* Define to 1 if you have the `putenv' function. */
#define HAVE_PUTENV 1

/* Define to 1 if you have the `readlink' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_READLINK 1
#endif

/* Define to 1 if you have the `realpath' function. */
#ifndef LIMITED_POSIX
#define HAVE_REALPATH 1
#endif

/* Define if you have readline 2.1 */
#define HAVE_RL_CALLBACK 1

/* Define if you can turn off readline's signal handling. */
/* #undef HAVE_RL_CATCH_SIGNAL */

/* Define if you have readline 2.2 */
#define HAVE_RL_COMPLETION_APPEND_CHARACTER 1

/* Define if you have readline 4.0 */
/* #undef HAVE_RL_COMPLETION_DISPLAY_MATCHES_HOOK */

/* Define if you have readline 4.2 */
#define HAVE_RL_COMPLETION_MATCHES 1

/* Define if you have rl_completion_suppress_append */
/* #undef HAVE_RL_COMPLETION_SUPPRESS_APPEND */

/* Define if you have readline 4.0 */
#define HAVE_RL_PRE_INPUT_HOOK 1

/* Define to 1 if you have the `round' function. */
#define HAVE_ROUND 1

/* Define to 1 if you have the `select' function. */
#ifndef MS_WINDOWS
#define HAVE_SELECT 1
#endif

/* Define to 1 if you have the `sem_getvalue' function. */
#define HAVE_SEM_GETVALUE 1

/* Define to 1 if you have the `sem_open' function. */
#define HAVE_SEM_OPEN 1

/* Define to 1 if you have the `sem_timedwait' function. */
/* #undef HAVE_SEM_TIMEDWAIT */

/* Define to 1 if you have the `sem_unlink' function. */
#define HAVE_SEM_UNLINK 1

/* Define to 1 if you have the `setegid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETEGID 1
#endif

/* Define to 1 if you have the `seteuid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETEUID 1
#endif

/* Define to 1 if you have the `setgid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETGID 1
#endif

/* Define if you have the 'setgroups' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETGROUPS 1
#endif

/* Define to 1 if you have the `setitimer' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETITIMER 1
#endif

/* Define to 1 if you have the `setlocale' function. */
#define HAVE_SETLOCALE 1

/* Define to 1 if you have the `setpgid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETPGID 1
#endif

/* Define to 1 if you have the `setpgrp' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETPGRP 1
#endif

/* Define to 1 if you have the `setregid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETREGID 1
#endif

/* Define to 1 if you have the `setresgid' function. */
/* #undef HAVE_SETRESGID */

/* Define to 1 if you have the `setresuid' function. */
/* #undef HAVE_SETRESUID */

/* Define to 1 if you have the `setreuid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETREUID 1
#endif

/* Define to 1 if you have the `setsid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETSID 1
#endif

/* Define to 1 if you have the `setuid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SETUID 1
#endif

/* Define to 1 if you have the `setvbuf' function. */
#define HAVE_SETVBUF 1

/* Define to 1 if you have the <shadow.h> header file. */
/* #undef HAVE_SHADOW_H */

/* Define to 1 if you have the `sigaction' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SIGACTION 1
#endif

/* Define to 1 if you have the `siginterrupt' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SIGINTERRUPT 1
#endif

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the `sigrelse' function. */
#define HAVE_SIGRELSE 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define if sockaddr has sa_len member */
#ifdef __APPLE__
#define HAVE_SOCKADDR_SA_LEN 1
#endif

/* struct sockaddr_storage (sys/socket.h) */
#define HAVE_SOCKADDR_STORAGE 1

/* Define if you have the 'socketpair' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SOCKETPAIR 1
#endif

/* Define to 1 if you have the <spawn.h> header file. */
#define HAVE_SPAWN_H 1

/* Define if your compiler provides ssize_t */
#ifdef MS_WINDOWS
/* Define like size_t, omitting the "unsigned" */
#ifdef MS_WIN64
typedef __int64 ssize_t;
#else
typedef _W64 int ssize_t;
#endif
#endif /* MS_WINDOWS */
#define HAVE_SSIZE_T 1

#ifdef __APPLE__
/* Define if you have struct stat.st_mtimensec */
#define HAVE_STAT_TV_NSEC2 1
#elif !defined(MS_WINDOWS) && !defined(PSP)
/* Define if you have struct stat.st_mtim.tv_nsec */
# define HAVE_STAT_TV_NSEC
#endif /* __APPLE__ */

/* Define if your compiler supports variable length function prototypes (e.g.
 void fprintf(FILE *, char *, ...);) *and* <stdarg.h> */
#define HAVE_STDARG_PROTOTYPES 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `stpcpy' function. */
#if !defined(__ANDROID__) && !defined(MS_WINDOWS)
#define HAVE_STPCPY 1
#endif

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <stropts.h> header file. */
/* #undef HAVE_STROPTS_H */

/* Define to 1 if `st_birthtime' is a member of `struct stat'. */
#ifdef __APPLE__
#define HAVE_STRUCT_STAT_ST_BIRTHTIME 1
#endif

/* Define to 1 if `st_blksize' is a member of `struct stat'. */
#ifndef MS_WINDOWS
#define HAVE_STRUCT_STAT_ST_BLKSIZE 1
#endif

/* Define to 1 if `st_blocks' is a member of `struct stat'. */
#ifndef MS_WINDOWS
#define HAVE_STRUCT_STAT_ST_BLOCKS 1
#endif

/* Define to 1 if `st_flags' is a member of `struct stat'. */
#ifdef __APPLE__
#define HAVE_STRUCT_STAT_ST_FLAGS 1
#endif

/* Define to 1 if `st_gen' is a member of `struct stat'. */
#ifdef __APPLE__
#define HAVE_STRUCT_STAT_ST_GEN 1
#endif

/* Define to 1 if `st_rdev' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_RDEV 1

/* Define to 1 if `tm_zone' is a member of `struct tm'. */
#ifndef PSP
#define HAVE_STRUCT_TM_TM_ZONE 1
#endif

/* Define to 1 if your `struct stat' has `st_blocks'. Deprecated, use
 `HAVE_STRUCT_STAT_ST_BLOCKS' instead. */
#define HAVE_ST_BLOCKS 1

/* Define if you have the 'symlink' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SYMLINK 1
#endif

/* Define to 1 if you have the `sysconf' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_SYSCONF 1
#endif

/* Define to 1 if you have the <sysexits.h> header file. */
#if !defined(MS_WINDOWS) && !defined(PSP)
#define HAVE_SYSEXITS_H 1
#endif

/* Define to 1 if you have the <sys/audioio.h> header file. */
/* #undef HAVE_SYS_AUDIOIO_H */

/* Define to 1 if you have the <sys/bsdtty.h> header file. */
/* #undef HAVE_SYS_BSDTTY_H */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
 */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/epoll.h> header file. */
/* #undef HAVE_SYS_EPOLL_H */

/* Define to 1 if you have the <sys/event.h> header file. */
#ifdef __APPLE__
#define HAVE_SYS_EVENT_H 1
#endif

/* Define to 1 if you have the <sys/file.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_SYS_FILE_H 1
#endif

/* Define to 1 if you have the <sys/loadavg.h> header file. */
/* #undef HAVE_SYS_LOADAVG_H */

/* Define to 1 if you have the <sys/lock.h> header file. */
#define HAVE_SYS_LOCK_H 1

/* Define to 1 if you have the <sys/mkdev.h> header file. */
/* #undef HAVE_SYS_MKDEV_H */

/* Define to 1 if you have the <sys/modem.h> header file. */
/* #undef HAVE_SYS_MODEM_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
 */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_SYS_PARAM_H 1
#endif

/* Define to 1 if you have the <sys/poll.h> header file. */
#if !defined(MS_WINDOWS) && !defined(PSP)
#define HAVE_SYS_POLL_H 1
#endif

/* Define to 1 if you have the <sys/resource.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_SYS_RESOURCE_H 1
#endif

/* Define to 1 if you have the <sys/select.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_SYS_SELECT_H 1
#endif

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/statvfs.h> header file. */
#define HAVE_SYS_STATVFS_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/termio.h> header file. */
/* #undef HAVE_SYS_TERMIO_H */

/* Define to 1 if you have the <sys/times.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_SYS_TIMES_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
#if !defined(MS_WINDOWS) && !defined(PSP)
#define HAVE_SYS_UN_H 1
#endif

/* Define to 1 if you have the <sys/utsname.h> header file. */
#if !defined(MS_WINDOWS) && !defined(PSP)
#define HAVE_SYS_UTSNAME_H 1
#endif

/* Define to 1 if you have the <sys/wait.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_SYS_WAIT_H 1
#endif

/* Define to 1 if you have the `tcgetpgrp' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_TCGETPGRP 1
#endif

/* Define to 1 if you have the `tcsetpgrp' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_TCSETPGRP 1
#endif

/* Define to 1 if you have the `tempnam' function. */
#ifndef __NX__
#define HAVE_TEMPNAM 1
#endif

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* Define to 1 if you have the <term.h> header file. */
#define HAVE_TERM_H 1

/* Define to 1 if you have the `tgamma' function. */
#define HAVE_TGAMMA 1

/* Define to 1 if you have the <thread.h> header file. */
/* #undef HAVE_THREAD_H */

/* Define to 1 if you have the `timegm' function. */
#define HAVE_TIMEGM 1

/* Define to 1 if you have the `times' function. */
#if !defined(MS_WINDOWS) && !defined(__NX__)
#define HAVE_TIMES 1
#endif

/* Define to 1 if you have the `tmpfile' function. */
#define HAVE_TMPFILE 1

/* Define to 1 if you have the `tmpnam' function. */
#ifndef __NX__
#define HAVE_TMPNAM 1
#endif

/* Define to 1 if you have the `tmpnam_r' function. */
/* #undef HAVE_TMPNAM_R */

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
 `HAVE_STRUCT_TM_TM_ZONE' instead. */
#ifndef MS_WINDOWS
#define HAVE_TM_ZONE 1
#endif

/* Define to 1 if you have the `truncate' function. */
#define HAVE_TRUNCATE 1

/* Define to 1 if you don't have `tm_zone' but do have the external array
 `tzname'. */
#ifdef MS_WINDOWS
#define HAVE_TZNAME
#endif

/* Define this if you have tcl and TCL_UTF_MAX==6 */
/* #undef HAVE_UCS4_TCL */

/* Define to 1 if the system has the type `uintptr_t'. */
#define HAVE_UINTPTR_T 1

/* Define to 1 if you have the `uname' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_UNAME 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the `unsetenv' function. */
#ifndef MS_WINDOWS
#define HAVE_UNSETENV 1
#endif

/* Define if you have a useable wchar_t type defined in wchar.h; useable means
 wchar_t must be an unsigned type with at least 16 bits. (see
 Include/unicodeobject.h). */
/* #undef HAVE_USABLE_WCHAR_T */

/* Define to 1 if you have the <util.h> header file. */
#ifdef __APPLE__
#define HAVE_UTIL_H 1
#endif

/* Define to 1 if you have the `utimes' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_UTIMES 1
#endif

/* Define to 1 if you have the <utime.h> header file. */
#ifndef MS_WINDOWS
#define HAVE_UTIME_H 1
#endif

/* Define to 1 if you have the `wait3' function. */
#if !defined(__ANDROID__) && !defined(MS_WINDOWS)  && !defined(LIMITED_POSIX)
#define HAVE_WAIT3 1
#endif

/* Define to 1 if you have the `wait4' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_WAIT4 1
#endif

/* Define to 1 if you have the `waitpid' function. */
#if !defined(MS_WINDOWS) && !defined(LIMITED_POSIX)
#define HAVE_WAITPID 1
#endif

/* Define if the compiler provides a wchar.h header file. */
#define HAVE_WCHAR_H 1

/* Define to 1 if you have the `wcscoll' function. */
#define HAVE_WCSCOLL 1

/* Define if tzset() actually switches the local timezone in a meaningful way.
 */
#if !defined(MS_WINDOWS) && !defined(__NX__)
#define HAVE_WORKING_TZSET 1
#endif

/* Define if the zlib library has inflateCopy */
#define HAVE_ZLIB_COPY 1

/* Define to 1 if you have the `_getpty' function. */
/* #undef HAVE__GETPTY */

/* Define if you are using Mach cthreads directly under /include */
/* #undef HURD_C_THREADS */

/* Define if you are using Mach cthreads under mach / */
/* #undef MACH_C_THREADS */

/* Define to 1 if `major', `minor', and `makedev' are declared in <mkdev.h>.
 */
/* #undef MAJOR_IN_MKDEV */

/* Define to 1 if `major', `minor', and `makedev' are declared in
 <sysmacros.h>. */
#ifdef __linux__
#define MAJOR_IN_SYSMACROS
#endif

/* Define if mvwdelch in curses.h is an expression. */
#define MVWDELCH_IS_EXPRESSION 1

/* Define to the address where bug reports for this package should be sent. */
/* #undef PACKAGE_BUGREPORT */

/* Define to the full name of this package. */
/* #undef PACKAGE_NAME */

/* Define to the full name and version of this package. */
/* #undef PACKAGE_STRING */

/* Define to the one symbol short name of this package. */
/* #undef PACKAGE_TARNAME */

/* Define to the home page for this package. */
/* #undef PACKAGE_URL */

/* Define to the version of this package. */
/* #undef PACKAGE_VERSION */

/* Define if POSIX semaphores aren't enabled on your system */
/* #undef POSIX_SEMAPHORES_NOT_ENABLED */

/* Defined if PTHREAD_SCOPE_SYSTEM supported. */
#ifndef LIMITED_POSIX
#define PTHREAD_SYSTEM_SCHED_SUPPORTED 1
#endif

/* Define as the preferred size in bits of long digits */
/* #undef PYLONG_BITS_IN_DIGIT */

/* Define to printf format modifier for long long type */
#define PY_FORMAT_LONG_LONG "ll"

/* Define to printf format modifier for Py_ssize_t */
#define PY_FORMAT_SIZE_T "z"

/* Define as the integral type used for Unicode representation. */
#define PY_UNICODE_TYPE unsigned short

/* Define if you want to build an interpreter with many run-time checks. */
/* #undef Py_DEBUG */

/* Defined if Python is built as a shared library. */
/* #undef Py_ENABLE_SHARED */

/* Define as the size of the unicode type. */
#define Py_UNICODE_SIZE 2

/* Define if you want to have a Unicode type. */
#define Py_USING_UNICODE 1

/* assume C89 semantics that RETSIGTYPE is always void */
#define RETSIGTYPE void

/* Define if setpgrp() must be called as setpgrp(0, 0). */
/* #undef SETPGRP_HAVE_ARG */

/* Define this to be extension of shared libraries (including the dot!). */
#ifdef MS_WINDOWS
#define SHLIB_EXT ".dll"
#else
#define SHLIB_EXT ".so"
#endif

/* Define if i>>j for signed int i does not extend the sign bit when i < 0 */
/* #undef SIGNED_RIGHT_SHIFT_ZERO_FILLS */

/* The size of `double', as computed by sizeof. */
#ifdef __SIZEOF_DOUBLE__
#define SIZEOF_DOUBLE __SIZEOF_DOUBLE__
#else
#define SIZEOF_DOUBLE sizeof(double)
#endif

/* The size of `float', as computed by sizeof. */
#ifdef __SIZEOF_FLOAT__
#define SIZEOF_FLOAT __SIZEOF_FLOAT__
#else
#define SIZEOF_FLOAT sizeof(float)
#endif

/* The size of `fpos_t', as computed by sizeof. */
#define SIZEOF_FPOS_T 8

/* The size of `int', as computed by sizeof. */
#ifdef __SIZEOF_INT__
#define SIZEOF_INT __SIZEOF_INT__
#else
#define SIZEOF_INT sizeof(int)
#endif

/* The size of `long', as computed by sizeof. */
#ifdef __SIZEOF_LONG__
#define SIZEOF_LONG __SIZEOF_LONG__
#else
#ifdef __LP64__
#define SIZEOF_LONG 8
#else
#define SIZEOF_LONG sizeof(long)
#endif
#endif

/* The size of `long double', as computed by sizeof. */
#ifdef __SIZEOF_LONG_DOUBLE__
#define SIZEOF_LONG_DOUBLE __SIZEOF_LONG_DOUBLE__
#else
#define SIZEOF_LONG_DOUBLE sizeof(long double)
#endif

/* The size of `long long', as computed by sizeof. */
#ifdef __SIZEOF_LONG_LONG__
#define SIZEOF_LONG_LONG __SIZEOF_LONG_LONG__
#else
#define SIZEOF_LONG_LONG sizeof(long long)
#endif

/* The size of `off_t', as computed by sizeof. */
#ifdef __LP64__
#define SIZEOF_OFF_T 8
#else
#define SIZEOF_OFF_T 4
#endif

/* The size of `pid_t', as computed by sizeof. */
#define SIZEOF_PID_T 4

/* The size of `pthread_t', as computed by sizeof. */
#ifdef __LP64__
#define SIZEOF_PTHREAD_T 8
#else
#define SIZEOF_PTHREAD_T 4
#endif

/* The size of `short', as computed by sizeof. */
#ifdef __SIZEOF_SHORT__
#define SIZEOF_SHORT __SIZEOF_SHORT__
#else
#define SIZEOF_SHORT sizeof(short)
#endif

/* The size of `size_t', as computed by sizeof. */
#ifdef __LP64__
#define SIZEOF_SIZE_T 8
#else
#define SIZEOF_SIZE_T 4
#endif

/* The size of `time_t', as computed by sizeof. */
#ifdef __LP64__
#define SIZEOF_TIME_T 8
#else
#define SIZEOF_TIME_T 4
#endif

/* The size of `uintptr_t', as computed by sizeof. */
#ifdef __LP64__
#define SIZEOF_UINTPTR_T 8
#else
#define SIZEOF_UINTPTR_T 4
#endif

/* The size of `void *', as computed by sizeof. */
#if defined(__LP64__) || defined(MS_WIN64)
#define SIZEOF_VOID_P 8
#else
#ifdef __SIZEOF_POINTER__
#define SIZEOF_VOID_P __SIZEOF_POINTER__
#else
#define SIZEOF_VOID_P sizeof(void*)
#endif
#endif

/* The size of `wchar_t', as computed by sizeof. */
#ifdef __SIZEOF_WCHAR_T__
#define SIZEOF_WCHAR_T __SIZEOF_WCHAR_T__
#else
#define SIZEOF_WCHAR_T sizeof(wchar_t)
#endif

/* The size of `_Bool', as computed by sizeof. */
#define SIZEOF__BOOL 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/select.h> and <sys/time.h>
 (which you can't on SCO ODT 3.0). */
#define SYS_SELECT_WITH_SYS_TIME 1

/* Define if tanh(-0.) is -0., or if platform doesn't have signed zeros */
#define TANH_PRESERVES_ZERO_SIGN 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#ifndef MS_WINDOWS
#define TIME_WITH_SYS_TIME 1
#endif

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Define if you want to use MacPython modules on MacOSX in unix-Python. */
#define USE_TOOLBOX_OBJECT_GLUE 1

/* Define if a va_list is an array of some kind */
/* #undef VA_LIST_IS_ARRAY */

/* Define if you want SIGFPE handled (see Include/pyfpe.h). */
/* #undef WANT_SIGFPE_HANDLER */

/* Define if you want wctype.h functions to be used instead of the one
 supplied by Python itself. (see Include/unicodectype.h). */
/* #undef WANT_WCTYPE_FUNCTIONS */

/* Define if WINDOW in curses.h offers a field _flags. */
/* #undef WINDOW_HAS_FLAGS */

/* Define if you want documentation strings in extension modules */
#define WITH_DOC_STRINGS 1

/* Define if you want to use the new-style (Openstep, Rhapsody, MacOS) dynamic
 linker (dyld) instead of the old-style (NextStep) dynamic linker (rld).
 Dyld is necessary to support frameworks. */
/* #define WITH_DYLD 1 */

/* Define to 1 if libintl is needed for locale functions. */
/* #undef WITH_LIBINTL */

/* Define if you want to produce an OpenStep/Rhapsody framework (shared
 library plus accessory files). */
// This is also MacOSX. However, it assumes e.g. existance of _NSGetEnviron
// and other things and thus isn't really suitable for iOS.
/* #define WITH_NEXT_FRAMEWORK 1 */

/* Define if you want to compile in Python-specific mallocs */
// Pool size too big by default for iOS? Anyway, maybe not the best
// thing for embedded systems where memory is low.
/* #define WITH_PYMALLOC 1 */

/* Define if you want to compile in rudimentary thread support */
#define WITH_THREAD 1

/* Define to profile with the Pentium timestamp counter */
/* #undef WITH_TSC */

/* Define if you want pymalloc to be disabled when running under valgrind */
/* #undef WITH_VALGRIND */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
 significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define if arithmetic is subject to x87-style double rounding issue */
/* #undef X87_DOUBLE_ROUNDING */

/* Define on OpenBSD to activate all library features */
/* #undef _BSD_SOURCE */

/* Define on Irix to enable u_int */
#define _BSD_TYPES 1

/* Define on Darwin to activate all library features */
#define _DARWIN_C_SOURCE 1

/* This must be set to 64 on some systems to enable large file support. */
#define _FILE_OFFSET_BITS 64

/* Define on Linux to activate all library features */
#define _GNU_SOURCE 1

/* This must be defined on some systems to enable large file support. */
#define _LARGEFILE_SOURCE 1

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define on NetBSD to activate all library features */
#define _NETBSD_SOURCE 1

/* Define _OSF_SOURCE to get the makedev macro. */
/* #undef _OSF_SOURCE */

/* Define to 2 if the system does not provide POSIX.1 features except with
 this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to activate features from IEEE Stds 1003.1-2001 */
/* #undef _POSIX_C_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define if you have POSIX threads, and your system does not define that. */
/* #undef _POSIX_THREADS */

/* Define to force use of thread-safe errno, h_errno, and other functions */
#define _REENTRANT 1

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
 <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
 #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
 <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
 #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define to the level of X/Open that your system supports */
/* #undef _XOPEN_SOURCE */

/* Define to activate Unix95-and-earlier features */
/* #undef _XOPEN_SOURCE_EXTENDED */

/* Define on FreeBSD to activate all library features */
#define __BSD_VISIBLE 1

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Defined on Solaris to see additional function prototypes. */
#define __EXTENSIONS__ 1

/* Define to 'long' if <time.h> doesn't define. */
/* #undef clock_t */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
#ifdef MS_WINDOWS
typedef int gid_t;
#endif

/* Define to the type of a signed integer type of width exactly 32 bits if
 such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
 such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to `int' if <sys/types.h> does not define. */
#ifdef MS_WINDOWS
typedef int mode_t;
#endif

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
#ifdef MS_WINDOWS
typedef int pid_t;
#endif

/* Define to empty if the keyword does not work. */
/* #undef signed */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/socket.h> does not define. */
/* #undef socklen_t */

/* Define to `int' if <sys/types.h> doesn't define. */
#ifdef MS_WINDOWS
typedef int uid_t;
#endif

/* Define to the type of an unsigned integer type of width exactly 32 bits if
 such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
 such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to empty if the keyword does not work. */
/* #undef volatile */


/* Define the macros needed if on a UnixWare 7.x system. */
#if defined(__USLC__) && defined(__SCO_VERSION__)
#define STRICT_SYSV_CURSES /* Don't use ncurses extensions */
#endif

#endif /*Py_PYCONFIG_H*/

#ifndef HAVE_STDDEF_H
#define HAVE_STDDEF_H 1
#endif

// In posixmodule.c for Apple, these use weak linking. This is
// not supported if we build a static library, so just disable them for now.

/* Define to 1 if you have the `fstatvfs' function. */
/* #define HAVE_FSTATVFS 1 */

/* Define to 1 if you have the `statvfs' function. */
/* #define HAVE_STATVFS 1 */

/* Define to 1 if you have the `lchown' function. */
/* #define HAVE_LCHOWN 1 */

/* #define SIZEOF_SHORT sizeof(short) */
