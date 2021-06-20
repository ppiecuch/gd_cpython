#include "core/class_db.h"
#include "register_types.h"

#include "godot_cpython.h"

void register_gd_cpython_types() {
	ClassDB::register_class<CPythonInstance>();
}

void unregister_gd_cpython_types() {
}
