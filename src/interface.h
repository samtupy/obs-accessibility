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
#include <QTCore/QEvent>
#include <QTCore/QObject>
class QListWidgetItem;

class qt_event_filter : public QObject {
	Q_OBJECT
	bool debug;
public:
	qt_event_filter(QObject* parent, bool debug = false);
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
};

void init_interface();
void shutdown_interface();
