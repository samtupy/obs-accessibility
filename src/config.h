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

#pragma once
#include <obs-data.h>
#include <obs-properties.h>

// Functions defined by our event source to make properties work, set up in audio.cpp.
void event_source_save(void* data, obs_data_t* settings);
void event_source_update(void* data, obs_data_t* settings);
void event_source_defaults(obs_data_t* settings);
obs_properties_t* event_source_getprops(void* data);

bool get_property_bool(const std::string& key, obs_source_t* source = nullptr);
std::string get_property_string(const std::string& key, obs_source_t* source = nullptr);
bool get_event_bool(const std::string& event_id, const std::string& key, bool default_value = false, obs_source_t* source = nullptr);
std::string get_event_string(const std::string& event_id, const std::string& key, const std::string& default_value = "", obs_source_t* source = nullptr);
bool save_config(); // Call once when app begins to exit, before shutdown_audio().
obs_data_t* load_config(); // Call once on module load and pass return value to init_audio().
