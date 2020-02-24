
Debian
====================
This directory contains files used to package AVCd/AVC-qt
for Debian-based Linux systems. If you compile AVCd/AVC-qt yourself, there are some useful files here.

## AVC: URI support ##


AVC-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install AVC-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your AVC-qt binary to `/usr/bin`
and the `../../share/pixmaps/AVC128.png` to `/usr/share/pixmaps`

AVC-qt.protocol (KDE)

