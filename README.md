# OBS Accessibility plugin

Binaries available on the  [releases page](https://github.com/samtupy/obs-accessibility/releases)

## Introduction

This is a plugin for [OBS](https://obsproject.com) that adds audio queues, screen reader announcements and other accessibility features to the software.

This plugin is currently only available for windows, I may add support for other platforms later if requested and if possible.

### Features

* Comprehensive audio and speech event feedback system which can notify you about over 70 different events.
* The vast mejority of widgets in all source or filter property dialogs are labeled.
* Filters are named in the source filters list.
* Pressing a button in a source properties dialog no longer requires refocusing the window to regain keyboard navigation.
* Configurable hotkey pair to hide and unhide the OBS main window.

## building

This project uses CMake and VCPKG. VCPKG is provided via submodule, CMake is expected to be on your path.

Clone the repository:
```git clone --recursive https://github.com/samtupy/obs-accessibility```

Configure CMake:
```cmake --preset windows-x64```

Build:
```cmake --build --preset windows-x64```

Optional - install to OBS program data if on system (make sure OBS is not running):
```cmake --install build_x64 --config RelWithDebInfo```

Optional - install to a custom directory (replace dirname with your path):
```cmake --install build_x64 --config RelWithDebInfo --prefix dirname```

## Event feedback

This plugin can provide sound or text feedback for over 70 different events or signals from the OBS application. These could be anything from recording starting pausing or stopping, to sources being shown/hidden, to many other different notifications OBS gives developers.

Not all events have sounds by default, you can add sounds from their names in the data/earcon folder, or you can customize the path sounds are searched from in the accessibility settings found under the tools menu. We use miniaudio for sound playback, so the supported formats are currently .wav, .flac and .mp3 with plans for .ogg and .opus soon.

While by default the audio produced by this plugin is heard through the configured monitoring device in OBS, it is also possible to add the accessibility events as a source in your scene, and set a different sound path / mute different sets of events for each source. While not useful to most, it was simple to add and could be useful if one wants certain accessibility events to be heard on their streams.

Similar to the audio, most events do not have a speech message by default. Default event messages are stored in the data/locale folder of the plugin. However, you can set the messages in the accessibility settings interface as well.

### Dynamic event message content
Many times you may want some piece of information to be spoken during an event notification. For example if you wish to hear a message when a source is muted, you might want to know the name of the source that is muted. For this reason, event messages are passed through a tiny template engine allowing you to insert dynamic content into them. ```{source.name} muted``` for example.

Dynamic variable names are placed within braces. If you wish to actually insert a brace into the message, put a backslash (\) before it. If you want to retrieve a variable in an object, the syntax ```{object.variable}``` or ```{object.subobject.variable}``` will function at any depth in the object tree.

The list of available variables is growing, this is what we have available so far:
* id: The event or string ID
* scene: Object representing the current scene.
* tbar: Transition bar position.
* source: Object representing the source that this event relates to.
* filter: Object representing the filter source that this event relates to.

Other variables may be available per event, as some attempts are made to include some data provided by OBS which this plugin does not manage.

The following variables are available in source objects:
* name: The name of the source.
* uuid: The UUID of the source, most times not worth printing.
* typeid: The ID of the source type E. All audio monitor filters will have the same ID.
* typename: The display name for this sources type.
* volume: Sources volume between 0 and 1.
* volume%: Sources volume between 0 and 100.
* balance: Sources balance between 0 and 1.
* balance%: Sources balance between 0 and 100.

## Window visibility hotkey

After this plugin is installed, you can visit the OBS hotkey settings and type "obs window" into the filter box to locate the plugin's hotkey pair to minimize/restore the OBS main window. Particularly when "always minimize to system tray instead of task bar" is checked in the OBS general settings, this is a great way to keep OBS invisible while being able to bring it back and make a tweak exactly when you need. The feature might need a bit of improvement when the window is set to minimize to task bar instead of tray.

## User Interface Tweaks

This plugin automatically hooks into the QT Widgets event stream when it is installed, and attempts to make certain dialogs more accessible. Many times OBS does provide labels for things, but they aren't linked to the associated control in a way that screen readers can see them correctly. QT provides an accessibility API though, so in some cases when we know where labels for certain controls are, we can set the accessible names and/or descriptions for those controls, thus labeling them hopefully without altering the visual interface. At some point I hope to submit some of the interface fixes this plugin performs as pull requests to the OBS program itself.

Filters and Scenes are actually just types of sources, and all sources use a common API to display their properties. Without this plugin, almost all controls save checkboxes and buttons are unlabeled, but once the plugin is installed there will be labels for the vast mejority of control types in all source property dialogs. Previously if you pressed a button in a source properties dialog which caused it's widgets to redraw, the keyboard focus would be lost and you would need to leave and reenter the window sometimes twice to get it back. Once this plugin is installed, you will be automatically focused on the default button in the properties dialog if the widgets redraw and keyboard focus is lost.

The filters dialog for sources was previously not showing any filter name labels to screen readers, including the filter list items and the enable/disable checkboxes. This plugin resolves that.

## Goals

Some of my future hopes for this plugin include:
* Either fixes to the existing hotkey settings panel, or a custom one.
* Volume meter events, allowing sound or speech notifications when volume of incoming audio changes.
* Potentially look into making custom browser docks properly accessible.
* Simple tone playback as well as sound files to make defining different audio per event a bit easier.
* Many more template variables for event speech messages.
* Possibly an optional way to hide source and filter visibility and lock checkboxes to be replaced with local hotkey bindings within the sources list.
* Invisible interface allowing manipulation of sources and basic controls from any window.
* Various hotkey registrations to report status information.
* More efficient way of fixing QT dialogs that don't end up getting fixed in OBS itself, the event filter method we are using now can get a bit redundant.

## Feedback, contributions, etc

If you know of more areas within OBS that are not fully accessible or if you've discovered a bug with the plugin, please don't hesitate to open an issue with details. If you wish to contribute code, pull requests are welcome and appreciated!
