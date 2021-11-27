#include "godot_cpython.h"

#include "core/rid.h"
#include "core/engine.h"
#include "core/math/geometry.h"
#include "servers/visual_server.h"

#include "pylib/godot/py_godot.h"

#include <Python.h>
#include <marshal.h>


static bool py_has_error();

String object_to_string(PyObject *p_val);
PyObject* import_module(const String& p_code_obj, const String& p_module_name);
PyObject *call_function(PyObject *p_module, String p_func_name, PyObject *p_args);

constexpr const char *__init_func = "gd_init";
constexpr const char *__tick_func = "gd_tick";
constexpr const char *__draw_func = "gd_draw";
constexpr const char *__event_func = "gd_event";
constexpr const char *__term_func = "gd_term";

CPythonRun::CPythonRun(Node2D *p_owner) {
	static char exec_name[] = "pygodot";
	static char pythoncaseok[] = "PYTHONCASEOK";
	static char vhome[] = "VHOME=user://";

	owner = p_owner;
	Py_SetProgramName(exec_name);

	Py_NoSiteFlag = 1;

	__putenv(pythoncaseok);
	__putenv(vhome);

	Py_InitializeEx(0);

	char* n_argv[] = { exec_name };
	PySys_SetArgv(1, n_argv);

	print_line(vformat("Python interpreter version: %s on %s", Py_GetVersion(), Py_GetPlatform()));
	print_line(vformat("Python standard library path: %s", Py_GetPath()));

#ifdef DEBUG_ENABLED
	_PyObject_Dump( PyThreadState_Get()->interp->modules );
#endif
}

CPythonRun::~CPythonRun() {
	Py_Finalize();
}

void CPythonRun::run_code(const String& p_python_code) {
	if (!p_python_code.empty()) {
		const char *code = p_python_code.utf8().get_data();
		if (PyRun_SimpleString(code) == -1) {
			WARN_PRINT("Executing code with errors.");
		}
	}
}

void CPythonRun::run_file(const String& p_python_file) {
	if (!p_python_file.empty()) {
		const std::string file(p_python_file.utf8().get_data());
		PYFILE *fp = pyfopen(file.c_str(), "r");
		if (fp != nullptr) {
			if (PyObject *sys_path = PySys_GetObject("path")) {
				PyList_SetItem(sys_path, 0, PyString_FromString(p_python_file.get_base_dir().utf8().get_data()));
			}
			PyRun_SimpleFile(fp, file.c_str());
			PyErr_Clear();
			pyfclose(fp);
		} else {
			int save_errno;
			save_errno = errno;
			PySys_WriteStderr("Could not open file\n");
			errno = save_errno;
			PyErr_SetFromErrnoWithFilename(PyExc_IOError, file.c_str());
			PyErr_Print();
			PyErr_Clear();
		}
	}
}

// Node instance

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
		if (_py->process_events(p_event, __event_func)) {
			return;
		}
	}
}

void CPythonInstance::_notification(int p_what) {
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
				_py->call(__term_func);
				_running = false;
			}
		} break;
		case NOTIFICATION_DRAW: {
			if (_running) {
				_py->call(__draw_func); // call draw function
			}
		} break;
		case NOTIFICATION_READY: {
			if (not _cpython) {
				_cpython = std::make_gd_unique_ptr<CPythonRun>(memnew(CPythonRun(this)));
			}
			if (python_autorun) {
				if (!python_file.empty() && _last_file_run != python_file) {
					_cpython->run_file(python_file);
					_last_file_run = python_file;
					_running = !py_has_error();
				}
				if (!python_code.empty() && _last_code_run != python_code.md5_text()) {
					_cpython->run_code(python_code);
					_last_code_run = python_code.md5_text();
					_running = !py_has_error();
				}
				if (_running) {
					if (!python_gd_build_func.empty()) {
						_py->build_pygodot(get_instance_id(), python_gd_build_func);
					}
					_py->call(__init_func); // call init functions
				}
			}
		} break;
		case NOTIFICATION_PROCESS: {
			if (_running) {
				const real_t delta = get_process_delta_time();
				if (_py->call(__tick_func, delta)) { // call tick function
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

void CPythonInstance::set_python_code(const String &code) {
	python_code = code;
	_dirty = true;
	emit_signal("python_code_changed");
}

String CPythonInstance::get_python_code() const {
	return python_code;
}

void CPythonInstance::set_python_file(const String &path) {
	python_file = path;
	_dirty = true;
	emit_signal("python_file_changed");
}

String CPythonInstance::get_python_file() const {
	return python_file;
}

void CPythonInstance::set_autorun(bool autorun) {
	python_autorun = autorun;
}

bool CPythonInstance::is_autorun() const {
	return python_autorun;
}

void CPythonInstance::set_gd_build_func(const String &func) {
	python_gd_build_func = func;
}

String CPythonInstance::get_gd_build_func() const {
	return python_gd_build_func;
}

void CPythonInstance::set_debug_level(const int level) {
	Py_DebugFlag = level;
}

int CPythonInstance::get_debug_level() const {
	return Py_DebugFlag;
}

void CPythonInstance::set_verbose_level(const int level) {
	Py_VerboseFlag = level;
}

int CPythonInstance::get_verbose_level() const {
	return Py_VerboseFlag;
}

void CPythonInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_python_code", "code"), &CPythonInstance::set_python_code);
	ClassDB::bind_method(D_METHOD("get_python_code"), &CPythonInstance::get_python_code);
	ClassDB::bind_method(D_METHOD("set_python_file", "file"), &CPythonInstance::set_python_file);
	ClassDB::bind_method(D_METHOD("get_python_file"), &CPythonInstance::get_python_file);
	ClassDB::bind_method(D_METHOD("set_autorun", "autorun"), &CPythonInstance::set_autorun);
	ClassDB::bind_method(D_METHOD("is_autorun"), &CPythonInstance::is_autorun);
	ClassDB::bind_method(D_METHOD("set_gd_build_func", "func"), &CPythonInstance::set_gd_build_func);
	ClassDB::bind_method(D_METHOD("get_gd_build_func"), &CPythonInstance::get_gd_build_func);
	ClassDB::bind_method(D_METHOD("set_debug_level"), &CPythonInstance::set_debug_level);
	ClassDB::bind_method(D_METHOD("get_debug_level"), &CPythonInstance::get_debug_level);
	ClassDB::bind_method(D_METHOD("set_verbose_level"), &CPythonInstance::set_verbose_level);
	ClassDB::bind_method(D_METHOD("get_verbose_level"), &CPythonInstance::get_verbose_level);

	ClassDB::bind_method(D_METHOD("_input"), &CPythonInstance::_input);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autorun"), "set_autorun", "is_autorun");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "gd_build_func"), "set_gd_build_func", "get_gd_build_func");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "python_code", PROPERTY_HINT_MULTILINE_TEXT, "", PROPERTY_USAGE_DEFAULT_INTL), "set_python_code", "get_python_code");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "python_file_path"), "set_python_file", "get_python_file");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "debug_level"), "set_debug_level", "get_debug_level");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "verbose_level"), "set_verbose_level", "get_verbose_level");

	ADD_SIGNAL(MethodInfo("python_file_changed"));
	ADD_SIGNAL(MethodInfo("python_code_changed"));
}

CPythonInstance::CPythonInstance() : _py(std::make_gd_unique_ptr<PyGodotInstance>(memnew(PyGodotInstance))) {
	_running = false;
	_pausing = false;
	_dirty = false;

	python_gd_build_func = "_gd_build";
	python_autorun = false;

	Py_DebugFlag = 0;
	Py_VerboseFlag = 0;
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

String object_to_string(PyObject *p_val) {
	String val;
	if(p_val != nullptr) {
		if(PyUnicode_Check(p_val)) {
			const char *str = PyString_AsString(p_val);
			if(!py_has_error()) {
				val = String(str);
			}
		}
		Py_XDECREF(p_val); // Release reference to object
	}
	return val;
}

PyObject* import_module(const String& p_code_obj, const String& p_module_name) {
	PyObject *po_module = nullptr;
	PyObject *main_module = PyImport_AddModule("__main__"); // Get reference to main module
	PyObject *code_obj = PyMarshal_ReadObjectFromString(p_code_obj.utf8().ptr(), p_code_obj.size()); // De-serialize Python code object
	if(!py_has_error()) {
		po_module = PyImport_ExecCodeModule(p_module_name.utf8().get_data(), code_obj); // Load module from code object
		if(!py_has_error()) {
			PyModule_AddObject(main_module, p_module_name.utf8().get_data(), po_module); // Add module to main module as p_module_name
		}
		Py_XDECREF(code_obj); // Release object reference (Python cannot track references automatically in C++!)
	}
	return po_module;
}

PyObject *call_function(PyObject *p_module, String p_func_name, PyObject *p_args) {
	PyObject *ret = nullptr;
	PyObject *func = PyObject_GetAttrString(p_module, p_func_name.utf8().get_data()); // Get reference to function p_func_name in module p_module
	if(!py_has_error()) {
		ret = PyObject_CallObject(func, p_args); // Call function with arguments p_args
		if(py_has_error()) {
			ret = nullptr;
		}
		Py_XDECREF(func); // Release reference to function
	}
	Py_XDECREF(p_args); // Release reference to arguments
	return ret;
}
