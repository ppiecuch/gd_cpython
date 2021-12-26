#ifndef GODOT_CPYTHON_H
#define GODOT_CPYTHON_H

#include "core/reference.h"
#include "core/variant.h"
#include "scene/2d/node_2d.h"
#include "common/gd_core.h"

#include "pylib/godot/py_godot.h"

class CPythonEngine : public Object {
	GDCLASS(CPythonEngine, Object);

	String exec_file;

	static CPythonEngine *instance;

public:
	static CPythonEngine *get_singleton();

	bool has_error();
	void run_file(const String& p_python_file);
	void run_code(const String& p_python_code);

	CPythonEngine();
	~CPythonEngine();
};

struct PyGodotInstance;

class CPythonInstance : public Node2D {
	GDCLASS(CPythonInstance, Node2D);

	Size2 view_size;
	String python_code;
	String python_file;
	Dictionary python_builtins;
	bool python_autorun;
	String python_gd_build_func;
	int debug_level;
	int verboe_level;

	PyGodotInstance _py;

	bool _running, _pausing;
	String _last_file_run;
	String _last_code_run;
	bool _dirty;

protected:
	void _notification(int p_what);
	void _input(const Ref<InputEvent> &p_event);
	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	void _edit_set_rect(const Rect2 &p_rect);
	bool _edit_use_rect() const;
#endif

	void set_view_size(const Size2 &p_size) { view_size = p_size; update(); }
	Size2 get_view_size() const { return view_size; }

	void set_python_code(const String &p_code);
	String get_python_code() const;
	void set_python_file(const String &p_path);
	String get_python_file() const;
	void set_python_builtins(const Dictionary &p_dict);
	Dictionary get_python_builtins() const;
	void set_autorun(bool p_autorun);
	bool is_autorun() const;
	void set_gd_build_func(const String &p_func);
	String get_gd_build_func() const;
	void set_debug_level(int p_level);
	int get_debug_level() const;
	void set_verbose_level(int p_level);
	int get_verbose_level() const;
	void set_optimize_flag(bool p_optimize);
	bool get_optimize_flag() const;

	bool run();

	CPythonInstance();
};

#endif // GODOT_CPYTHON_H
