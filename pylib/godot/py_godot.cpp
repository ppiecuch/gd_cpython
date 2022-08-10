#include "godot_cpython.h"

#include "pybind11/pybind11.h"
#include "pybind11/embed.h"
#include "pybind11/operators.h"
#include "pybind11/stl.h"

#include "core/rid.h"
#include "core/variant.h"
#include "core/version.h"
#include "core/version_generated.gen.h"
#include "core/math/geometry.h"
#include "core/os/os.h"
#include "scene/2d/audio_stream_player_2d.h"
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

template <typename... Args>
using overload_cast_ = pybind11::detail::overload_cast_impl<Args...>;

#ifdef _HAS_EXCEPTIONS
py::object py_eval(const char *expr, const py::object &o);
#endif
py::object py_call(py::object p_obj, String p_func_name, py::args p_args = py::args());
py::object py_call(String p_func_name, py::args p_args = py::args(), String p_module = "__main__");

static std::map<std::tuple<std::string, int, int, int>, Ref<Font>> _font_cache;

static String get_full_version_string() {
	static String _version;
	if (_version.empty()) {
		String hash = String(VERSION_HASH);
		if (hash.length() != 0) {
			hash = "." + hash.left(9);
		}
		_version = String(VERSION_NAME) + " v" + String(VERSION_FULL_BUILD) + hash + " - " + String(VERSION_WEBSITE);
	}
	return _version;
}

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
			GdEvent ev{GdEvent::MOUSEMOTION};
			ev.position = m->get_position();
			py_call(_p->py_app, p_event_func, py::make_tuple(ev));
			return true;
		}
		if (const InputEventMouseButton *mb = Object::cast_to<InputEventMouseButton>(*p_event)) {
			GdEvent ev{mb->is_pressed() ? GdEvent::MOUSEBUTTONDOWN : GdEvent::MOUSEBUTTONUP};
			ev.position = mb->get_position();
			ev.button = mb->get_button_index();
			py_call(_p->py_app, p_event_func, py::make_tuple(ev));
			return true;
		}
		if (const InputEventKey *mk = Object::cast_to<InputEventKey>(*p_event)) {
			GdEvent ev{mk->is_pressed() ? GdEvent::KEYDOWN : GdEvent::KEYUP};
			ev.key = mk->get_scancode();
			ev.unicode = mk->get_unicode();
			py_call(_p->py_app, p_event_func, py::make_tuple(ev));
			return true;
		}
	}
	return false;
}

Variant PyGodotInstance::pycall(const String &p_func, real_t p_arg) {
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

Variant PyGodotInstance::pycall(const String &p_func) {
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

void PyGodotInstance::destroy_pygodot() {
	if (!_p->py_app.is_none()) {
		_p->py_app = py::none();
		py::module_::import("gc").attr("collect")();
		// clear global data:
		_font_cache.clear();
		py::print("*** Application is closing.");
	}
}


// BEGIN Godot/Python wrapper objects

#ifdef MODULE_FREETYPE_ENABLED
static Ref<Font> _get_default_dynamic_font(int size, int outline_size, Color outline_color, int stretch) {
	static Ref<DynamicFontData> default_font_data;
	std::tuple<std::string, int, int, int> key = std::make_tuple("__default__", size, stretch, outline_size);
	if (_font_cache.count(key)) {
		return _font_cache[key];
	}
	if (default_font_data.is_null()) {
		default_font_data.instance();
		default_font_data->set_font_ptr(_default_ttf, _default_ttf_size);
		_register_global_ref(default_font_data); // remove at exit
	}
	Ref<DynamicFont> font = memnew(DynamicFont);
	font->set_font_data(default_font_data);
	font->set_size(size);
	if (stretch > 0 && stretch < 100) {
		font->set_stretch_scale(stretch);
	}
	if (outline_size > 0) {
		font->set_outline_size(outline_size);
		font->set_outline_color(outline_color);
	}
	_font_cache[key] = font;
	return font;
}

static Ref<Font> _get_dynamic_font(const std::string &path, int size, int outline_size, Color outline_color, int stretch) {
	std::tuple<std::string, int, int, int> key = std::make_tuple(path, size, stretch, outline_size);
	if (_font_cache.count(key)) {
		return _font_cache[key];
	}
	Ref<DynamicFont> font = memnew(DynamicFont);
	font->set_font_data(ResourceLoader::load(path.c_str()));
	font->set_size(size);
	if (stretch > 0 && stretch < 100) {
		font->set_stretch_scale(stretch);
	}
	if (outline_size > 0) {
		font->set_outline_size(outline_size);
		font->set_outline_color(outline_color);
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

static Ref<BitmapFont> make_font_from_grid(const String &p_characters, int p_grid_width, int p_grid_height, const String &p_img) {
	Ref<BitmapFont> font(memnew(BitmapFont));

	Ref<Image> image = memnew(Image);
	if (image->load(p_img) == OK) {
		Ref<ImageTexture> tex = memnew(ImageTexture);
		tex->create_from_image(image);

		font->add_texture(tex);

		const Size2i cell_size = image->get_size() / Size2(p_grid_width, p_grid_height);
		for (int x = 0; x < p_grid_width; x++) {
			for (int y = 0; y < p_grid_height; y++) {
				const int index = x + y * p_grid_width;
				if (index < p_characters.length()) {
					const int chr = p_characters[index];
					Rect2 frect(Point2(x * cell_size.width, y * cell_size.height), cell_size);
					font->add_char(chr, 0, frect, Point2(), cell_size.width);
				} else {
					break;
				}
			}
		}
		font->set_height(cell_size.height);
	}
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

static bool play_sound(Node *p_owner, Ref<AudioStream> p_sound) {
	ERR_FAIL_NULL_V(p_owner, false);
	ERR_FAIL_NULL_V(p_sound, false);

	const int _initial = 2;
	static Vector<AudioStreamPlayer2D*> _audios;

	auto _create_stream = [](Node *_owner) {
		if (AudioStreamPlayer2D *ply = memnew(AudioStreamPlayer2D)) {
			_audios.push_back(ply);
			_owner->add_child(ply);
		} else {
			WARN_PRINT("Failed to create AudioStreamPlayer2D");
		}
	};

	auto _play_last_stream = [=](Ref<AudioStream> _sound) {
		auto a = _audios.last();
		a->set_stream(_sound);
		a->play();
#ifdef DEBUG_ENABLED
		print_verbose(vformat("(PyGodot) Playing sample: %d from AudioStream: %d", _sound, _audios.find(a)));
#endif
	};

	if (_audios.size() == 0) {
		for (int c = 0; c < _initial; c++) {
			_create_stream(p_owner);
		}
	}

	for (auto a : _audios) {
		if (a->is_playing()) {
			if (a == _audios.last()) {
				_create_stream(p_owner);
				_play_last_stream(p_sound);
				break;
			} else {
				continue;
			}
		} else {
			a->set_stream(p_sound);
			a->play();
#ifdef DEBUG_ENABLED
			print_verbose(vformat("(PyGodot) Playing sample: %d from AudioStream: %d", p_sound, _audios.find(a)));
#endif
			break;
		}
	}

	return true;
}


void GdSound::set_volume(real_t vol) { }
void GdSound::play(bool loop) { }
void GdSound::fadeout(int fadetime) { }
void GdSound::stop() { }


bool GdFont::load(const std::string &path, int size, int outline_size, Color outline_color, int stretch) {
	if (path.empty() || path == "_") {
#ifdef MODULE_FREETYPE_ENABLED
		font = _get_default_dynamic_font(size, outline_size, outline_color, stretch);
#else
		font = _get_default_bitmap_font();
#endif // MODULE_FREETYPE_ENABLED
	} else {
		// create font depending of the extension
		const String ext = String(path.c_str()).get_extension();
		if (ext == "ttf" || ext == "otf") {
#ifdef MODULE_FREETYPE_ENABLED
			if (ResourceLoader::exists(path.c_str())) {
				font = _get_dynamic_font(path, size, outline_size, outline_color, stretch);
			} else {
				WARN_PRINT("Failed to open font at: " + String(path.c_str()));
				font = _get_default_dynamic_font(size, outline_size, outline_color, stretch);
			}
#else
			WARN_PRINT("TrueType font support is not available.");
#endif // MODULE_FREETYPE_ENABLED
		} else if (ext == "fnt") {
			Ref<BitmapFont> _font = ResourceLoader::load(String(path.c_str()), "BitmapFont");
		} else if (ext == "hfont") {
			FileAccessRef fnt(FileAccess::open(path.c_str(), FileAccess::READ));
			if (fnt) {
				const String &characters = fnt->get_line();
				const String &font_file = fnt->get_line();
				font = make_font_from_hstrip(font_file, characters);
			} else {
				WARN_PRINT("Failed to open font at: " + String(path.c_str()));
			}
		} else if (ext == "gfont") {
			FileAccessRef fnt(FileAccess::open(path.c_str(), FileAccess::READ));
			if (fnt) {
				const String &characters = fnt->get_line();
				const String &font_file = fnt->get_line();
				const Vector<String> &grid = fnt->get_line().split(",");
				font = make_font_from_grid(font_file, grid[0].to_int(), grid[1].to_int(), characters);
			} else {
				WARN_PRINT("Failed to open font at: " + String(path.c_str()));
			}
		}
	}
	return font.is_valid();
}

// END

// BEGIN Python bindings

// gdgame:
//  +--core
//     +-Vector2
//     +-Vector3
//     +-Rect2
//     +-Rect2i
//  +--info
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
	m_utils.def("print_verbose", &utils::print_verbose);
	m_utils.def("lin_ipol", &utils::lin_ipol, "value"_a, "a"_a, "b"_a, "begin"_a = 0, "end"_a = 1.0);
	m_utils.def("get_instance_size", &utils::get_instance_size);
	m_utils.def("quit", &utils::quit);
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
		.def("length_squared", &Vector2::length_squared)
		.def("min", [](const Vector2 &v, const Vector2 &arg) { return v.min(arg); })
		.def("max", [](const Vector2 &v, const Vector2 &arg) { return v.max(arg); })
		.def("positive", [](const Vector2 &v) { return v.x >= 0 && v.y >= 0; })
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self - py::self)
		.def(py::self > py::self)
		.def(py::self < py::self)
		.def(py::self >= py::self)
		.def(py::self <= py::self)
		.def(py::self *= real_t())
		.def(py::self * real_t())
		.def(py::self / real_t())
		.def(py::self > real_t())
		.def(py::self < real_t())
		.def(py::self >= real_t())
		.def(py::self <= real_t())
		.def(-py::self)
		.def("get_tuple", [](const Vector2 &v) { return std::make_tuple(v.x, v.y); })
		.def_static("from_tuple", [](const std::vector<real_t> &args) { return Vector2(args[0], args[1]); })
		.def_property_readonly_static("ZERO", [](const py::object&) { return Vector2(); })
		.def_property_readonly_static("ONE", [](const py::object&) { return Vector2(1,1); })
		.def("__copy__", [](const Vector2 &v){ return Vector2(v); })
		.def("__repr__", [](const Vector2 &v) { return std::str(vformat("Vector2%s", v)); })
		.def("__getitem__", [](const Vector2 &v, int index) { return v[index]; })
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Vector2i>(m_core, "Vector2i")
		.def(py::init<int, int>())
		.def_readwrite("x", &Vector2i::x)
		.def_readwrite("y", &Vector2i::y)
		.def_readwrite("width", &Vector2i::x)
		.def_readwrite("height", &Vector2i::y)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= int())
		.def(py::self - py::self)
		.def(py::self * int())
		.def(py::self / int())
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def(-py::self)
		.def("get_tuple", [](const Vector2i &v) { return std::make_tuple(v.x, v.y); })
		.def_static("from_tuple", [](const std::vector<real_t> &args) { return Vector2i(args[0], args[1]); })
		.def_property_readonly_static("ZERO", [](const py::object&) { return Vector2i(); })
		.def_property_readonly_static("ONE", [](const py::object&) { return Vector2i(1,1); })
		.def("__copy__", [](const Vector2i &v){ return Vector2i(v); })
		.def("__repr__", [](const Vector2i &v) { return std::str(vformat("Vector2i(%d, %d)", v.x, v.y)); })
		.def("__getitem__", [](const Vector2i &v, int index) { return v[index]; })
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Vector3>(m_core, "Vector3")
		.def(py::init<real_t, real_t, real_t>())
		.def_readwrite("x", &Vector3::x)
		.def_readwrite("y", &Vector3::y)
		.def_readwrite("z", &Vector3::z)
		.def("length", &Vector3::length)
		.def("length_squared", &Vector3::length_squared)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= real_t())
		.def(py::self - py::self)
		.def(py::self * real_t())
		.def(py::self / real_t())
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def(-py::self)
		.def("get_tuple", [](const Vector3 &v) { return std::make_tuple(v.x, v.y, v.z);})
		.def("__copy__", [](const Vector3 &v){ return Vector3(v); })
		.def("__repr__", [](const Vector3 &v) { return std::str(vformat("Vector3%s", v));})
		.def("__getitem__", [](const Vector3 &v, int index) { return v[index]; })
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Rect2>(m_core, "Rect2")
		.def(py::init<real_t, real_t, real_t, real_t>())
		.def(py::init<const Point2&, const Size2&>())
		.def("get_position", &Rect2::get_position)
		.def("set_position", &Rect2::set_position)
		.def_property("pos", &Rect2::get_position, &Rect2::set_position)
		.def("get_size", &Rect2::get_size)
		.def("set_size", &Rect2::set_size)
		.def_property("size", &Rect2::get_size, &Rect2::set_size)
		.def_property_readonly("left", [](const Rect2 &rc) { return rc.position.x; })
		.def_property_readonly("top", [](const Rect2 &rc) { return rc.position.y; })
		.def_property_readonly("right", [](const Rect2 &rc) { return rc.position.x + rc.size.width; })
		.def_property_readonly("bottom", [](const Rect2 &rc) { return rc.position.y + rc.size.height; })
		.def_property_readonly("width", [](const Rect2 &rc) { return rc.size.width; })
		.def_property_readonly("height", [](const Rect2 &rc) { return rc.size.height; })
		.def("get_right", [](const Rect2 &rc) { return rc.position.x + rc.size.width; })
		.def("get_left", [](const Rect2 &rc) { return rc.position.x; })
		.def("get_top", [](const Rect2 &rc) { return rc.position.y; })
		.def("get_bottom", [](const Rect2 &rc) { return rc.position.y + rc.size.height; })
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
		.def("get_tuple", [](const Rect2 &rc) { return std::make_tuple(rc.position.x, rc.position.y, rc.size.width, rc.size.height); })
		.def_static("from_tuple", [](const std::vector<real_t> &args) { return Rect2(args[0], args[1], args[2], args[3]); })
		.def_static("from_pos_size", [](const Vector2 &pos, const Size2 &size) { return Rect2(pos, size); })
		.def("__and__", [](const Rect2 &rc1, const Rect2 &rc2){ return rc1.clip(rc2); })
		.def("__copy__", [](const Rect2 &rc){ return Rect2(rc); })
		.def("__repr__", [](const Rect2 &rc) { return std::str(vformat("Rect2%s", rc)); })
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Rect2i>(m_core, "Rect2i")
		.def(py::init<real_t, real_t, real_t, real_t>())
		.def("get_position", &Rect2i::get_position)
		.def("set_position", &Rect2i::set_position)
		.def_property("pos", &Rect2i::get_position, &Rect2i::set_position)
		.def("get_size", &Rect2i::get_size)
		.def("set_size", &Rect2i::set_size)
		.def_property("size", &Rect2i::get_size, &Rect2i::set_size)
		.def_property_readonly("left", [](const Rect2i &rc) { return rc.position.x; })
		.def_property_readonly("top", [](const Rect2i &rc) { return rc.position.y; })
		.def_property_readonly("right", [](const Rect2i &rc) { return rc.position.x + rc.size.width; })
		.def_property_readonly("bottom", [](const Rect2i &rc) { return rc.position.y + rc.size.height; })
		.def_property_readonly("width", [](const Rect2i &rc) { return rc.size.width; })
		.def_property_readonly("height", [](const Rect2i &rc) { return rc.size.height; })
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
		.def("get_tuple", [](const Rect2i &rc) { return std::make_tuple(rc.position.x, rc.position.y, rc.size.width, rc.size.height); })
		.def_static("from_tuple", [](const std::vector<int> &args) { return Rect2i(args[0], args[1], args[2], args[3]); })
		.def_static("from_pos_size", [](const Vector2i &pos, const Size2i &size) { return Rect2i(pos, size); })
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
		.def_static("html_is_valid", &Color::html_is_valid)
		.def_static("named", &Color::named)
		.def_static("named", [](const std::string &name){ return Color::named(name.c_str()); })
		.def("to_html", &Color::to_html)
		.def("from_hsv", &Color::from_hsv)
		.def_static("from_abgr", &Color::from_abgr)
		.def_static("from_rgbe9995", &Color::from_rgbe9995)
		.def_static("solid", &Color::solid)
		.def(py::self + py::self)
		.def(py::self * py::self)
		.def(py::self < py::self)
		.def(py::self *= real_t())
		.def(py::self /= real_t())
		.def("__copy__", [](const Color &c){ return Color(c); })
		.def("__repr__", [](const Color &c) { return std::str(vformat("Color%s", c)); })
		.def("__getitem__", [](const Color &c, int index) { return c[index]; })
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<GdSurface>(m, "Surface")
		.def(py::init<real_t, real_t>())
		.def(py::init<std::vector<real_t>>())
		.def("get_width", &GdSurface::get_width)
		.def("get_height", &GdSurface::get_height)
		.def("get_size", &GdSurface::get_size)
		.def_property_readonly("width", &GdSurface::get_width)
		.def_property_readonly("height", &GdSurface::get_height)
		.def_property_readonly("size", &GdSurface::get_size)
		.def("fill", &GdSurface::fill)
		.def("blit", overload_cast_<GdSurface&, const std::vector<real_t>&>()(&GdSurface::blit)) // pos
		.def("blit", overload_cast_<GdSurface&, const std::vector<real_t>&, const std::vector<real_t>&>()(&GdSurface::blit)) // pos + rect
		.def("blit", overload_cast_<GdSurface&, const std::vector<real_t>&, const Rect2&>()(&GdSurface::blit)) // pos + dest + rect
		.def("__repr__", [](const GdSurface &s) {
			switch(s.impl->get_surface_type()) {
				case GdSurfaceImpl::DISPLAY_SURFACE: return std::str(vformat("GdSurface 0x%0x {DISPLAY_SURFACE}", int64_t(&s)));
				case GdSurfaceImpl::COLOR_SURFACE: return std::str(vformat("GdSurface 0x%0x {COLOR_SURFACE, %s}", int64_t(&s), *s.get_as_color()->surf_color));
				case GdSurfaceImpl::TEXT_SURFACE: return std::str(vformat("GdSurface 0x%0x {TEXT_SURFACE, '%s'}", int64_t(&s), s.get_as_text()->text));
				case GdSurfaceImpl::TEXTURE_SURFACE: return std::str(vformat("GdSurface 0x%0x {TEXTURE_SURFACE}", int64_t(&s)));
			}
			return std::str(vformat("GdSurface 0x%0x {Unknown type: %d}", int64_t(&s), s.impl->get_surface_type()));
		})
		.attr("__version__") = VERSION_FULL_CONFIG;
	// gdgame.info
	py::module m_info = m.def_submodule("info", "System and build info.");
#ifdef DEBUG_ENABLED
	m_info.attr("DEBUG") = true;
#else
	m_info.attr("DEBUG") = false;
#endif
	m_info.attr("GODOT") = std::string(get_full_version_string().utf8().c_str());
	m_info.attr("VERBOSE") = OS::get_singleton()->is_stdout_verbose();
	m_info.attr("DATA_DIR") = std::string(OS::get_singleton()->get_data_path().utf8().c_str());
	m_info.attr("USER_DATA_DIR") = std::string(OS::get_singleton()->get_user_data_dir().utf8().c_str());
	// gdgame.locals
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
	// gdgame.key
	py::module m_key = m.def_submodule("key", "gdgame module to work with the keyboard.");
	m_key.def("name", [](int key) {
		switch (key) {
			case KEY_SPACE: return "space";
			case KEY_ESCAPE: return "escape";
			case KEY_ENTER: return "return";
			case KEY_KP_ENTER: return "enter";
			case KEY_LEFT: return "left";
			case KEY_RIGHT: return "right";
			case KEY_UP: return "up";
			case KEY_DOWN: return "down";
			case KEY_PAGEUP: return "page up";
			case KEY_PAGEDOWN: return "page down";
			case KEY_A: return "a";
			case KEY_B: return "b";
			case KEY_C: return "c";
			case KEY_D: return "d";
			case KEY_E: return "e";
			case KEY_F: return "f";
			case KEY_G: return "g";
			case KEY_H: return "h";
			case KEY_I: return "i";
			case KEY_J: return "j";
			case KEY_K: return "k";
			case KEY_L: return "l";
			case KEY_M: return "m";
			case KEY_N: return "n";
			case KEY_O: return "o";
			case KEY_P: return "p";
			case KEY_Q: return "w";
			case KEY_R: return "r";
			case KEY_S: return "s";
			case KEY_T: return "t";
			case KEY_Y: return "y";
			case KEY_0: return "0";
			case KEY_1: return "1";
			case KEY_2: return "2";
			case KEY_3: return "3";
			case KEY_4: return "4";
			case KEY_5: return "5";
			case KEY_6: return "6";
			case KEY_7: return "7";
			case KEY_8: return "8";
			case KEY_9: return "9";
			case KEY_F1: return "f1";
			case KEY_F2: return "f2";
			case KEY_F3: return "f3";
			case KEY_F4: return "f4";
			case KEY_F5: return "f5";
			case KEY_F6: return "f6";
			case KEY_F7: return "f7";
			case KEY_F8: return "f8";
			case KEY_F9: return "f9";
			case KEY_F10: return "f10";
			case KEY_F11: return "f11";
			case KEY_F12: return "f12";
		}
		return "unknown";
	});
	// gdgame.time
	py::module m_time = m.def_submodule("time", "gdgame module for monitoring time.");
	m_time.def("get_ticks", []() { return OS::get_singleton()->get_ticks_msec(); });
	// gdgame.event
	py::module m_event = m.def_submodule("event", "gdgame module for interacting with events and queues.");
	py::class_<GdEvent>(m_event, "Event")
		.def_readonly("type", &GdEvent::type)
		.def_readonly("button", &GdEvent::button)
		.def_readonly("pos", &GdEvent::position)
		.def_readonly("key", &GdEvent::key)
		.def_readonly("unicode", &GdEvent::unicode)
		.def("get_pos", [](const GdEvent &e) { return Point2(e.position); })
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
					const Vector2i pos = canvas->get_local_mouse_position();
					return std::make_tuple(pos.x, pos.y);
				} else {
					WARN_PRINT("Not an CanvasItem");
				}
			}
			return std::make_tuple(0, 0);
	});
	m_mouse.def("get_pos", [](const GdSurface &surf) {
		ERR_FAIL_COND_V(surf.get_surface_type() != GdSurfaceImpl::DISPLAY_SURFACE, std::make_tuple(0, 0));
		if (Node2D *canvas = Object::cast_to<Node2D>(surf.get_as_display()->instance)) {
			const Vector2i pos = canvas->get_local_mouse_position();
			return std::make_tuple(pos.x, pos.y);
		} else {
			WARN_PRINT("Not an CanvasItem");
		}
		return std::make_tuple(0, 0);
	});
	m_mouse.def("get_loc", [](const GdSurface &surf) {
		ERR_FAIL_COND_V(surf.get_surface_type() != GdSurfaceImpl::DISPLAY_SURFACE, std::make_tuple(0, 0));
		if (Node2D *canvas = Object::cast_to<Node2D>(surf.get_as_display()->instance)) {
			const Vector2i pos = canvas->get_local_mouse_position().max({0, 0});
			return std::make_tuple(pos.x, pos.y);
		} else {
			WARN_PRINT("Not an CanvasItem");
		}
		return std::make_tuple(0, 0);
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
		.def("stop", &GdSound::stop)
		.def("fadeout", &GdSound::fadeout)
		.attr("__version__") = VERSION_FULL_CONFIG;
	// gdgame.display
	py::module m_display = m.def_submodule("display", "gdgame module to control the display window and screen.");
	m_display.def("set_mode", [](const py::tuple &size) { });
	m_display.def("set_caption", &display::set_caption);
	m_display.def("get_surface", &display::get_surface);
	m_display.def("flip", &display::flip);
	m_display.def("render_stats", &display::render_stats);
	// gdgame.draw
	py::module m_draw = m.def_submodule("draw", "gdgame module for drawing shapes.");
	m_draw.def("rect", overload_cast_<const GdSurface&, const Color&, const Rect2&, int>()(&draw::rect));
	m_draw.def("rect", overload_cast_<const GdSurface&, const std::vector<uint8_t>&, const std::vector<float>&, int>()(&draw::rect));
	m_draw.def("rect", overload_cast_<const GdSurface&, const std::vector<uint8_t>&, const Rect2&, int>()(&draw::rect));
	m_draw.def("rect", overload_cast_<const GdSurface&, int, const Rect2&, int>()(&draw::rect));
	m_draw.def("style_rect", overload_cast_<const GdSurface&, const Color&, const Rect2&, int, int, const Color&, int, const Color&, const Vector2&>()(&draw::style_rect), "surf"_a, "color"_a, "rect"_a, "width"_a = 3, "radius"_a = 4, "bg_color"_a = Color::named("lightyellow"), "shadow_size"_a = 2, "shadow_color"_a = Color::named("black"), "shadow_offset"_a = Vector2(2,2));
	m_draw.def("style_rect", overload_cast_<const GdSurface&, const std::vector<uint8_t>&, const std::vector<float>&, int, int, const std::vector<uint8_t>&, int, const std::vector<uint8_t>&, const std::vector<float>&>()(&draw::style_rect), "surf"_a, "color"_a, "rect"_a, "width"_a = 3, "radius"_a = 4, "bg_color"_a = py::make_tuple(255,255,224), "shadow_size"_a = 2, "shadow_color"_a = py::make_tuple(0,0,0), "shadow_offset"_a = py::make_tuple(2,2));
	// gdgame.image
	py::module m_image = m.def_submodule("image", "gdgame module for image transfer.");
	m_image.def("load", &image::load);
	m_image.def("exists", &image::exists);
	// gdgame.font
	py::module m_font = m.def_submodule("font", "gdgame module for loading and rendering fonts.");
	py::class_<GdFont>(m_font, "Font")
		.def(py::init<const std::string&, int>())
		.def(py::init<const std::string&, int, int>())
		.def(py::init<const std::string&, int, int, const std::vector<uint8_t>&>())
		.def("get_height", &GdFont::get_height)
		.def("size", &GdFont::size)
		.def("render", overload_cast_<const std::string&, bool, int>()(&GdFont::render))
		.def("render", overload_cast_<const std::string&, bool, const std::vector<uint8_t>&>()(&GdFont::render))
		.attr("__version__") = VERSION_FULL_CONFIG;
	m_font.def("init", []() { });
	m_font.def("quit", []() { });
}

// END

// BEGIN Python utilities

#ifdef _HAS_EXCEPTIONS
py::object py_eval(const char *expr, const py::object &o) {
	py::object res = py::none();
	try {
		if (!o.is_none()) {
			auto locals = py::dict("_v"_a = o);
			res = py::eval(expr, py::globals(), locals);
		} else {
			res = py::eval(expr, py::globals());
		}
	} catch (py::error_already_set &e) {
		std::cout << "py_eval error_already_set: " << e.what() << std::endl;
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

py::object py_call(py::object p_obj, String p_func_name, py::args p_args) {
	return p_obj.attr(p_func_name.utf8().get_data())(*p_args);
}

py::object py_call(String p_func_name, py::args p_args, String p_module) {
	std::string module = p_module.utf8().get_data();
	std::string function = p_func_name.utf8().get_data();

	if (module.empty()) {
		module = "__main__";
	}
	return py::module::import(module.c_str()).attr(function.c_str())(*p_args);
}

// END
