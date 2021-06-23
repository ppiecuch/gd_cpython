#include "godot_cpython.h"

#include "core/math/geometry.h"
#include "core/rid.h"
#include "servers/visual_server.h"

#include "pylib/godot/py_godot.h"

// Reference:
// ----------
// https://github.com/RomkoSI/Blender3D/blob/d8ee882db43daeb4859348e94a66abd0353150aa/source/blender/python/generic/bgl.c#L1693
// http://python3porting.com/cextensions.html
// https://realpython.com/build-python-c-extension-module/
// https://groups.google.com/g/cython-users/c/G3O6YM6YgY4
// https://stackoverflow.com/questions/39250524/programmatically-define-a-package-structure-in-embedded-python-3
// https://www.oreilly.com/library/view/python-cookbook/0596001673/ch16s06.html
// https://stackabuse.com/enhancing-python-with-custom-c-extensions
// https://github.com/pasimako/embedPython

CPythonRun::CPythonRun(Node2D *p_owner) {
	static char exec_name[] = "pygodot";

	owner = p_owner;
	Py_SetProgramName(exec_name);

	Py_NoSiteFlag = 1;
#ifdef DEBUG_ENABLED
	Py_DebugFlag = 1;
	Py_VerboseFlag = 2;
#else
	Py_DebugFlag = 0;
	Py_VerboseFlag = 0;
#endif

	__putenv("PYTHONCASEOK");

	Py_InitializeEx(0);

	char* n_argv[] = { exec_name };
	PySys_SetArgv(1, n_argv);

	print_line(vformat("Python interpreter version: %s", Py_GetVersion()));
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
		const char *file = p_python_file.utf8().get_data();
		PYFILE *fp = pyfopen(file, "r");
		if (fp != nullptr) {
			if (PyObject *sys_path = PySys_GetObject("path")) {
				PyList_SetItem(sys_path, 0, PyString_FromString(p_python_file.get_base_dir().utf8().get_data()));
			}
			PyRun_SimpleFile(fp, file);
			PyErr_Clear();
			pyfclose(fp);
		} else {
			int save_errno;
			save_errno = errno;
			PySys_WriteStderr("Could not open file\n");
			errno = save_errno;
			PyErr_SetFromErrnoWithFilename(PyExc_IOError, file);
			PyErr_Print();
			PyErr_Clear();
		}
	}
}

PyObject* CPythonRun::import_module(const String& p_code_obj, const String& p_module_name) {
	PyObject *po_module = nullptr;

	// Get reference to main module
	PyObject *main_module = PyImport_AddModule("__main__");

	// De-serialize Python code object
	PyObject *code_obj = PyMarshal_ReadObjectFromString(p_code_obj.utf8().ptr(), p_code_obj.size());

	if(!has_error()) {
		// Load module from code object
		po_module = PyImport_ExecCodeModule(p_module_name.utf8().get_data(), code_obj);

		if(!has_error()) {
			// Add module to main module as p_module_name
			PyModule_AddObject(main_module, p_module_name.utf8().get_data(), po_module);
		}

		// Release object reference (Python cannot track references automatically in C++!)
		Py_XDECREF(code_obj);
	}

	return po_module;
}

PyObject *CPythonRun::call_function(PyObject *p_module, String p_func_name, PyObject *p_args) {
	PyObject *ret = nullptr;

	// Get reference to function p_func_name in module p_module
	PyObject *func = PyObject_GetAttrString(p_module, p_func_name.utf8().get_data());

	if(!has_error()) {
		// Call function with arguments p_args
		ret = PyObject_CallObject(func, p_args);

		if(has_error()) {
			ret = nullptr;
		}

		// Release reference to function
		Py_XDECREF(func);
	}

	// Release reference to arguments
	Py_XDECREF(p_args);

	return ret;
}

String CPythonRun::object_to_string(PyObject *p_val) {
	String val;

	if(p_val != nullptr) {
		if(PyUnicode_Check(p_val)) {
			// Convert Python Unicode object to UTF8 and return pointer to buffer
			// PyObject* objectsRepresentation = PyObject_Repr(yourObject);
			const char *str = PyString_AsString(p_val);

			if(!has_error()) {
				val = String(str);
			}
		}

		// Release reference to object
		Py_XDECREF(p_val);
	}

	return val;
}

bool CPythonRun::has_error() {
	bool error = false;

	if(PyErr_Occurred()) {
		// Output error to stderr and clear error indicator
		PyErr_Print();
		error = true;
	}

	return error;
}


// Node instance

void CPythonInstance::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (_cpython.is_null()) {
				_cpython = Ref<CPythonRun>(memnew(CPythonRun(this)));
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
			if (_cpython.is_null()) {
				_cpython = Ref<CPythonRun>(memnew(CPythonRun(this)));
			}
			if (python_autorun) {
				if (!python_file.empty() && _last_file_run != python_file) {
					_cpython->run_file(python_file);
					_last_file_run = python_file;
				}
				if (!python_code.empty() && _last_code_run != python_code.md5_text()) {
					_cpython->run_code(python_code);
					_last_code_run = python_code.md5_text();
				}
				if (!python_init_func.empty()) {
					// _cpython->call_function();
				}
			}
			set_process(true);
		} break;
		case NOTIFICATION_PROCESS: {
			if (_running) {
				// call tick function
				if (!python_tick_func.empty()) {
				}
			}
		}
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

void CPythonInstance::set_init_func(const String &func) {
	python_init_func = func;
}

String CPythonInstance::get_init_func() const {
	return python_init_func;
}

void CPythonInstance::set_tick_func(const String &func) {
	python_tick_func = func;
}

String CPythonInstance::get_tick_func() const {
	return python_tick_func;
}

void CPythonInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_python_code", "code"), &CPythonInstance::set_python_code);
	ClassDB::bind_method(D_METHOD("get_python_code"), &CPythonInstance::get_python_code);
	ClassDB::bind_method(D_METHOD("set_python_file", "file"), &CPythonInstance::set_python_file);
	ClassDB::bind_method(D_METHOD("get_python_file"), &CPythonInstance::get_python_file);
	ClassDB::bind_method(D_METHOD("set_autorun", "autorun"), &CPythonInstance::set_autorun);
	ClassDB::bind_method(D_METHOD("is_autorun"), &CPythonInstance::is_autorun);
	ClassDB::bind_method(D_METHOD("set_init_func", "func"), &CPythonInstance::set_init_func);
	ClassDB::bind_method(D_METHOD("get_init_func"), &CPythonInstance::get_init_func);
	ClassDB::bind_method(D_METHOD("set_tick_func", "func"), &CPythonInstance::set_tick_func);
	ClassDB::bind_method(D_METHOD("get_tick_func"), &CPythonInstance::get_tick_func);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autorun"), "set_autorun", "is_autorun");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "init_func"), "set_init_func", "get_init_func");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "tick_func"), "set_tick_func", "get_tick_func");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "python_code", PROPERTY_HINT_MULTILINE_TEXT, "", PROPERTY_USAGE_DEFAULT_INTL), "set_python_code", "get_python_code");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "python_file_path"), "set_python_file", "get_python_file");

	ADD_SIGNAL(MethodInfo("python_file_changed"));
	ADD_SIGNAL(MethodInfo("python_code_changed"));
}

CPythonInstance::CPythonInstance() {
	_running = false;
	_dirty = false;
	python_init_func = "_gd_init";
	python_tick_func = "_gd_tick";
	python_autorun = false;
}
