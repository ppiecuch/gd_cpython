#pragma once

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

// Wrapper around Godot events
struct GdEvent {
	enum {
		QUIT,
		KEYDOWN,
		KEYUP,
		MOUSEMOTION,
		MOUSEBUTTONDOWN,
		MOUSEBUTTONUP,
		JOYBUTTONDOWN,
		JOYBUTTONUP,
	};
	int type;
	Point2 position;
	GdEvent(int t) : type(t) { }
	GdEvent(int t, Point2 p) : type(t), position(p) { }
};
