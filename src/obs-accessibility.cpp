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
#include <obs-frontend-api.h>
#include <obs.hpp>
#include <obs-accessibility.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "audio.h"
#include "config.h"
#include "events.h"
#include "interface.h"

// plugin module definition and initialization
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
bool obs_module_load(void) {
	OBSDataAutoRelease settings = load_config();
	bool success = init_audio(settings);
	if (!success) {
		obs_log(LOG_INFO, "event_source initialization fail (version %s)", PLUGIN_VERSION);
		return false;
	}
	init_events();
	init_interface();
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void) {
	shutdown_events();
	shutdown_interface();
	obs_log(LOG_INFO, "plugin unloaded");
}
