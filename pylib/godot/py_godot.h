#pragma once

#include "pybind11/pybind11.h"
#include "pybind11/embed.h"
#include "pybind11/operators.h"

#include "core/color.h"
#include "core/image.h"
#include "core/reference.h"
#include "core/version.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/math/rect2.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/sprite.h"
#include "scene/resources/texture.h"

namespace py = pybind11;

struct ImageRef : public Ref<Image> {
	Image *get() {
		if (!ptr()) {
			instance();
		}
		return ptr();
	}
};

struct ImageTextureRef : public Ref<ImageTexture> {
	Texture *get() {
		if (!ptr()) {
			instance();
		}
		return ptr();
	}
};

PYBIND11_EMBEDDED_MODULE(gdgame, m) {
    m.doc() = "Python2 Godot bindings";
	py::class_<Math>(m, "MathFuncs")
		.def_static("sin", static_cast<real_t (*)(real_t)>(&Math::sin))
		.def_static("cos", static_cast<real_t (*)(real_t)>(&Math::cos))
		.def_static("tan", static_cast<real_t (*)(real_t)>(&Math::tan))
		.def_static("sinh", static_cast<real_t (*)(real_t)>(&Math::sinh))
		.def_static("sinc", static_cast<real_t (*)(real_t)>(&Math::sinc))
		.def_static("sincn", static_cast<real_t (*)(real_t)>(&Math::sincn))
		.def_static("cosh", static_cast<real_t (*)(real_t)>(&Math::cosh))
		.def_static("tanh", static_cast<real_t (*)(real_t)>(&Math::tanh))
		.def_static("asin", static_cast<real_t (*)(real_t)>(&Math::asin))
		.def_static("acos", static_cast<real_t (*)(real_t)>(&Math::acos))
		.def_static("atan", static_cast<real_t (*)(real_t)>(&Math::atan))
		.def_static("atan2", static_cast<real_t (*)(real_t, real_t)>(&Math::atan2))
		.def_static("sqrt", static_cast<real_t (*)(real_t)>(&Math::sqrt))
		.def_static("fmod", static_cast<real_t (*)(real_t, real_t)>(&Math::fmod))
		.def_static("floor", static_cast<real_t (*)(real_t)>(&Math::floor))
		.def_static("ceil", static_cast<real_t (*)(real_t)>(&Math::ceil))
		.def_static("pow", static_cast<real_t (*)(real_t, real_t)>(&Math::pow))
		.def_static("log", static_cast<real_t (*)(real_t)>(&Math::log))
		.def_static("log10", static_cast<real_t (*)(real_t)>(&Math::log10))
		.def_static("exp", static_cast<real_t (*)(real_t)>(&Math::exp))
		.def_static("is_nan", static_cast<bool (*)(real_t)>(&Math::is_nan))
		.def_static("is_inf", static_cast<bool (*)(real_t)>(&Math::is_inf))
		.def_static("abs", static_cast<real_t (*)(real_t)>(&Math::abs))
		.def_static("abs", static_cast<int (*)(int)>(&Math::abs))
		.def_static("fposmod", static_cast<real_t (*)(real_t, real_t)>(&Math::fposmod))
		.def_static("posmod", static_cast<int64_t (*)(int64_t, int64_t)>(&Math::posmod))
		.def_static("deg2rad", static_cast<real_t (*)(real_t)>(&Math::deg2rad))
		.def_static("rad2deg", static_cast<real_t (*)(real_t)>(&Math::rad2deg))
		.def_static("lerp", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::lerp))
		.def_static("lerp_angle", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::lerp_angle))
		.def_static("inverse_lerp", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::lerp_angle))
		.def_static("range_lerp", static_cast<real_t (*)(real_t, real_t, real_t, real_t, real_t)>(&Math::range_lerp))
		.def_static("smoothstep", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::smoothstep))
		.def_static("move_toward", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::move_toward))
		.def_static("linear2db", static_cast<real_t (*)(real_t)>(&Math::linear2db))
		.def_static("db2linear", static_cast<real_t (*)(real_t)>(&Math::linear2db))
		.def_static("round", static_cast<real_t (*)(real_t)>(&Math::round))
		.def_static("wrapi", &Math::wrapi)
		.def_static("wrapf", static_cast<real_t (*)(real_t, real_t, real_t)>(&Math::wrapf))
		.def_static("ease", &Math::ease)
		.def_static("step_decimals", &Math::step_decimals)
		.def_static("range_step_decimals", &Math::range_step_decimals)
		.def_static("stepify", &Math::stepify)
		.def_static("dectime", &Math::dectime)
		.def_static("larger_prime", &Math::larger_prime)
		.def_static("seed", &Math::seed)
		.def_static("randomize", &Math::seed)
		.def_static("rand_from_seed", &Math::rand_from_seed)
		.def_static("rand", &Math::rand)
		.def_static("randd", &Math::randd)
		.def_static("randf", &Math::randf)
		.def_static("random", static_cast<real_t (*)(real_t, real_t)>(&Math::random))
		.def_static("random", static_cast<real_t (*)(int, int)>(&Math::random))
		.def_static("is_equal_approx_ratio", static_cast<bool (*)(real_t, real_t, real_t, real_t)>(&Math::is_equal_approx_ratio))
		.def_static("is_equal_approx", static_cast<bool (*)(real_t, real_t)>(&Math::is_equal_approx))
		.def_static("is_equal_approx", static_cast<bool (*)(real_t, real_t, real_t)>(&Math::is_equal_approx))
		.def_static("is_zero_approx", static_cast<bool (*)(real_t)>(&Math::is_zero_approx))
		.def_static("absf", &Math::absf)
		.def_static("absd", &Math::absd)
		.def_static("fast_ftoi", &Math::fast_ftoi)
		.def_static("halfbits_to_floatbits", &Math::halfbits_to_floatbits)
		.def_static("halfptr_to_float", &Math::halfptr_to_float)
		.def_static("half_to_float", &Math::half_to_float)
		.def_static("make_half_float", &Math::make_half_float)
		.def_static("snap_scalar", &Math::snap_scalar)
		.def_static("snap_scalar_separation", &Math::snap_scalar_separation)
		.def_static("map", &Math::map)
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Vector2>(m, "Vector2")
		.def(py::init<real_t, real_t>())
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= real_t())
		.def(real_t() * py::self)
		.def(py::self * real_t())
		.def(-py::self)
		.def("__repr__", [](const Vector2 &v) { return String(v);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Vector3>(m, "Vector3")
		.def(py::init<real_t, real_t, real_t>())
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= real_t())
		.def(real_t() * py::self)
		.def(py::self * real_t())
		.def(-py::self)
		.def("__repr__", [](const Vector3 &v) { return String(v);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Rect2>(m, "Rect2")
		.def(py::init<real_t, real_t, real_t, real_t>())
		.def("get_position", &Rect2::get_position)
		.def("set_position", &Rect2::set_position)
		.def("get_size", &Rect2::get_size)
		.def("set_size", &Rect2::set_size)
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
		.def("__repr__", [](const Rect2 &v) { return String(v);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Rect2i>(m, "Rect2i")
		.def(py::init<real_t, real_t, real_t, real_t>())
		.def("get_position", &Rect2i::get_position)
		.def("set_position", &Rect2i::set_position)
		.def("get_size", &Rect2i::get_size)
		.def("set_size", &Rect2i::set_size)
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
		.def("__repr__", [](const Rect2i &v) { return String(v);})
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Color>(m, "Color")
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
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Node2D>(m, "Node2d")
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Image>(m, "Image")
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<ImageRef>(m, "ImageRef")
		.def("get", &ImageRef::get)
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Texture>(m, "Texture")
		.def("get_width", &Texture::get_width)
		.def("get_height", &Texture::get_height)
		.def("get_size", &Texture::get_size)
		.def("get_rid", &Texture::get_rid)
		.def("is_pixel_opaque", &Texture::is_pixel_opaque)
		.def("has_alpha", &Texture::has_alpha)
		.def("set_flags", &Texture::set_flags)
		.def("get_flags", &Texture::get_flags)
		.def("draw", &Texture::draw)
		.def("draw_rect", &Texture::draw_rect)
		.def("draw_rect_region", &Texture::draw_rect_region)
		.def("get_rect_region", &Texture::get_rect_region)
		.def("get_data", &Texture::get_data)
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<ImageTexture>(m, "ImageTexture")
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::enum_<Texture::Flags>(m, "TextureFlags")
		.value("TEXTURE_MIPMAPS", Texture::FLAG_MIPMAPS)
		.value("TEXTURE_REPEAT", Texture::FLAG_REPEAT)
		.value("TEXTURE_FILTER", Texture::FLAG_FILTER)
		.value("TEXTURE_ANISOTROPIC_FILTER", Texture::FLAG_ANISOTROPIC_FILTER)
		.value("TEXTURE_CONVERT_TO_LINEAR", Texture::FLAG_CONVERT_TO_LINEAR)
		.value("TEXTURE_VIDEO_SURFACE", Texture::FLAG_VIDEO_SURFACE)
		.value("TEXTURE_MIRRORED_REPEAT", Texture::FLAG_MIRRORED_REPEAT)
		.value("TEXTURE_DEFAULT", Texture::FLAGS_DEFAULT)
		.export_values();
	py::class_<ImageTextureRef>(m, "ImageTextureRef")
		.def("get", &ImageTextureRef::get)
		.attr("__version__") = VERSION_FULL_CONFIG;
	py::class_<Sprite>(m, "Sprite")
		.attr("__version__") = VERSION_FULL_CONFIG;
}
