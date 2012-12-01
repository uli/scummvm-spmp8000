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

#include <libgame.h>

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/modular-backend.h"
#include "base/main.h"

#include "backends/mutex/null/null-mutex.h"
#include "backends/platform/spmp8000/spmp8000-graphics.h"

#if defined(USE_SPMP8000_DRIVER)
#include "backends/saves/default/default-saves.h"
#include "backends/events/default/default-events.h"
#include "backends/timer/default/default-timer.h"
#include "audio/mixer_intern.h"
#include "common/scummsys.h"

/*
 * Include header files needed for the getFilesystemFactory() method.
 */
#include "backends/fs/posix/posix-fs-factory.h"

class OSystem_SPMP8000 : public ModularBackend, Common::EventSource {
public:
	OSystem_SPMP8000();
	virtual ~OSystem_SPMP8000();

	virtual void initBackend();

	virtual bool pollEvent(Common::Event &event);

	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &t) const {}

	virtual void logMessage(LogMessageType::Type type, const char *message);
	Common::EventSource *getDefaultEventSource() { return this; }
private:
	void updateSound(void);
	Spmp8000GraphicsManager *gm;
	bool keyEvent(Common::Event &event, uint32_t keys, int emu_key, Common::KeyCode keycode, int ascii);
	uint64_t _lastMix;
};

OSystem_SPMP8000::OSystem_SPMP8000() {
	_fsFactory = new POSIXFilesystemFactory();
}

OSystem_SPMP8000::~OSystem_SPMP8000() {
}

emu_keymap_t keymap;
emu_sound_params_t sp;
uint16_t sound_bufs[2][22050 / 10 * 2 * 2];
int current_sound_buf = 0;

void OSystem_SPMP8000::updateSound()
{
	if (libgame_utime() < _lastMix + 60000)
		return;
	current_sound_buf = (current_sound_buf + 1) % 2;
	((Audio::MixerImpl *)_mixer)->mixCallback((byte *)sound_bufs[current_sound_buf], 22050 / 10 * 2 * 2);
	sp.buf = (uint8_t *)sound_bufs[current_sound_buf];
	emuIfSoundPlay(&sp);
	_lastMix = libgame_utime();
}

void OSystem_SPMP8000::initBackend() {
	_mutexManager = new NullMutexManager();
	_timerManager = new DefaultTimerManager();
	_eventManager = new DefaultEventManager(this);
	_savefileManager = new DefaultSaveFileManager();
	_graphicsManager = gm = new Spmp8000GraphicsManager();
	_mixer = new Audio::MixerImpl(this, 22050);

	sp.rate = 22050;
	sp.channels = 2;
	sp.buf_size = 22050 / 10 * 2 * 2;
	sp.depth = 0;
	sp.callback = 0;
	emuIfSoundInit(&sp);
	_lastMix = 0;

	((Audio::MixerImpl *)_mixer)->setReady(true);

	// Note that both the mixer and the timer manager are useless
	// this way; they need to be hooked into the system somehow to
	// be functional. Of course, can't do that in a SPMP8000 backend :).

	ModularBackend::initBackend();
	keymap.controller = 0;
	emuIfKeyInit(&keymap);
}

int count = 0;
uint32_t last_move = 0;
bool button_down = false;
uint32_t last_keys = 0;
bool OSystem_SPMP8000::keyEvent(Common::Event &event, uint32_t keys, int emu_key, Common::KeyCode keycode, int ascii)
{
	if (keys & keymap.scancode[emu_key] && !(last_keys & keymap.scancode[emu_key])) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = keycode;
		event.kbd.ascii = ascii;
		return true;
	}
	else if (!(keys & keymap.scancode[emu_key]) && last_keys & keymap.scancode[emu_key]) {
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = keycode;
		event.kbd.ascii = ascii;
		return true;
	}
	return false;
}

bool OSystem_SPMP8000::pollEvent(Common::Event &event) {
	updateSound();
	bool have_event = false;
	uint32_t keys = emuIfKeyGetInput(&keymap);
	if (keyEvent(event, keys, EMU_KEY_START, Common::KEYCODE_F5, Common::ASCII_F5)) {
		have_event = true;
		goto done;
	}
	if (keyEvent(event, keys, EMU_KEY_SELECT, Common::KEYCODE_ESCAPE, 27)) {
		have_event = true;
		goto done;
	}
	if (keys & keymap.scancode[EMU_KEY_RIGHT] && getMillis() - last_move > 3) {
		event.type = Common::EVENT_MOUSEMOVE;
		if (gm->mouse_x < gm->getWidth() - 1)
			gm->mouse_x++;
		event.mouse.x = gm->mouse_x;
		event.mouse.y = gm->mouse_y;
		have_event = true;
		last_move = getMillis();
	}
	else if (keys & keymap.scancode[EMU_KEY_LEFT] && getMillis() - last_move > 3) {
		event.type = Common::EVENT_MOUSEMOVE;
		if (gm->mouse_x > 0)
			gm->mouse_x--;
		event.mouse.x = gm->mouse_x;
		event.mouse.y = gm->mouse_y;
		have_event = true;
		last_move = getMillis();
	}
	if (keys & keymap.scancode[EMU_KEY_DOWN] && getMillis() - last_move > 3) {
		event.type = Common::EVENT_MOUSEMOVE;
		if (gm->mouse_y < gm->getHeight() - 1)
			gm->mouse_y++;
		event.mouse.x = gm->mouse_x;
		event.mouse.y = gm->mouse_y;
		have_event = true;
		last_move = getMillis();
	}
	else if (keys & keymap.scancode[EMU_KEY_UP] && getMillis() - last_move > 3) {
		event.type = Common::EVENT_MOUSEMOVE;
		if (gm->mouse_y > 0)
			gm->mouse_y--;
		event.mouse.x = gm->mouse_x;
		event.mouse.y = gm->mouse_y;
		have_event = true;
		last_move = getMillis();
	}
	if (keys & keymap.scancode[EMU_KEY_O]) {
		if (!button_down) {
			button_down = true;
			event.type = Common::EVENT_LBUTTONDOWN;
			event.mouse.x = gm->mouse_x;
			event.mouse.y = gm->mouse_y;
			have_event = true;
		}
	}
	else if (button_down) {
		button_down = false;
		event.type = Common::EVENT_LBUTTONUP;
		event.mouse.x = gm->mouse_x;
		event.mouse.y = gm->mouse_y;
		have_event = true;
	}
		
done:
	count++;
	last_keys = keys;
	return have_event;
}

uint32 OSystem_SPMP8000::getMillis() {
	return libgame_utime() / 1000;
}

void OSystem_SPMP8000::delayMillis(uint msecs) {
	cyg_thread_delay(msecs / 10);
}

void OSystem_SPMP8000::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);
}

OSystem *OSystem_SPMP8000_create() {
	return new OSystem_SPMP8000();
}

void *foo;
int main(int argc, char *argv[]) {
	int res = 0;
	libgame_init();
	FILE *fp = fopen("scummvm_stderr.txt", "w");
	if (fp) {
		stderr = fp;
		setbuf(fp, NULL);
	}
#if 1
	fp = fopen("scummvm_stdout.txt", "w");
	if (fp) {
		stdout = fp;
	}
#endif
	g_system = OSystem_SPMP8000_create();
	assert(g_system);

	libgame_chdir_game();
	// Invoke the actual ScummVM main entry point:
	foo = (void *)scummvm_main;
	//goto out;
	res = scummvm_main(0, 0);
	delete (OSystem_SPMP8000 *)g_system;
	
out:	fclose(stderr);
	fclose(stdout);
	return res;
}

#else /* USE_SPMP8000_DRIVER */

OSystem *OSystem_SPMP8000_create() {
	return SPMP8000;
}

#endif
