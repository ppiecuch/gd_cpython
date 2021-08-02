#include "godot_cpython.h"

#include "pybind11/pybind11.h"
#include "pybind11/embed.h"

#include "core/rid.h"
#include "core/variant.h"
#include "core/math/geometry.h"
#include "servers/visual_server.h"

#include "pylib/godot/py_godot.h"

#include <iostream>

// Reference:
// ----------
// 1. https://github.com/pybind/pybind11/issues/1619

namespace py = pybind11;
using namespace py::literals;

static py::object py_eval(const char *expr, const py::object &o = py::none());
static py::object py_call(py::object p_obj, String p_func_name, py::args p_args = py::args());
static py::object py_call(String p_func_name, py::args p_args = py::args(), String p_module = "__main__");


struct PyGodotInstance::InstancePrivateData {
	py::object py_app;
};

PyGodotInstance::PyGodotInstance() : _p(memnew(InstancePrivateData)) {
}

PyGodotInstance::~PyGodotInstance() {
	if (_p) {
		memdelete(_p);
	}
}

bool PyGodotInstance::process_events(const Ref<InputEvent> &p_event, const String &p_event_func) {
	if (not _p->py_app.is_none()) {
		if (const InputEventMouseMotion *m = Object::cast_to<InputEventMouseMotion>(*p_event)) {
			GdEvent ev(GdEvent::MOUSEMOTION, m->get_position());
			py_call(_p->py_app, p_event_func, py::make_tuple(ev));
			return true;
		}
	}
	return false;
}

Variant PyGodotInstance::call(const String &p_func, real_t p_arg) {
	if (not _p->py_app.is_none()) {
		auto r = py_call(_p->py_app, p_func, py::make_tuple(p_arg));
		if (not r.is_none()) {
			if (r.is(py::bool_())) {
				return r.cast<bool>();
			}
			if (r.is(py::int_())) {
				return r.cast<int>();
			}
		}
	}
	return Variant();
}

Variant PyGodotInstance::call(const String &p_func) {
	if (not _p->py_app.is_none()) {
		auto r = py_call(_p->py_app, p_func);
		if (not r.is_none()) {
			if (r.is(py::bool_())) {
				return r.cast<bool>();
			}
			if (r.is(py::int_())) {
				return r.cast<int>();
			}
		}
	}
	return Variant();
}

bool PyGodotInstance::build_pygodot(int p_instance_id, const String &p_build_func) {
	_p->py_app = py_call(p_build_func, py::make_tuple(p_instance_id));
	return (not _p->py_app.is_none());
}

// Python utilities

static py::object py_eval(const char *expr, const py::object &o) {
	py::object res = py::none();
	try {
		if (not o.is_none()) {
			auto locals = py::dict("_v"_a = o);
			res = py::eval(expr, py::globals(), locals);
		} else {
			res = py::eval(expr, py::globals());
		}
#ifdef DEBUG_ENABLED
		if (not res.is_none()) {
			py::print("Return value from expression:", res);
		}
#endif
	} catch (py::error_already_set &e) {
		std::cout << "py_eval error_already_set: " << std::endl;
	} catch (std::runtime_error &e) {
		std::cout << "py_eval runtime_error: " << e.what() << std::endl;
	} catch (...) {
		std::cout << "py_eval unknown exception" << std::endl;
	}
	return res;
}

// Reference:
// ----------
// https://developpaper.com/using-pybind11-to-call-between-c-and-python-code-on-windows-10/

static py::object py_call(py::object p_obj, String p_func_name, py::args p_args) {
	auto r = p_obj.attr(p_func_name.utf8().get_data())(*p_args);
#ifdef DEBUG_ENABLED
	if (!r.is_none()) {
		py::print("Return value: ", p_func_name.utf8().get_data(), " -> ", r);
	}
#endif
	return r;
}

static py::object py_call(String p_func_name, py::args p_args, String p_module) {
	std::string module = p_module.utf8().get_data();
	std::string function = p_func_name.utf8().get_data();

	if (module.empty()) {
		module = "__main__";
	}

	auto m = py::module::import(module.c_str());
	auto r = m.attr(function.c_str())(*p_args);
#ifdef DEBUG_ENABLED
	if (not r.is_none()) {
		py::print("Return value: ", function, " -> ", r);
	}
#endif
	return r;
}
