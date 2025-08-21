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

#include <windows.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <fmt/format.h>
#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QObject>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include "audio.h" // get_audio_event_source()
#include "config.h"
#include "interface.h"
#include "speech.h"
#include "text.h"

using namespace std;
using namespace fmt;

// This file contains all code pertaining strictly to the UI, whether that be adding/handling menu items, dealing with hotkeys, calling into qt or anything similar.

// Hack which somewhat corrects properties dialog focus after a widget redraw.
QPushButton* g_last_default_button = nullptr;
void correct_properties_focus() {
	if (g_last_default_button) g_last_default_button->setFocus();
}

// Experimintal QT event handler and related helper functions which try to make various interfaces more accessible, these are hacks that should really be implemented properly into obs.
qt_event_filter* g_qt_filter = nullptr;
void fix_property_controls(QFormLayout* layout);
void fix_property_editable_list_buttons(QVBoxLayout* buttons) {
	// Editable lists create 5 unlabeled buttons. In order they are add, remove, edit, move up, and move down.
	if (!buttons || buttons->count() < 5) return;
	const char* button_names[5] = {"add", "remove", "edit", "move_up", "move_down"};
	for (int i = 0; i < 5; i++) {
		QPushButton* btn = qobject_cast<QPushButton*>(buttons->itemAt(i)->widget());
		if (btn) btn->setAccessibleName(obs_module_text(button_names[i]));
	}
}
void fix_property_control(QWidget* control, QLayout* layout, QWidget* label_widget = nullptr) {
	QLabel* label = label_widget? qobject_cast<QLabel*>(label_widget) : nullptr;
	if (label_widget && !label) {
		// Usually this means that a label was provided, but a tooltip was provided as well. When this happens the label widget turns into a layout with the actual text we're interested in provided in the first sub widget in this layout.
		QHBoxLayout* lbl_layout = qobject_cast<QHBoxLayout*>(label_widget->layout());
		if (lbl_layout) label = qobject_cast<QLabel*>(lbl_layout->itemAt(0)->widget());
	}
	if (control) {
		QString name = control->accessibleName();
		QGroupBox* group = qobject_cast<QGroupBox*>(control);
		if (group) {
			if (name == "group" && group->title() != "") control->setAccessibleName(group->title());
			QFormLayout* group_layout = qobject_cast<QFormLayout*>(group->layout());
			if (group_layout) fix_property_controls(group_layout);
		}
		else if (name == "" && label) control->setAccessibleName(label->text());
		control->setAccessibleDescription(control->toolTip());
	} else if (layout && layout->count() > 0) {
		QLineEdit* sub_edit = qobject_cast<QLineEdit*>(layout->itemAt(0)->widget());
		QSlider* slider = qobject_cast<QSlider*>(layout->itemAt(0)->widget());
		QSpinBox* spin = qobject_cast<QSpinBox*>(layout->itemAt(layout->count() > 1? 1 : 0)->widget());
		QDoubleSpinBox* dspin = qobject_cast<QDoubleSpinBox*>(layout->itemAt(layout->count() > 1? 1 : 0)->widget());
		QListWidget* list = qobject_cast<QListWidget*>(layout->itemAt(0)->widget());
		if (sub_edit) sub_edit->setAccessibleName(label && label->text() != ""? label->text() : sub_edit->toolTip());
		else if (slider) slider->setAccessibleName(label && label->text() != ""? label->text() : slider->toolTip());
		if (spin) spin->setAccessibleName(label && label->text() != ""? label->text() : spin->toolTip());
		else if (dspin) dspin->setAccessibleName(label && label->text() != ""? label->text() : dspin->toolTip());
		if (list) {
			list->setAccessibleName(label && label->text() != ""? label->text() : list->toolTip());
			if (layout->count() > 1) fix_property_editable_list_buttons(qobject_cast<QVBoxLayout*>(layout->itemAt(1)->layout()));
		}
	}
}
void fix_property_controls(QFormLayout* layout) {
	// Labels all dynamic properties of a dialog created from an obs_properties_t object and insures keyboard focus at least exists. Result is undefined if a QFormLayout that is not created by OBSBasicProperties is used.
	if (!QApplication::focusWidget()) correct_properties_focus();
	for (int i = 0; i < layout->count(); i++) {
		QWidget* w = layout->itemAt(i)->widget();
		QLayout* l = layout->itemAt(i)->layout();
		QWidget* lbl_widget = w? layout->labelForField(w) : layout->labelForField(l);
		fix_property_control(w, l, lbl_widget);
	}
}
void fix_filters_dialog(QDialog* dlg) {	
	// Labels filter list items as well as their visibility checkboxes.
	QListWidget* filters = nullptr;
	QListWidget* async_filters = dlg->findChild<QListWidget*>("asyncFilters");
	if (async_filters) {
		QLabel* lbl = dlg->findChild<QLabel*>("asyncLabel");
		if (lbl) async_filters->setAccessibleName(lbl->text());
		if (async_filters->count() > 0) filters = async_filters;
	}
	QListWidget* effect_filters = dlg->findChild<QListWidget*>("effectFilters");
	if (effect_filters) {
		QLabel* lbl = dlg->findChild<QLabel*>("label_2");
		if (lbl) effect_filters->setAccessibleName(lbl->text());
		if (effect_filters->count() > 0) filters = effect_filters;
	}
	if (!filters) return;
	for (int i = 0; i < filters->count(); i++) {
		QListWidgetItem* filter_item = filters->item(i);
		if (!filters->itemWidget(filter_item)) continue;
		QHBoxLayout* filter_item_layout = qobject_cast<QHBoxLayout*>(filters->itemWidget(filter_item)->layout());
		if (!filter_item_layout || filter_item_layout->count() < 2) continue;
		QCheckBox* filter_toggle = qobject_cast<QCheckBox*>(filter_item_layout->itemAt(0)->widget());
		QLabel* filter_label = qobject_cast<QLabel*>(filter_item_layout->itemAt(1)->widget());
		if (!filter_toggle || !filter_label) continue;
		filter_toggle->setAccessibleName(filter_label->text());
		filter_item->setData(Qt::AccessibleTextRole, filter_label->text());
	}
}
qt_event_filter::qt_event_filter(QObject* parent, bool debug) : QObject(parent), debug(debug) {}
bool qt_event_filter::eventFilter(QObject* watched, QEvent* event) {
	if (event->type() == QEvent::Timer) return false;
	if (debug) {
		speak(QDebug::toString(event).toStdWString());
		return false;
	}
	QWidget* widget = qobject_cast<QWidget*>(watched);
	if (event->type() == QEvent::WindowActivate) {
		QPushButton* btn = qobject_cast<QPushButton*>(widget);
		if (btn && btn->isDefault()) g_last_default_button = btn;
	} else if (event->type() == QEvent::Paint) {
		// Todo: Find a better event that fires far less but still enough.
		QDialog* dlg = qobject_cast<QDialog*>(widget);
		if (dlg && dlg->objectName() == "OBSBasicFilters") fix_filters_dialog(dlg);
	} else if (event->type() == QEvent::ParentChange) {
		if (watched->objectName() == "PropertiesContainer") {
			QFormLayout* layout = qobject_cast<QFormLayout*>(widget->layout());
			if (!layout) return false;
			fix_property_controls(layout);
		}
	}
	return false;
}

void on_global_properties(void* data) {
	event_source_data* src = get_audio_event_source();
	if (!src) return;
	obs_frontend_open_source_properties(src->source);
}
bool on_hk_window_hide(void* data, obs_hotkey_pair_id id, obs_hotkey_t *hotkey, bool pressed) {
	QMainWindow* win = static_cast<QMainWindow*>(obs_frontend_get_main_window());
	if (!pressed || !win || !win->isVisible()) return false;
	if (win->menuBar()->hasFocus()) return false;
	win->showMinimized();
	return true;
}
bool on_hk_window_show(void* data, obs_hotkey_pair_id id, obs_hotkey_t *hotkey, bool pressed) {
	QMainWindow* win = static_cast<QMainWindow*>(obs_frontend_get_main_window());
	if (!pressed || !win || win->isVisible()) return false;
	win->raise();
	win->show();
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
	QMainWindow* win = static_cast<QMainWindow*>(obs_frontend_get_main_window());
	win->menuBar()->setNativeMenuBar(true);
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
