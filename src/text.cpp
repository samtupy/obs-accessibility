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

#define FMT_HEADER_ONLY
#include <fmt/base.h>
#include <obs-module.h>
#include "text.h"

using namespace std;
using namespace fmt;

template<typename T> T *GetCalldataPointer(const calldata_t *data, const char *name) {
	// Taken from obs-websocket
	void *ptr = nullptr;
	calldata_get_ptr(data, name, &ptr);
	return static_cast<T*>(ptr);
}
string _t(const string& string_id) {
	const char* result = obs_module_text(string_id.c_str());
	return result? result : string_id;
}
size_t find_unescaped(const string& input, const string& search, size_t start = 0) {
	size_t pos = start;
	while ((pos = input.find(search, pos)) != string::npos) {
		if (pos == 0 || input[pos -1] != '\\') return pos;
		else pos += 1;
	}
	return string::npos;
}
string obs_source_identify(obs_source_t* src) {
	string name = obs_source_get_name(src);
	string id = obs_source_get_id(src);
	return format("{}({})", id, name);
}
string get_obs_source_variable(const string& variable, obs_source_t* src, const string& if_true = "true", const string& if_false = "false") {
	if (!src) return _t("source_invalid");
	if (variable == "name") return obs_source_get_name(src);
	else if (variable == "uuid") return obs_source_get_uuid(src);
	else if (variable == "typeid") return obs_source_get_id(src);
	else if (variable == "typename") return obs_source_get_display_name(obs_source_get_id(src));
	else if (variable == "volume") return format("{}", obs_source_get_volume(src));
	else if (variable == "volume%") return format("{}", int(obs_source_get_volume(src) * 100));
	else if (variable == "balance") return format("{}", obs_source_get_balance_value(src));
	else if (variable == "balance%") return format("{}", int(obs_source_get_balance_value(src) * 100));
	else return format(runtime(_t("object_variable_invalid")), variable, obs_source_identify(src));
}
string get_obs_variables(string variable, const calldata_t* data = nullptr, const string& string_id = "") {
	if (variable == "id") return string_id;
	string if_true = "true", if_false = "false";
	size_t pos = find_unescaped(variable, ":");
	if (pos != string::npos) {
		string conditionals = variable.substr(pos + 1);
		variable.erase(pos);
		pos = find_unescaped(conditionals, ":");
		if (pos == string::npos) pos = conditionals.size();
		if_true = conditionals.substr(0, pos);
		if_false = conditionals.substr(pos + 1);
	}
	if (variable.starts_with("scene.")) {
		obs_source_t* scene = obs_frontend_get_current_scene();
		if (!scene) return format(runtime(_t("scene_invalid")), variable);
		string output = get_obs_source_variable(variable.substr(6), scene);
		obs_source_release(scene);
	} else if (variable == "tbar") return to_string(obs_frontend_get_tbar_position());
	else if (data) {
		if (variable.starts_with("source.")) return get_obs_source_variable(variable.substr(7), GetCalldataPointer<obs_source_t>(data, "source"));
		bool calldata_b = false;
		const char* calldata_s = calldata_string(data, variable.c_str());
		if (calldata_s) return calldata_s;
		else if (calldata_get_bool(data, variable.c_str(), &calldata_b)) return calldata_b? if_true : if_false;
	}
	return format(runtime(_t("variable_invalid")), variable);
}
string _t(const string& string_id, const calldata_t* data) {
	const char* str = obs_module_text(string_id.c_str());
	if (!str || string_id == str) return "";
	string text = str;
	// Replace sequences such as {source.name} with their proper data.
	bool escape_sequence = false;
	size_t brace_level = 0, replacement_start = 0;
	for (size_t i = 0; i < text.size(); i++) {
		char c = text[i];
		if (!escape_sequence && c == '\\') {
			escape_sequence = true;
			continue;
		} else if (escape_sequence) {
			if (c == '{' or c == '}' or c == '\\') {
				i--;
				text.erase(i, 1);
			}
			escape_sequence = false;
			continue;
		} else if (c == '{') {
			if (!brace_level) replacement_start = i;
			brace_level += 1;
		} else if (c == '}') {
			brace_level -= 1;
			if (brace_level) continue;
			text.replace(replacement_start, i - replacement_start + 1, get_obs_variables(text.substr(replacement_start + 1, i - replacement_start -1), data, string_id));
			i = replacement_start -1;
		}
	}
	return text;
}
string obs_frontend_event_id(obs_frontend_event event) {
	switch (event) {
		case OBS_FRONTEND_EVENT_STREAMING_STARTING: return "streaming_starting";
		case OBS_FRONTEND_EVENT_STREAMING_STARTED: return "streaming_started";
		case OBS_FRONTEND_EVENT_STREAMING_STOPPING: return "streaming_stopping";
		case OBS_FRONTEND_EVENT_STREAMING_STOPPED: return "streaming_stopped";
		case OBS_FRONTEND_EVENT_RECORDING_STARTING: return "recording_starting";
		case OBS_FRONTEND_EVENT_RECORDING_STARTED: return "recording_started";
		case OBS_FRONTEND_EVENT_RECORDING_STOPPING: return "recording_stopping";
		case OBS_FRONTEND_EVENT_RECORDING_STOPPED: return "recording_stopped";
		case OBS_FRONTEND_EVENT_SCENE_CHANGED: return "scene_changed";
		case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED: return "scene_list_changed";
		case OBS_FRONTEND_EVENT_TRANSITION_CHANGED: return "transition_changed";
		case OBS_FRONTEND_EVENT_TRANSITION_STOPPED: return "transition_stopped";
		case OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED: return "transition_list_changed";
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED: return "scene_collection_changed";
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED: return "scene_collection_list_changed";
		case OBS_FRONTEND_EVENT_PROFILE_CHANGED: return "profile_changed";
		case OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED: return "profile_list_changed";
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING: return "replay_buffer_starting";
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED: return "replay_buffer_started";
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING: return "replay_buffer_stopping";
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED: return "replay_buffer_stopped";
		case OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED: return "studio_mode_enabled";
		case OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED: return "studio_mode_disabled";
		case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED: return "preview_scene_changed";
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP: return "scene_collection_cleanup";
		case OBS_FRONTEND_EVENT_FINISHED_LOADING: return "finished_loading";
		case OBS_FRONTEND_EVENT_RECORDING_PAUSED: return "recording_paused";
		case OBS_FRONTEND_EVENT_RECORDING_UNPAUSED: return "recording_unpaused";
		case OBS_FRONTEND_EVENT_TRANSITION_DURATION_CHANGED: return "transition_duration_changed";
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED: return "replay_buffer_saved";
		case OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED: return "virtualcam_started";
		case OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED: return "virtualcam_stopped";
		case OBS_FRONTEND_EVENT_TBAR_VALUE_CHANGED: return "tbar_value_changed";
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING: return "scene_collection_changing";
		case OBS_FRONTEND_EVENT_PROFILE_CHANGING: return "profile_changing";
		case OBS_FRONTEND_EVENT_PROFILE_RENAMED: return "profile_renamed";
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_RENAMED: return "scene_collection_renamed";
		case OBS_FRONTEND_EVENT_THEME_CHANGED: return "theme_changed";
		case OBS_FRONTEND_EVENT_SCREENSHOT_TAKEN: return "screenshot_taken";
		case OBS_FRONTEND_EVENT_CANVAS_ADDED: return "canvas_added";
		case OBS_FRONTEND_EVENT_CANVAS_REMOVED: return "canvas_removed";
		default: return "";
	}
}
