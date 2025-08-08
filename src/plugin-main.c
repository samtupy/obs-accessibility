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

#include <obs-module.h>
#include <plugin-support.h>
#include <windows.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

void on_source_show(void* param, calldata_t* data) {
	Beep(880, 50);
}
void on_source_hide(void* param, calldata_t* data) {
	Beep(440, 50);
}

bool obs_module_load(void) {
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}
void obs_module_post_load(void) {
	signal_handler_t* core_handler = obs_get_signal_handler();
	signal_handler_connect(core_handler, "source_show", on_source_show, NULL);
	signal_handler_connect(core_handler, "source_hide", on_source_hide, NULL);
}

void obs_module_unload(void) {
	obs_log(LOG_INFO, "plugin unloaded");
}
