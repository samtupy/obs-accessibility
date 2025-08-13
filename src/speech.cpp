/*
 OBS Accessibility
 Copyright (C) 2025 Sam Tupy Productions <webmaster@samtupy.com>
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this program. If not, see <https://www.gnu.org/licenses/>
*/

#ifdef _WIN32
	#include <windows.h>
	#define UNIVERSAL_SPEECH_STATIC
	#include <UniversalSpeech.h>
	#include <obs-module.h>
	#include <filesystem>
	#include <QTCore/QString>
#endif
#include "speech.h"

using namespace std;

bool g_speech_active = false;
bool init_speech() {
	if (g_speech_active) return true;
	#ifdef _WIN32
		obs_module_t* mod = obs_current_module();
		filesystem::path module_bin(obs_get_module_binary_path(mod));
		SetDllDirectoryW(module_bin.parent_path().wstring().c_str());
		return g_speech_active = true;
	#else
		return false; // no speech on this platform.
	#endif
}
bool speak(const string& text, bool interrupt) {
	return speak(QString::fromUtf8(text.c_str(), text.size()).toStdWString(), interrupt);
}
bool speak(const wstring& text, bool interrupt) {
	if (!init_speech()) return false;
	#ifdef _WIN32
		return speechSay(text.c_str(), interrupt);
	#else
		return false;
	#endif
}

