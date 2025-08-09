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
#include <miniaudio.h>
#include <obs.h>
#include <util/threading.h>
#include <util/platform.h>
#include <memory>
#include <string>

// Defines custom data required for our event audio delivery and configuration source to function.
struct event_source_data {
	bool global_events;
	bool initialized_thread;
	pthread_t thread;
	os_event_t *event;
	obs_source_t *source;
	std::unique_ptr<ma_engine> engine;
};

bool init_audio(obs_data_t* settings = nullptr);
void shutdown_audio();
bool play(const std::string& earcon);
event_source_data* get_audio_event_source();
