#pragma once

#include "pybind11/pybind11.h"
#include "pybind11/embed.h"
#include "pybind11/operators.h"
#include "pybind11/stl.h"

#include "core/color.h"
#include "core/image.h"
#include "core/reference.h"
#include "core/version.h"
#include "core/os/keyboard.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/math/rect2.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/sprite.h"
#include "scene/resources/texture.h"
#include "scene/resources/font.h"

#include <iostream>
#include <string>
#include <tuple>

namespace py = pybind11;

// Wrapper around Image/Texture/Sprite object
struct GdSurface {
	GdSurface() { }
	GdSurface convert_alpha() { return GdSurface(); }
	int get_width() const { return 1; }
	int get_height() const { return 1; }
	void fill(const py::tuple &color) {
	}
	void blit(const GdSurface &source, const py::tuple &dest) {
	}
	void blit_area(const GdSurface &source, const py::tuple &dest, const std::vector<real_t> &area) {
	}
};

// Wrapper around Bitmap or Dynamic font
struct GdFont {
	Font *font = nullptr;
	GdFont(const std::string &filename, int size) {
		// create font dependeing on extension
	}
	~GdFont() {
		if (font) {
			memdelete(font);
		}
	}
	GdSurface render(const std::string &text, bool alias, const py::tuple &color) { return GdSurface(); }
};

// Wrapper around Godot sound
struct GdSound {
	GdSound(const std::string &filename) { }
	void set_volume(real_t vol) { }
	void play(bool loop) { }
};

// Wrapper around Godot events
struct GdEvent {
	GdEvent() { }
};

namespace utils {
	void print_dict(py::dict dict) {
		for (auto item : dict)
			std::cout
				<< "key=" << std::string(py::str(item.first)) << ", "
				<< "value=" << std::string(py::str(item.second)) << std::endl;
	}

	py::str get_text(const py::str &s) {
		return s;
	}
} // utils

namespace event {
	py::list get() {
		return py::list();
	}
	void set_grab(bool grab) {
	}
} // event

namespace image {
	GdSurface load(const std::string &filename) {
		return GdSurface();
	}
} // image

namespace display {
	void set_caption(const std::string &caption) { }
	GdSurface get_surface() { return GdSurface(); }
	void flip() { }
} // display

// gdgame:
//  +--core
//     +-Vector2
//     +-Vector3
//     +-Rect2
//     +-Rect2i
//  +--utils
//  +--math
//  +--locals
//  +--mouse
//  +--joystick
//  +--event
//  +--display
//  +--image
//  +--draw
//  +--time
//  +--mixer
//  +--font
//

PYBIND11_EMBEDDED_MODULE(gdgame, m) {
    m.doc() = "Godot bindings";
	// gdgame
	m.def("init", []() { });
	m.def("quit", []() { });
	// gdgame.utils
	py::module m_utils = m.def_submodule("utils", "gdgame module with different utilities.");
	m_utils.def("get_text", &utils::get_text);
	m_utils.def("print_dict", &utils::print_dict);
	// gdgame.math
	py::module m_math = m.def_submodule("math", "gdgame module with math definitions.");
	m_math.def("sin", static_cast<real_t (*)(real_t)>(&Math::sin));
	m_math.def("cos", static_cast<real_t (*)(real_t)>(&Math::cos));
	m_math.def("tan", static_cast<real_t (*)(real_t)>(&Math::tan));
	m_math.def("sinh", static_cast<real_t (*)(real_t)>(&Math::sinh));
	m_math.def("sinc", static_cast<real_t (*)(real_t)>(&Math::sinc));
	m_math.def("sincn", static_cast<real_t (*)(real_t)>(&Math::sincn));
	m_math.def("cosh", static_cast<real_t (*)(real_t)>(&Math::cosh));
	m_math.def("tanh", static_cast<real_t (*)(real_t)>(&Math::tanh));
	m_math.def("asin", static_cast<real_t (*)(real_t)>(&Math::asin));
	m_math.def("acos", static_cast<real_t (*)(real_t)>(&Math::acos));
	m_math.def("atan", static_cast<real_t (*)(real_t)>(&Math::atan));
	m_math.def("atan2", static_cast<real_t (*)(real_t, real_t)>(&Math::atan2));
	m_math.def("sqrt", static_cast<real_t (*)(real_t)>(&Math::sqrt));
	m_math.def("fmod", static_cast<real_t (*)(real_t, real_t)>(&Math::fmod));
	m_math.def("floor", static_cast<real_t (*)(real_t)>(&Math::floor));
	m_math.def("ceil", static_cast<real_t (*)(real_t)>(&Math::ceil));
	m_math.def("pow", static_cast<real_t (*)(real_t, real_t)>(&Math::pow));
	m_math.def("log", static_cast<real_t (*)(real_t)>(&Math::log));
	m_math.def("log10", static_cast<real_t (*)(real_t)>(&Math::log10));
	m_math.def("exp", static_cast<real_t (*)(real_t)>(&Math::exp));
	m_math.def("is_nan", static_cast<bool (*)(real_t)>(&Math::is_nan));
	m_math.def("is_inf", static_cast<bool (*)(real_t)>(&Math::is_inf));
	m_math.def("abs", static_cast<real_t (*)(real_t)>(&Math::abs));
	m_math.def("abs", static_cast<int (*)(int)>(&Math::abs));
	m_math.def("fposmod", static_cast<real_t (*)(real_t, real_t)>(&Math::fposmod));
	m_math.def("posmod", static_cast<int64_t (*)(int64_t, int64_t)>(&Math::posmod));
	m_math.def("deg2rad", static_cast<real_t (*)(real_t)>(&Math::deg2rad));
	m_math.def("rad2deg", static_cast<real_t (*)(real_t)>(&Math::rad2deg));
	m_math.def("lerp", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::lerp));
	m_math.def("lerp_angle", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::lerp_angle));
	m_math.def("inverse_lerp", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::lerp_angle));
	m_math.def("range_lerp", static_cast<real_t (*)(real_t, real_t, real_t, real_t, real_t)>(&Math::range_lerp));
	m_math.def("smoothstep", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::smoothstep));
	m_math.def("move_toward", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::move_toward));
	m_math.def("linear2db", static_cast<real_t (*)(real_t)>(&Math::linear2db));
	m_math.def("db2linear", static_cast<real_t (*)(real_t)>(&Math::linear2db));
	m_math.def("round", static_cast<real_t (*)(real_t)>(&Math::round));
	m_math.def("wrapi", &Math::wrapi);
	m_math.def("wrapf", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::wrapf));
	m_math.def("ease", &Math::ease);
	m_math.def("step_decimals", &Math::step_decimals);
	m_math.def("range_step_decimals", &Math::range_step_decimals);
	m_math.def("stepify", &Math::stepify);
	m_math.def("dectime", &Math::dectime);
	m_math.def("larger_prime", &Math::larger_prime);
	m_math.def("seed", &Math::seed);
	m_math.def("randomize", &Math::seed);
	m_math.def("rand_from_seed", &Math::rand_from_seed);
	m_math.def("rand", &Math::rand);
	m_math.def("randd", &Math::randd);
	m_math.def("randf", &Math::randf);
	m_math.def("random", static_cast<real_t (*)(real_t, real_t)>(&Math::random));
	m_math.def("random", static_cast<real_t (*)(int, int)>(&Math::random));
	m_math.def("is_equal_approx_ratio", static_cast<bool (*)(real_t, real_t, real_t, real_t)>(&Math::is_equal_approx_ratio));
	m_math.def("is_equal_approx", static_cast<bool (*)(real_t, real_t)>(&Math::is_equal_approx));
	m_math.def("is_equal_approx", static_cast<bool (*)(real_t, real_t, real_t)>(&Math::is_equal_approx));
	m_math.def("is_zero_approx", static_cast<bool (*)(real_t)>(&Math::is_zero_approx));
	m_math.def("absf", &Math::absf);
	m_math.def("absd", &Math::absd);
	m_math.def("fast_ftoi", &Math::fast_ftoi);
	m_math.def("halfbits_to_floatbits", &Math::halfbits_to_floatbits);
	m_math.def("halfptr_to_float", &Math::halfptr_to_float);
	m_math.def("half_to_float", &Math::half_to_float);
	m_math.def("make_half_float", &Math::make_half_float);
	m_math.def("snap_scalar", &Math::snap_scalar);
	m_math.def("snap_scalar_separation", &Math::snap_scalar_separation);
	m_math.def("map", &Math::map);
	// gdgame.core
	py::module m_core = m.def_submodule("core", "gdgame module with Godot core definitions.");
	py::class_<Vector2>(m_core, "Vector2")
		.def(py::init<real_t, real_t>())
		.def_readwrite("x", &Vector2::x)
		.def_readwrite("y", &Vector2::y)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= real_t())
		.def(py::self - py::self)
		.def(py::self * real_t())
		.def(py::self / real_t())
		.def(-py::self)
		.def("get_tuple", [](const Vector2 &v) { return std::make_tuple (v.x,v.y);})
		.def("__repr__", [](const Vector2 &v) { return String(v);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Vector3>(m_core, "Vector3")
		.def(py::init<real_t, real_t, real_t>())
		.def_readwrite("x", &Vector3::x)
		.def_readwrite("y", &Vector3::y)
		.def_readwrite("z", &Vector3::z)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= real_t())
		.def(py::self - py::self)
		.def(py::self * real_t())
		.def(py::self / real_t())
		.def(-py::self)
		.def("get_tuple", [](const Vector3 &v) { return std::make_tuple (v.x,v.y, v.z);})
		.def("__repr__", [](const Vector3 &v) { return String(v);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Rect2>(m_core, "Rect2")
		.def(py::init<real_t, real_t, real_t, real_t>())
		.def("get_position", &Rect2::get_position)
		.def("set_position", &Rect2::set_position)
		.def_property("pos", &Rect2::get_position, &Rect2::set_position)
		.def("get_size", &Rect2::get_size)
		.def("set_size", &Rect2::set_size)
		.def_property("size", &Rect2::get_size, &Rect2::set_size)
		.def("get_area", &Rect2::get_area)
		.def("get_center", &Rect2::get_center)
		.def("interpolate", &Rect2::interpolate)
		.def("intersects", &Rect2::intersects)
		.def("distance_to", &Rect2::distance_to)
		.def("intersects_transformed", &Rect2::intersects_transformed)
		.def("intersects_segment", &Rect2::intersects_segment)
		.def("encloses", &Rect2::encloses)
		.def("has_no_area", &Rect2::has_no_area)
		.def("clip", &Rect2::clip)
		.def("merge", &Rect2::merge)
		.def("has_point", &Rect2::has_point)
		.def("is_equal_approx", &Rect2::is_equal_approx)
		.def("grow", &Rect2::grow)
		.def("grow_by", &Rect2::grow_by)
		.def("grow_margin", &Rect2::grow_margin)
		.def("grow_individual", &Rect2::grow_individual)
		.def("expand", &Rect2::expand)
		.def("expand_to", &Rect2::expand_to)
		.def("abs", &Rect2::abs)
		.def("move_by", &Rect2::move_by)
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def("from_tuple", [](const Rect2 &rc, const py::tuple &args) { return Rect2();})
		.def("__repr__", [](const Rect2 &rc) { return String(rc);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Rect2i>(m_core, "Rect2i")
		.def(py::init<real_t, real_t, real_t, real_t>())
		.def("get_position", &Rect2i::get_position)
		.def("set_position", &Rect2i::set_position)
		.def_property("pos", &Rect2i::get_position, &Rect2i::set_position)
		.def("get_size", &Rect2i::get_size)
		.def("set_size", &Rect2i::set_size)
		.def_property("size", &Rect2i::get_size, &Rect2i::set_size)
		.def("get_area", &Rect2i::get_area)
		.def("intersects", &Rect2i::intersects)
		.def("encloses", &Rect2i::encloses)
		.def("has_no_area", &Rect2i::has_no_area)
		.def("clip", &Rect2i::clip)
		.def("merge", &Rect2i::merge)
		.def("has_point", &Rect2i::has_point)
		.def("grow", &Rect2i::grow)
		.def("grow_margin", &Rect2i::grow_margin)
		.def("grow_individual", &Rect2i::grow_individual)
		.def("expand", &Rect2i::expand)
		.def("expand_to", &Rect2i::expand_to)
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def("from_tuple", [](const Rect2i &rc, const py::tuple &args) { return Rect2();})
		.def("__repr__", [](const Rect2i &rc) { return String(rc);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Color>(m_core, "Color")
        .def(py::init<float, float, float, float>())
		.def("to_rgba32", &Color::to_rgba32)
		.def("to_argb32", &Color::to_argb32)
		.def("to_abgr32", &Color::to_abgr32)
		.def("to_rgba64", &Color::to_rgba64)
		.def("to_argb64", &Color::to_argb64)
		.def("to_abgr64", &Color::to_abgr64)
		.def("gray", &Color::gray)
		.def("get_h", &Color::get_h)
		.def("get_s", &Color::get_s)
		.def("get_v", &Color::get_v)
		.def("set_hsv", &Color::set_hsv)
		.def("with_alpha", &Color::with_alpha)
		.def("is_equal_approx", &Color::is_equal_approx)
		.def("invert", &Color::invert)
		.def("contrast", &Color::contrast)
		.def("inverted", &Color::inverted)
		.def("contrasted", &Color::contrasted)
		.def("linear_interpolate", &Color::linear_interpolate)
		.def("darkened", &Color::darkened)
		.def("lightened", &Color::lightened)
		.def("to_rgbe9995", &Color::to_rgbe9995)
		.def("to_rgbe9995", &Color::to_rgbe9995)
		.def("blend", &Color::blend)
		.def("to_linear", &Color::to_linear)
		.def("to_srgb", &Color::to_srgb)
		.def_static("hex", &Color::hex)
		.def_static("hex64", &Color::hex64)
		.def_static("html", &Color::html)
		.def_static("html_is_valid", &Color::html)
		.def_static("named", &Color::html)
		.def("to_html", &Color::to_html)
		.def("from_hsv", &Color::from_hsv)
		.def("from_hsv", &Color::from_hsv)
		.def_static("from_abgr", &Color::html)
		.def_static("from_rgbe9995", &Color::html)
		.def_static("solid", &Color::html)
		.def(py::self + py::self)
		.def(py::self * py::self)
		.def(py::self < py::self)
		.def(py::self *= real_t())
		.def(py::self /= real_t())
		.def("__repr__", [](const Color &c) { return String(c);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<GdSurface>(m, "Surface")
		.def("convert_alpha", &GdSurface::convert_alpha)
		.def("get_width", &GdSurface::get_width)
		.def("get_height", &GdSurface::get_height)
		.def("fill", &GdSurface::fill)
		.def("blit", &GdSurface::blit)
		.def("blit", &GdSurface::blit_area)
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::module m_locals = m.def_submodule("locals", "Module contains various constants used by gdgame.");
	m_locals.attr("K_SPACE") = py::int_(int(KEY_SPACE));
	m_locals.attr("K_ESCAPE") = py::int_(int(KEY_ESCAPE));
	m_locals.attr("K_RETURN") = py::int_(int(KEY_ENTER));
	m_locals.attr("K_KP_ENTER") = py::int_(int(KEY_KP_ENTER));
	m_locals.attr("K_LEFT") = py::int_(int(KEY_LEFT));
	m_locals.attr("K_RIGHT") = py::int_(int(KEY_RIGHT));
	m_locals.attr("K_UP") = py::int_(int(KEY_UP));
	m_locals.attr("K_DOWN") = py::int_(int(KEY_DOWN));
	m_locals.attr("K_PAGEUP") = py::int_(int(KEY_PAGEUP));
	m_locals.attr("K_PAGEDOWN") = py::int_(int(KEY_PAGEDOWN));
	m_locals.attr("K_a") = py::int_(int(KEY_A));
	m_locals.attr("K_b") = py::int_(int(KEY_B));
	m_locals.attr("K_c") = py::int_(int(KEY_C));
	m_locals.attr("K_d") = py::int_(int(KEY_D));
	m_locals.attr("K_e") = py::int_(int(KEY_E));
	m_locals.attr("K_f") = py::int_(int(KEY_F));
	m_locals.attr("K_g") = py::int_(int(KEY_G));
	m_locals.attr("K_h") = py::int_(int(KEY_H));
	m_locals.attr("K_i") = py::int_(int(KEY_I));
	m_locals.attr("K_j") = py::int_(int(KEY_J));
	m_locals.attr("K_k") = py::int_(int(KEY_K));
	m_locals.attr("K_l") = py::int_(int(KEY_L));
	m_locals.attr("K_m") = py::int_(int(KEY_M));
	m_locals.attr("K_n") = py::int_(int(KEY_N));
	m_locals.attr("K_o") = py::int_(int(KEY_O));
	m_locals.attr("K_p") = py::int_(int(KEY_P));
	m_locals.attr("K_q") = py::int_(int(KEY_Q));
	m_locals.attr("K_r") = py::int_(int(KEY_R));
	m_locals.attr("K_s") = py::int_(int(KEY_S));
	m_locals.attr("K_t") = py::int_(int(KEY_T));
	m_locals.attr("K_y") = py::int_(int(KEY_Y));
	m_locals.attr("K_0") = py::int_(int(KEY_0));
	m_locals.attr("K_1") = py::int_(int(KEY_1));
	m_locals.attr("K_2") = py::int_(int(KEY_2));
	m_locals.attr("K_3") = py::int_(int(KEY_3));
	m_locals.attr("K_4") = py::int_(int(KEY_4));
	m_locals.attr("K_5") = py::int_(int(KEY_5));
	m_locals.attr("K_6") = py::int_(int(KEY_6));
	m_locals.attr("K_7") = py::int_(int(KEY_7));
	m_locals.attr("K_8") = py::int_(int(KEY_8));
	m_locals.attr("K_9") = py::int_(int(KEY_9));
	m_locals.attr("K_F1") = py::int_(int(KEY_F1));
	m_locals.attr("K_F2") = py::int_(int(KEY_F2));
	m_locals.attr("K_F3") = py::int_(int(KEY_F3));
	m_locals.attr("K_F4") = py::int_(int(KEY_F4));
	m_locals.attr("K_F5") = py::int_(int(KEY_F5));
	m_locals.attr("K_F6") = py::int_(int(KEY_F6));
	m_locals.attr("K_F7") = py::int_(int(KEY_F7));
	m_locals.attr("K_F8") = py::int_(int(KEY_F8));
	m_locals.attr("K_F9") = py::int_(int(KEY_F9));
	m_locals.attr("K_F10") = py::int_(int(KEY_F10));
	m_locals.attr("K_F11") = py::int_(int(KEY_F11));
	m_locals.attr("K_F12") = py::int_(int(KEY_F12));
	// gdgame.time
	py::module m_time = m.def_submodule("time", "gdgame module for monitoring time.");
	m_time.def("get_ticks", []() { return 0; });
	// gdgame.event
	py::module m_event = m.def_submodule("event", "gdgame module for interacting with events and queues.");
	py::class_<GdEvent>(m_event, "Event")
		.attr("__version__") = VERSION_FULL_CONFIG;
	m_event.def("get", &event::get);
	m_event.def("set_grab", &event::set_grab);
	// gdgame.mouse
	py::module m_mouse = m.def_submodule("mouse", "gdgame module to work with the mouse.");
	m_mouse.def("set_visible", [](bool visible) { });
	m_mouse.def("get_pos", []() { return std::make_tuple(0, 0);});
	// gdgame.joystick
	py::module m_joystick = m.def_submodule("joystick", "gdgame module for interacting with joysticks, gamepads, and trackballs.");
	m_joystick.def("init", []() { });
	m_joystick.def("quit", []() { });
	m_joystick.def("get_count", []() { return 0; });
	// gdgame.mixer
	py::module m_mixer = m.def_submodule("mixer", "gdgame module for loading and playing sounds.");
	py::class_<GdSound>(m_mixer, "Sound")
		.def(py::init<const std::string&>())
		.def("set_volume", &GdSound::set_volume)
		.def("play", &GdSound::play)
		.attr("__version__") = VERSION_FULL_CONFIG;
	m_mixer.def("pre_init", [](int frequency=22050, int size=-16, int channels=2, int buffersize=4096) { });
	m_mixer.def("init", []() { });
	m_mixer.def("quit", []() { });
	m_mixer.def("set_num_channels", [](int channels) { });
	// gdgame.display
	py::module m_display = m.def_submodule("display", "gdgame module to control the display window and screen.");
	m_display.def("set_mode", [](const py::tuple &size) { });
	m_display.def("set_caption", &display::set_caption);
	m_display.def("get_surface", &display::get_surface);
	m_display.def("flip", &display::flip);
	// gdgame.draw
	py::module m_draw = m.def_submodule("draw", "gdgame module for drawing shapes.");
	// gdgame.image
	py::module m_image = m.def_submodule("image", "gdgame module for image transfer.");
	m_image.def("load", &image::load);
	// gdgame.font
	py::module m_font = m.def_submodule("font", "gdgame module for loading and rendering fonts.");
	py::class_<GdFont>(m_font, "Font")
		.def(py::init<const std::string&, int>())
		.def("render", &GdFont::render)
		.attr("__version__") = VERSION_FULL_CONFIG;
	m_font.def("init", []() { });
	m_font.def("quit", []() { });
}
