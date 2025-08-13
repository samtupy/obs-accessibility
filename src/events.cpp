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

#include <exception>
#include <string>
#include <unordered_map>
#include <fmt/format.h>
#include <obs.h>
#include <obs-frontend-api.h>
#include <obs-source.h>
#include "audio.h"
#include "config.h"
#include "events.h"
#include "speech.h"
#include "text.h"

using namespace std;
using namespace fmt;

unordered_map<obs_frontend_event, event_type*> g_frontend_event_types;
unordered_map<string, event_type*> g_event_types;
event_type::event_type(obs_frontend_event event, const std::string& id) : id(id), has_event(true), event(event) {
	g_event_types[id] = this;
	g_frontend_event_types[event] = this;
}
event_type::event_type(const std::string& id, const std::string& primary_data) : id(id), has_event(false), primary_data(primary_data) {
	g_event_types[id] = this;
}
string event_type::get_id() const { return id; }
string event_type::get_name() const { return _t(id + ".name", id); }
string event_type::get_description() const { return _t(id + ".description", ""); }
string event_type::get_default_message() const { return _t(id + ".message", ""); }
string event_type::describe() const {
	string desc = get_description();
	if (!desc.empty()) desc = _t("column_join", "; ") + desc;
	return get_name() + desc;
}
bool event_type::get_muted(obs_source_t* event_source) const { return get_event_bool(id, "muted", false, event_source); }
string event_type::get_message(obs_source_t* event_source) const { return get_event_string(id, "message", get_default_message(), event_source); }
bool event_type::is_frontend_event() const { return has_event; }
bool event_type::is_signal() const { return !has_event; }
obs_frontend_event event_type::get_frontend_event() const {
	if (!has_event) throw runtime_error(format("{} is not a frontend event", id));
	return event;
}
string event_type::get_primary_data() const {
	if (has_event) throw runtime_error(format("{} is not a signal", id));
	return primary_data;
}
event_type* get_event_type(obs_frontend_event event) {
	if (!g_frontend_event_types.contains(event)) return nullptr;
	return g_frontend_event_types[event];
}
string get_event_type_id(obs_frontend_event event) {
	event_type* e = get_event_type(event);
	return e? e->get_id() : "";
}
event_type* get_event_type(const string& id) {
	if (!g_event_types.contains(id)) return nullptr;
	return g_event_types[id];
}
void get_event_types(vector<string>& out_events) {
	out_events.reserve(g_event_types.size());
	for (auto i : g_event_types) out_events.push_back(i.first);
}
void register_event_types() {
	new event_type(OBS_FRONTEND_EVENT_STREAMING_STARTING, "streaming_starting");
	new event_type(OBS_FRONTEND_EVENT_STREAMING_STARTED, "streaming_started");
	new event_type(OBS_FRONTEND_EVENT_STREAMING_STOPPING, "streaming_stopping");
	new event_type(OBS_FRONTEND_EVENT_STREAMING_STOPPED, "streaming_stopped");
	new event_type(OBS_FRONTEND_EVENT_RECORDING_STARTING, "recording_starting");
	new event_type(OBS_FRONTEND_EVENT_RECORDING_STARTED, "recording_started");
	new event_type(OBS_FRONTEND_EVENT_RECORDING_STOPPING, "recording_stopping");
	new event_type(OBS_FRONTEND_EVENT_RECORDING_STOPPED, "recording_stopped");
	new event_type(OBS_FRONTEND_EVENT_SCENE_CHANGED, "scene_changed");
	new event_type(OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED, "scene_list_changed");
	new event_type(OBS_FRONTEND_EVENT_TRANSITION_CHANGED, "transition_changed");
	new event_type(OBS_FRONTEND_EVENT_TRANSITION_STOPPED, "transition_stopped");
	new event_type(OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED, "transition_list_changed");
	new event_type(OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED, "scene_collection_changed");
	new event_type(OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED, "scene_collection_list_changed");
	new event_type(OBS_FRONTEND_EVENT_PROFILE_CHANGED, "profile_changed");
	new event_type(OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED, "profile_list_changed");
	new event_type(OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING, "replay_buffer_starting");
	new event_type(OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED, "replay_buffer_started");
	new event_type(OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING, "replay_buffer_stopping");
	new event_type(OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED, "replay_buffer_stopped");
	new event_type(OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED, "studio_mode_enabled");
	new event_type(OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED, "studio_mode_disabled");
	new event_type(OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED, "preview_scene_changed");
	new event_type(OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP, "scene_collection_cleanup");
	new event_type(OBS_FRONTEND_EVENT_FINISHED_LOADING, "finished_loading");
	new event_type(OBS_FRONTEND_EVENT_RECORDING_PAUSED, "recording_paused");
	new event_type(OBS_FRONTEND_EVENT_RECORDING_UNPAUSED, "recording_unpaused");
	new event_type(OBS_FRONTEND_EVENT_TRANSITION_DURATION_CHANGED, "transition_duration_changed");
	new event_type(OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED, "replay_buffer_saved");
	new event_type(OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED, "virtualcam_started");
	new event_type(OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED, "virtualcam_stopped");
	new event_type(OBS_FRONTEND_EVENT_TBAR_VALUE_CHANGED, "tbar_value_changed");
	new event_type(OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING, "scene_collection_changing");
	new event_type(OBS_FRONTEND_EVENT_PROFILE_CHANGING, "profile_changing");
	new event_type(OBS_FRONTEND_EVENT_PROFILE_RENAMED, "profile_renamed");
	new event_type(OBS_FRONTEND_EVENT_SCENE_COLLECTION_RENAMED, "scene_collection_renamed");
	new event_type(OBS_FRONTEND_EVENT_THEME_CHANGED, "theme_changed");
	new event_type(OBS_FRONTEND_EVENT_SCREENSHOT_TAKEN, "screenshot_taken");
	new event_type(OBS_FRONTEND_EVENT_CANVAS_ADDED, "canvas_added");
	new event_type(OBS_FRONTEND_EVENT_CANVAS_REMOVED, "canvas_removed");
	new event_type("source_create", "source");
	new event_type("source_destroy", "source");
	new event_type("source_remove", "source");
	new event_type("source_update", "source");
	new event_type("source_save", "source");
	new event_type("source_load", "source");
	new event_type("source_activate", "source");
	new event_type("source_deactivate", "source");
	new event_type("source_show", "source");
	new event_type("source_hide", "source");
	new event_type("source_rename", "source");
	new event_type("source_volume", "source");
	new event_type("source_audio_activate", "source");
	new event_type("source_audio_deactivate", "source");
	new event_type("source_filter_add", "source");
	new event_type("source_filter_remove", "source");
	new event_type("source_transition_start", "source");
	new event_type("source_transition_video_stop", "source");
	new event_type("source_transition_stop", "source");
	new event_type("channel_change", "source");
	new event_type("channel_change", "source");
	new event_type("hotkey_layout_change", "");
	new event_type("hotkey_register", "hotkey");
	new event_type("hotkey_unregister", "hotkey");
	new event_type("hotkey_bindings_changed", "hotkey");
	new event_type("canvas_create", "canvas");
	new event_type("canvas_remove", "canvas");
	new event_type("canvas_destroy", "canvas");
	new event_type("canvas_video_reset", "canvas");
	new event_type("canvas_rename", "canvas");
	new event_type("video_reset", "");
}
void unregister_event_types() {
	for (auto i : g_event_types) delete i.second;
	g_event_types.clear();
	g_frontend_event_types.clear();
}

bool g_receive_events = false; // Set to true when program finishes loading, false when we start to exit.
void on_event(obs_frontend_event event, void* user) {
	switch (event) {
		case OBS_FRONTEND_EVENT_FINISHED_LOADING:
			g_receive_events = true;
			break;
		case OBS_FRONTEND_EVENT_EXIT:
		case OBS_FRONTEND_EVENT_SCRIPTING_SHUTDOWN:
			g_receive_events = false;
			save_config();
			shutdown_audio();
			break;
		default:
			break;
	}
	if (!g_receive_events) return;
	event_type* event_obj = get_event_type(event);
	if (!event_obj) return;
	play(event_obj->get_id());
	if (!get_property_bool("speech")) return;
	string text = replace_obs_variables(event_obj->get_message(), nullptr);
	if (text.empty()) return;
	speak(text, get_property_bool("speech_interrupt"));
}
void on_signal(void* param, const char* signal_name, calldata_t* data) {
	if (!g_receive_events) return;
	event_type* event_obj = get_event_type(signal_name);
	if (!event_obj) return;
	play(signal_name);
	if (!get_property_bool("speech")) return;
	string text = replace_obs_variables(event_obj->get_message(), data);
	if (text.empty()) return;
	speak(text, get_property_bool("speech_interrupt"));
}
void init_events() {
	register_event_types();
	signal_handler_t* core_handler = obs_get_signal_handler();
	signal_handler_connect_global(core_handler, on_signal, nullptr);
	obs_frontend_add_event_callback(on_event, nullptr);
}
void shutdown_events() {
	signal_handler_t* core_handler = obs_get_signal_handler();
	signal_handler_disconnect_global(core_handler, on_signal, nullptr);
	obs_frontend_remove_event_callback(on_event, nullptr);
	unregister_event_types();
}
