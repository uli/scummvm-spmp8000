/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS_NULL_H
#define BACKENDS_GRAPHICS_NULL_H

#include "backends/graphics/graphics.h"
#include "graphics/surface.h"
#include <libgame.h>

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
        {"1x", "Standard", 0},
        {0, 0, 0}
};

class Spmp8000GraphicsManager : public GraphicsManager {
public:
	Spmp8000GraphicsManager() {
		gp.width = getWidth();
		gp.height = getHeight();
		gp.pixels = new uint16_t[gp.width * gp.height];
		gp.unknown_flag = 0;
		gp.src_clip_x = 0;
		gp.src_clip_y = 0;
		gp.src_clip_w = gp.width;
		gp.src_clip_h = gp.height;
		emuIfGraphInit(&gp);
		mouse_x = 10;
		mouse_y = 10;
		mouse_visible = true;
		bytes_per_pixel = 1;
	}
	virtual ~Spmp8000GraphicsManager() {
		emuIfGraphCleanup();
		delete[] gp.pixels;
	}

	bool hasFeature(OSystem::Feature f) { return false; }
	void setFeatureState(OSystem::Feature f, bool enable) {}
	bool getFeatureState(OSystem::Feature f) { return false; }

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const { return s_supportedGraphicsModes; }
	int getDefaultGraphicsMode() const { return 0; }
	bool setGraphicsMode(int mode) { return true; }
	void resetGraphicsScale(){}
	int getGraphicsMode() const { return 0; }
	inline Graphics::PixelFormat getScreenFormat() const {
		return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	}
	inline Common::List<Graphics::PixelFormat> getSupportedFormats() const {
		Common::List<Graphics::PixelFormat> list;
		list.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
		list.push_back(Graphics::PixelFormat::createFormatCLUT8());
		return list;
	}
	void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) {}
	virtual int getScreenChangeID() const { return 0; }

	void beginGFXTransaction() {}
	OSystem::TransactionError endGFXTransaction() { return OSystem::kTransactionSuccess; }

	int16 getHeight() { return gDisplayDev->getHeight(); }
	int16 getWidth() { return gDisplayDev->getWidth(); }
	void setPalette(const byte *colors, uint start, uint num) {
		int i;
		for (i = start; i < start + num; i++) {
			palette[i] = MAKE_RGB565(colors[0], colors[1], colors[2]);
			colors += 3;
		}
	}
	void grabPalette(byte *colors, uint start, uint num) {}
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
		if (bytes_per_pixel == 2) {
			int i;
			uint16_t *fb = gp.pixels + y * gp.width + x;
			const uint16_t *b = (const uint16_t *)buf;
			for (i = 0; i < h; i++) {
				memcpy(fb, b, w * 2);
				fb += gp.width;
				b += pitch / 2;
			}
		}
		else {
			int i, j;
			uint16_t *fb = gp.pixels + y * gp.width + x;
			const uint8_t *b = (const uint8_t *)buf;
			for (i = 0; i < h; i++) {
				for (j = 0; j < w; j++) {
					fb[j] = palette[b[j]];
				}
				fb += gp.width;
				b += pitch;
			}
		}
	}
	Graphics::Surface *lockScreen() {
		_framebuffer.pixels = gp.pixels;
		_framebuffer.w = gp.width;
		_framebuffer.h = gp.height;
		_framebuffer.pitch = gp.width * 2;
		_framebuffer.format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
		return &_framebuffer;
	}
	void unlockScreen() {
		emuIfGraphShow();
	}
	void fillScreen(uint32 col) {}
	void updateScreen() {
		fprintf(stderr, "updateScreen\n");
		gp.pixels[mouse_y * gp.width + mouse_x] = 0xffff;
		emuIfGraphShow();
	}
	void setShakePos(int shakeOffset) {}
	void setFocusRectangle(const Common::Rect& rect) {}
	void clearFocusRectangle() {}

	void showOverlay() {}
	void hideOverlay() {}
	Graphics::PixelFormat getOverlayFormat() const { return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0); }
	void clearOverlay() {
		memset(gp.pixels, 0, gp.width * gp.height * 2);
	}
	void grabOverlay(void *buf, int pitch) {
		int i;
		uint16_t *fb = gp.pixels;
		uint16_t *b = (uint16_t *)buf;
		for (i = 0; i < gp.height; i++) {
			memcpy(b, fb, gp.width * 2);
			fb += gp.width;
			b += pitch / 2;
		}
	}
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
		int i;
		uint16_t *fb = gp.pixels + y * gp.width + x;
		const uint16_t *b = (const uint16_t *)buf;
		for (i = 0; i < h; i++) {
			memcpy(fb, b, w * 2);
			fb += gp.width;
			b += pitch / 2;
		}
		gp.pixels[mouse_y * gp.width + mouse_x] = 0xffff;
		emuIfGraphShow();
	}
	int16 getOverlayHeight() { return getHeight(); }
	int16 getOverlayWidth() { return getWidth(); }

	bool showMouse(bool visible) { 
		bool old = mouse_visible;
		fprintf(stderr, "showMouse %d -> %d\n", old, visible);
		mouse_visible = visible;
		return old;
	}
	void warpMouse(int x, int y) {
		fprintf(stderr, "warpMouse %d/%d\n", x, y);
		mouse_x = x;
		mouse_y = y;
		gp.pixels[mouse_y * gp.width + mouse_x] = 0xffff;
		emuIfGraphShow();
	}
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) {
	}
	void setCursorPalette(const byte *colors, uint start, uint num) {}
public:
	uint16_t palette[256];
	int bytes_per_pixel;
	bool mouse_visible;
	int mouse_x, mouse_y;
	emu_graph_params_t gp;
	Graphics::Surface _framebuffer;
};

#endif
