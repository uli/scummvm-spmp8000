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
#if defined(__amigaos4__)
	#include "backends/fs/amigaos4/amigaos4-fs-factory.h"
#elif defined(POSIX)
	#include "backends/fs/posix/posix-fs-factory.h"
#elif defined(WIN32)
	#include "backends/fs/windows/windows-fs-factory.h"
#endif

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
};

OSystem_SPMP8000::OSystem_SPMP8000() {
	#if defined(__amigaos4__)
		_fsFactory = new AmigaOSFilesystemFactory();
	#elif defined(POSIX)
		_fsFactory = new POSIXFilesystemFactory();
	#elif defined(WIN32)
		_fsFactory = new WindowsFilesystemFactory();
	#else
		#error Unknown and unsupported FS backend
	#endif
}

OSystem_SPMP8000::~OSystem_SPMP8000() {
}

void OSystem_SPMP8000::initBackend() {
	fprintf(stderr, "s8kib\n");
	_mutexManager = new NullMutexManager();
	_timerManager = new DefaultTimerManager();
	_eventManager = new DefaultEventManager(this);
	_savefileManager = new DefaultSaveFileManager();
	_graphicsManager = new Spmp8000GraphicsManager();
	_mixer = new Audio::MixerImpl(this, 22050);

	((Audio::MixerImpl *)_mixer)->setReady(false);

	// Note that both the mixer and the timer manager are useless
	// this way; they need to be hooked into the system somehow to
	// be functional. Of course, can't do that in a SPMP8000 backend :).

	ModularBackend::initBackend();
}

bool OSystem_SPMP8000::pollEvent(Common::Event &event) {
	return false;
}

uint32 OSystem_SPMP8000::getMillis() {
	return 0;
}

void OSystem_SPMP8000::delayMillis(uint msecs) {
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
	fprintf(stderr, "create\n");
	g_system = OSystem_SPMP8000_create();
	fprintf(stderr, "assert\n");
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	fprintf(stderr, "main\n");
	foo = (void *)scummvm_main;
	//goto out;
	res = scummvm_main(0, 0);
	fprintf(stderr, "delete\n");
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
