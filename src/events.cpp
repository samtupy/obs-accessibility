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
#include <obs.h>
#include <obs-frontend-api.h>
#include "audio.h"
#include "config.h"
#include "events.h"
#include "speech.h"
#include "text.h"

using namespace std;

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
	string event_id = obs_frontend_event_id(event);
	if (event_id.empty()) return;
	play(event_id);
	string text = _t(event_id, nullptr);
	if (text.empty()) return;
	speak(text, false);
}
void on_signal(void* param, const char* signal_name, calldata_t* data) {
	if (!g_receive_events) return;
	play(signal_name);
	string text = _t(signal_name, data);
	if (!text.empty()) speak(text, false);
}
void init_events() {
	signal_handler_t* core_handler = obs_get_signal_handler();
	signal_handler_connect_global(core_handler, on_signal, nullptr);
	obs_frontend_add_event_callback(on_event, nullptr);
}
void shutdown_events() {
	signal_handler_t* core_handler = obs_get_signal_handler();
	signal_handler_disconnect_global(core_handler, on_signal, nullptr);
	obs_frontend_remove_event_callback(on_event, nullptr);
}
