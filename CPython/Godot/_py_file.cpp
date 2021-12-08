
#include "_py_file.h"

#include "core/os/os.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "core/math/math_funcs.h"
#include "handle_map.h"

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <map>
#include <memory>

#define SUCCESS (0)
#define FAILURE (-1)

enum ExModeFlags {

	EX_CREATE = 256,
	EX_APPEND = 512,
};

static DirAccess *_current_dir = nullptr;

static int _conv_perm(const String &p_path, int mode) {
    // Convert permissions
    int flags = 0;
    if (mode & O_RDONLY) {
		flags = FileAccess::READ;
    } else if (mode & O_WRONLY) {
        flags = FileAccess::WRITE;
    }

    if (mode & O_APPEND) {
		if (FileAccess::exists(p_path)) {
			flags = FileAccess::READ_WRITE | EX_APPEND;
		} else {
			flags = FileAccess::WRITE_READ;
		}
    }

    if (flags == 0) {
        flags = FileAccess::READ; // default
    }

    return flags;
}

static int _conv_perm(const String &p_path, const String &mode) {
#define check(C) (mode.has(C))
	// Convert permissions
	int flags = 0;
	if (check("r+"))
			flags = FileAccess::READ_WRITE;
	else if (check("w+"))
			flags = FileAccess::WRITE_READ;
	else if (check("r"))
			flags = FileAccess::READ;
	else if (check("w"))
			flags = FileAccess::WRITE;

	if (check("a")) {
		if (FileAccess::exists(p_path)) {
			flags = FileAccess::READ_WRITE | EX_APPEND;
		} else {
			flags = FileAccess::WRITE_READ;
		}
	}

	if (flags == 0)
		flags = FileAccess::READ; // default
#undef check
	return flags;
}

struct PYFILE {
	FileAccess *fa;
	static String fixpath(const String &p_path) {
		if (_current_dir
			&& !p_path.is_abs_path()
			&& !FileAccess::exists(p_path)
		) {
			return _current_dir->get_current_dir().append_path(p_path);
		}
		return p_path;
	}
	static PYFILE *fopen(const String &p_path, int p_mode_flags) {
		if (!p_path.empty()) {
			const String real_path = fixpath(p_path);
			const int flags = _conv_perm(real_path, p_mode_flags);
			if (FileAccess *_fa = FileAccess::open(real_path, flags&0xff)) {
				if (flags & EX_APPEND) {
					_fa->seek_end();
				}
				return memnew(PYFILE(_fa));
			} else {
				return nullptr;
			}
		}
		return nullptr;
	}
	static PYFILE *fopen(const String &p_path, const String &p_mode_flags) {
		if (!p_path.empty()) {
			const String real_path = fixpath(p_path);
			const int flags = _conv_perm(real_path, p_mode_flags);
			if (FileAccess *_fa = FileAccess::open(real_path, flags&0xff)) {
				if (flags & EX_APPEND) {
					_fa->seek_end();
				}
				return memnew(PYFILE(_fa));
			} else {
				return nullptr;
			}
		}
		return nullptr;
	}
	PYFILE(FileAccess *p_fa) {
		fa = p_fa;
	}
	PYFILE() {
		fa = nullptr;
	}
	~PYFILE() {
		if (fa) {
			memdelete(fa);
		}
	}
};

static PYFILE gd_stdin;
static PYFILE gd_stdout;
static PYFILE gd_stderr;

handle_map<PYFILE*> _handles(1, 32);

int _gd_open(const char* name, int flags, ...) {
	PYFILE *f = PYFILE::fopen(name, flags);
	if (f) {
		return _handles.insert(f).value;
	}
	return 0;
}

int _gd_close(int fd) {
	if (fd > 0) {
		const Id_T t =  make_handle(fd);
		if (_handles.is_valid(t)) {
			PYFILE *f = _handles[t];
			f->fa->close();
			_handles.erase(t);
			memdelete(f);
			return 0;
		}
	}
	return FAILURE;
}

int _gd_lseek(int fd, off_t offset, int whence) {
	if (fd > 0) {
		const Id_T t =  make_handle(fd);
		if (_handles.is_valid(t)) {
			return _gd_fseek(_handles[t], offset, whence);
		}
	}
	return FAILURE;
}

off_t _gd_ltell(int fd) {
	if (fd > 0) {
		const Id_T t =  make_handle(fd);
		if (_handles.is_valid(t)) {
			return _gd_ftell(_handles[t]);
		}
	}
	return 0;
}

ssize_t _gd_read(int fd, void* buf, size_t len) {
	if (fd > 0) {
		const Id_T t =  make_handle(fd);
		if (_handles.is_valid(t)) {
			return _gd_fread(buf, len, 1, _handles[t]);
		}
	}
	return 0;
}

ssize_t _gd_write(int fd, const void* buf, size_t len) {
	if (fd > 0) {
		const Id_T t =  make_handle(fd);
		if (_handles.is_valid(t)) {
			return _gd_fwrite(buf, len, 1, _handles[t]);
		}
	}
	return 0;
}

ssize_t _gd_filesize(int fd) {
	if (fd > 0) {
		const Id_T t =  make_handle(fd);
		if (_handles.is_valid(t)) {
			return _gd_ffilesize(_handles[t]);
		}
	}
	return FAILURE;
}

PYFILE *_gd_fopen(const char* name, const char *mode) {
	return PYFILE::fopen(name, mode);
}

PYFILE *_gd_wfopen(const wchar_t *name, const wchar_t *mode) {
	return PYFILE::fopen(String(name), String(mode));
}

PYFILE *_gd_fdopen(const int fd, const char *mode) {
	if (fd > 0) {
		const Id_T t = make_handle(fd);
		if (_handles.is_valid(t)) {
			return _handles[t];
		}
	}
	return nullptr;
}

static bool _is_link(const String &path) {
	DirAccessRef da(DirAccess::create_for_path(path));
	return da->is_link(path);
}

int _gd_chdir(const char *dir) {
	if (!_current_dir) {
		_current_dir = DirAccess::create_for_path(dir);
	}
	switch (_current_dir->change_dir(dir)) {
		case ERR_INVALID_PARAMETER: return ENOENT;
		case ERR_UNAVAILABLE: return EACCES;
		case OK: return 0;
		default: break;
	}
	return EFAULT;
}

char *_gd_getcwd(char *buf, int size) {
	if (!_current_dir) {
		return nullptr;
	}
	if (const char *curr = _current_dir->get_current_dir().utf8().get_data()) {
		if (!buf) {
			buf = (char*)malloc(strlen(curr));
		}
		strncpy(buf, curr, size);
		return buf;
	}
	return nullptr;
}

int _gd_unlink(const char *path) {
	DirAccessRef da(DirAccess::create_for_path(path));
	return da->remove(path) == OK ? SUCCESS : FAILURE;
}

int _gd_fstat(PYFILE *f, struct stat *buf) {
	if (f) {
		String path = f->fa->get_path();
		if (DirAccess::exists(path)) { if (buf) buf->st_mode = S_IFDIR; }
		else if (FileAccess::exists(path)) { if (buf) buf->st_mode = S_IFREG; }
		else return FAILURE;
#ifdef S_IFLNK
		if (_is_link(path)) { if (buf) buf->st_mode = S_IFLNK; }
#endif
		if (buf) { buf->st_ctime = buf->st_mtime = FileAccess::get_modified_time(path); }
		return SUCCESS;
	}
	return FAILURE;
}

int _gd_stat(const char *path, struct stat *buf) {
	if (DirAccess::exists(path)) { if (buf) buf->st_mode = S_IFDIR; }
	else if (FileAccess::exists(path)) { if (buf) buf->st_mode = S_IFREG; }
	else return FAILURE;
#ifdef S_IFLNK
	if (_is_link(path)) { if (buf) buf->st_mode = S_IFLNK; }
#endif
	if (buf) {  buf->st_ctime = buf->st_mtime = FileAccess::get_modified_time(path); }
	return SUCCESS;
}

int _gd_fclose(PYFILE *f) {
	if (f) {
		if (f->fa) {
			f->fa->close();
			return SUCCESS;
		} else {
			return FAILURE;
		}
	}
	return FAILURE;
}

int _gd_fseek(PYFILE *f, off_t offset, int whence) {
	if (f) {
		switch (whence) {
			case SEEK_SET: f->fa->seek(offset); break;
			case SEEK_CUR: f->fa->seek(f->fa->get_position() + offset); break;
			case SEEK_END: f->fa->seek_end(offset); break;
			default: return FAILURE;
		}
		return SUCCESS;
	}
	return FAILURE;
}

off_t _gd_ftell(PYFILE *f) {
	if (f) {
		return f->fa->get_position();
	}
	return FAILURE;
}

void _gd_rewind(PYFILE *f) {
	return f->fa->seek(0);
}

ssize_t _gd_fread(void* buf, size_t len, size_t cnt, PYFILE *f) {
	if (f) {
		return f->fa->get_buffer((uint8_t*)buf, cnt * len);
	}
	return SUCCESS;
}

ssize_t _gd_fwrite(const void* buf, size_t len, size_t cnt, PYFILE *f) {
	if (f) {
		if (f->fa) {
			f->fa->store_buffer((const uint8_t*)buf, cnt * len);
		} else if (f == _gd_stdout()) {
			return fwrite(buf, len, cnt, stdout);
		} else if (f == _gd_stderr()) {
			return fwrite(buf, len, cnt, stderr);
#ifdef PYSTDERR_TO_FILE
			FileAccessRef log(FileAccess::open("py_stderr.txt", FileAccess::READ_WRITE));
			if (log) {
				log->store_buffer((const uint8_t*)buf, cnt * len);
			}
#endif
		} else if (f == _gd_stdin()) {
			WARN_PRINT("File handle should not be stdin - information lost.");
			return 0;
		} else {
			WARN_PRINT("Undefined file access - information lost.");
			return 0;
		}
		return cnt * len;
	}
	return 0;
}

#if defined(MINGW_ENABLED) || defined(_MSC_VER) && _MSC_VER < 1900
#define gd_vsnprintf(m_buffer, m_count, m_format, m_args_copy) vsnprintf_s(m_buffer, m_count, _TRUNCATE, m_format, m_args_copy)
#define gd_vscprintf(m_format, m_args_copy) _vscprintf(m_format, m_args_copy)
#else
#define gd_vsnprintf(m_buffer, m_count, m_format, m_args_copy) vsnprintf(m_buffer, m_count, m_format, m_args_copy)
#define gd_vscprintf(m_format, m_args_copy) vsnprintf(NULL, 0, m_format, m_args_copy)
#endif

void _gd_fprintf(PYFILE *f, const char *format, ...) {
	va_list argp;
	va_start(argp, format);
	_gd_vfprintf(f, format, argp);
	va_end(argp);
}

void _gd_vfprintf(PYFILE *f, const char *format, va_list ap) {
	if (f) {
		va_list list;

		va_copy(list, ap);
		int len = gd_vscprintf(format, list);
		va_end(list);

		len += 1; // for the trailing '/0'

		char *buffer(memnew_arr(char, len));

		va_copy(list, ap);
		gd_vsnprintf(buffer, len, format, list);
		va_end(list);

		if (f->fa) {
			f->fa->store_buffer((const uint8_t*)buffer, len);
		} else if (f == _gd_stdout()) {
			vfprintf(stdout, format, ap);
		} else if (f == _gd_stderr()) {
			vfprintf(stderr, format, ap);
#ifdef PYSTDERR_TO_FILE
			FileAccessRef log(FileAccess::open("py_stderr.txt", FileAccess::READ_WRITE));
			if (log) {
				log->store_buffer((const uint8_t*)buffer, len);
			}
#endif
		} else if (f == _gd_stdin()) {
			WARN_PRINT("File handle should not be stdin - information lost.");
		} else {
			WARN_PRINT("Undefined file access - information lost.");
		}

		memdelete_arr(buffer);
	} else {
		WARN_PRINT("File handle should not be null - information lost.");
	}
}

char *_gd_fgets(char* buf, size_t len, PYFILE *f) {
	if (f) {
		const char *line = f->fa->get_line().utf8().get_data();
		strncpy(buf, line, len);
		return buf;
	}
	return 0;
}

int _gd_fputs(const char* buf, PYFILE *f) {
	if (f) {
		if (f->fa) {
			f->fa->store_string(String(buf));
		} else if (f == _gd_stdout()) {
			fputs(buf, stdout);
		} else if (f == _gd_stderr()) {
			fputs(buf, stderr);
		} else if (f == _gd_stdin()) {
			WARN_PRINT("File handle should not be stdin - information lost.");
		} else {
			WARN_PRINT("Undefined file access - information lost.");
		}
		return 1;
	}
	return EOF;
}

int _gd_getc(PYFILE *f) {
	if (f) {
		const int b = f->fa->get_8();
		return f->fa->eof_reached() ? EOF : b;
	}
	return EOF;
}

int _gd_putc(int ch, PYFILE *f) {
	if (f) {
		if (f->fa) {
			f->fa->store_8(ch);
		} else if (f == _gd_stdout()) {
			putchar(ch);
		} else if (f == _gd_stderr()) {
			fputc(ch, stderr);
		} else if (f == _gd_stdin()) {
			WARN_PRINT("File handle should not be stdin - information lost.");
		} else {
			WARN_PRINT("Undefined file access - information lost.");
		}
		return ch;
	}
	return EOF;
}

int _gd_ungetc(int c, PYFILE *f) {
	if (f) {
		const uint64_t cur = f->fa->get_position();
		if (cur) {
			f->fa->seek(cur - 1);
			return c;
		}
	}
	return EOF;
}

PYFILE *_gd_tmpfile() {
	Math::randomize();
	const OS::Date dt = OS::get_singleton()->get_date();
	const uint64_t tk = OS::get_singleton()->get_ticks_usec();
	const String tmpfilename = vformat("_%d%d%d_%d%d.tmp", dt.day, dt.month, dt.year, Math::rand(), tk%10);
	return PYFILE::fopen(tmpfilename, "w");
}

int _gd_fflush(PYFILE *f) {
	if (f) {
		if (f->fa) {
			f->fa->flush();
		} else if (f == _gd_stdout()) {
			fflush(stdout);
		} else if (f == _gd_stderr()) {
			fflush(stderr);
		} else if (f == _gd_stdin()) {
			fflush(stdin);
		} else {
			WARN_PRINT("Undefined file access - information lost.");
		}
		return SUCCESS;
	}
	return EOF;
}

int _gd_fileno(PYFILE *f) {
	Id_T h;
	if (_handles.find(f, h)) {
		return h.value;
	}
	return _handles.insert(f).value;
}

int _gd_feof(PYFILE *f) {
	if (f) {
		return f->fa->eof_reached();
	}
	return 1;
}

int _gd_ferror(PYFILE *f) {
	int r = 0;
	return r;
}

void _gd_clearerr(PYFILE *f) {
}

ssize_t _gd_ffilesize(PYFILE *f) {
	if (f) {
		return f->fa->get_len();
	}
	return FAILURE;
}

PYFILE *_gd_stderr() {
	return &gd_stderr;
}

PYFILE *_gd_stdin() {
	return &gd_stdin;
}

PYFILE *_gd_stdout() {
	return &gd_stdout;
}
