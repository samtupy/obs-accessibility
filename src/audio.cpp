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

#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS //MA_NO_DEVICE is broken right now
#define MA_NO_ENCODING
#include <vector>
#include <obs.h>
#include <obs-module.h>
#include <obs-source.h>
#include "audio.h"

using namespace std;

const char* g_audio_extensions[] = {".wav", ".flac", ".ogg", ".mp3", nullptr};
vector<event_source_data*> g_audio_event_sources;
event_source_data* g_audio_event_source = nullptr; // We specifically manage a hidden, global source.
static void* event_source_thread(void* user) {
	event_source_data* d = (event_source_data*)user;
	uint64_t ts = 0;
	uint64_t last_time = os_gettime_ns();
	float buffer[960];
	while (os_event_try(d->event) == EAGAIN) {
		if (!os_sleepto_ns(last_time += 10000000)) last_time = os_gettime_ns();
		ma_uint64 frames_read;
		ma_engine_read_pcm_frames(&*d->engine, &buffer, 480, &frames_read);
		struct obs_source_audio data;
		data.data[0] = (uint8_t*)buffer;
		data.frames = 480;
		data.speakers = SPEAKERS_STEREO;
		data.samples_per_sec = 48000;
		data.timestamp = ts;
		data.format = AUDIO_FORMAT_FLOAT;
		obs_source_output_audio(d->source, &data);
		ts += 10000000;
	}
	return nullptr;
}
static const char* event_source_getname(void *unused) {
	UNUSED_PARAMETER(unused);
	return "Accessibility Audio Events";
}
static void event_source_destroy(void* data) {
	event_source_data* d = (event_source_data*)data;
	if (!d) return;
	if (d->initialized_thread) {
		void *ret;
		os_event_signal(d->event);
		pthread_join(d->thread, &ret);
	}
	if (d->event) os_event_destroy(d->event);
	if (d->engine) ma_engine_uninit(&*d->engine);
	auto it = find(g_audio_event_sources.begin(), g_audio_event_sources.end(), d);
	if (it != g_audio_event_sources.end()) g_audio_event_sources.erase(it);
	delete d;
}
static void* event_source_create(obs_data_t* settings, obs_source_t* source) {
	event_source_data* d = new event_source_data();
	ma_engine_config cfg = ma_engine_config_init();
	cfg.noDevice   = MA_TRUE;
	cfg.channels   = 2;
	cfg.sampleRate = 48000;
	d->engine = make_unique<ma_engine>();
	if (ma_engine_init(&cfg, &*d->engine) != MA_SUCCESS) {
		d->engine.reset();
		goto fail;
	}
	d->source = source;
	if (os_event_init(&d->event, OS_EVENT_TYPE_MANUAL) != 0) goto fail;
	if (pthread_create(&d->thread, NULL, event_source_thread, d) != 0) goto fail;
	d->initialized_thread = true;
	d->global_events = false;
	g_audio_event_sources.push_back(d);
	return d;
	fail:
		event_source_destroy(d);
		return nullptr;
}
obs_source_info event_source = {
	.id = "accessibility_event_audio",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_AUDIO | OBS_SOURCE_DO_NOT_DUPLICATE | OBS_SOURCE_MONITOR_BY_DEFAULT,
	.get_name = event_source_getname,
	.create = event_source_create,
	.destroy = event_source_destroy,
};
bool init_audio(obs_data_t* settings) {
	if (g_audio_event_source) return true; // audio already initialized.
	obs_register_source(&event_source);
	obs_source_t* src = nullptr;
	if (settings) {	
		obs_data_set_bool(settings, "global_events", true);
		src = obs_load_private_source(settings);
	}
	if (!src) {
		src = obs_source_create_private("accessibility_event_audio", "Accessibility", nullptr);
		if (!src) return false;
		obs_source_set_monitoring_type(src, OBS_MONITORING_TYPE_MONITOR_ONLY);
	}
	g_audio_event_source = g_audio_event_sources.back();
	g_audio_event_source->global_events = true;
	obs_source_inc_active(src);
	obs_source_inc_showing(src);
	return true;
}
void shutdown_audio() {
	// We only shut down our global source and trust that OBS did whatever was needed for any that were user created.
	if (!g_audio_event_source) return;
	obs_source_dec_showing(g_audio_event_source->source);
	obs_source_dec_active(g_audio_event_source->source);
	obs_source_remove(g_audio_event_source->source);
	obs_source_release(g_audio_event_source->source);
	g_audio_event_source = nullptr;
}
bool play(const string& earcon) {
	bool success = false;
	for (const char** extension = g_audio_extensions; *extension; ++extension) {
		char* file = obs_module_file(("earcon/"s + earcon + *extension).c_str());
		if (!file) continue;
		for (auto s : g_audio_event_sources)
			success = ma_engine_play_sound(&*s->engine, file, nullptr) == MA_SUCCESS;
		bfree(file);
	}
	return success;
}
event_source_data* get_audio_event_source() { return g_audio_event_source; }
