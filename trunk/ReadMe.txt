*** FFRend 2 source distribution ***

REQUIREMENTS:

FFRend 2 is intended to build under .NET 2008. It builds cleanly
under MFC 6, but this is not recommended for released versions.
FFRend has mainly been tested under XP SP2, but it should also 
work fine under Vista and Windows 7.

You'll need the following files from the DirectX 8.1 SDK:

ddraw.h
ddrawex.h
ddraw.lib
dxguid.lib

The December 2004 SDK contains the correct version of DirectDraw.
FFRend may not build properly against later versions of the SDK.

You'll also need the following files from HTML help:

htmlhelp.h
htmlhelp.lib

And these files from the zlib compression library, version 1.2.3:

zconf.h
zlib.h
zlib.lib

You may need to adjust the location of dependencies from DirectDraw,
zlib and BmpToAvi in the project settings, or in the IDE options.

INSTALLATION:

The source distribution of FFRend doesn't have an installer.  FFRend
expects the following files to be in the same folder as FFRend.exe:

FFRend.chm
BmpToAvi.dll
PlayerFF.dll
MetaFFRend.dll

Note that to use FFRend's movie export feature, you must register the
BmpToAvi DirectShow source filter, which is included in this release.
The filter only needs to be registered once, via the following command
(assuming BmpToAvi.ax is in the current folder):

regsvr32 BmpToAvi.ax

If you have installed Whorld (http://whorld.org), BmpToAvi is already
registered and you can skip this step.

UNINSTALLING:

To uninstall FFRend 2, you need to do three things:

1.  Delete FFRend's program files from the folder you copied them to.
2.  Delete FFRend's registry entries, using regedit.  The key to
delete is:

"HKEY_CURRENT_USER\Software\Anal Software\FFRend2"

3.  If you registered the BmpToAvi filter, unregister it, via the 
following command:

regsvr32 /u BmpToAvi.ax

NOTES:

The binary version of this release is available from the FFRend web
site, http://ffrend.sourceforge.net/.

LICENSE:

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.  This program is distributed in the hope that
it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.  You should have
received a copy of the GNU General Public License along with this
program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111 USA.

END
