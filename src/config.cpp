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

#include <string>
#include <vector>
#include <obs.h>
#include <obs.hpp>
#include <obs-module.h>
#include <obs-properties.h>
#include "audio.h" // Config and audio are somewhat linked because OBS makes it convenient for us to not only save config data in sources but to also generate a properties dialog for them.
#include "config.h"
#include "events.h"
#include "interface.h"
#include "speech.h"

using namespace std;

OBSDataAutoRelease get_event_config(const string& event_id, obs_source_t* source = nullptr, bool create = false);

// Configuration properties. We might be stretching the intent for the obs_properties API juuust a bit, but the idea is to use the properties API to construct a settings dialog for the entire plugin while also allowing users to change fewer settings if they manually create an audio events source.
bool on_event_edit(obs_properties_t* settings, obs_property_t* property, void* data) {
	event_source_data* src = (event_source_data*)obs_properties_get_param(settings);
	string event_id = get_property_string("event_list", src->source);
	event_type* event = get_event_type(event_id);
	if (!event) return false;
	src->ui_event = event;
	obs_property_set_visible(property, false);
	obs_property_t* event_edit_group = obs_properties_get(settings, "event_edit");
	obs_property_set_description(event_edit_group, event->get_name().c_str());
	obs_property_set_visible(event_edit_group, true);
	OBSDataAutoRelease change = obs_source_get_settings(src->source);
	obs_data_set_string(change, "event_id", event_id.c_str());
	obs_data_set_bool(change, "event_muted", event->get_muted(src->source));
	obs_data_set_string(change, "event_message", event->get_message(src->source).c_str());
	obs_source_update(src->source, change);
	correct_properties_focus();
	return true;
}
bool on_event_edit_message_default(obs_properties_t* settings, obs_property_t* property, void* data) {
	event_source_data* src = (event_source_data*)obs_properties_get_param(settings);
	if (!src->ui_event) return false;
	string msg = src->ui_event->get_default_message();
	OBSDataAutoRelease change = obs_source_get_settings(src->source);
	obs_data_set_string(change, "event_message", msg.c_str());
	obs_source_update(src->source, change);
	correct_properties_focus();
	return true;
}
bool on_event_edit_save(obs_properties_t* settings, obs_property_t* property, void* data) {
	event_source_data* src = (event_source_data*)obs_properties_get_param(settings);
	if (!src->ui_event) return false;
	OBSDataAutoRelease src_settings = obs_source_get_settings(src->source);
	OBSDataAutoRelease event = get_event_config(src->ui_event->get_id(), src->source, true);
	obs_data_set_bool(event, "muted", obs_data_get_bool(src_settings, "event_muted"));
	obs_data_set_string(event, "message", obs_data_get_string(src_settings, "event_message"));
	obs_property_set_visible(obs_properties_get(settings, "event_edit"), false);
	obs_property_set_visible(obs_properties_get(settings, "event_edit_btn"), true);
	src->ui_event = nullptr;
	correct_properties_focus();
	return true;
}
bool on_event_edit_cancel(obs_properties_t* settings, obs_property_t* property, void* data) {
	event_source_data* src = (event_source_data*)obs_properties_get_param(settings);
	obs_property_set_visible(obs_properties_get(settings, "event_edit"), false);
	obs_property_set_visible(obs_properties_get(settings, "event_edit_btn"), true);
	src->ui_event = nullptr;
	correct_properties_focus();
	return true;
}
void event_source_defaults(obs_data_t* settings) {
	obs_data_set_default_bool(settings, "speech", true);
	obs_data_set_default_bool(settings, "speech_interrupt", "false");
	obs_data_set_default_bool(settings, "sound", true);
	obs_data_set_default_string(settings, "earcon_path", "");
}
void event_source_update(void* data, obs_data_t* settings) {
}
void event_source_save(void* data, obs_data_t* settings) {
	// Remove any temporary variables only used for the UI.
	obs_data_erase(settings, "event_list");
	obs_data_erase(settings, "event_muted");
	obs_data_erase(settings, "event_message");
	obs_data_erase(settings, "event_edit");
	obs_data_erase(settings, "event_id");
}
obs_properties_t* event_source_getprops(void* data) {
	event_source_data* d = (event_source_data*)data;
	obs_properties_t* props = obs_properties_create();
	obs_properties_set_param(props, d, nullptr);
	if (d->global_events) {
		obs_properties_add_bool(props, "speech", obs_module_text("props.speech"));
		obs_properties_add_bool(props, "speech_interrupt", obs_module_text("props.speech_interrupt"));
	}
	obs_properties_add_bool(props, "sound", obs_module_text("props.sound"));
	obs_property_t* earcon_path = obs_properties_add_path(props, "earcon_path", obs_module_text("props.earcon_path"), OBS_PATH_DIRECTORY, "", "");
	obs_property_set_long_description(earcon_path, obs_module_text("props.earcon_path"));
	obs_property_t* event_list = obs_properties_add_list(props, "event_list", obs_module_text("props.events"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_set_long_description(event_list, obs_module_text("props.events"));
	vector<string> events;
	get_event_types(events);
	for (auto i : events) obs_property_list_add_string(event_list, get_event_type(i)->describe().c_str(), i.c_str());
	obs_properties_add_button(props, "event_edit_btn", obs_module_text("props.event.edit"), on_event_edit);
	obs_properties_t* event_edit = obs_properties_create();
	obs_property_t* event_edit_group = obs_properties_add_group(props, "event_edit", "", OBS_GROUP_NORMAL, event_edit);
	obs_properties_add_text(event_edit, "event_id", obs_module_text("props.event.id"), OBS_TEXT_INFO);
	obs_properties_add_bool(event_edit, "event_muted", obs_module_text("props.event.mute"));
	if (d->global_events) {
		obs_properties_add_text(event_edit, "event_message", obs_module_text("props.event.message"), OBS_TEXT_DEFAULT);
		obs_properties_add_button(event_edit, "event_edit_message_default", obs_module_text("props.event.message_default"), on_event_edit_message_default);
	}
	obs_properties_add_button(event_edit, "event_edit_save", obs_module_text("props.event.save"), on_event_edit_save);
	obs_properties_add_button(event_edit, "event_edit_cancel", obs_module_text("props.event.cancel"), on_event_edit_cancel);
	obs_property_set_visible(event_edit_group, false);
	return props;
}

inline OBSDataAutoRelease get_properties(obs_source_t* source = nullptr) {
	if (!source) {
		event_source_data* event_source = get_audio_event_source();
		if (!event_source) return nullptr;
		source = event_source->source;
	}
	return obs_source_get_settings(source);
}
OBSDataAutoRelease get_config(obs_source_t* source) {
	if (!source) {
		event_source_data* event_source = get_audio_event_source();
		if (!event_source) return nullptr;
		source = event_source->source;
	}
	return obs_source_get_private_settings(source);
}
OBSDataAutoRelease get_hotkeys_config() {
	OBSDataAutoRelease config = get_config();
	if (!config) return nullptr;
	OBSDataAutoRelease hotkeys = obs_data_get_obj(config, "hotkeys");
	if (!hotkeys) {
		hotkeys = obs_data_create();
		obs_data_set_obj(config, "hotkeys", hotkeys);
	}
	return hotkeys;
}
bool get_property_bool(const string& key, obs_source_t* source) {
	OBSDataAutoRelease settings = get_properties(source);
	return settings? obs_data_get_bool(settings, key.c_str()) : false;
}
string get_property_string(const string& key, obs_source_t* source) {
	OBSDataAutoRelease settings = get_properties(source);
	const char* str = settings? obs_data_get_string(settings, key.c_str()) : nullptr;
	return str? str : "";
}
OBSDataAutoRelease get_event_config(const string& event_id, obs_source_t* source, bool create) {
	OBSDataAutoRelease settings = get_config(source);
	if (!settings) return nullptr;
	OBSDataAutoRelease events = obs_data_get_obj(settings, "events");
	if (!events) {
		events = obs_data_create();
		obs_data_set_obj(settings, "events", events);
	}
	OBSDataAutoRelease event =  obs_data_get_obj(events, event_id.c_str());
	if (!event && create) {
		event = obs_data_create();
		obs_data_set_obj(events, event_id.c_str(), event);
	}
	return event;
}
bool get_event_bool(const string& event_id, const string& key, bool default_value, obs_source_t* source) {
	OBSDataAutoRelease event = get_event_config(event_id);
	return event? obs_data_get_bool(event, key.c_str()) : default_value;
}
string get_event_string(const string& event_id, const string& key, const string& default_value, obs_source_t* source) {
	OBSDataAutoRelease event = get_event_config(event_id);
	const char* str = event? obs_data_get_string(event, key.c_str()) : nullptr;
	return str? str : default_value;
}
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
