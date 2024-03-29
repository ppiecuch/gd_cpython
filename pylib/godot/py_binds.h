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
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/resources/texture.h"
#include "scene/resources/font.h"
#include "scene/resources/dynamic_font.h"
#include "scene/resources/theme.h"
#include "common/gd_core.h"

#include <cstdint>
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
	real_t surf_alpha;

	_FORCE_INLINE_ Type get_surface_type() const { return TEXTURE_SURFACE; }
	_FORCE_INLINE_ int get_width() const { ERR_FAIL_NULL_V(texture, 1); return texture->get_width(); }
	_FORCE_INLINE_ int get_height() const { ERR_FAIL_NULL_V(texture, 1); return texture->get_height(); }

	_FORCE_INLINE_ std::unique_ptr<GdSurfaceImpl> clone() const { return std::make_unique<GdTextureSurface>(texture); }

	GdTextureSurface(const Ref<Texture> &texture) : texture(texture), surf_alpha(1) { }
	GdTextureSurface(const String &filename) {
		texture = ResourceLoader::load(filename, "Texture");
		surf_alpha = 1;
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

struct _SurfaceStats {
#ifdef DEBUG_ENABLED
	bool _enabled = false;
	int _create = 0;
	int _destroy = 0;
	uint64_t _frame = 0;
	void enable() { _enabled = true; }
	void disable() { _enabled = false; }
	String _stats() const {
		return vformat("(GdCPython) Surface stats: frame: %d, created: %d, deleted: %d", _frame, _create, _destroy);
	}
	void dump() const {
		if (_enabled) {
			print_verbose(_stats());
		}
	}
	void next_frame() { _create = _destroy = 0; _frame++; }
	void new_surf() { _create++; }
	void del_surf() { _destroy++; }
#else
	void enable() const { }
	void disable() const { }
	String _stats() const { return ""; }
	void dump() const { }
	void next_frame() const { }
	void new_surf() const { }
	void del_surf() const { }
#endif
} _surf_stats;

struct GdSurface {
	std::unique_ptr<GdSurfaceImpl> impl;
	std::vector<RenderLaterCmd> _render_later;

	_FORCE_INLINE_ GdSurfaceImpl::Type get_surface_type() const { return impl->get_surface_type(); }
	_FORCE_INLINE_ GdDisplaySurface *get_as_display() const { return (GdDisplaySurface*)impl.get(); }
	_FORCE_INLINE_ GdColorSurface *get_as_color() const { return (GdColorSurface*)impl.get(); }
	_FORCE_INLINE_ GdTextureSurface *get_as_texture() const { return (GdTextureSurface*)impl.get(); }
	_FORCE_INLINE_ GdTextSurface *get_as_text() const { return (GdTextSurface*)impl.get(); }

	GdSurface(const GdSurface &surf) :  impl(surf.impl->clone()), _render_later(surf._render_later) { _surf_stats.new_surf(); }
	GdSurface(int surf_width, int surf_height) : impl(std::make_unique<GdColorSurface>(surf_width, surf_height)) { _surf_stats.new_surf(); }
	GdSurface(const std::vector<real_t> surf_size) : impl(std::make_unique<GdColorSurface>(surf_size[0], surf_size[1])) { _surf_stats.new_surf(); }
	GdSurface(const Ref<Font> &font, const String &text, const Color &color) : impl(std::make_unique<GdTextSurface>(font, text, color)) { _surf_stats.new_surf(); }
	GdSurface(const String &filename) : impl(std::make_unique<GdTextureSurface>(filename)) { _surf_stats.new_surf(); }
	GdSurface(int &instance_id) : impl(std::make_unique<GdDisplaySurface>(instance_id)) { _surf_stats.new_surf(); }

	~GdSurface() { _surf_stats.del_surf(); }

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

	GdFont(const std::string &path, int size) { load(path, size); }
	GdFont(const std::string &path, int size, int stretch) { load(path, size, 0, Color(), stretch); }
	GdFont(const std::string &path, int size, int outline_size, Color outline_color) { load(path, size, outline_size, outline_color); }
	GdFont(const std::string &path, int size, int outline_size, const std::vector<uint8_t>& outline_color) { load(path, size, outline_size, vec_to_color(outline_color)); }

	bool load(const std::string &path, int size, int outline_size = 0, Color outline_color = Color(), int stretch = 0);

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
			// to avoid gcc problems with Point2 as part of the union
			struct {
				real_t x, y;
				void operator=(const Point2 &pt) { x = pt.x, y = pt.y; }
				operator Point2() const { return Point2(x, y); }
			} position;
			int button;
		};
		struct {
			int key;
			wchar_t unicode;
		};
	};
};

// Wrapper around Godot sound
struct GdSound {
	GdSound(const std::string &filename) { }
	void set_volume(real_t vol);
	void play(bool loop);
	void fadeout(int fadetime);
	void stop();
};

namespace utils {
	void print_dict(py::dict dict) {
		for (auto item : dict)
			std::cout
				<< "key=" << std::string(py::str(item.first)) << ", "
				<< "value=" << std::string(py::str(item.second)) << std::endl;
	}

	void print_verbose(const std::string &msg) {
		print_verbose(String(msg.c_str()));
	}

	py::str get_text(const py::str &s) {
		return s;
	}

	py::str unget_text(const py::str &s1, const py::str &s2, int n) {
		return n == 1 ? s1 : s2;
	}

	real_t lin_ipol(real_t value, real_t a, real_t b, real_t begin = 0, real_t end = 1.0) {
		return Math::map1(value, a, b, begin, end);
	}
	std::tuple<int, int> get_instance_size(int instance_id) {
		if (Object *owner = ObjectDB::get_instance(instance_id)) {
			if (Node2D *canvas = Object::cast_to<Node2D>(owner)) {
				Size2 size = canvas->call("get_view_size");
				return std::make_tuple(size.width, size.height);
			} else {
				WARN_PRINT("Not an CanvasItem");
			}
		}
		return std::make_tuple(0, 0);
	}

	bool quit() {
		if (SceneTree *sc = SceneTree::get_singleton()) {
			sc->quit(0);
			return true;
		}
		return false;
	}
} // utils

namespace event {
	void set_grab(bool grab) {
	}
} // event

namespace image {
	_FORCE_INLINE_ GdSurface load(const std::string &filename) {
		return GdSurface(filename.c_str());
	}
	_FORCE_INLINE_ bool exists(const std::string &filename) {
		return ResourceLoader::exists(filename.c_str());
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
		ERR_FAIL_COND(geom.size()!=4);
		rect(surf, vec_to_color(c), Rect2(geom[0], geom[1], geom[2], geom[3]), width);
	}
	_FORCE_INLINE_ void rect(const GdSurface &surf, int c, const Rect2 &geom, int width) {
		rect(surf, Color::hex(c), geom, width);
	}
	_FORCE_INLINE_ void style_rect(const GdSurface &surf, const Color &color, const Rect2 &rect, int width, int radius, const Color &bg_color, int shadow_size, const Color &shadow_color, const Vector2 &shadow_offset) {
		ERR_FAIL_COND(surf.get_surface_type() != GdSurfaceImpl::DISPLAY_SURFACE);
		if (Node2D *canvas = Object::cast_to<Node2D>(surf.get_as_display()->instance)) {
			static Ref<StyleBoxFlat> _style;
			if (!_style) {
				_style = newref(StyleBoxFlat);
				_register_global_ref(_style);
			}
			_style->set_border_width_all(width);
			_style->set_corner_radius_all(radius);
			_style->set_bg_color(bg_color);
			_style->set_shadow_size(shadow_size);
			_style->set_shadow_color(shadow_color);
			_style->set_shadow_offset(shadow_offset);
			canvas->draw_style_box(_style, rect);
		} else {
			WARN_PRINT("Not an Node2D");
		}
	}
	_FORCE_INLINE_ void style_rect(const GdSurface &surf, const std::vector<uint8_t> &color, const std::vector<float> &rect, int width, int radius, const std::vector<uint8_t> &bg_color, int shadow_size, const std::vector<uint8_t> &shadow_color, const std::vector<float> &shadow_offset) {
		ERR_FAIL_COND(rect.size()!=4);
		ERR_FAIL_COND(shadow_offset.size()!=2);
		style_rect(surf, vec_to_color(color), Rect2(rect[0], rect[1], rect[2], rect[3]), width, radius, vec_to_color(bg_color), shadow_size, vec_to_color(shadow_color), Vector2(shadow_offset[0], shadow_offset[1]));
	}
} // draw

namespace display {
	void set_caption(const std::string &caption) { OS::get_singleton()->set_window_title(caption.c_str()); }
	_FORCE_INLINE_ GdSurface get_surface(int instance_id) { return GdSurface(instance_id); }
	_FORCE_INLINE_ void flip(int instance_id) {
		_surf_stats.dump();
		_surf_stats.next_frame();
	}
	_FORCE_INLINE_ void render_stats(bool state) {
		if (state) {
			_surf_stats.enable();
		} else {
			_surf_stats.disable();
		}
	}
	_FORCE_INLINE_ void mirror(int instance_id) {
		if (Object *owner = ObjectDB::get_instance(instance_id)) {
			if (Node2D *canvas = Object::cast_to<Node2D>(owner)) {
				canvas->set_rotation_degrees(180);
			} else {
				WARN_PRINT("Not an Node2D");
			}
		}
	}
} // display

// https://stackoverflow.com/questions/231767/what-does-the-yield-keyword-do
namespace network {
	// local multiplyer
	std::vector<uint64_t> get_peers();
	std::string get_peer_name(uint64_t peer_id);
	// leaderboards
	bool request_leaderboard(const std::string &ld_name);
	std::vector<Dictionary> get_leaderboard(const std::string &ld_name);
	int get_leaderboard_request_status();
	bool add_score(const std::string &score);
} // network

#endif // PY_BINDS_H
