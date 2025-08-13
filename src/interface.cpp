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
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QDebug>
#include <QObject>
#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QListWidget>
#include <QWidget>
#include "audio.h" // get_audio_event_source()
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
void init_interface() {
	obs_frontend_add_tools_menu_item(obs_module_text("props.show"), on_global_properties, nullptr);
	QCoreApplication* app = QCoreApplication::instance();
	g_qt_filter = new qt_event_filter(app);
	app->installEventFilter(g_qt_filter);
}
void shutdown_interface() {
	QCoreApplication* app = QCoreApplication::instance();
	app->removeEventFilter(g_qt_filter);
	delete g_qt_filter;
}
