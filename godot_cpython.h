#ifndef GODOT_CPYTHON_H
#define GODOT_CPYTHON_H

#include <Python.h>
#include <marshal.h>

#include "core/reference.h"
#include "core/variant.h"
#include "scene/2d/node_2d.h"

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
	PyObject* import_module(const String& p_code_obj, const String& p_module_name);
	PyObject* call_function(PyObject *p_module, String p_func_name, PyObject *p_args);
	String object_to_string(PyObject*);
};

class CPythonInstance : public Node2D {
	GDCLASS(CPythonInstance, Node2D);

	String python_code;
	String python_file;
	bool python_autorun;
	String python_init_func;
	String python_tick_func;
	String python_event_func;
	int debug_level;
	int verboe_level;

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
	void set_init_func(const String &func);
	String get_init_func() const;
	void set_tick_func(const String &func);
	String get_tick_func() const;
	void set_event_func(const String &func);
	String get_event_func() const;
	void set_debug_level(const int level);
	int get_debug_level() const;
	void set_verbose_level(const int level);
	int get_verbose_level() const;

	CPythonInstance();
};

#endif // GODOT_CPYTHON_H
