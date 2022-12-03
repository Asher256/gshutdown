* Website: http://github.com/Asher256/gshutdown
* Authors: See AUTHORS file.

## Description:

GShutdown is an advanced shutdown utility which allows you to schedule 
the shutdown, the restart of your computer of the logout.

## Features:

* Shutdown, reboot and logout: immediately or scheduled
* No need to be root
* Possibility to choose custom commands for the actions
* Three different ways to schedule the action
* Systray icon
* Visuals notifications

## Compilation:
```
apt-get install libgtk2.0-dev libglib2.0-dev libnotify-dev	
./configure && make
sudo make install
```

## License:

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

(read COPYING file)

## Requirements, to compile the source code:

* Gtk+ 2.0   http://www.gtk.org
* libnotify  http://www.galago-project.org/files/releases/source
* libglade   http://glade.gnome.org/

In Ubuntu, you can install the dependencies with:
```
sudo apt-get install libgtk2.0-dev libglade2-dev libnotify-dev
```

In Fedora Core, you can install it with:
```
yum install gtk2-devel libglade2-devel libnotify-devel
```

In Fedora Core, if your version of libnotify is too old, you
can compile the source code without it:
```
./configure --enable-libnotify=no
```

## Tips:

Middle click on the systray icon opens the immediate action menu.

