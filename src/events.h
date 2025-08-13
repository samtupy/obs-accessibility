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
#include <string>
#include <vector>
#include <obs-frontend-api.h>
#include <obs-source.h>

// Describes a frontend event or signal we can listen for.
class event_type {
	std::string id;
	bool has_event;
	obs_frontend_event event;
	std::string primary_data;
public:
	event_type(obs_frontend_event event, const std::string& id);
	event_type(const std::string& id, const std::string& primary_data);
	std::string get_id() const;
	std::string get_name() const; // translated id.name
	std::string get_description() const; // translated id.description
	std::string get_default_message() const; // translated id.message
	std::string describe() const; // translated id.name; id.description
	bool get_muted(obs_source_t* event_source = nullptr) const; // Returns true if user has muted earcons for this event.
	std::string get_message(obs_source_t* event_source = nullptr) const; // Gets either the configured or default spoken message for this event.
	bool is_frontend_event() const;
	bool is_signal() const;
	obs_frontend_event get_frontend_event() const; // Throws exception if not a frontend event.
	std::string get_primary_data() const; // Throws exception if no primary signal data.
};
event_type* get_event_type(obs_frontend_event event);
std::string get_event_type_id(obs_frontend_event event);
event_type* get_event_type(const std::string& id);
void get_event_types(std::vector<std::string>& out_events);

void init_events(); // Registers event types and listeners, call once on module load.
void shutdown_events(); // Disconnects event listeners, call once on module unload.
