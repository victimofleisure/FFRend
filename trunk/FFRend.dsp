# Microsoft Developer Studio Project File - Name="FFRend" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=FFRend - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FFRend.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FFRend.mak" CFG="FFRend - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FFRend - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "FFRend - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "FFRend - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "FFRend - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "FFRend - Win32 ReleaseDbgInfo" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FFRend - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\Fractice\BmpToAvi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 shlwapi.lib imagehlp.lib version.lib ddraw.lib dxguid.lib winmm.lib vfw32.lib htmlhelp.lib zlib.lib ..\..\..\Fractice\BmpToAvi\BmpToAvi.lib minizip.lib wininet.lib /nologo /subsystem:windows /map /machine:I386 /nodefaultlib:"libc"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\..\..\Fractice\BmpToAvi\BmpToAvi.dll Release	copy ..\..\Plugins\PlayerFF\Release\PlayerFF.dll Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "FFRend - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\Fractice\BmpToAvi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 shlwapi.lib imagehlp.lib version.lib ddraw.lib dxguid.lib winmm.lib vfw32.lib htmlhelp.lib zlib.lib minizip.lib wininet.lib ..\..\..\Fractice\BmpToAvi\BmpToAviDebug.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"msvcrt" /nodefaultlib:"libc" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\..\..\Fractice\BmpToAvi\BmpToAviDebug.dll Debug	copy ..\..\Plugins\PlayerFF\Release\PlayerFF.dll Debug
# End Special Build Tool

!ELSEIF  "$(CFG)" == "FFRend - Win32 Unicode Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "FFRend___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "FFRend___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "UnicodeRelease"
# PROP Intermediate_Dir "UnicodeRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\Fractice\BmpToAvi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib shlwapi.lib imagehlp.lib version.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 shlwapi.lib imagehlp.lib version.lib ddraw.lib dxguid.lib winmm.lib vfw32.lib htmlhelp.lib zlib.lib ..\..\..\Fractice\BmpToAvi\BmpToAviU.lib minizip.lib wininet.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /map /machine:I386
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\..\..\Fractice\BmpToAvi\BmpToAviU.dll UnicodeRelease	copy ..\..\Plugins\PlayerFF\Release\PlayerFF.dll UnicodeRelease
# End Special Build Tool

!ELSEIF  "$(CFG)" == "FFRend - Win32 Unicode Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "FFRend___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "FFRend___Win32_Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "UnicodeDebug"
# PROP Intermediate_Dir "UnicodeDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\Fractice\BmpToAvi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib shlwapi.lib imagehlp.lib version.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 shlwapi.lib imagehlp.lib version.lib ddraw.lib dxguid.lib winmm.lib vfw32.lib htmlhelp.lib zlib.lib ..\..\..\Fractice\BmpToAvi\BmpToAviUDebug.lib minizip.lib wininet.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"msvcrt" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\..\..\Fractice\BmpToAvi\BmpToAviUDebug.dll UnicodeDebug	copy ..\..\Plugins\PlayerFF\Release\PlayerFF.dll UnicodeDebug
# End Special Build Tool

!ELSEIF  "$(CFG)" == "FFRend - Win32 ReleaseDbgInfo"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "FFRend___Win32_ReleaseDbgInfo"
# PROP BASE Intermediate_Dir "FFRend___Win32_ReleaseDbgInfo"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDbgInfo"
# PROP Intermediate_Dir "ReleaseDbgInfo"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\..\Fractice\BmpToAvi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\..\Fractice\BmpToAvi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 shlwapi.lib imagehlp.lib version.lib ddraw.lib dxguid.lib winmm.lib htmlhelp.lib ..\..\..\Fractice\BmpToAvi\BmpToAvi.lib /nologo /subsystem:windows /map /debug /machine:I386
# ADD LINK32 shlwapi.lib imagehlp.lib version.lib ddraw.lib dxguid.lib winmm.lib vfw32.lib htmlhelp.lib zlib.lib ..\..\..\Fractice\BmpToAvi\BmpToAvi.lib minizip.lib wininet.lib /nologo /subsystem:windows /map /debug /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\..\..\Fractice\BmpToAvi\BmpToAvi.dll ReleaseDbgInfo	copy ..\..\Plugins\PlayerFF\Release\PlayerFF.dll ReleaseDbgInfo
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "FFRend - Win32 Release"
# Name "FFRend - Win32 Debug"
# Name "FFRend - Win32 Unicode Release"
# Name "FFRend - Win32 Unicode Debug"
# Name "FFRend - Win32 ReleaseDbgInfo"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\AccelTable.cpp
# End Source File
# Begin Source File

SOURCE=.\AccelTable.h
# End Source File
# Begin Source File

SOURCE=.\AdvancedEngineOptsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AdvancedEngineOptsDlg.h
# End Source File
# Begin Source File

SOURCE=.\ArrayEx.h
# End Source File
# Begin Source File

SOURCE=.\AviFileErrs.h
# End Source File
# Begin Source File

SOURCE=.\AviToBmp.cpp
# End Source File
# Begin Source File

SOURCE=.\AviToBmp.h
# End Source File
# Begin Source File

SOURCE=.\BackBufDD.cpp
# End Source File
# Begin Source File

SOURCE=.\BackBufDD.h
# End Source File
# Begin Source File

SOURCE=.\Benchmark.cpp
# End Source File
# Begin Source File

SOURCE=.\Benchmark.h
# End Source File
# Begin Source File

SOURCE=.\BitmapInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapInfo.h
# End Source File
# Begin Source File

SOURCE=.\Caption.cpp
# End Source File
# Begin Source File

SOURCE=.\Caption.h
# End Source File
# Begin Source File

SOURCE=.\ClickSliderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ClickSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Clipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\Clipboard.h
# End Source File
# Begin Source File

SOURCE=.\ClipEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\ClipEngine.h
# End Source File
# Begin Source File

SOURCE=.\CritSec.h
# End Source File
# Begin Source File

SOURCE=.\CtrlResize.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlResize.h
# End Source File
# Begin Source File

SOURCE=.\CustomMsgBox.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomMsgBox.h
# End Source File
# Begin Source File

SOURCE=.\Dib.cpp
# End Source File
# Begin Source File

SOURCE=.\Dib.h
# End Source File
# Begin Source File

SOURCE=.\DirDrawErrs.h
# End Source File
# Begin Source File

SOURCE=.\DirList.cpp
# End Source File
# Begin Source File

SOURCE=.\DirList.h
# End Source File
# Begin Source File

SOURCE=.\DragRowView.cpp
# End Source File
# Begin Source File

SOURCE=.\DragRowView.h
# End Source File
# Begin Source File

SOURCE=.\DragTabCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DragTabCtrl.h
# End Source File
# Begin Source File

SOURCE=.\DragVirtualListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DragVirtualListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\EditSliderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\EditSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\EmbDllInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\EmbDllInfo.h
# End Source File
# Begin Source File

SOURCE=.\Engine.cpp
# End Source File
# Begin Source File

SOURCE=.\Engine.h
# End Source File
# Begin Source File

SOURCE=.\EngineErrs.h
# End Source File
# Begin Source File

SOURCE=.\EngineOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\EngineOptions.h
# End Source File
# Begin Source File

SOURCE=.\EngineTap.cpp
# End Source File
# Begin Source File

SOURCE=.\EngineTap.h
# End Source File
# Begin Source File

SOURCE=.\EngineThread.cpp
# End Source File
# Begin Source File

SOURCE=.\EngineThread.h
# End Source File
# Begin Source File

SOURCE=.\EngineTypes.h
# End Source File
# Begin Source File

SOURCE=.\Event.cpp
# End Source File
# Begin Source File

SOURCE=.\Event.h
# End Source File
# Begin Source File

SOURCE=.\ExtractThumb.cpp
# End Source File
# Begin Source File

SOURCE=.\ExtractThumb.h
# End Source File
# Begin Source File

SOURCE=.\FFEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\FFEngine.h
# End Source File
# Begin Source File

SOURCE=.\FFEngineErrs.h
# End Source File
# Begin Source File

SOURCE=.\FFInstance.cpp
# End Source File
# Begin Source File

SOURCE=.\FFInstance.h
# End Source File
# Begin Source File

SOURCE=.\FFParm.cpp
# End Source File
# Begin Source File

SOURCE=.\FFParm.h
# End Source File
# Begin Source File

SOURCE=.\FFPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\FFPlugin.h
# End Source File
# Begin Source File

SOURCE=.\FFPluginEx.cpp
# End Source File
# Begin Source File

SOURCE=.\FFPluginEx.h
# End Source File
# Begin Source File

SOURCE=.\FFPlugInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\FFPlugInfo.h
# End Source File
# Begin Source File

SOURCE=.\FFPlugsRow.cpp
# End Source File
# Begin Source File

SOURCE=.\FFPlugsRow.h
# End Source File
# Begin Source File

SOURCE=.\FFProject.cpp
# End Source File
# Begin Source File

SOURCE=.\FFProject.h
# End Source File
# Begin Source File

SOURCE=.\FFRend.cpp
# End Source File
# Begin Source File

SOURCE=.\FFRend.h
# End Source File
# Begin Source File

SOURCE=.\FFRend.rc
# End Source File
# Begin Source File

SOURCE=.\FFRendDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\FFRendDoc.h
# End Source File
# Begin Source File

SOURCE=.\FFRendView.cpp
# End Source File
# Begin Source File

SOURCE=.\FFRendView.h
# End Source File
# Begin Source File

SOURCE=.\FileBrowserBar.cpp
# End Source File
# Begin Source File

SOURCE=.\FileBrowserBar.h
# End Source File
# Begin Source File

SOURCE=.\FileBrowserFF.cpp
# End Source File
# Begin Source File

SOURCE=.\FileBrowserFF.h
# End Source File
# Begin Source File

SOURCE=.\FileBrowserListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\FileBrowserListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\FileInfoCache.cpp
# End Source File
# Begin Source File

SOURCE=.\FileInfoCache.h
# End Source File
# Begin Source File

SOURCE=.\FileSearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FileSearchDlg.h
# End Source File
# Begin Source File

SOURCE=.\FocusCheckbox.cpp
# End Source File
# Begin Source File

SOURCE=.\FocusCheckbox.h
# End Source File
# Begin Source File

SOURCE=.\FocusEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\FocusEdit.h
# End Source File
# Begin Source File

SOURCE=.\FolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderDialog.h
# End Source File
# Begin Source File

SOURCE=.\FrameQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\FrameQueue.h
# End Source File
# Begin Source File

SOURCE=.\FrameSizeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\FrameSizeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\FreeFrame.h
# End Source File
# Begin Source File

SOURCE=.\Globals.h
# End Source File
# Begin Source File

SOURCE=.\GraphBar.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphBar.h
# End Source File
# Begin Source File

SOURCE=.\GraphView.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphView.h
# End Source File
# Begin Source File

SOURCE=.\HistoryBar.cpp
# End Source File
# Begin Source File

SOURCE=.\HistoryBar.h
# End Source File
# Begin Source File

SOURCE=.\HistoryInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\HistoryInfo.h
# End Source File
# Begin Source File

SOURCE=.\HistoryView.cpp
# End Source File
# Begin Source File

SOURCE=.\HistoryView.h
# End Source File
# Begin Source File

SOURCE=.\HookMsgBox.cpp
# End Source File
# Begin Source File

SOURCE=.\HookMsgBox.h
# End Source File
# Begin Source File

SOURCE=.\HttpDownload.cpp
# End Source File
# Begin Source File

SOURCE=.\HttpDownload.h
# End Source File
# Begin Source File

SOURCE=.\HttpSession.cpp
# End Source File
# Begin Source File

SOURCE=.\HttpSession.h
# End Source File
# Begin Source File

SOURCE=.\Hyperlink.cpp
# End Source File
# Begin Source File

SOURCE=.\Hyperlink.h
# End Source File
# Begin Source File

SOURCE=.\IconButton.cpp
# End Source File
# Begin Source File

SOURCE=.\IconButton.h
# End Source File
# Begin Source File

SOURCE=.\ILockRingBuf.h
# End Source File
# Begin Source File

SOURCE=.\JobControlDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\JobControlDlg.h
# End Source File
# Begin Source File

SOURCE=.\JobInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\JobInfo.h
# End Source File
# Begin Source File

SOURCE=.\JobList.cpp
# End Source File
# Begin Source File

SOURCE=.\JobList.h
# End Source File
# Begin Source File

SOURCE=.\LoadBalanceBar.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadBalanceBar.h
# End Source File
# Begin Source File

SOURCE=.\LoadBalanceRow.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadBalanceRow.h
# End Source File
# Begin Source File

SOURCE=.\MainEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\MainEngine.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MapEx.h
# End Source File
# Begin Source File

SOURCE=.\MasterBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MasterBar.h
# End Source File
# Begin Source File

SOURCE=.\MasterDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MasterDlg.h
# End Source File
# Begin Source File

SOURCE=.\Metaparm.cpp
# End Source File
# Begin Source File

SOURCE=.\Metaparm.h
# End Source File
# Begin Source File

SOURCE=.\MetaparmArray.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaparmArray.h
# End Source File
# Begin Source File

SOURCE=.\MetaparmBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaparmBar.h
# End Source File
# Begin Source File

SOURCE=.\MetaparmGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaparmGroup.h
# End Source File
# Begin Source File

SOURCE=.\MetaparmGroupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaparmGroupDlg.h
# End Source File
# Begin Source File

SOURCE=.\MetaparmPropsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaparmPropsDlg.h
# End Source File
# Begin Source File

SOURCE=.\MetaparmRow.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaparmRow.h
# End Source File
# Begin Source File

SOURCE=.\MetaplugDefaultsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaplugDefaultsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Metaplugin.cpp
# End Source File
# Begin Source File

SOURCE=.\Metaplugin.h
# End Source File
# Begin Source File

SOURCE=.\MetaplugPropsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MetaplugPropsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Metaproject.cpp
# End Source File
# Begin Source File

SOURCE=.\Metaproject.h
# End Source File
# Begin Source File

SOURCE=.\MidiEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiEngine.h
# End Source File
# Begin Source File

SOURCE=.\MidiInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiInfo.h
# End Source File
# Begin Source File

SOURCE=.\MidiIO.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiIO.h
# End Source File
# Begin Source File

SOURCE=.\MidiSetupBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiSetupBar.h
# End Source File
# Begin Source File

SOURCE=.\MidiSetupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiSetupDlg.h
# End Source File
# Begin Source File

SOURCE=.\MidiSetupRow.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiSetupRow.h
# End Source File
# Begin Source File

SOURCE=.\MidiTypes.h
# End Source File
# Begin Source File

SOURCE=.\Minizip.cpp
# End Source File
# Begin Source File

SOURCE=.\Minizip.h
# End Source File
# Begin Source File

SOURCE=.\MissingFilesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MissingFilesDlg.h
# End Source File
# Begin Source File

SOURCE=.\MonitorBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MonitorBar.h
# End Source File
# Begin Source File

SOURCE=.\MonitorInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\MonitorInfo.h
# End Source File
# Begin Source File

SOURCE=.\MonitorWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MonitorWnd.h
# End Source File
# Begin Source File

SOURCE=.\MsgBoxDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgBoxDlg.h
# End Source File
# Begin Source File

SOURCE=.\MultiFileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\MySizingControlBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MySizingControlBar.h
# End Source File
# Begin Source File

SOURCE=.\NoteEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteEdit.h
# End Source File
# Begin Source File

SOURCE=.\NumEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\NumEdit.h
# End Source File
# Begin Source File

SOURCE=.\NumSpin.cpp
# End Source File
# Begin Source File

SOURCE=.\NumSpin.h
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptionsInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsInfo.h
# End Source File
# Begin Source File

SOURCE=.\OptsEngineDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsEngineDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsMidiDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsMidiDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsViewDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsViewDlg.h
# End Source File
# Begin Source File

SOURCE=.\Oscillator.cpp
# End Source File
# Begin Source File

SOURCE=.\Oscillator.h
# End Source File
# Begin Source File

SOURCE=.\ParaPET.h
# End Source File
# Begin Source File

SOURCE=.\PatchBar.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchBar.h
# End Source File
# Begin Source File

SOURCE=.\PatchRow.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchRow.h
# End Source File
# Begin Source File

SOURCE=.\PathStr.cpp
# End Source File
# Begin Source File

SOURCE=.\PathStr.h
# End Source File
# Begin Source File

SOURCE=.\PercentProgressCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\PercentProgressCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Persist.cpp
# End Source File
# Begin Source File

SOURCE=.\Persist.h
# End Source File
# Begin Source File

SOURCE=.\PersistDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistDlg.h
# End Source File
# Begin Source File

SOURCE=.\Picture.cpp
# End Source File
# Begin Source File

SOURCE=.\Picture.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistDlg.h
# End Source File
# Begin Source File

SOURCE=.\Plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\Plugin.h
# End Source File
# Begin Source File

SOURCE=.\PluginHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginHelper.h
# End Source File
# Begin Source File

SOURCE=.\PreviewBar.cpp
# End Source File
# Begin Source File

SOURCE=.\PreviewBar.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\QueueBar.cpp
# End Source File
# Begin Source File

SOURCE=.\QueueBar.h
# End Source File
# Begin Source File

SOURCE=.\QueueView.cpp
# End Source File
# Begin Source File

SOURCE=.\QueueView.h
# End Source File
# Begin Source File

SOURCE=.\RandList.cpp
# End Source File
# Begin Source File

SOURCE=.\RandList.h
# End Source File
# Begin Source File

SOURCE=.\RecordDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RecordDlg.h
# End Source File
# Begin Source File

SOURCE=.\Recorder.cpp
# End Source File
# Begin Source File

SOURCE=.\Recorder.h
# End Source File
# Begin Source File

SOURCE=.\RecordInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\RecordInfo.h
# End Source File
# Begin Source File

SOURCE=.\RecStatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RecStatDlg.h
# End Source File
# Begin Source File

SOURCE=.\RefPtr.h
# End Source File
# Begin Source File

SOURCE=.\Renderer.cpp
# End Source File
# Begin Source File

SOURCE=.\Renderer.h
# End Source File
# Begin Source File

SOURCE=.\ReplaceFilesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ReplaceFilesDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\RingBuf.h
# End Source File
# Begin Source File

SOURCE=.\RowDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RowDlg.h
# End Source File
# Begin Source File

SOURCE=.\RowForm.cpp
# End Source File
# Begin Source File

SOURCE=.\RowForm.h
# End Source File
# Begin Source File

SOURCE=.\RowView.cpp
# End Source File
# Begin Source File

SOURCE=.\RowView.h
# End Source File
# Begin Source File

SOURCE=.\RunAvg.cpp
# End Source File
# Begin Source File

SOURCE=.\RunAvg.h
# End Source File
# Begin Source File

SOURCE=.\scbarg.cpp
# End Source File
# Begin Source File

SOURCE=.\scbarg.h
# End Source File
# Begin Source File

SOURCE=.\SelectSliderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\sizecbar.cpp
# End Source File
# Begin Source File

SOURCE=.\sizecbar.h
# End Source File
# Begin Source File

SOURCE=.\SizingDockFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\SizingDockFrame.h
# End Source File
# Begin Source File

SOURCE=.\SliderSelectionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SliderSelectionDlg.h
# End Source File
# Begin Source File

SOURCE=.\SmartBuf.cpp
# End Source File
# Begin Source File

SOURCE=.\SmartBuf.h
# End Source File
# Begin Source File

SOURCE=.\SmartPtr.h
# End Source File
# Begin Source File

SOURCE=.\Spider.cpp
# End Source File
# Begin Source File

SOURCE=.\Spider.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SubFileFind.cpp
# End Source File
# Begin Source File

SOURCE=.\SubFileFind.h
# End Source File
# Begin Source File

SOURCE=.\TabCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=.\TabCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\Thumb.cpp
# End Source File
# Begin Source File

SOURCE=.\Thumb.h
# End Source File
# Begin Source File

SOURCE=.\ThumbCache.cpp
# End Source File
# Begin Source File

SOURCE=.\ThumbCache.h
# End Source File
# Begin Source File

SOURCE=.\ThumbThread.cpp
# End Source File
# Begin Source File

SOURCE=.\ThumbThread.h
# End Source File
# Begin Source File

SOURCE=.\Timer.cpp
# End Source File
# Begin Source File

SOURCE=.\Timer.h
# End Source File
# Begin Source File

SOURCE=.\ToolDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolDlg.h
# End Source File
# Begin Source File

SOURCE=.\Undoable.cpp
# End Source File
# Begin Source File

SOURCE=.\Undoable.h
# End Source File
# Begin Source File

SOURCE=.\UndoCodeData.h
# End Source File
# Begin Source File

SOURCE=.\UndoCodes.h
# End Source File
# Begin Source File

SOURCE=.\UndoManager.cpp
# End Source File
# Begin Source File

SOURCE=.\UndoManager.h
# End Source File
# Begin Source File

SOURCE=.\UndoState.cpp
# End Source File
# Begin Source File

SOURCE=.\UndoState.h
# End Source File
# Begin Source File

SOURCE=.\UndoTest.cpp
# End Source File
# Begin Source File

SOURCE=.\UndoTest.h
# End Source File
# Begin Source File

SOURCE=.\UpdateCheck.cpp
# End Source File
# Begin Source File

SOURCE=.\UpdateCheck.h
# End Source File
# Begin Source File

SOURCE=.\Version.h
# End Source File
# Begin Source File

SOURCE=.\VersionInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\VersionInfo.h
# End Source File
# Begin Source File

SOURCE=.\VideoComprState.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoComprState.h
# End Source File
# Begin Source File

SOURCE=.\Win32Console.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32Console.h
# End Source File
# Begin Source File

SOURCE=.\WinAppEx.cpp
# End Source File
# Begin Source File

SOURCE=.\WinAppEx.h
# End Source File
# Begin Source File

SOURCE=.\WndTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\WndTimer.h
# End Source File
# Begin Source File

SOURCE=.\WObject.h
# End Source File
# Begin Source File

SOURCE=.\WorkerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\WorkerThread.h
# End Source File
# Begin Source File

SOURCE=.\ZDeflate.cpp
# End Source File
# Begin Source File

SOURCE=.\ZDeflate.h
# End Source File
# Begin Source File

SOURCE=.\ZInflate.cpp
# End Source File
# Begin Source File

SOURCE=.\ZInflate.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\dragmulti.cur
# End Source File
# Begin Source File

SOURCE=.\res\dragsingle.cur
# End Source File
# Begin Source File

SOURCE=.\res\ff_hdr_play.ico
# End Source File
# Begin Source File

SOURCE=.\res\FFRend.ico
# End Source File
# Begin Source File

SOURCE=.\res\FFRend.rc2
# End Source File
# Begin Source File

SOURCE=.\res\FFRendDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\loopd.ico
# End Source File
# Begin Source File

SOURCE=.\res\loopu.ico
# End Source File
# Begin Source File

SOURCE=.\res\monitor.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_loopd.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_loopu.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_normal.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_playd.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_playing.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_playu.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_shuffled.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_shuffleu.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_stopped.ico
# End Source File
# Begin Source File

SOURCE=.\res\playd.ico
# End Source File
# Begin Source File

SOURCE=.\res\playu.ico
# End Source File
# Begin Source File

SOURCE=.\res\sort_down.ico
# End Source File
# Begin Source File

SOURCE=.\res\sort_up.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# End Target
# End Project
