#ifndef GODOT_CPYTHON_H
#define GODOT_CPYTHON_H

#include <Python.h>
#include <marshal.h>

#include "core/reference.h"
#include "core/variant.h"
#include "scene/2d/node_2d.h"

#include "pybind11/pytypes.h"

class CPythonRun : public Reference {

public:
	CPythonRun(Node2D *p_owner);
	~CPythonRun();

private:
	String exec_file;
	Node2D *owner;

public:
	bool has_error();
	void run_file(const String& p_python_file);
	void run_code(const String& p_python_code);
};

class CPythonInstance : public Node2D {
	GDCLASS(CPythonInstance, Node2D);

	String python_code;
	String python_file;
	bool python_autorun;
	String python_gd_build_func;
	int debug_level;
	int verboe_level;

	pybind11::object py_app;

	Ref<CPythonRun> _cpython;
	bool _running;
	String _last_file_run;
	String _last_code_run;
	bool _dirty;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_python_code(const String &code);
	String get_python_code() const;
	void set_python_file(const String &path);
	String get_python_file() const;
	void set_autorun(bool autorun);
	bool is_autorun() const;
	void set_gd_build_func(const String &func);
	String get_gd_build_func() const;
	void set_debug_level(const int level);
	int get_debug_level() const;
	void set_verbose_level(const int level);
	int get_verbose_level() const;

	CPythonInstance();
};

#if __has_feature(cxx_exceptions) || defined(__cpp_exceptions) \
	|| defined(__EXCEPTIONS) \
	|| (defined(_MSC_VER) && defined(_CPPUNWIND))
#define _HAS_EXCEPTIONS
#endif

#endif // GODOT_CPYTHON_H
