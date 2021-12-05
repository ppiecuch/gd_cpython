#include "core/engine.h"
#include "core/class_db.h"
#include "register_types.h"

#include "godot_cpython.h"

void register_gd_cpython_types() {
	Engine::get_singleton()->add_singleton(Engine::Singleton("CPythonEngine", memnew(CPythonEngine)));
	ClassDB::register_class<CPythonInstance>();
}

void unregister_gd_cpython_types() {
	if (CPythonEngine *instance = CPythonEngine::get_singleton()) {
		memdelete(instance);
	}
}
