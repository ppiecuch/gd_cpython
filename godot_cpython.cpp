#include "godot_cpython.h"

#include "core/rid.h"
#include "core/engine.h"
#include "core/math/math_defs.h"
#include "core/math/geometry.h"
#include "scene/resources/font.h"
#include "scene/resources/theme.h"
#include "servers/visual_server.h"

#include "pylib/godot/py_godot.h"

#include <Python.h>
#include <osdefs.h>
#include <marshal.h>


static bool py_has_error();

String pystr_to_string(PyObject *p_str);
String pyobj_to_string(PyObject *p_obj);
PyObject* import_module(const String& p_code_obj, const String& p_module_name);
PyObject *call_function(PyObject *p_module, String p_func_name, PyObject *p_args);
bool add_builtin_symbol(String p_key, Variant p_val);
bool add_builtin_symbols(Dictionary p_vals);

constexpr const char *__init_func = "gd_init";
constexpr const char *__tick_func = "gd_tick";
constexpr const char *__draw_func = "gd_draw";
constexpr const char *__event_func = "gd_event";
constexpr const char *__term_func = "gd_term";

CPythonEngine *CPythonEngine::instance = nullptr;

CPythonEngine *CPythonEngine::get_singleton() {
	static char exec_name[] = "pygodot";
	static char pythoncaseok[] = "PYTHONCASEOK";
	static char pythonpycacheprefix[] = "PYTHONPYCACHEPREFIX=res://pycache/";
	static char vhome[] = "VHOME=user://";

	if (!Py_IsInitialized()) {
		Py_SetProgramName(exec_name);

		Py_NoSiteFlag = 1;

		__putenv(pythonpycacheprefix);
		__putenv(pythoncaseok);
		__putenv(vhome);

		Py_InitializeEx(0);

		char* n_argv[] = { exec_name };
		PySys_SetArgv(1, n_argv);

		print_line(vformat("Python interpreter version: %s on %s", Py_GetVersion(), Py_GetPlatform()));
		print_verbose(vformat("Python standard library path: %s", Py_GetPath()));
		print_verbose(vformat("Python settings: NoSiteFlag=%d, VerboseFlag=%d, DebugFlag=%d, OptimizeFlag=%d", Py_NoSiteFlag, Py_VerboseFlag, Py_DebugFlag, Py_OptimizeFlag));
	}

	return instance;
}

bool CPythonEngine::_add_path(const String &p_path, const String &p_object) {
	if (!p_path.empty()) {
		const String key = vformat("[%s]%s", p_object, p_path);
		if (!search_paths.has(key)) { // keep track of added paths
			if (PyObject *sys_path = PySys_GetObject(p_object.utf8().c_str())) {
				PyList_Insert(sys_path, 0, PyString_FromString(p_path.utf8().c_str()));
				search_paths.push_back(key);
				return true;
			}
		}
	}
	return false;
}

Error CPythonEngine::run_code(const String& p_python_code) {
	if (!p_python_code.empty()) {
		const char *code = p_python_code.utf8().get_data();
		if (PyRun_SimpleString(code) == -1) {
			WARN_PRINT("Executing code with errors.");
			return ERR_SCRIPT_FAILED;
		}
	} else {
		return ERR_INVALID_PARAMETER;
	}
	return OK;
}

Error CPythonEngine::run_file(const String& p_python_file) {
	if (!p_python_file.empty()) {
		const std::string file(p_python_file.utf8().get_data());
		PYFILE *fp = pyfopen(file.c_str(), "r");
		if (fp != nullptr) {
			_add_path(p_python_file.get_base_dir(), "path");
			Error ret = PyRun_SimpleFile(fp, file.c_str()) == 0 ? OK : ERR_SCRIPT_FAILED;
			PyErr_Clear();
			pyfclose(fp);
			return ret;
		} else {
			int save_errno = errno;
			WARN_PRINT("Could not find/open python file " + p_python_file);
			errno = save_errno;
			PyErr_SetFromErrnoWithFilename(PyExc_IOError, file.c_str());
			PyErr_Print();
			PyErr_Clear();
			return ERR_FILE_CANT_OPEN;
		}
	} else {
		return ERR_INVALID_PARAMETER;
	}
}

Error CPythonEngine::run_module(const String& p_python_module) {
	const int set_argv0 = 1;
	PyObject *runpy, *runmodule, *runargs, *result;
	runpy = PyImport_ImportModule("runpy");
	if (runpy == NULL) {
		ERR_PRINT("Could not import runpy module");
		return ERR_SCRIPT_FAILED;
	}
	runmodule = PyObject_GetAttrString(runpy, "_run_module_as_main");
	if (runmodule == NULL) {
		ERR_PRINT("Could not access runpy._run_module_as_main.");
		Py_DECREF(runpy);
		return ERR_SCRIPT_FAILED;
	}
	runargs = Py_BuildValue("(si)", p_python_module.utf8().get_data(), set_argv0);
	if (runargs == NULL) {
		ERR_PRINT("Could not create arguments for runpy._run_module_as_main.");
		Py_DECREF(runpy);
		Py_DECREF(runmodule);
		return ERR_SCRIPT_FAILED;
	}
	result = PyObject_Call(runmodule, runargs, NULL);
	if (result == NULL) {
		PyErr_Print();
	}
	Py_DECREF(runpy);
	Py_DECREF(runmodule);
	Py_DECREF(runargs);
	if (result == NULL) {
		return ERR_SCRIPT_FAILED;
	}
	Py_DECREF(result);
	return OK;
}

Error CPythonEngine::run_python(const String& p_python, const Dictionary &p_context) {
	Array search_paths = p_context[KEY_SEARCH_PATHS];
	for (const String p : search_paths) {
		_add_path(p, "path");
	}
	Dictionary builtins = p_context[KEY_BUILTINS];
	add_builtin_symbols(builtins);
	if (FileAccess::exists(p_python)) {
		return run_file(p_python);
	} else if (run_module(p_python) == OK) {
		return OK;
	} else {
		return run_code(p_python);
	}
}

CPythonEngine::CPythonEngine() {
	instance = this;
}

CPythonEngine::~CPythonEngine() {
	Py_Finalize();
	instance = nullptr;
}

// Node instance

#ifdef TOOLS_ENABLED
Dictionary CPythonInstance::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["view_size"] = get_view_size();

	return state;
}

void CPythonInstance::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_view_size(p_state["view_size"]);
}

bool CPythonInstance::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return _edit_get_rect().has_point(p_point);
};

Rect2 CPythonInstance::_edit_get_rect() const {
	return Rect2(Point2(), get_view_size());
}

void CPythonInstance::_edit_set_rect(const Rect2 &p_rect) {
	set_view_size(p_rect.size);
	_change_notify();
}

bool CPythonInstance::_edit_use_rect() const {
	return true;
}
#endif

void CPythonInstance::_get_property_list(List<PropertyInfo> *p_list) const {
	if (p_list) {
		for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
			PropertyInfo &prop = E->get();
			if (prop.name.to_lower() == "python_data") {
				switch (python_data_hint) {
					case 0: { // Code
						prop.hint = PROPERTY_HINT_MULTILINE_TEXT;
					} break;
					case 1: { // Script Path
						prop.hint = PROPERTY_HINT_FILE;
					} break;
					case 2: { // Module Name
						prop.hint = PROPERTY_HINT_NONE;
					} break;
				}
			}
		}
	}
}

void CPythonInstance::_input(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	if (!get_tree()) {
		return;
	}

	ERR_FAIL_COND(!is_visible_in_tree());

	if (_running) {
		if (_py.process_events(p_event, __event_func)) {
			return;
		}
	}
}

void CPythonInstance::_notification(int p_what) {
	static Color white = Color::named("white");

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			set_process(true);
			if (!Engine::get_singleton()->is_editor_hint()) {
				set_process_input(is_visible_in_tree());
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			set_process(false);
			set_process_input(false);
			if (_running) {
				_py.pycall(__term_func);
				_running = false;
				_py.destroy_pygodot();
			}
		} break;
		case NOTIFICATION_DRAW: {
			if (_running) {
				_py.pycall(__draw_func); // call draw function
			} else {
				// not active indicator
				draw_rect(Rect2(Point2(), view_size), white, false);
				draw_line(Point2(), view_size, white);
				draw_line(Point2(0, view_size.height), Point2(view_size.width, 0), white);
				const String msg = "Not running";
				Ref<Font> default_font = Theme::get_default()->get_font("_", "_");
				const Size2 msg_size = default_font->get_string_size(msg);
				if (view_size >  msg_size) {
					draw_string(default_font, (view_size - msg_size) / 2, msg);
				}
			}
		} break;
		case NOTIFICATION_READY: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				if (python_autorun) {
					run();
				}
			}
		} break;
		case NOTIFICATION_PROCESS: {
			if (_running && !_pausing) {
				const real_t delta = get_process_delta_time();
				if (_py.pycall(__tick_func, delta)) { // call tick function
					update();
				}
			}
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				set_process_input(is_visible_in_tree());
			}
		} break;
		case NOTIFICATION_PAUSED: {
			_pausing = true;
		} break;
		case NOTIFICATION_UNPAUSED: {
			_pausing = false;
		} break;
	}
}

void CPythonInstance::_set_python_data_hint(int p_hint) {
	python_data_hint = p_hint;
	_change_notify();
}

int CPythonInstance::_get_python_data_hint() const {
	return python_data_hint;
}

void CPythonInstance::set_python_data(const String &p_data) {
	python_data = p_data;
	_dirty = true;
	emit_signal("python_data_changed");
}

String CPythonInstance::get_python_data() const {
	return python_data;
}

void CPythonInstance::set_python_builtins(const Dictionary &p_dict) {
	python_builtins = p_dict;
}

Dictionary CPythonInstance::get_python_builtins() const {
	return python_builtins;
}

void CPythonInstance::set_python_search_paths(const Array &p_paths) {
	python_search_paths = p_paths;
}

Array CPythonInstance::get_python_search_paths() const {
	return python_search_paths;
}

void CPythonInstance::set_autorun(bool autorun) {
	python_autorun = autorun;
}

bool CPythonInstance::is_autorun() const {
	return python_autorun;
}

void CPythonInstance::set_gd_build_func(const String &p_func) {
	python_gd_build_func = p_func;
}

String CPythonInstance::get_gd_build_func() const {
	return python_gd_build_func;
}

void CPythonInstance::set_debug_level(int p_level) {
	Py_DebugFlag = p_level;
}

int CPythonInstance::get_debug_level() const {
	return Py_DebugFlag;
}

void CPythonInstance::set_verbose_level(int p_level) {
	Py_VerboseFlag = p_level;
}

int CPythonInstance::get_verbose_level() const {
	return Py_VerboseFlag;
}

void CPythonInstance::set_optimize_flag(bool p_optimize) {
	Py_OptimizeFlag = p_optimize;
}

bool CPythonInstance::get_optimize_flag() const {
	return Py_OptimizeFlag;
}

bool CPythonInstance::run() {
	CPythonEngine *cpython = CPythonEngine::get_singleton();

	ERR_FAIL_NULL_V(cpython, false);

	if (!python_data.empty() && _last_python_data != python_data) {
		Dictionary context;
		context[CPythonEngine::KEY_SEARCH_PATHS] = python_search_paths;
		context[CPythonEngine::KEY_BUILTINS] = python_builtins;
		cpython->run_python(python_data, context);
		_last_python_data = python_data;
		_running = !py_has_error();
	}
	if (_running) {
		if (!python_gd_build_func.empty()) {
			_py.build_pygodot(get_instance_id(), python_gd_build_func);
		}
		auto r = _py.pycall(__init_func); // call init functions
		#ifdef DEBUG_ENABLED
		if (!r.is_nil()) {
			print_verbose(vformat("Return value from %s: %s", __init_func, r));
		}
		#endif
	}
	return _running;
}

void CPythonInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_python_data", "data"), &CPythonInstance::set_python_data);
	ClassDB::bind_method(D_METHOD("get_python_data"), &CPythonInstance::get_python_data);
	ClassDB::bind_method(D_METHOD("set_python_builtins", "builtins"), &CPythonInstance::set_python_builtins);
	ClassDB::bind_method(D_METHOD("get_python_builtins"), &CPythonInstance::get_python_builtins);
	ClassDB::bind_method(D_METHOD("set_python_search_paths", "paths"), &CPythonInstance::set_python_search_paths);
	ClassDB::bind_method(D_METHOD("get_python_search_paths"), &CPythonInstance::get_python_search_paths);
	ClassDB::bind_method(D_METHOD("set_view_size", "size"), &CPythonInstance::set_view_size);
	ClassDB::bind_method(D_METHOD("get_view_size"), &CPythonInstance::get_view_size);
	ClassDB::bind_method(D_METHOD("set_autorun", "autorun"), &CPythonInstance::set_autorun);
	ClassDB::bind_method(D_METHOD("is_autorun"), &CPythonInstance::is_autorun);
	ClassDB::bind_method(D_METHOD("set_gd_build_func", "func"), &CPythonInstance::set_gd_build_func);
	ClassDB::bind_method(D_METHOD("get_gd_build_func"), &CPythonInstance::get_gd_build_func);
	ClassDB::bind_method(D_METHOD("set_debug_level"), &CPythonInstance::set_debug_level);
	ClassDB::bind_method(D_METHOD("get_debug_level"), &CPythonInstance::get_debug_level);
	ClassDB::bind_method(D_METHOD("set_verbose_level"), &CPythonInstance::set_verbose_level);
	ClassDB::bind_method(D_METHOD("get_verbose_level"), &CPythonInstance::get_verbose_level);
	ClassDB::bind_method(D_METHOD("set_optimize_flag"), &CPythonInstance::set_optimize_flag);
	ClassDB::bind_method(D_METHOD("get_optimize_flag"), &CPythonInstance::get_optimize_flag);

	ClassDB::bind_method(D_METHOD("_set_python_data_hint", "data"), &CPythonInstance::_set_python_data_hint);
	ClassDB::bind_method(D_METHOD("_get_python_data_hint"), &CPythonInstance::_get_python_data_hint);

	ClassDB::bind_method(D_METHOD("run"), &CPythonInstance::run);
	ClassDB::bind_method(D_METHOD("_input"), &CPythonInstance::_input);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "view_size"), "set_view_size", "get_view_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autorun"), "set_autorun", "is_autorun");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "gd_build_func"), "set_gd_build_func", "get_gd_build_func");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "python_data_hint", PROPERTY_HINT_ENUM, "Code,Script Path,Module Name", PROPERTY_USAGE_EDITOR), "_set_python_data_hint", "_get_python_data_hint");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "python_data"), "set_python_data", "get_python_data");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "python_search_paths"), "set_python_search_paths", "get_python_search_paths");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "python_builtins"), "set_python_builtins", "get_python_builtins");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "debug_level"), "set_debug_level", "get_debug_level");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "verbose_level"), "set_verbose_level", "get_verbose_level");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "optimize_flag"), "set_optimize_flag", "get_optimize_flag");

	ADD_SIGNAL(MethodInfo("python_data_changed"));
}

CPythonInstance::CPythonInstance() {
	_running = false;
	_pausing = false;
	_dirty = false;

	view_size = Size2(640, 480);

	python_gd_build_func = "_gd_build";
	python_autorun = false;
	python_data_hint = 2; // Module Name

#ifdef DEBUG_ENABLED
	Py_DebugFlag = 1;
#else
	Py_DebugFlag = 0;
#endif
	Py_VerboseFlag = 0;
#ifdef TOOLS_ENABLED
	Py_OptimizeFlag = Engine::get_singleton()->is_editor_hint() ? 0 : 1;
#else
	Py_OptimizeFlag = 1;
#endif
}

// Python utilities

// Reference:
// ----------
// https://stackoverflow.com/questions/8436578/passing-a-c-pointer-around-with-the-python-c-api

static bool py_has_error() {
	bool error = false;

	if(PyErr_Occurred()) {
		// Output error to stderr and clear error indicator
		PyErr_Print();
		error = true;
	}

	return error;
}

String pyobj_to_string(PyObject *p_obj) {
	String val;
	if(p_obj != nullptr) {
		const char *str = PyString_AsString(PyObject_Repr(p_obj));
		if (str) {
			val = String(str);
		}
		Py_XDECREF(p_obj); // Release reference to object
	}
	return val;
}

String pystr_to_string(PyObject *p_val) {
	String val;
	if(p_val != nullptr) {
		if(PyUnicode_Check(p_val)) {
			const char *str = PyString_AsString(p_val);
			if (str) {
				val = String(str);
			}
		}
		Py_XDECREF(p_val); // Release reference to object
	}
	return val;
}

PyObject* import_module(const String& p_code_obj, const String& p_module_name) {
	PyObject *po_module = nullptr;
	if (PyObject *main_module = PyImport_AddModule("__main__")) { // Get reference to main module
		if (PyObject *code_obj = PyMarshal_ReadObjectFromString(p_code_obj.utf8().ptr(), p_code_obj.size())) { // De-serialize Python code object
			if(!py_has_error()) {
				po_module = PyImport_ExecCodeModule(p_module_name.utf8().get_data(), code_obj); // Load module from code object
				if(!py_has_error()) {
					PyModule_AddObject(main_module, p_module_name.utf8().get_data(), po_module); // Add module to main module as p_module_name
				}
			}
			Py_XDECREF(code_obj); // Release object reference (Python cannot track references automatically in C++!)
		}
	}
	return po_module;
}

PyObject *call_function(PyObject *p_module, String p_func_name, PyObject *p_args) {
	PyObject *ret = nullptr;
	if (p_module) {
		if (PyObject *func = PyObject_GetAttrString(p_module, p_func_name.utf8().get_data())) { // Get reference to function p_func_name in module p_module
			if(!py_has_error()) {
				ret = PyObject_CallObject(func, p_args); // Call function with arguments p_args
				if(py_has_error()) {
					ret = nullptr;
				}
			}
			Py_XDECREF(func); // Release reference to function
		}
	}
	Py_XDECREF(p_args); // Release reference to arguments
	return ret;
}

bool add_builtin_symbol(String p_key, Variant p_val) {
	if (p_key.empty()) {
		return false;
	}
	bool ret = true;
	if (PyObject *name = PyString_FromString("__builtin__")) {
		if (PyObject *builtin = PyImport_Import(name)) {
			PyObject *builtin_dict = PyModule_GetDict(builtin);
			switch (p_val.get_type()) {
				case Variant::INT: {
					int val = p_val;
					PyDict_SetItem(builtin_dict, PyString_FromString(p_key.utf8().c_str()), Py_BuildValue("i", val));
				} break;
				case Variant::REAL: {
					real_t val = p_val;
		#ifdef REAL_T_IS_DOUBLE
					PyDict_SetItem(builtin_dict, PyString_FromString(p_key.utf8().c_str()), Py_BuildValue("d", val));
		#else
					PyDict_SetItem(builtin_dict, PyString_FromString(p_key.utf8().c_str()), Py_BuildValue("f", val));
		#endif
				} break;
				case Variant::STRING: {
					String val = p_val;
					PyDict_SetItemString(builtin_dict, p_key.utf8().c_str(), PyString_FromString(val.utf8().c_str()));
				} break;
				default: {
					ret = false;
				}
			}
			Py_XDECREF(builtin);
		} else {
			ret = false;
		}
		Py_XDECREF(name);
	} else {
		ret = false;
	}
	return ret;
}

bool add_builtin_symbols(Dictionary p_vals) {
	bool ret = true;
	const Variant *key = nullptr;
	while ((key = p_vals.next(key))) {
		ret |= add_builtin_symbol(String(*key), p_vals[*key]);
	}
	return ret;
}
