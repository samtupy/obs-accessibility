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
	#include <obs-module.h>
	#include <filesystem>
	#include <QTCore/QString>
#endif
#include <prism.h>
#include "speech.h"

using namespace std;

PrismContext* g_speech_ctx = nullptr;
PrismBackend* g_speech_backend = nullptr;
bool init_speech() {
	if (g_speech_ctx && g_speech_backend) return true;
	if (!g_speech_ctx) {
		#ifdef _WIN32
			obs_module_t* mod = obs_current_module();
			filesystem::path module_bin(obs_get_module_binary_path(mod));
			SetDllDirectoryW(module_bin.parent_path().wstring().c_str()); // Now any additional screen reader dlls can load from the specified plugin bin directory.
		#endif
		g_speech_ctx = prism_init(nullptr);
		if (!g_speech_ctx) return false;
	}
	g_speech_backend = prism_registry_acquire_best(g_speech_ctx);
	return g_speech_backend != nullptr;
}
void shutdown_speech() {
	if (g_speech_backend) prism_backend_free(g_speech_backend);
	g_speech_backend = nullptr;
	prism_shutdown(g_speech_ctx);
	g_speech_ctx = nullptr;
}
bool speak(const string& text, bool interrupt) {
	if (!init_speech()) return false;
	return prism_backend_speak(g_speech_backend, text.c_str(), interrupt) == PRISM_OK;
}

