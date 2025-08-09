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

#include <obs.h>
#include <obs-module.h>
#include "audio.h" // Config and audio are somewhat linked because OBS makes it convenient for us to not only save config data in sources but to also generate a properties dialog for them.
#include "config.h"

bool save_config() {
	event_source_data* src = get_audio_event_source();
	if (!src) return false;
	char* config_dir = obs_module_config_path(nullptr);
	if (!config_dir) return false;
	os_mkdirs(config_dir);
	bfree(config_dir);
	char* file = obs_module_config_path("accessibility.json");
	if (!file) return false;
	obs_data_t* settings = obs_save_source(src->source);
	if (!settings) {
		bfree(file);
		return false;
	}
	bool success =obs_data_save_json_pretty_safe(settings, file, ".tmp", ".bak");
	bfree(file);
	obs_data_release(settings);
	return success;
}
obs_data_t* load_config() {
	char* file = obs_module_config_path("accessibility.json");
	if (!file) return nullptr;
	obs_data_t* settings = obs_data_create_from_json_file_safe(file, ".bak");
	bfree(file);
	obs_data_item_t* item = obs_data_first(settings);
	if (!item) {
		obs_data_release(settings);
		return nullptr;
	}
	obs_data_item_release(&item);
	return settings;
}
