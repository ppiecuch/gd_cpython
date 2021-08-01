#ifndef GODOT_CPYTHON_H
#define GODOT_CPYTHON_H

#include <Python.h>
#include <marshal.h>

#include "core/reference.h"
#include "core/variant.h"
#include "scene/2d/node_2d.h"

class CPythonRun : public Reference {
	GDCLASS(CPythonRun, Reference);

	String exec_file;
	Node2D *owner;

public:
	bool has_error();
	void run_file(const String& p_python_file);
	void run_code(const String& p_python_code);

	CPythonRun(Node2D *p_owner);
	~CPythonRun();
};

class CPythonInstance : public Node2D {
	GDCLASS(CPythonInstance, Node2D);

	struct InstancePrivateData;

	String python_code;
	String python_file;
	bool python_autorun;
	String python_gd_build_func;
	int debug_level;
	int verboe_level;

	Ref<CPythonRun> _cpython;
	bool _running, _pausing;
	String _last_file_run;
	String _last_code_run;
	bool _dirty;

	Ref<InstancePrivateData> p;

protected:
	void _notification(int p_what);
	void _input(const Ref<InputEvent> &p_event);
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

#endif // GODOT_CPYTHON_H
