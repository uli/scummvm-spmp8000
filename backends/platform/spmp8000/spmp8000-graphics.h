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
		gp.width = getOverlayWidth();
		gp.height = getOverlayHeight();
		_mouseMaxX = gp.width;
		_mouseMaxY = gp.height;
		_composedBuffer = new uint16_t[gp.width * gp.height];
		_overlayStage = new uint16_t[gp.width * gp.height];
		_screenWidth = 320;
		_screenHeight = 200;
		_screenStage16 = new uint16_t[_screenWidth * _screenHeight];
		gp.pixels = _composedBuffer;
		gp.has_palette = 0;
		gp.src_clip_x = 0;
		gp.src_clip_y = 0;
		gp.src_clip_w = gp.width;
		gp.src_clip_h = gp.height;
		emuIfGraphInit(&gp);
		mouse_x = 10;
		mouse_y = 10;
		mouse_visible = true;
		_overlayShown = true;
		_cursorBuffer = new uint16_t[1];
		_cursorBuffer[0] = 0xffff;
		_cursorHeight = _cursorWidth = 1;
		_cursorHotX = _cursorHotY = 0;
		_cursorDontScale = false;
		_cursorKey = 0;
	}
	virtual ~Spmp8000GraphicsManager() {
		emuIfGraphCleanup();
		delete[] _composedBuffer;
		delete[] _overlayStage;
		delete[] _screenStage8;
		delete[] _cursorBuffer;
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
	void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) {
		delete[] _screenStage8;
		if (format && format->bytesPerPixel == 2) {
			_screenBpp = 2;
			_screenStage16 = new uint16_t[width * height];
		}
		else {
			_screenBpp = 1;
			_screenStage8 = new uint8_t[width * height];
		}
		_screenWidth = width;
		_screenHeight = height;
		//adbg_printf("====initSize==== %d/%d\n", width, height);
		mouse_x = width / 2;
		mouse_y = height / 2;
	}
	virtual int getScreenChangeID() const { return 0; }

	void beginGFXTransaction() {}
	OSystem::TransactionError endGFXTransaction() { return OSystem::kTransactionSuccess; }

	int16 getHeight() { return gDisplayDev->getHeight(); }
	int16 getWidth() { return gDisplayDev->getWidth(); }
	void setPalette(const byte *colors, uint start, uint num) {
		uint i;
		for (i = start; i < start + num; i++) {
			palette[i] = MAKE_RGB565(colors[0], colors[1], colors[2]);
			colors += 3;
		}
		updateScreen();
	}
	void grabPalette(byte *colors, uint start, uint num) {}
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
		//adbg_printf("copyrect\n");
		if (_screenBpp == 2) {
			int i;
			uint16_t *fb = _screenStage16 + y * _screenWidth + x;
			const uint16_t *b = (const uint16_t *)buf;
			for (i = 0; i < h; i++) {
				memcpy(fb, b, w * 2);
				fb += _screenWidth;
				b += pitch / 2;
			}
		}
		else {
			int i;
			uint8_t *fb = _screenStage8 + y * _screenWidth + x;
			const uint8_t *b = (const uint8_t *)buf;
			for (i = 0; i < h; i++) {
				memcpy(fb, b, w);
				fb += _screenWidth;
				b += pitch;
			}
		}
	}
	Graphics::Surface *lockScreen() {
		_framebuffer.pixels = _screenStage;
		_framebuffer.w = _screenWidth;
		_framebuffer.h = _screenHeight;
		_framebuffer.pitch = _screenWidth * _screenBpp;
		_framebuffer.format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
		return &_framebuffer;
	}
	void unlockScreen() {
	}
	void fillScreen(uint32 col) {}
	void updateScreen() {
		if (libgame_utime() - _lastUpdate < 33333)
			return;
		//adbg_printf("updateScreen\n");
		int sw, sh;
		uint16_t *src, *dst;
		if (_overlayShown) {
			sw = getOverlayWidth();
			sh = getOverlayHeight();
			src = _overlayStage;
		}
		else {
			sw = _screenWidth;
			sh = _screenHeight;
			src = _screenStage16;
		}
		dst = _composedBuffer;
		if (_screenBpp == 2 || _overlayShown)
			memcpy(dst, src, sw * sh * 2);
		else {
			uint8_t *sb = _screenStage8;
			uint16_t *db = dst;
			int i, j;
			for (i = 0; i < sh; i++) {
				for (j = 0; j < sw; j++) {
					db[j] = palette[sb[j]];
				}
				db += sw;
				sb += sw;
			}
		}
		if (mouse_visible) {
			int mx = mouse_x - _cursorHotX;
			int my = mouse_y - _cursorHotY;
			uint16_t *mb = _cursorBuffer;
			int mw = _cursorWidth;
			int mh = _cursorHeight;
			if (mx < 0) {
				mw -= -mx;
				mb += -mx;
				mx = 0;
			}
			else if (mx + mw > sw) {
				mw -= mx + mw - sw;
			}
			if (my < 0) {
				mh -= -my;
				mb += (-my) * _cursorWidth;
				my = 0;
			}
			else if (my + mh > sh) {
				mh -= my + mh - sh;
			}
			uint16_t *dm = dst + mx + my * sw;
			int i,j;
			for (i = 0; i < mh; i++) {
				for (j = 0; j < mw; j++) {
					if (mb[j] != _cursorKey)
						dm[j] = mb[j];
				}
				dm += sw;
				mb += _cursorWidth;
			}
		}
		emuIfGraphShow();
		_lastUpdate = libgame_utime();
	}
	void setShakePos(int shakeOffset) {}
	void setFocusRectangle(const Common::Rect& rect) {}
	void clearFocusRectangle() {}

	void showOverlay() {
		gp.width = getOverlayWidth();
		gp.height = getOverlayHeight();
		_mouseMaxX = gp.width - 1;
		_mouseMaxY = gp.height - 1;
		if (mouse_x > _mouseMaxX)
			mouse_x = _mouseMaxX;
		if (mouse_y > _mouseMaxY)
			mouse_y = _mouseMaxY;
		delete[] _composedBuffer;
		_composedBuffer = new uint16_t[gp.width * gp.height];
		gp.pixels = _composedBuffer;
		gp.src_clip_x = 0;
		gp.src_clip_y = 0;
		gp.src_clip_w = gp.width;
		gp.src_clip_h = gp.height;
		emuIfGraphChgView(&gp);
		_overlayShown = true;
	}
	void hideOverlay() {
		gp.width = _screenWidth;
		gp.height = _screenHeight;
		_mouseMaxX = gp.width - 1;
		_mouseMaxY = gp.height - 1;
		if (mouse_x > _mouseMaxX)
			mouse_x = _mouseMaxX;
		if (mouse_y > _mouseMaxY)
			mouse_y = _mouseMaxY;
		delete[] _composedBuffer;
		_composedBuffer = new uint16_t[gp.width * gp.height];
		gp.pixels = _composedBuffer;
		gp.src_clip_x = 0;
		gp.src_clip_y = 0;
		gp.src_clip_w = gp.width;
		gp.src_clip_h = gp.height;
		emuIfGraphChgView(&gp);
		_overlayShown = false;
	}
	Graphics::PixelFormat getOverlayFormat() const { return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0); }
	void clearOverlay() {
		memset(_overlayStage, 0, getOverlayWidth() * getOverlayHeight() * 2);
	}
	void grabOverlay(void *buf, int pitch) {
		int i;
		uint16_t *fb = _overlayStage;
		uint16_t *b = (uint16_t *)buf;
		for (i = 0; i < getOverlayHeight(); i++) {
			memcpy(b, fb, getOverlayWidth() * 2);
			fb += getOverlayWidth();
			b += pitch / 2;
		}
	}
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
		int i;
		uint16_t *fb = _overlayStage + y * getOverlayWidth() + x;
		const uint16_t *b = (const uint16_t *)buf;
		for (i = 0; i < h; i++) {
			memcpy(fb, b, w * 2);
			fb += getOverlayWidth();
			b += pitch / 2;
		}
	}
	int16 getOverlayHeight() { return getHeight(); }
	int16 getOverlayWidth() { return getWidth(); }

	bool showMouse(bool visible) { 
		bool old = mouse_visible;
		mouse_visible = visible;
		return old;
	}
	void warpMouse(int x, int y) {
		mouse_x = x;
		mouse_y = y;
	}
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) {
		if (w != _cursorWidth || h != _cursorHeight) {
			delete[] _cursorBuffer;
			_cursorBuffer = new uint16_t[w * h];
		}
		_cursorWidth = w;
		_cursorHeight = h;
		_cursorHotX = hotspotX;
		_cursorHotY = hotspotY;
		if (format && format->bytesPerPixel == 2)
			_cursorKey = keycolor & 0xffff;
		else
			_cursorKey = _cursorPalette[keycolor & 0xff];
		_cursorDontScale = dontScale;
		uint i,j;
		uint16_t *cb = _cursorBuffer;
		const uint8_t *sb = (const uint8_t *)buf;
		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				*cb++ = _cursorPalette[*sb++];
			}
		}
	}
	void setCursorPalette(const byte *colors, uint start, uint num) {
		uint i;
		for (i = start; i < start + num; i++) {
			_cursorPalette[i] = MAKE_RGB565(colors[0], colors[1], colors[2]);
			colors += 3;
		}
	}
public:
	uint16_t palette[256];
	uint16_t _cursorPalette[256];
	bool mouse_visible;
	int mouse_x, mouse_y;
	emu_graph_params_t gp;
	Graphics::Surface _framebuffer;
	uint16_t *_composedBuffer;
	uint16_t *_overlayStage;
	uint _screenWidth, _screenHeight;
	int _screenBpp;
	bool _overlayShown;
	uint _cursorWidth;
	uint _cursorHeight;
	uint _cursorHotX;
	uint _cursorHotY;
	uint32 _cursorKey;
	bool _cursorDontScale;
	uint16_t *_cursorBuffer;
	uint64_t _lastUpdate;
	int _mouseMaxX, _mouseMaxY;
	union {
		uint16_t *_screenStage16;
		uint8_t *_screenStage8;
		void *_screenStage;
	};
};

#endif
