#include "godot_cpython.h"

#include "pybind11/pybind11.h"
#include "pybind11/embed.h"
#include "pybind11/operators.h"
#include "pybind11/stl.h"

#include "core/rid.h"
#include "core/variant.h"
#include "core/math/geometry.h"
#include "servers/visual_server.h"

#include "pylib/godot/py_godot.h"
#include "pylib/godot/py_binds.h"

#include "default_ttf.gen.h"
#include "default_bitmap.gen.h"
#include "default_mono.gen.h"

#include <iostream>
#include <string>

// Reference:
// ----------
// 1. https://github.com/pybind/pybind11/issues/1619

#if __GNUC__
# define _attr_visibility_hidden __attribute__ ((visibility("hidden")))
#else
# define _attr_visibility_hidden
#endif

namespace py = pybind11;
using namespace py::literals;

#ifdef _HAS_EXCEPTIONS
static py::object py_eval(const char *expr, const py::object &o);
#endif
static py::object py_call(py::object p_obj, String p_func_name, py::args p_args = py::args());
static py::object py_call(String p_func_name, py::args p_args = py::args(), String p_module = "__main__");

struct _attr_visibility_hidden PyGodotInstance::InstancePrivateData {
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
	if (!_p->py_app.is_none()) {
		if (const InputEventMouseMotion *m = Object::cast_to<InputEventMouseMotion>(*p_event)) {
			GdEvent ev{GdEvent::MOUSEMOTION, m->get_position()};
			py_call(_p->py_app, p_event_func, py::make_tuple(ev));
			return true;
		}
		if (const InputEventMouseButton *mb = Object::cast_to<InputEventMouseButton>(*p_event)) {
			GdEvent ev{mb->is_pressed() ? GdEvent::MOUSEBUTTONDOWN : GdEvent::MOUSEBUTTONUP, mb->get_position(), mb->get_button_index()};
			py_call(_p->py_app, p_event_func, py::make_tuple(ev));
			return true;
		}
	}
	return false;
}

Variant PyGodotInstance::call(const String &p_func, real_t p_arg) {
	if (!_p->py_app.is_none()) {
		auto r = py_call(_p->py_app, p_func, py::make_tuple(p_arg));
		if (!r.is_none()) {
			if (py::isinstance<py::bool_>(r)) {
				return r.cast<bool>();
			}
			if (py::isinstance<py::int_>(r)) {
				return r.cast<int>();
			}
			if (py::isinstance<py::float_>(r)) {
				return r.cast<float>();
			}
			if (py::isinstance<py::str>(r)) {
				return r.cast<std::string>().c_str();
			}
			WARN_PRINT("Ignoring function return value");
		}
	}
	return Variant();
}

Variant PyGodotInstance::call(const String &p_func) {
	if (!_p->py_app.is_none()) {
		auto r = py_call(_p->py_app, p_func);
		if (!r.is_none()) {
			if (py::isinstance<py::bool_>(r)) {
				return r.cast<bool>();
			}
			if (py::isinstance<py::int_>(r)) {
				return r.cast<int>();
			}
			if (py::isinstance<py::float_>(r)) {
				return r.cast<float>();
			}
			if (py::isinstance<py::str>(r)) {
				return r.cast<std::string>().c_str();
			}
			WARN_PRINT("Ignoring function return value");
		}
	}
	return Variant();
}

bool PyGodotInstance::build_pygodot(int p_instance_id, const String &p_build_func) {
	_p->py_app = py_call(p_build_func, py::make_tuple(p_instance_id));
	return (!_p->py_app.is_none());
}


// BEGIN Godot/Python wrapper objects

static std::map<std::tuple<std::string, int, int, int>, Ref<Font>> _font_cache;

#ifdef MODULE_FREETYPE_ENABLED
static Ref<Font> _get_default_dynamic_font(int size, int stretch = 0, int outline = 0) {
	static Ref<DynamicFontData> dfont;
	std::tuple<std::string, int, int, int> key = std::make_tuple("__default__", size, stretch, outline);
	if (_font_cache.count(key)) {
		return _font_cache[key];
	}
	if (dfont.is_null()) {
		dfont.instance();
		dfont->set_font_ptr(_default_ttf, _default_ttf_size);
	}
	Ref<DynamicFont> font = memnew(DynamicFont);
	font->set_font_data(dfont);
	font->set_size(size);
	if (stretch > 0 && stretch < 100) {
		font->set_stretch_scale(stretch);
	}
	if (outline > 0) {
		font->set_outline_size(outline);
	}
	_font_cache[key] = font;
	return font;
}

static Ref<Font> _get_dynamic_font(const std::string &path, int size, int stretch = 0, int outline = 0) {
	std::tuple<std::string, int, int, int> key = std::make_tuple(path, size, stretch, outline);
	if (_font_cache.count(key)) {
		return _font_cache[key];
	}
	Ref<DynamicFont> font = memnew(DynamicFont);
	font->set_font_data(ResourceLoader::load(path.c_str()));
	font->set_size(size);
	if (stretch > 0 && stretch < 100) {
		font->set_stretch_scale(stretch);
	}
	if (outline > 0) {
		font->set_outline_size(outline);
	}
	_font_cache[key] = font;
	return font;
}

#else // MODULE_FREETYPE_ENABLED

static Ref<BitmapFont> _get_default_bitmap_font() {
	static Ref<BitmapFont> bfont;
	if (bfont.is_null()) {
		bfont.instance();
		Ref<BitmapFont> _font = memnew(BitmapFont);
		if (_font->create_from_fnt_ptr(_default_bitmap_fnt, strlen(_default_bitmap_fnt), _default_bitmap_png) != OK) {
			// this should not happen, since all data are embedded
			// if so, we are rather running out of resources/crashing
			WARN_PRINT("Failed to load default bitmap font.");
		}
	}
	return bfont;
}
#endif // MODULE_FREETYPE_ENABLED

static Ref<BitmapFont> make_font_from_grid(int p_height, int p_ascent, int p_charcount, const int *p_char_rects, int p_w, int p_h, const unsigned char *p_img) {
	Ref<BitmapFont> font(memnew(BitmapFont));

	Ref<Image> image = memnew(Image(p_img));
	Ref<ImageTexture> tex = memnew(ImageTexture);
	tex->create_from_image(image);

	font->add_texture(tex);

	for (int i = 0; i < p_charcount; i++) {
		const int *c = &p_char_rects[i * 8];

		int chr = c[0];
		Rect2 frect;
		frect.position.x = c[1];
		frect.position.y = c[2];
		frect.size.x = c[3];
		frect.size.y = c[4];
		Point2 align(c[6], c[5]);
		int advance = c[7];

		font->add_char(chr, 0, frect, align, advance);
	}

	font->set_height(p_height);
	font->set_ascent(p_ascent);

	return font;
}

static Ref<BitmapFont> make_font_from_hstrip(const String &p_font_path, const String &p_characters) {
	Ref<BitmapFont> font(memnew(BitmapFont));

	Ref<Image> image = memnew(Image);
	if (image->load(p_font_path) == OK) {
		image->lock();

		auto is_null_column = [](const Ref<Image> img, int x) {
			for (int y = 0; y < img->get_height(); ++y) {
				if (img->get_pixel(x, y).a != 0) {
					return false;
				}
			}
			return true;
		};

		bool in_char = false;
		int char_begin = 0, char_cnt = 0;
		for (int x = 0; x < image->get_width(); ++x) {
			if (is_null_column(image, x)) {
				if (in_char) {
					// end of char started from char_begin
					const int char_width = x - char_begin;
					Rect2 frect;
					frect.position = Point2(char_begin, 0);
					frect.size = Size2(char_width, image->get_height());
					font->add_char(p_characters[char_cnt], 0, frect, Point2(), char_width + 1);

					in_char = false;
					char_cnt++;
				}
			} else {
				if (!in_char) {
					// begin of char
					in_char = true;
					char_begin = x;
				}
			}
		}
		image->unlock();
	}
	return font;
}

void GdFont::load(const std::string &path, int size, int stretch) {
	if (path.empty() or path == "_") {
#ifdef MODULE_FREETYPE_ENABLED
		font = _get_default_dynamic_font(size, stretch);
#else
		font = _get_default_bitmap_font();
#endif // MODULE_FREETYPE_ENABLED
	} else {
		// create font depending of the extension
		const String ext = String(path.c_str()).get_extension();
		if (ext == "ttf" || ext == "otf") {
#ifdef MODULE_FREETYPE_ENABLED
			FileAccessRef fnt(FileAccess::open(path.c_str(), FileAccess::READ));
			if (fnt) {
				font = _get_dynamic_font(path, size, stretch);
			} else {
				WARN_PRINT("Failed to open font at: " + String(path.c_str()));
				font = _get_default_dynamic_font(size, stretch);
			}
#else
			WARN_PRINT("TrueType font not available.");
#endif // MODULE_FREETYPE_ENABLED
		} else if (ext == "fnt") {
			Ref<BitmapFont> _font = ResourceLoader::load(String(path.c_str()), "BitmapFont");
		} else if (ext == "font") {
			FileAccessRef fnt(FileAccess::open(path.c_str(), FileAccess::READ));
			if (fnt) {
				const String &characters = fnt->get_line();
				const String &font_file = fnt->get_line();
				font = make_font_from_hstrip(font_file, characters);
			} else {
				WARN_PRINT("Failed to open font at: " + String(path.c_str()));
			}
		}
	}
}

// END

// BEGIN Python bindings

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
	m_utils.def("unget_text", &utils::unget_text);
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
	m_math.def("random", static_cast<int (*)(int, int)>(&Math::random));
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
	py::class_<String>(m_core, "String")
		.def("remove", &String::remove)
		.def("clear", &String::clear)
		.def("size", &String::size)
		.def("length", &String::length)
		.def("casecmp_to", &String::casecmp_to)
		.def("nocasecmp_to", &String::nocasecmp_to)
		.def("naturalnocasecmp_to", &String::nocasecmp_to)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def("__repr__", [](const String &s) { return std::str(s); })
		.def("__copy__", [](const String &s){ return String(s); })
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Vector2>(m_core, "Vector2")
		.def(py::init<real_t, real_t>())
		.def_readwrite("x", &Vector2::x)
		.def_readwrite("y", &Vector2::y)
		.def_readwrite("width", &Vector2::x)
		.def_readwrite("height", &Vector2::y)
		.def("length", &Vector2::length)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= real_t())
		.def(py::self - py::self)
		.def(py::self * real_t())
		.def(py::self / real_t())
		.def(-py::self)
		.def("get_tuple", [](const Vector2 &v) { return std::make_tuple (v.x,v.y); })
		.def("__copy__", [](const Vector2 &v){ return Vector2(v); })
		.def("__repr__", [](const Vector2 &v) { return std::str(v); })
		.def("__getitem__", [](const Vector2 &v, int index) { return v[index]; })
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Vector3>(m_core, "Vector3")
		.def(py::init<real_t, real_t, real_t>())
		.def_readwrite("x", &Vector3::x)
		.def_readwrite("y", &Vector3::y)
		.def_readwrite("z", &Vector3::z)
		.def("length", &Vector3::length)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= real_t())
		.def(py::self - py::self)
		.def(py::self * real_t())
		.def(py::self / real_t())
		.def(-py::self)
		.def("get_tuple", [](const Vector3 &v) { return std::make_tuple (v.x,v.y, v.z);})
		.def("__copy__", [](const Vector3 &v){ return Vector3(v); })
		.def("__repr__", [](const Vector3 &v) { return std::str(v);})
		.def("__getitem__", [](const Vector3 &v, int index) { return v[index]; })
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
		.def("contains", &Rect2::has_point)
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
		.def("from_tuple", [](const Rect2 &rc, const py::tuple &args) { return Rect2(); })
		.def("__copy__", [](const Rect2 &rc){ return Rect2(rc); })
		.def("__repr__", [](const Rect2 &rc) { return std::str(rc); })
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
		.def("contains", &Rect2i::has_point)
		.def("grow", &Rect2i::grow)
		.def("grow_margin", &Rect2i::grow_margin)
		.def("grow_individual", &Rect2i::grow_individual)
		.def("expand", &Rect2i::expand)
		.def("expand_to", &Rect2i::expand_to)
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def("from_tuple", [](const Rect2i &rc, const py::tuple &args) { return Rect2(); })
		.def("__copy__", [](const Rect2i &rc){ return Rect2i(rc); })
		.def("__repr__", [](const Rect2i &rc) { return std::str(rc); })
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
		.def("__copy__", [](const Color &c){ return Color(c); })
		.def("__repr__", [](const Color &c) { return std::str(c);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<GdSurface>(m, "Surface")
		.def("get_width", &GdSurface::get_width)
		.def("get_height", &GdSurface::get_height)
		.def("fill", &GdSurface::fill)
		.def("blit", &GdSurface::blit)
		.def("blit", &GdSurface::blit_area)
		.def("blit", &GdSurface::blit_or_area)
		.def("__repr__", [](const GdSurface &s) {
			switch(s.impl->get_surface_type()) {
				case GdSurfaceImpl::DISPLAY_SURFACE: return std::str(vformat("GdSurface 0x%0x {DISPLAY_SURFACE}", int64_t(&s)));
				case GdSurfaceImpl::COLOR_SURFACE: return std::str(vformat("GdSurface 0x%0x {COLOR_SURFACE, %s}", int64_t(&s), s.get_as_color()->surf_color));
				case GdSurfaceImpl::TEXT_SURFACE: return std::str(vformat("GdSurface 0x%0x {TEXT_SURFACE, '%s'}", int64_t(&s), s.get_as_text()->text));
				case GdSurfaceImpl::TEXTURE_SURFACE: return std::str(vformat("GdSurface 0x%0x {TEXTURE_SURFACE}", int64_t(&s)));
			}
		})
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
	m_locals.attr("MOUSE_MODE_VISIBLE") = py::int_(int(Input::MOUSE_MODE_VISIBLE));
	m_locals.attr("MOUSE_MODE_HIDDEN") = py::int_(int(Input::MOUSE_MODE_HIDDEN));
	m_locals.attr("QUIT") = py::int_(int(GdEvent::QUIT));
	m_locals.attr("KEYDOWN") = py::int_(int(GdEvent::KEYDOWN));
	m_locals.attr("KEYUP") = py::int_(int(GdEvent::KEYUP));
	m_locals.attr("MOUSEMOTION") = py::int_(int(GdEvent::MOUSEMOTION));
	m_locals.attr("MOUSEBUTTONDOWN") = py::int_(int(GdEvent::MOUSEBUTTONDOWN));
	m_locals.attr("MOUSEBUTTONUP") = py::int_(int(GdEvent::MOUSEBUTTONUP));
	m_locals.attr("JOYBUTTONDOWN") = py::int_(int(GdEvent::JOYBUTTONDOWN));
	m_locals.attr("JOYBUTTONUP") = py::int_(int(GdEvent::JOYBUTTONUP));
	// gdgame.time
	py::module m_time = m.def_submodule("time", "gdgame module for monitoring time.");
	m_time.def("get_ticks", []() { return OS::get_singleton()->get_ticks_msec(); });
	// gdgame.event
	py::module m_event = m.def_submodule("event", "gdgame module for interacting with events and queues.");
	py::class_<GdEvent>(m_event, "Event")
		.def_readonly("type", &GdEvent::type)
		.def_readonly("button", &GdEvent::button)
		.def_readonly("pos", &GdEvent::position)
		.def("get_pos", [](const GdEvent &e) { return e.position; })
		.def("update", []() { })
		.attr("__version__") = VERSION_FULL_CONFIG;
	m_event.def("set_grab", &event::set_grab);
	// gdgame.mouse
	py::module m_mouse = m.def_submodule("mouse", "gdgame module to work with the mouse.");
	m_mouse.def("set_visible", [](bool visible) {
		if (visible) {
			Input::get_singleton()->set_custom_mouse_cursor(RES()); // restore default cursor
		} else {
			Input::get_singleton()->set_custom_mouse_cursor(_get_dot_cursor()); // load dot-cursor
		}
	});
	m_mouse.def("set_mouse_mode", [](int mouse_mode) {
		Input::get_singleton()->set_mouse_mode(static_cast<Input::MouseMode>(mouse_mode));
	});
	m_mouse.def("set_custom_mouse_cursor", [](std::string path) {
		Input::get_singleton()->set_custom_mouse_cursor(ResourceLoader::load(path.c_str()));
	});
	m_mouse.def("get_pos", [](int instance_id) {
			if (Object *instance = ObjectDB::get_instance(instance_id)) {
				if (CanvasItem *canvas = Object::cast_to<CanvasItem>(instance)) {
					const Vector2 pos = canvas->get_local_mouse_position();
					return std::make_tuple(pos.x, pos.y);
				} else {
					WARN_PRINT("Not an CanvasItem");
				}
			}
			return std::make_tuple(real_t(0), real_t(0));
	});
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
		.def(py::init<const std::string&, int, int>())
		.def("get_height", &GdFont::get_height)
		.def("size", &GdFont::size)
		.def("render", &GdFont::render)
		.attr("__version__") = VERSION_FULL_CONFIG;
	m_font.def("init", []() { });
	m_font.def("quit", []() { });
}

// END

// BEGIN Python utilities

#ifdef _HAS_EXCEPTIONS
static py::object py_eval(const char *expr, const py::object &o) {
	py::object res = py::none();
	try {
		if (not o.is_none()) {
			auto locals = py::dict("_v"_a = o);
			res = py::eval(expr, py::globals(), locals);
		} else {
			res = py::eval(expr, py::globals());
		}
	} catch (py::error_already_set &e) {
		std::cout << "py_eval error_already_set: " << std::endl;
	} catch (std::runtime_error &e) {
		std::cout << "py_eval runtime_error: " << e.what() << std::endl;
	} catch (...) {
		std::cout << "py_eval unknown exception" << std::endl;
	}
	return res;
}
#endif // _HAS_EXCEPTIONS

// Reference:
// ----------
// https://developpaper.com/using-pybind11-to-call-between-c-and-python-code-on-windows-10/

static py::object py_call(py::object p_obj, String p_func_name, py::args p_args) {
	return p_obj.attr(p_func_name.utf8().get_data())(*p_args);
}

static py::object py_call(String p_func_name, py::args p_args, String p_module) {
	std::string module = p_module.utf8().get_data();
	std::string function = p_func_name.utf8().get_data();

	if (module.empty()) {
		module = "__main__";
	}
	return py::module::import(module.c_str()).attr(function.c_str())(*p_args);
}

// END
