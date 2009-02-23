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
 * $URL$
 * $Id$
 *
 */

#ifdef WIN32
#  include <direct.h>
#  include <windows.h>
#elif defined (__DC__)
#  include <dc.h>
#endif

#include "common/str.h"
#include "common/savefile.h"

#include "sci/include/engine.h"
#include "sci/sci.h"

#include <errno.h>
#include <sys/stat.h>		// for S_IREAD/S_IWRITE

// FIXME: Get rid of the following (needed for O_RDONLY etc.)
#include <fcntl.h>


namespace Sci {

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifdef WIN32
#  define FO_BINARY "b"
#else
#  define FO_BINARY ""
#endif

static int _savegame_indices_nr = -1; // means 'uninitialized'

static struct _savegame_index_struct {
	int id;
	int date;
	int time;
} _savegame_indices[MAX_SAVEGAME_NR];

// This assumes modern stream implementations. It may break on DOS.


/* Attempts to mirror a file by copying it from the resource firectory
** to the working directory. Returns NULL if the file didn't exist.
** Otherwise, the new file is then opened for reading or writing.
*/
static FILE *f_open_mirrored(EngineState *s, char *fname) {
	int fd;
	char *buf = NULL;
	int fsize;


	debug(3, "f_open_mirrored(%s)", fname);
#if 0
	// TODO/FIXME: Use s->resource_dir to locate the file???
	File file;
	if (!file.open(fname))
		return NULL;

	fsize = file.size();
	if (fsize > 0) {
		buf = (char *)sci_malloc(fsize);
		file.read(buf, fsize);
	}

	file.close();

	....
	copy the file to a savegame -> only makes sense to perform this change
	if we at the same time change the code for loading files to look among the
	savestates, and also change *all* file writing code to write to savestates,
	as it should
	...
#endif

	chdir(s->resource_dir);
	fd = sci_open(fname, O_RDONLY | O_BINARY);
	if (!IS_VALID_FD(fd)) {
		chdir(s->work_dir);
		return NULL;
	}

	fsize = sci_fd_size(fd);
	if (fsize > 0) {
		buf = (char*)sci_malloc(fsize);
		read(fd, buf, fsize);
	}

	close(fd);

	chdir(s->work_dir);

	// Visual C++ doesn't allow to specify O_BINARY with creat()
#ifdef _MSC_VER
	fd = _open(fname, O_CREAT | O_BINARY | O_RDWR, S_IREAD | S_IWRITE);
#else
	fd = open(fname, O_CREAT | O_BINARY | O_RDWR, S_IREAD | S_IWRITE);
#endif

	if (!IS_VALID_FD(fd) && buf) {
		free(buf);
		sciprintf("kfile.c: f_open_mirrored(): Warning: Could not create '%s' in '%s' (%d bytes to copy)\n", fname, s->work_dir, fsize);
		return NULL;
	}

	if (fsize) {
		int ret;
		ret = write(fd, buf, fsize);
		if (ret < fsize) {
			sciprintf("kfile.c: f_open_mirrored(): Warning: Could not write all %ld bytes to '%s' in '%s' (only wrote %d)\n",
			          (long)fsize, fname, s->work_dir, ret);
		}

		free(buf);
	}

	close(fd);

	return sci_fopen(fname, "r" FO_BINARY "+");
}

#define _K_FILE_MODE_OPEN_OR_CREATE 0
#define _K_FILE_MODE_OPEN_OR_FAIL 1
#define _K_FILE_MODE_CREATE 2

void file_open(EngineState *s, char *filename, int mode) {
	FILE *file = NULL;

	SCIkdebug(SCIkFILE, "Opening file %s with mode %d\n", filename, mode);
	if ((mode == _K_FILE_MODE_OPEN_OR_FAIL) || (mode == _K_FILE_MODE_OPEN_OR_CREATE)) {
		file = sci_fopen(filename, "r" FO_BINARY "+"); // Attempt to open existing file
		SCIkdebug(SCIkFILE, "Opening file %s with mode %d\n", filename, mode);
		if (!file) {
			SCIkdebug(SCIkFILE, "Failed. Attempting to copy from resource dir...\n");
			file = f_open_mirrored(s, filename);
			if (file)
				SCIkdebug(SCIkFILE, "Success!\n");
			else
				SCIkdebug(SCIkFILE, "Not found.\n");
		}
	}

	if ((!file) && ((mode == _K_FILE_MODE_OPEN_OR_CREATE) || (mode == _K_FILE_MODE_CREATE))) {
		file = sci_fopen(filename, "w" FO_BINARY "+"); /* Attempt to create file */
		SCIkdebug(SCIkFILE, "Creating file %s with mode %d\n", filename, mode);
	}
	if (!file) { // Failed
		SCIkdebug(SCIkFILE, "file_open() failed\n");
		s->r_acc = make_reg(0, 0xffff);
		return;
	}

	uint retval = 1; // Ignore _fileHandles[0]
	while ((retval < s->_fileHandles.size()) && s->_fileHandles[retval]._file)
		retval++;

	if (retval == s->_fileHandles.size()) { // Hit size limit => Allocate more space
		s->_fileHandles.resize(s->_fileHandles.size() + 1);
	}

	s->_fileHandles[retval]._file = file;

	s->r_acc = make_reg(0, retval);
}

reg_t kFOpen(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *name = kernel_dereference_char_pointer(s, argv[0], 0);
	int mode = UKPV(1);

	file_open(s, name, mode);
	debug(3, "kFOpen(%s,0x%x) -> %d", name, mode, s->r_acc.offset);
	return s->r_acc;
}

static FILE *getFileFromHandle(EngineState *s, uint handle) {
	if (handle == 0) {
		SCIkwarn(SCIkERROR, "Attempt to use file handle 0\n");
		return 0;
	}

	if ((handle >= s->_fileHandles.size()) || (s->_fileHandles[handle]._file == NULL)) {
		SCIkwarn(SCIkERROR, "Attempt to use invalid/unused file handle %d\n", handle);
		return 0;
	}

	return s->_fileHandles[handle]._file;
}

void file_close(EngineState *s, int handle) {
	SCIkdebug(SCIkFILE, "Closing file %d\n", handle);

	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	fclose(f);

	s->_fileHandles[handle]._file = NULL;
}

reg_t kFClose(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	debug(3, "kFClose(%d)", UKPV(0));
	file_close(s, UKPV(0));
	return s->r_acc;
}

void fputs_wrapper(EngineState *s, int handle, int size, char *data) {
	SCIkdebug(SCIkFILE, "FPuts'ing \"%s\" to handle %d\n", data, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (f)
		fwrite(data, 1, size, f);
}

void fwrite_wrapper(EngineState *s, int handle, char *data, int length) {
	SCIkdebug(SCIkFILE, "fwrite()'ing \"%s\" to handle %d\n", data, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (f)
		fwrite(data, 1, length, f);
}

reg_t kFPuts(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int handle = UKPV(0);
	char *data = kernel_dereference_char_pointer(s, argv[1], 0);

	fputs_wrapper(s, handle, strlen(data), data);
	return s->r_acc;
}

static void fgets_wrapper(EngineState *s, char *dest, int maxsize, int handle) {
	SCIkdebug(SCIkFILE, "FGets'ing %d bytes from handle %d\n", maxsize, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	fgets(dest, maxsize, f);

	SCIkdebug(SCIkFILE, "FGets'ed \"%s\"\n", dest);
}

static void fread_wrapper(EngineState *s, char *dest, int bytes, int handle) {
	SCIkdebug(SCIkFILE, "fread()'ing %d bytes from handle %d\n", bytes, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	s->r_acc = make_reg(0, fread(dest, 1, bytes, f));
}

static void fseek_wrapper(EngineState *s, int handle, int offset, int whence) {
	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	s->r_acc = make_reg(0, fseek(f, offset, whence));
}

#define TEST_DIR_OR_QUIT(dir) if (!dir) { return NULL_REG; }

reg_t kFGets(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *dest = kernel_dereference_char_pointer(s, argv[0], 0);
	int maxsize = UKPV(1);
	int handle = UKPV(2);

	debug(3, "kFGets(%d,%d)", handle, maxsize);
	fgets_wrapper(s, dest, maxsize, handle);
	return argv[0];
}

/* kGetCWD(address):
** Writes the cwd to the supplied address and returns the address in acc.
*/
reg_t kGetCWD(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *wd = sci_getcwd();
	char *targetaddr = kernel_dereference_char_pointer(s, argv[0], 0);

	strncpy(targetaddr, wd, MAX_SAVE_DIR_SIZE - 1);
	targetaddr[MAX_SAVE_DIR_SIZE - 1] = 0; // Terminate
	debug(3, "kGetCWD() -> %s", targetaddr);

	SCIkdebug(SCIkFILE, "Copying cwd='%s'(%d chars) to %p", wd, strlen(wd), targetaddr);

	free(wd);
	return argv[0];
}

void delete_savegame(EngineState *s, int savedir_nr) {
	Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);

	sciprintf("Deleting savegame '%s'\n", filename.c_str());

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	saveFileMan->removeSavefile(filename.c_str());
}

#define K_DEVICE_INFO_GET_DEVICE 0
#define K_DEVICE_INFO_GET_CURRENT_DEVICE 1
#define K_DEVICE_INFO_PATHS_EQUAL 2
#define K_DEVICE_INFO_IS_FLOPPY 3
#define K_DEVICE_INFO_GET_SAVECAT_NAME 7
#define K_DEVICE_INFO_GET_SAVEFILE_NAME 8

reg_t kDeviceInfo(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int mode = UKPV(0);
	char *game_prefix, *input_s, *output_s;

	switch (mode) {
	case K_DEVICE_INFO_GET_DEVICE:
		input_s = kernel_dereference_char_pointer(s, argv[1], 0);
		output_s = kernel_dereference_char_pointer(s, argv[2], 0);
		assert(input_s != output_s);

		strcpy(output_s, "/");
		debug(3, "K_DEVICE_INFO_GET_DEVICE(%s) -> %s", input_s, output_s);
		break;

	case K_DEVICE_INFO_GET_CURRENT_DEVICE:
		output_s = kernel_dereference_char_pointer(s, argv[1], 0);

		strcpy(output_s, "/");
		debug(3, "K_DEVICE_INFO_GET_CURRENT_DEVICE() -> %s", output_s);
		break;

	case K_DEVICE_INFO_PATHS_EQUAL: {
		char *path1_s = kernel_dereference_char_pointer(s, argv[1], 0);
		char *path2_s = kernel_dereference_char_pointer(s, argv[2], 0);
		debug(3, "K_DEVICE_INFO_PATHS_EQUAL(%s,%s)", path1_s, path2_s);

		return make_reg(0, Common::matchString(path2_s, path1_s, true));
		}
		break;

	case K_DEVICE_INFO_IS_FLOPPY:
		input_s = kernel_dereference_char_pointer(s, argv[1], 0);
		debug(3, "K_DEVICE_INFO_IS_FLOPPY(%s)\n", input_s);
		return NULL_REG; /* Never */

	/* SCI uses these in a less-than-portable way to delete savegames.
	** Read http://www-plan.cs.colorado.edu/creichen/freesci-logs/2005.10/log20051019.html
	** for more information on our workaround for this.
	*/
	case K_DEVICE_INFO_GET_SAVECAT_NAME: {
		output_s = kernel_dereference_char_pointer(s, argv[1], 0);
		game_prefix = kernel_dereference_char_pointer(s, argv[2], 0);

		sprintf(output_s, "%s/__throwaway", s->work_dir);
		debug(3, "K_DEVICE_INFO_GET_SAVECAT_NAME(%s) -> %s", game_prefix, output_s);
		}

	break;
	case K_DEVICE_INFO_GET_SAVEFILE_NAME: {
		output_s = kernel_dereference_char_pointer(s, argv[1], 0);
		game_prefix = kernel_dereference_char_pointer(s, argv[2], 0);
		int savegame_id = UKPV(3);
		sprintf(output_s, "%s/__throwaway", s->work_dir);
		delete_savegame(s, savegame_id);
		debug(3, "K_DEVICE_INFO_GET_SAVEFILE_NAME(%s,%d) -> %s", game_prefix, savegame_id, output_s);
		}
		break;

	default:
		SCIkwarn(SCIkERROR, "Unknown DeviceInfo() sub-command: %d\n", mode);
		break;
	}

	return s->r_acc;
}

reg_t kGetSaveDir(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	return make_reg(s->sys_strings_segment, SYS_STRING_SAVEDIR);
}

reg_t kCheckFreeSpace(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *path = kernel_dereference_char_pointer(s, argv[0], 0);

	debug(3, "kCheckFreeSpace(%s)", path);
	// We simply always pretend that there is enough space.
	// The alternative would be to write a big test file, which is not nice
	// on systems where doing so is very slow.
	return make_reg(0, 1);
}

static int _savegame_index_struct_compare(const void *a, const void *b) {
	struct _savegame_index_struct *A = (struct _savegame_index_struct *)a;
	struct _savegame_index_struct *B = (struct _savegame_index_struct *)b;

	if (B->date != A->date)
		return B->date - A->date;
	return B->time - A->time;
}

static void update_savegame_indices() {
	int i;

	_savegame_indices_nr = 0;

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	for (i = 0; i < MAX_SAVEGAME_NR; i++) {
		Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(i);
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename.c_str()))) {
			SavegameMetadata meta;
			if (!get_savegame_metadata(in, &meta)) {
				// invalid
				delete in;
				continue;
			}
			delete in;

			fprintf(stderr, "Savegame in %s file ok\n", filename.c_str());
			_savegame_indices[_savegame_indices_nr].id = i;
			_savegame_indices[_savegame_indices_nr].date = meta.savegame_date;
			_savegame_indices[_savegame_indices_nr].time = meta.savegame_time;
			_savegame_indices_nr++;
		}
	}

	qsort(_savegame_indices, _savegame_indices_nr, sizeof(struct _savegame_index_struct), _savegame_index_struct_compare);
}

reg_t kCheckSaveGame(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *game_id = kernel_dereference_char_pointer(s, argv[0], 0);
	int savedir_nr = UKPV(1);

	debug(3, "kCheckSaveGame(%s, %d)", game_id, savedir_nr);
	if (_savegame_indices_nr < 0) {
		warning("Savegame index list not initialized");
		update_savegame_indices();
	}

	savedir_nr = _savegame_indices[savedir_nr].id;

	if (savedir_nr > MAX_SAVEGAME_NR - 1) {
		return NULL_REG;
	}

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);
	Common::SeekableReadStream *in;
	if ((in = saveFileMan->openForLoading(filename.c_str()))) {
		// found a savegame file

		SavegameMetadata meta;
		if (!get_savegame_metadata(in, &meta)) {
			// invalid
			s->r_acc = make_reg(0, 0);
		} else {
			s->r_acc = make_reg(0, 1);
		}
		delete in;
	} else {
		s->r_acc = make_reg(0, 1);
	}

	return s->r_acc;
}

reg_t kGetSaveFiles(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *game_id = kernel_dereference_char_pointer(s, argv[0], 0);
	char *nametarget = kernel_dereference_char_pointer(s, argv[1], 0);
	reg_t nametarget_base = argv[1];
	reg_t *nameoffsets = kernel_dereference_reg_pointer(s, argv[2], 0);
	int i;

	debug(3, "kGetSaveFiles(%s,%s)", game_id, nametarget);
	update_savegame_indices();

	s->r_acc = NULL_REG;
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	for (i = 0; i < _savegame_indices_nr; i++) {
		Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(_savegame_indices[i].id);
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename.c_str()))) {
			// found a savegame file

			SavegameMetadata meta;
			if (!get_savegame_metadata(in, &meta)) {
				// invalid
				delete in;
				continue;
			}

			char namebuf[SCI_MAX_SAVENAME_LENGTH]; // Save game name buffer
			strncpy(namebuf, meta.savegame_name, SCI_MAX_SAVENAME_LENGTH);
			namebuf[SCI_MAX_SAVENAME_LENGTH-1] = 0;

			if (strlen(namebuf) > 0) {
				if (namebuf[strlen(namebuf) - 1] == '\n')
					namebuf[strlen(namebuf) - 1] = 0; // Remove trailing newline

				*nameoffsets = s->r_acc; // Store savegame ID
				++s->r_acc.offset; // Increase number of files found

				nameoffsets++; // Make sure the next ID string address is written to the next pointer
				strncpy(nametarget, namebuf, SCI_MAX_SAVENAME_LENGTH); // Copy identifier string
				*(nametarget + SCI_MAX_SAVENAME_LENGTH - 1) = 0; // Make sure it's terminated
				nametarget += SCI_MAX_SAVENAME_LENGTH; // Increase name offset pointer accordingly
				nametarget_base.offset += SCI_MAX_SAVENAME_LENGTH;
			}
			delete in;
		}
	}

	//free(gfname);
	*nametarget = 0; // Terminate list

	return s->r_acc;
}

reg_t kSaveGame(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *game_id = kernel_dereference_char_pointer(s, argv[0], 0);
	int savedir_nr = UKPV(1);
	int savedir_id; // Savegame ID, derived from savedir_nr and the savegame ID list
	char *game_description = kernel_dereference_char_pointer(s, argv[2], 0);
	char *version = argc > 3 ? strdup(kernel_dereference_char_pointer(s, argv[3], 0)) : NULL;

	debug(3, "kSaveGame(%s,%d,%s,%s)", game_id, savedir_nr, game_description, version);
	s->game_version = version;

	update_savegame_indices();

	fprintf(stderr, "savedir_nr = %d\n", savedir_nr);

	if (savedir_nr >= 0 && savedir_nr < _savegame_indices_nr)
		// Overwrite
		savedir_id = _savegame_indices[savedir_nr].id;
	else if (savedir_nr >= 0 && savedir_nr < MAX_SAVEGAME_NR) {
		int i = 0;

		fprintf(stderr, "searching for hole\n");

		savedir_id = 0;

		// First, look for holes
		while (i < _savegame_indices_nr) {
			if (_savegame_indices[i].id == savedir_id) {
				++savedir_id;
				i = 0;
			} else ++i;
		}
		if (savedir_id >= MAX_SAVEGAME_NR) {
			sciprintf("Internal error: Free savegame ID is %d, shouldn't happen!\n", savedir_id);
			return NULL_REG;
		}

		// This loop terminates when savedir_id is not in [x | ex. n. _savegame_indices[n].id = x]
	} else {
		sciprintf("Savegame ID %d is not allowed!\n", savedir_nr);
		return NULL_REG;
	}

	Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_id);
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out;
	if (!(out = saveFileMan->openForSaving(filename.c_str()))) {
		sciprintf("Error opening savegame \"%s\" for writing\n", filename.c_str());
		s->r_acc = NULL_REG;
		return NULL_REG;
	}

	if (gamestate_save(s, out, game_description)) {
		sciprintf("Saving the game failed.\n");
		s->r_acc = NULL_REG;
	} else {
		out->finalize();
		if (out->err()) {
			delete out;
			sciprintf("Writing the savegame failed.\n");
			s->r_acc = NULL_REG;
		} else {
			delete out;
			s->r_acc = make_reg(0, 1);
		}
	}
	free(s->game_version);
	s->game_version = NULL;

	return s->r_acc;
}

reg_t kRestoreGame(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *game_id = kernel_dereference_char_pointer(s, argv[0], 0);
	int savedir_nr = UKPV(1);

	debug(3, "kRestoreGame(%s,%d)", game_id, savedir_nr);

	if (_savegame_indices_nr < 0) {
		warning("Savegame index list not initialized");
		update_savegame_indices();
	}

	savedir_nr = _savegame_indices[savedir_nr].id;

	if (savedir_nr > -1) {
		Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
		Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename.c_str()))) {
			// found a savegame file

			EngineState *newstate = gamestate_restore(s, in);
			delete in;

			if (newstate) {
				s->successor = newstate;
				script_abort_flag = SCRIPT_ABORT_WITH_REPLAY; // Abort current game
				s->execution_stack_pos = s->execution_stack_base;
			} else {
				s->r_acc = make_reg(0, 1);
				sciprintf("Restoring failed (game_id = '%s').\n", game_id);
			}
			return s->r_acc;
		}
	}

	s->r_acc = make_reg(0, 1);
	sciprintf("Savegame #%d not found!\n", savedir_nr);

	return s->r_acc;
}

reg_t kValidPath(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *pathname = kernel_dereference_char_pointer(s, argv[0], 0);
	char cpath[MAXPATHLEN + 1];
	getcwd(cpath, MAXPATHLEN + 1);


	s->r_acc = make_reg(0, !chdir(pathname)); // Try to go there. If it works, return 1, 0 otherwise.
	chdir(cpath);

	debug(3, "kValidPath(%s) -> %d", pathname, s->r_acc.offset);

	return s->r_acc;
}

#define K_FILEIO_OPEN		0
#define K_FILEIO_CLOSE		1
#define K_FILEIO_READ_RAW	2
#define K_FILEIO_WRITE_RAW	3
#define K_FILEIO_UNLINK		4
#define K_FILEIO_READ_STRING	5
#define K_FILEIO_WRITE_STRING	6
#define K_FILEIO_SEEK		7
#define K_FILEIO_FIND_FIRST	8
#define K_FILEIO_FIND_NEXT	9
#define K_FILEIO_STAT		10


class DirSeeker {
protected:
	EngineState *_vm;
	reg_t _outbuffer;
	sci_dir_t _dir;

	const char *write_filename_to_mem(const char *string);

public:
	DirSeeker(EngineState *s) : _vm(s) {
		_outbuffer = NULL_REG;
		sci_init_dir(&_dir);
	}
	
	void first_file(const char *dir, char *mask, reg_t buffer);
	void next_file();
};


const char *DirSeeker::write_filename_to_mem(const char *string) {
	char *mem = kernel_dereference_char_pointer(_vm, _outbuffer, 0);

	if (string) {
		memset(mem, 0, 13);
		strncpy(mem, string, 12);
	}

	return string;
}

void DirSeeker::next_file() {
	if (write_filename_to_mem(sci_find_next(&_dir)))
		_vm->r_acc = _outbuffer;
	else
		_vm->r_acc = NULL_REG;
}

void DirSeeker::first_file(const char *dir, char *mask, reg_t buffer) {
	if (!buffer.segment) {
		sciprintf("Warning: first_file(state,\"%s\",\"%s\", 0) invoked!\n", dir, mask);
		_vm->r_acc = NULL_REG;
		return;
	}

	if (strcmp(dir, ".")) {
		sciprintf("%s L%d: Non-local first_file: Not implemented yet\n", __FILE__, __LINE__);
		_vm->r_acc = NULL_REG;
		return;
	}

	// Get rid of the old find structure
	if (_outbuffer.segment)
		sci_finish_find(&_dir);
	
	_outbuffer = buffer;

	if (write_filename_to_mem(sci_find_first(&_dir, mask)))
		_vm->r_acc = _outbuffer;
	else
		_vm->r_acc = NULL_REG;
}

reg_t kFileIO(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int func_nr = UKPV(0);

	switch (func_nr) {
	case K_FILEIO_OPEN : {
		char *name = kernel_dereference_char_pointer(s, argv[1], 0);
		int mode = UKPV(2);

		file_open(s, name, mode);
		debug(3, "K_FILEIO_OPEN(%s,0x%x) -> %d", name, mode, s->r_acc.offset);
		break;
	}
	case K_FILEIO_CLOSE : {
		int handle = UKPV(1);
		debug(3, "K_FILEIO_CLOSE(%d)", handle);

		file_close(s, handle);
		break;
	}
	case K_FILEIO_READ_RAW : {
		int handle = UKPV(1);
		char *dest = kernel_dereference_char_pointer(s, argv[2], 0);
		int size = UKPV(3);
		debug(3, "K_FILEIO_READ_RAW(%d,%d)", handle, size);

		fread_wrapper(s, dest, size, handle);
		break;
	}
	case K_FILEIO_WRITE_RAW : {
		int handle = UKPV(1);
		char *buf = kernel_dereference_char_pointer(s, argv[2], 0);
		int size = UKPV(3);
		debug(3, "K_FILEIO_WRITE_RAW(%d,%d)", handle, size);

		fwrite_wrapper(s, handle, buf, size);
		break;
	}
	case K_FILEIO_UNLINK : {
		char *name = kernel_dereference_char_pointer(s, argv[1], 0);
		debug(3, "K_FILEIO_UNLINK(%s)", name);

		unlink(name);
		break;
	}
	case K_FILEIO_READ_STRING : {
		char *dest = kernel_dereference_char_pointer(s, argv[1], 0);
		int size = UKPV(2);
		int handle = UKPV(3);
		debug(3, "K_FILEIO_READ_STRING(%d,%d)", handle, size);

		fgets_wrapper(s, dest, size, handle);
		return argv[1];
	}
	case K_FILEIO_WRITE_STRING : {
		int handle = UKPV(1);
		int size = UKPV(3);
		char *buf = kernel_dereference_char_pointer(s, argv[2], size);
		debug(3, "K_FILEIO_WRITE_STRING(%d,%d)", handle, size);

		if (buf)
			fputs_wrapper(s, handle, size, buf);
		break;
	}
	case K_FILEIO_SEEK : {
		int handle = UKPV(1);
		int offset = UKPV(2);
		int whence = UKPV(3);
		debug(3, "K_FILEIO_SEEK(%d,%d,%d)", handle, offset, whence);

		fseek_wrapper(s, handle, offset, whence);
		break;
	}
	case K_FILEIO_FIND_FIRST : {
		char *mask = kernel_dereference_char_pointer(s, argv[1], 0);
		reg_t buf = argv[2];
		int attr = UKPV(3); // We won't use this, Win32 might, though...
		debug(3, "K_FILEIO_FIND_FIRST(%s,0x%x)", mask, attr);

#ifndef WIN32
		if (strcmp(mask, "*.*") == 0)
			strcpy(mask, "*"); // For UNIX
#endif
		if (!s->dirseeker)
			s->dirseeker = new DirSeeker(s);
		assert(s->dirseeker);
		s->dirseeker->first_file(".", mask, buf);

		break;
	}
	case K_FILEIO_FIND_NEXT : {
		assert(s->dirseeker);
		debug(3, "K_FILEIO_FIND_NEXT()");
		s->dirseeker->next_file();
		break;
	}
	case K_FILEIO_STAT : {
		char *name = kernel_dereference_char_pointer(s, argv[1], 0);
		s->r_acc = make_reg(0, sci_file_size(name) >= 0);
		debug(3, "K_FILEIO_STAT(%s) -> %d", name, s->r_acc.offset);
		break;
	}
	default :
		SCIkwarn(SCIkERROR, "Unknown FileIO() sub-command: %d\n", func_nr);
	}

	return s->r_acc;
}

} // End of namespace Sci
