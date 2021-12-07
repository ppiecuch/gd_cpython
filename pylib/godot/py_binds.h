#ifndef PY_BINDS_H
#define PY_BINDS_H

#include "py_godot.h"

#include "pybind11/pybind11.h"

#include "core/color.h"
#include "core/image.h"
#include "core/reference.h"
#include "core/version.h"
#include "core/os/os.h"
#include "core/os/file_access.h"
#include "core/os/keyboard.h"
#include "core/io/resource_loader.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/math/rect2.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/sprite.h"
#include "scene/resources/texture.h"
#include "scene/resources/font.h"
#include "scene/resources/dynamic_font.h"
#include "common/gd_core.h"

#include <iostream>
#include <string>
#include <memory>
#include <tuple>

namespace py = pybind11;

namespace {
	static unsigned char _dot_cursor_png[] = {
		0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
		0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03,
		0x08, 0x06, 0x00, 0x00, 0x00, 0x56, 0x28, 0xb5, 0xbf, 0x00, 0x00, 0x00,
		0x20, 0x49, 0x44, 0x41, 0x54, 0x08, 0x99, 0x63, 0xf8, 0xff, 0xff, 0xff,
		0x7f, 0x06, 0x06, 0x30, 0xf5, 0x9f, 0x11, 0xc4, 0x60, 0x80, 0x02, 0x26,
		0x88, 0x04, 0x03, 0xc3, 0xff, 0xff, 0xff, 0x19, 0x00, 0x62, 0x07, 0x0f,
		0xf4, 0x27, 0xd4, 0xcf, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
		0x44, 0xae, 0x42, 0x60, 0x82
	};
	static unsigned int _dot_cursor_png_len = 89;
	Ref<Texture> _get_dot_cursor() {
		static Ref<Texture> _dot_cursor_texture;
		if (_dot_cursor_texture.is_null()) {
			Ref<Image> image = memnew(Image(_dot_cursor_png));
			Ref<ImageTexture> texture = memnew(ImageTexture);
			texture->create_from_image(image);
			_dot_cursor_texture = texture;
			_register_global_ref(_dot_cursor_texture); // remove at exit
		}
		return _dot_cursor_texture;
	}
}

namespace std {
	template<typename FromType> std::string str(const FromType& e) { return std::string(String(e).utf8().get_data()); }
}

static _FORCE_INLINE_ Color vec_to_color(const std::vector<uint8_t> &vec) {
	switch(vec.size()) {
		case 1: return Color::solid(vec[0] / 255.0);
		case 3: return Color(vec[0] / 255.0, vec[1] / 255.0, vec[2] / 255.0);
		case 4: return Color(vec[0] / 255.0, vec[1] / 255.0, vec[2] / 255.0, vec[3] / 255.0);
	}
	return Color();
}

static _FORCE_INLINE_ Color vec_to_color(const std::vector<float> &vec) {
	switch(vec.size()) {
		case 1: return Color::solid(vec[0]);
		case 3: return Color(vec[0], vec[1], vec[2]);
		case 4: return Color(vec[0], vec[1], vec[2], vec[3]);
	}
	return Color();
}

// Deferred rendering commands
struct RenderLaterCmd {
	enum RenderLaterCmdType {
		BLIT_AT_POINT,
		BLIT_TO_RECT,
		BLIT_AREA_AT_POINT,
		BLIT_AREA_TO_RECT,
		BLIT_COLOR_RECT,
		BLIT_TEXT
	};
	RenderLaterCmdType cmd;
	Ref<Texture> texture;
	Ref<Font> font;
	union {
		Point2 pt_dest;
		Rect2 rc_dest;
	};
	String text;
	Color color;
	Rect2 area;

	RenderLaterCmd(const Ref<Texture> &t, const Point2 &p) : cmd(BLIT_AT_POINT), texture(t), pt_dest(p) { }
	RenderLaterCmd(const Ref<Texture> &t, const Rect2 &r) : cmd(BLIT_TO_RECT), texture(t), rc_dest(r) { }
	RenderLaterCmd(const Ref<Texture> &t, const Point2 &p, const Rect2 &a) : cmd(BLIT_AREA_AT_POINT), texture(t), pt_dest(p), area(a) { }
	RenderLaterCmd(const Ref<Texture> &t, const Rect2 &r, const Rect2 &a) : cmd(BLIT_AREA_TO_RECT), texture(t), rc_dest(r), area(a) { }
	RenderLaterCmd(const Rect2 &r, const Color &c) : cmd(BLIT_COLOR_RECT), rc_dest(r), color(c) { }
	RenderLaterCmd(const Ref<Font> &f, const String &t, const Point2 &p, const Color &c) : cmd(BLIT_TEXT), font(f), pt_dest(p), text(t), color(c) { }
};

// Available Surface types
struct GdSurfaceImpl {
	enum Type {
		TEXTURE_SURFACE,
		TEXT_SURFACE,
		COLOR_SURFACE,
		DISPLAY_SURFACE
	};

	virtual Type get_surface_type() const = 0;

	virtual int get_width() const = 0;
	virtual int get_height() const = 0;

	virtual std::unique_ptr<GdSurfaceImpl> clone() const = 0;

	virtual ~GdSurfaceImpl() {}
};

struct GdColorSurface : public GdSurfaceImpl {
	int surf_width, surf_height;
	std::unique_ptr<Color> surf_color; // no drawing if null

	_FORCE_INLINE_ Type get_surface_type() const { return COLOR_SURFACE; }
	_FORCE_INLINE_ int get_width() const { return surf_width; }
	_FORCE_INLINE_ int get_height() const { return surf_height; }

	_FORCE_INLINE_ std::unique_ptr<GdSurfaceImpl> clone() const { return std::make_unique<GdColorSurface>(surf_width, surf_height, *surf_color); }

	GdColorSurface(int width, int height) : surf_width(width), surf_height(height) { }
	GdColorSurface(int width, int height, const Color &color) : surf_width(width), surf_height(height), surf_color(std::make_unique<Color>(color)) { }
};

struct GdTextSurface : public GdSurfaceImpl {
	Ref<Font> font;
	String text;
	Color color;

	Size2 text_size = Size2(1, 1);

	_FORCE_INLINE_ Type get_surface_type() const { return TEXT_SURFACE; }
	_FORCE_INLINE_ int get_width() const { ERR_FAIL_NULL_V(font, 1); return text_size.width; }
	_FORCE_INLINE_ int get_height() const { ERR_FAIL_NULL_V(font, 1); return text_size.height; }

	_FORCE_INLINE_ std::unique_ptr<GdSurfaceImpl> clone() const { return std::make_unique<GdTextSurface>(font, text, color); }

	GdTextSurface(const Ref<Font> &font, const String &text, const Color &color) : font(font), text(text), color(color) {
		if (font) {
			text_size = font->get_string_size(text);
		}
	}
};

struct GdTextureSurface : public GdSurfaceImpl {
	Ref<Texture> texture;

	_FORCE_INLINE_ Type get_surface_type() const { return TEXTURE_SURFACE; }
	_FORCE_INLINE_ int get_width() const { ERR_FAIL_NULL_V(texture, 1); return texture->get_width(); }
	_FORCE_INLINE_ int get_height() const { ERR_FAIL_NULL_V(texture, 1); return texture->get_height(); }

	_FORCE_INLINE_ std::unique_ptr<GdSurfaceImpl> clone() const { return std::make_unique<GdTextureSurface>(texture); }

	GdTextureSurface(const Ref<Texture> &texture) : texture(texture) { }
	GdTextureSurface(const String &filename) {
		texture = ResourceLoader::load(filename, "Texture");
	}
};

struct GdDisplaySurface : public GdSurfaceImpl {
	Object *instance = nullptr;

	_FORCE_INLINE_ Type get_surface_type() const { return DISPLAY_SURFACE; }

	_FORCE_INLINE_ int get_width() const {
		ERR_FAIL_NULL_V(instance, 1);
		Size2 size = instance->call("get_view_size");
		return MAX(size.width, 1);
	}

	_FORCE_INLINE_ int get_height() const {
		ERR_FAIL_NULL_V(instance, 1);
		Size2 size = instance->call("get_view_size");
		return MAX(size.height, 1);
	}

	_FORCE_INLINE_ std::unique_ptr<GdSurfaceImpl> clone() const { return std::make_unique<GdDisplaySurface>(instance); }

	GdDisplaySurface(Object *instance) : instance(instance) { }
	GdDisplaySurface(int instance_id) : instance(ObjectDB::get_instance(instance_id)) { }

	_FORCE_INLINE_ void blit_texture(const Ref<Texture> &source, const Point2 &dest) {
		ERR_FAIL_NULL(instance);
		if (source.is_valid()) {
			if (CanvasItem *canvas = Object::cast_to<CanvasItem>(instance)) {
				canvas->draw_texture(source, dest);
			} else {
				WARN_PRINT("Not an CanvasItem");
			}
		}
	}

	_FORCE_INLINE_ void blit_texture(const Ref<Texture> &source, const Rect2 &area) {
		ERR_FAIL_NULL(instance);
		if (source.is_valid()) {
			if (CanvasItem *canvas = Object::cast_to<CanvasItem>(instance)) {
				canvas->draw_texture_rect(source, area);
			} else {
				WARN_PRINT("Not an CanvasItem");
			}
		}
	}

	_FORCE_INLINE_ void blit_texture(const Ref<Texture> &source, const Point2 &dest, const Rect2 &area) {
		return blit_texture(source, Rect2(dest, area.size), area);
	}

	_FORCE_INLINE_ void blit_texture(const Ref<Texture> &source, const Rect2 &dest, const Rect2 &area) {
		ERR_FAIL_NULL(instance);
		if (source.is_valid()) {
			if (CanvasItem *canvas = Object::cast_to<CanvasItem>(instance)) {
				canvas->draw_texture_rect_region(source, dest, area);
			} else {
				WARN_PRINT("Not an CanvasItem");
			}
		}
	}

	_FORCE_INLINE_ void render_text(const Ref<Font> &font, const String &text, const Point2 &dest, const Color &color = Color(1, 1, 1, 1)) {
		ERR_FAIL_NULL(instance);
		if (font.is_valid()) {
			if (CanvasItem *canvas = Object::cast_to<CanvasItem>(instance)) {
				canvas->draw_string(font, dest + Vector2(0, font->get_ascent()), text, color);
			} else {
				WARN_PRINT("Not an CanvasItem");
			}
		}
	}

	_FORCE_INLINE_ void render_rect(const Rect2 &dest, const Color &color = Color(1, 1, 1, 1)) {
		ERR_FAIL_NULL(instance);
		if (CanvasItem *canvas = Object::cast_to<CanvasItem>(instance)) {
			canvas->draw_rect(dest, color);
		} else {
			WARN_PRINT("Not an CanvasItem");
		}
	}

	_FORCE_INLINE_ void render_queue(const std::vector<RenderLaterCmd> &queue) {
		for (const auto &c : queue) {
			switch(c.cmd) {
				case RenderLaterCmd::BLIT_AT_POINT: blit_texture(c.texture, c.pt_dest); break;
				case RenderLaterCmd::BLIT_TO_RECT: blit_texture(c.texture, c.rc_dest); break;
				case RenderLaterCmd::BLIT_AREA_AT_POINT: blit_texture(c.texture, c.pt_dest, c.area); break;
				case RenderLaterCmd::BLIT_AREA_TO_RECT: blit_texture(c.texture, c.rc_dest, c.area); break;
				case RenderLaterCmd::BLIT_COLOR_RECT: render_rect(c.rc_dest, c.color); break;
				case RenderLaterCmd::BLIT_TEXT: render_text(c.font, c.text, c.pt_dest, c.color); break;
			}
		}
	}
};

// Wrapper around Image/Texture/Sprite object
struct GdSurface {
	std::unique_ptr<GdSurfaceImpl> impl;
	std::vector<RenderLaterCmd> _render_later;

	_FORCE_INLINE_ GdSurfaceImpl::Type get_surface_type() const { return impl->get_surface_type(); }
	_FORCE_INLINE_ GdDisplaySurface *get_as_display() const { return (GdDisplaySurface*)impl.get(); }
	_FORCE_INLINE_ GdColorSurface *get_as_color() const { return (GdColorSurface*)impl.get(); }
	_FORCE_INLINE_ GdTextureSurface *get_as_texture() const { return (GdTextureSurface*)impl.get(); }
	_FORCE_INLINE_ GdTextSurface *get_as_text() const { return (GdTextSurface*)impl.get(); }

	GdSurface(const GdSurface &surf) :  impl(surf.impl->clone()), _render_later(surf._render_later) { }
	GdSurface(int surf_width, int surf_height) : impl(std::make_unique<GdColorSurface>(surf_width, surf_height)) { }
	GdSurface(const std::vector<real_t> surf_size) : impl(std::make_unique<GdColorSurface>(surf_size[0], surf_size[1])) { }
	GdSurface(const Ref<Font> &font, const String &text, const Color &color) : impl(std::make_unique<GdTextSurface>(font, text, color)) { }
	GdSurface(const String &filename) : impl(std::make_unique<GdTextureSurface>(filename)) { }
	GdSurface(int &instance_id) : impl(std::make_unique<GdDisplaySurface>(instance_id)) { }

	_FORCE_INLINE_ int get_width() const { ERR_FAIL_NULL_V(impl, 1); return impl->get_width(); }
	_FORCE_INLINE_ int get_height() const { ERR_FAIL_NULL_V(impl, 1); return impl->get_height(); }
	_FORCE_INLINE_ Size2 get_size() const { return Size2(get_width(), get_height()); }

	void fill(const std::vector<uint8_t> &color) {
		if (impl->get_surface_type() == GdSurfaceImpl::COLOR_SURFACE) {
			get_as_color()->surf_color = std::make_unique<Color>(vec_to_color(color));
		}
	}

	_FORCE_INLINE_ void blit(GdSurface &source, const std::vector<real_t> &dest, const std::vector<real_t> &area) {
		ERR_FAIL_COND(area.size() != 0 && area.size() != 4);
		switch (area.size()) {
			case 0: blit(source, dest); return;
			case 4: blit(source, dest, Rect2(area[0], area[1], area[2], area[3])); return;
		}
	}

	_FORCE_INLINE_ void blit(GdSurface &source, const std::vector<real_t> &dest) {
		ERR_FAIL_NULL(impl);
		ERR_FAIL_NULL(source.impl);
		ERR_FAIL_COND(dest.size() != 2 && dest.size() != 4);
		switch(impl->get_surface_type()) {
			case GdSurfaceImpl::TEXTURE_SURFACE: {
				switch(source.impl->get_surface_type()) {
					case GdSurfaceImpl::DISPLAY_SURFACE: {
						WARN_PRINT("Not supported");
					} break;
					case GdSurfaceImpl::COLOR_SURFACE: {
						GdColorSurface *surf = source.get_as_color();
						if (surf->surf_color) {
							switch (dest.size()) {
								case 2: _render_later.push_back(RenderLaterCmd(
									Rect2(Point2(dest[0], dest[1]), Size2(surf->get_width(), surf->get_height())), *surf->surf_color)); break;
								case 4: _render_later.push_back(RenderLaterCmd(Rect2(dest[0], dest[1], dest[2], dest[3]), *surf->surf_color)); break;
							}
						}
					} break;
					case GdSurfaceImpl::TEXT_SURFACE: {
						GdTextSurface *surf = source.get_as_text();
						_render_later.push_back(RenderLaterCmd(surf->font, surf->text, Point2(dest[0], dest[1]), surf->color));
					}
					case GdSurfaceImpl::TEXTURE_SURFACE: {
						GdTextureSurface *surf = source.get_as_texture();
						switch (dest.size()) {
							case 2: _render_later.push_back(RenderLaterCmd(surf->texture, Point2(dest[0], dest[1]))); break;
							case 4: _render_later.push_back(RenderLaterCmd(surf->texture, Rect2(dest[0], dest[1], dest[2], dest[3]))); break;
						}
					} break;
				}
			} break;
			case GdSurfaceImpl::COLOR_SURFACE: {
				switch(source.impl->get_surface_type()) {
					case GdSurfaceImpl::TEXTURE_SURFACE: {
						GdTextureSurface *surf = source.get_as_texture();
						switch (dest.size()) {
							case 2: _render_later.push_back(RenderLaterCmd(surf->texture, Point2(dest[0], dest[1]))); break;
							case 4: _render_later.push_back(RenderLaterCmd(surf->texture, Rect2(dest[0], dest[1], dest[2], dest[3]))); break;
						}
					} break;
					default: {
						WARN_PRINT("Not supported");
					} break;
				}
			} break;
			case GdSurfaceImpl::TEXT_SURFACE: {
			} break;
			case GdSurfaceImpl::DISPLAY_SURFACE: {
				GdDisplaySurface *disp = get_as_display();
				switch(source.impl->get_surface_type()) {
					case GdSurfaceImpl::DISPLAY_SURFACE: {
					} break;
					case GdSurfaceImpl::COLOR_SURFACE: {
						if (dest.size() == 4) {
							GdColorSurface *surf = source.get_as_color();
							if (surf->surf_color) {
								disp->render_rect(Rect2(dest[0], dest[1], dest[2], dest[3]), *surf->surf_color);
							}
						}
					} break;
					case GdSurfaceImpl::TEXT_SURFACE: {
						GdTextSurface *surf = source.get_as_text();
						disp->render_text(surf->font, surf->text, Point2(dest[0], dest[1]), surf->color);
					} break;
					case GdSurfaceImpl::TEXTURE_SURFACE: {
						GdTextureSurface *surf = source.get_as_texture();
						switch (dest.size()) {
							case 2: disp->blit_texture(surf->texture, Point2(dest[0], dest[1])); break;
							case 4: disp->blit_texture(surf->texture, Rect2(dest[0], dest[1], dest[2], dest[3])); break;
						}
					} break;
				}
				if (source._render_later.size()) {
					disp->render_queue(source._render_later);
				}
			} break;
		}
	}

	_FORCE_INLINE_ void blit(GdSurface &source, const std::vector<real_t> &dest, const Rect2 &area) {
		ERR_FAIL_NULL(impl);
		ERR_FAIL_NULL(source.impl);
		ERR_FAIL_COND(dest.size() != 2 && dest.size() != 4);
		switch(impl->get_surface_type()) {
			case GdSurfaceImpl::TEXTURE_SURFACE: {
				switch(source.impl->get_surface_type()) {
					case GdSurfaceImpl::TEXTURE_SURFACE: {
						GdTextureSurface *surf = source.get_as_texture();
						switch (dest.size()) {
							case 2: _render_later.push_back(RenderLaterCmd(surf->texture, Point2(dest[0], dest[1]), area)); break;
							case 4: _render_later.push_back(RenderLaterCmd(surf->texture, Rect2(dest[0], dest[1], dest[2], dest[3]), area)); break;
						}
					} break;
					case GdSurfaceImpl::COLOR_SURFACE: {
						GdColorSurface *surf = source.get_as_color();
						if (surf->surf_color) {
							switch (dest.size()) {
								case 2: _render_later.push_back(RenderLaterCmd(area, *surf->surf_color)); break;
								case 4: _render_later.push_back(RenderLaterCmd(area, *surf->surf_color)); break;
							}
						}
					} break;
					default: {
						WARN_PRINT("Not supported");
					} break;
				}
			} break;
			case GdSurfaceImpl::COLOR_SURFACE: {
				switch(source.impl->get_surface_type()) {
					case GdSurfaceImpl::TEXTURE_SURFACE: {
						GdTextureSurface *surf = source.get_as_texture();
						switch (dest.size()) {
							case 2: _render_later.push_back(RenderLaterCmd(surf->texture, Point2(dest[0], dest[1]), area)); break;
							case 4: _render_later.push_back(RenderLaterCmd(surf->texture, Rect2(dest[0], dest[1], dest[2], dest[3]), area)); break;
						}
					} break;
					default: {
						WARN_PRINT("Not supported");
					} break;
				}
			} break;
			case GdSurfaceImpl::TEXT_SURFACE: {
			} break;
			case GdSurfaceImpl::DISPLAY_SURFACE: {
				GdDisplaySurface *disp = get_as_display();
				switch(source.impl->get_surface_type()) {
					case GdSurfaceImpl::DISPLAY_SURFACE: {
						WARN_PRINT("Not supported");
					} break;
					case GdSurfaceImpl::COLOR_SURFACE: {
						WARN_PRINT("Not supported");
					} break;
					case GdSurfaceImpl::TEXT_SURFACE: {
						WARN_PRINT("Not supported");
					} break;
					case GdSurfaceImpl::TEXTURE_SURFACE: {
						GdTextureSurface *surf = source.get_as_texture();
						switch (dest.size()) {
							case 2: disp->blit_texture(surf->texture, Point2(dest[0], dest[1]), area); break;
							case 4: disp->blit_texture(surf->texture, Rect2(dest[0], dest[1], dest[2], dest[3]), area); break;
						}
					} break;
				}
				if (source._render_later.size()) {
					disp->render_queue(source._render_later);
				}
			} break;
		}
	}
};

// Wrapper around Bitmap or Dynamic font
struct GdFont {
	Ref<Font> font;

	GdFont(const std::string &path, int size) {
		load(path, size, 0);
	}

	GdFont(const std::string &path, int size, int stretch) {
		load(path, size, stretch);
	}

	void load(const std::string &path, int size, int stretch);

	_FORCE_INLINE_ GdSurface render(const std::string &text, bool alias, int color) {
		return GdSurface(font, String(text.c_str()), Color::hex(color));
	}

	_FORCE_INLINE_ GdSurface render(const std::string &text, bool alias, const std::vector<uint8_t> &color) {
		return GdSurface(font, String(text.c_str()), vec_to_color(color));
	}

	_FORCE_INLINE_ Size2 size(const std::string &text) const { return font->get_string_size(String(text.c_str())); }
	_FORCE_INLINE_ int get_height() const { return font->get_height(); }
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
	union {
		struct {
			Point2 position;
			int button;
		};
		struct {
			int key;
			int unicode;
		};
	};
};

// Wrapper around Godot sound
struct GdSound {
	GdSound(const std::string &filename) { }
	void set_volume(real_t vol) { }
	void play(bool loop) { }
	void fadeout(int fadetime) { }
	void stop() { }
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

	py::str unget_text(const py::str &s1, const py::str &s2, int n) {
		return n == 1 ? s1 : s2;
	}
} // utils

namespace event {
	void set_grab(bool grab) {
	}
} // event

namespace image {
	_FORCE_INLINE_ GdSurface load(const std::string &filename) {
		return GdSurface(String(filename.c_str()));
	}
} // image

namespace draw {
	_FORCE_INLINE_ void rect(const GdSurface &surf, const Color &c, const Rect2 &geom, int width) {
		ERR_FAIL_COND(surf.get_surface_type() != GdSurfaceImpl::DISPLAY_SURFACE);
		if (Node2D *canvas = Object::cast_to<Node2D>(surf.get_as_display()->instance)) {
			canvas->draw_rect(geom, c, false, width);
		} else {
			WARN_PRINT("Not an Node2D");
		}
	}
	_FORCE_INLINE_ void rect(const GdSurface &surf, const std::vector<uint8_t> &c, const Rect2 &geom, int width) {
		rect(surf, vec_to_color(c), geom, width);
	}
	_FORCE_INLINE_ void rect(const GdSurface &surf, const std::vector<uint8_t> &c, const std::vector<float> &geom, int width) {
		rect(surf, vec_to_color(c), Rect2(geom[0], geom[1], geom[2], geom[3]), width);
	}
	_FORCE_INLINE_ void rect(const GdSurface &surf, int c, const Rect2 &geom, int width) {
		rect(surf, Color::hex(c), geom, width);
	}
} // draw

namespace display {
	void set_caption(const std::string &caption) { }
	_FORCE_INLINE_ GdSurface get_surface(int instance_id) { return GdSurface(instance_id); }
	_FORCE_INLINE_ void flip(int instance_id) {
		if (Object *parent = ObjectDB::get_instance(instance_id)) {
			if (Node2D *canvas = Object::cast_to<Node2D>(parent)) {
			} else {
				WARN_PRINT("Not an Node2D");
			}
		}
	}
} // display

#endif // PY_BINDS_H
