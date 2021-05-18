
Debian
====================
This directory contains files used to package orod/oro-qt
for Debian-based Linux systems. If you compile orod/oro-qt yourself, there are some useful files here.

## oro: URI support ##


oro-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install oro-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your oro-qt binary to `/usr/bin`
and the `../../share/pixmaps/oro128.png` to `/usr/share/pixmaps`

oro-qt.protocol (KDE)

