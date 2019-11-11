
Debian
====================
This directory contains files used to package simplicityd/simplicity-qt
for Debian-based Linux systems. If you compile simplicityd/simplicity-qt yourself, there are some useful files here.

## simplicity: URI support ##


simplicity-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install simplicity-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your simplicity-qt binary to `/usr/bin`
and the `../../share/pixmaps/simplicity128.png` to `/usr/share/pixmaps`

simplicity-qt.protocol (KDE)

