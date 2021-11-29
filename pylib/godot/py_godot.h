#ifndef PY_GODOT_H
#define PY_GODOT_H

#include "core/os/input.h"
#include "core/math/vector2.h"

#include <memory>

struct PyGodotInstance {
	struct InstancePrivateData;

	InstancePrivateData *_p;

	Variant call(const String &p_func);
	Variant call(const String &p_func, real_t p_arg);

	bool build_pygodot(int p_instance_id, const String &p_build_func);
	bool process_events(const Ref<InputEvent> &p_event, const String &p_event_func);

	PyGodotInstance();
	~PyGodotInstance();
};

#endif // PY_GODOT_H
