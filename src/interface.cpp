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

#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QDebug>
#include <QObject>
#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QWidget>
#include "audio.h" // get_audio_event_source()
#include "config.h"
#include "interface.h"
#include "speech.h"

using namespace std;

// This file contains all code pertaining strictly to the UI, whether that be adding/handling menu items, dealing with hotkeys, calling into qt or anything similar.

// Hack which somewhat corrects properties dialog focus after a widget redraw.
QPushButton* g_last_default_button;
void correct_properties_focus() {
	g_last_default_button->setFocus();
}

// Experimintal QT event handler which tries to make various interfaces more accessible, these are hacks that should really be implemented properly into obs.
qt_event_filter::qt_event_filter(QObject* parent) : QObject(parent) {}
bool qt_event_filter::eventFilter(QObject* watched, QEvent* event) {
	QWidget* widget = qobject_cast<QWidget*>(watched);
	if (event->type() == QEvent::WindowActivate) {
		QPushButton* btn = qobject_cast<QPushButton*>(widget);
		if (btn && btn->isDefault()) g_last_default_button = btn;
		if (watched->objectName() == "PropertiesContainer") {
			QFormLayout* layout = qobject_cast<QFormLayout*>(widget->layout());
			if (!layout) return false;
			for (int i = 0; i < layout->count(); i++) {
				QWidget* w = layout->itemAt(i)->widget();
				if (!w) continue;
				QString name = w->accessibleName();
				QGroupBox* group = qobject_cast<QGroupBox*>(w);
				if (group && name == "group" && group->title() != "") w->setAccessibleName(group->title());
			}
		}
	}
	return false;
}
qt_event_filter* g_qt_filter = nullptr;

void on_global_properties(void* data) {
	event_source_data* src = get_audio_event_source();
	if (!src) return;
	obs_frontend_open_source_properties(src->source);
}
bool obs_window_visible() {
		QMainWindow* win = static_cast<QMainWindow*>(obs_frontend_get_main_window());
		return win? win->isVisible() : false;
}
bool on_hk_window_hide(void* data, obs_hotkey_pair_id id, obs_hotkey_t *hotkey, bool pressed) {
	if (!pressed) return obs_window_visible();
	QMainWindow* win = static_cast<QMainWindow*>(obs_frontend_get_main_window());
	if (win) win->showMinimized();
	return false;
}
bool on_hk_window_show(void* data, obs_hotkey_pair_id id, obs_hotkey_t *hotkey, bool pressed) {
	QMainWindow* win = static_cast<QMainWindow*>(obs_frontend_get_main_window());
	if (!pressed || !win) return obs_window_visible();
	win->raise();
	win->activateWindow();
	win->setWindowState((win->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
	return true;
}
void on_hotkey_bindings_changed(void* param, calldata_t* data) {
	// Save hotkey binding changes for our global custom keys, is this the best way to do it, maybe make even the global keys use the source system instead?
	obs_hotkey_t* hotkey = static_cast<obs_hotkey_t*>(calldata_ptr(data, "key"));
	if (!hotkey) return;
	string hk_name = obs_hotkey_get_name(hotkey);
	if (hk_name == "window_show" || hk_name == "window_hide") {
		OBSDataAutoRelease hotkeys = get_hotkeys_config();
		if (!hotkeys) return;
		OBSDataArrayAutoRelease binding = obs_hotkey_save(obs_hotkey_get_id(hotkey));
		if (binding) obs_data_set_array(hotkeys, obs_hotkey_get_name(hotkey), binding);
		else obs_data_erase(hotkeys, obs_hotkey_get_name(hotkey));
	}
}
void init_interface() {
	obs_frontend_add_tools_menu_item(obs_module_text("props.show"), on_global_properties, nullptr);
	obs_hotkey_pair_id hk_window_vis = obs_hotkey_pair_register_frontend("window_hide", obs_module_text("hk_window_hide"), "window_show", obs_module_text("hk_window_show"), on_hk_window_hide, on_hk_window_show, nullptr, nullptr);
	OBSDataAutoRelease hotkeys = get_hotkeys_config();
	if (hotkeys) {
		OBSDataArrayAutoRelease b0 = obs_data_get_array(hotkeys, "window_hide"), b1 = obs_data_get_array(hotkeys, "window_show");
		obs_hotkey_pair_load(hk_window_vis, b0, b1);
	}
	signal_handler_t* core_handler = obs_get_signal_handler();
	signal_handler_connect(core_handler, "hotkey_bindings_changed", on_hotkey_bindings_changed, nullptr);
	QCoreApplication* app = QCoreApplication::instance();
	g_qt_filter = new qt_event_filter(app);
	app->installEventFilter(g_qt_filter);
}
void shutdown_interface() {
	signal_handler_t* core_handler = obs_get_signal_handler();
	signal_handler_disconnect(core_handler, "hotkey_bindings_changed", on_hotkey_bindings_changed, nullptr);
	QCoreApplication* app = QCoreApplication::instance();
	app->removeEventFilter(g_qt_filter);
	delete g_qt_filter;
}
