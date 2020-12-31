# Microsoft Developer Studio Project File - Name="win32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=win32 - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "win32.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "win32.mak" CFG="win32 - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "win32 - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "win32 - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "M:\080\win32\Release"
# PROP Intermediate_Dir "M:\080\win32\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "sdl" /I "..\psp" /I "..\com" /I "..\core\nes" /I "..\core\nes\debug" /I "..\core\nes\apu" /I "..\core\nes\ppu" /I "..\core\nes\cpu" /I "..\core\nes\libsnss" /I "..\core\sms" /I "..\libs\zlib" /I "..\libs\libzip" /FI"types.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /libpath:".\sdl"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "M:\080\win32\debug"
# PROP Intermediate_Dir "M:\080\win32\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /I "sdl" /I "..\psp" /I "..\com" /I "..\core\nes" /I "..\core\nes\debug" /I "..\core\nes\apu" /I "..\core\nes\ppu" /I "..\core\nes\cpu" /I "..\core\nes\libsnss" /I "..\core\sms" /I "..\libs\zlib" /I "..\libs\libzip" /I "..\core\meka\srcs" /I "..\core\meka\srcs\sound" /I "..\core\meka\srcs\sound\emu2413" /I "..\core\meka\srcs\z80marat" /FI"types.h" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PSPSDK" /D "HAVE_CONFIG_H" /D "_AFXDLL" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /profile /map /debug /machine:I386 /libpath:".\sdl"

!ENDIF 

# Begin Target

# Name "win32 - Win32 Release"
# Name "win32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\lib\comctl.c
# End Source File
# Begin Source File

SOURCE=.\font.c
# End Source File
# Begin Source File

SOURCE=.\hal.c
# End Source File
# Begin Source File

SOURCE=.\hal_file.c
# End Source File
# Begin Source File

SOURCE=.\hal_sound.c
# End Source File
# Begin Source File

SOURCE=.\mp3.c
# End Source File
# Begin Source File

SOURCE=.\sceWrapper.c
# End Source File
# Begin Source File

SOURCE=.\sceWrapper.h
# End Source File
# Begin Source File

SOURCE=.\SDL_framerate.c
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\win32.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "core"

# PROP Default_Filter ""
# Begin Group "WS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\swan\initval.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ws"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ws"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\swan\nec.c
# End Source File
# Begin Source File

SOURCE=..\core\swan\nec.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ws"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ws"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\swan\necea.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ws"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ws"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\swan\necinstr.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ws"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ws"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\swan\necintrf.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ws"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ws"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\swan\necmodrm.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ws"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ws"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\swan\rom.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ws"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ws"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\swan\ws.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ws"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ws"
# SUBTRACT CPP /I "sdl" /I "..\psp" /I "..\core\nes" /I "..\core\nes\debug" /I "..\core\nes\apu" /I "..\core\nes\ppu" /I "..\core\nes\cpu" /I "..\core\nes\libsnss" /I "..\core\sms" /I "..\libs\zlib" /I "..\libs\libzip" /I "..\core\meka\srcs" /I "..\core\meka\srcs\sound" /I "..\core\meka\srcs\sound\emu2413" /I "..\core\meka\srcs\z80marat"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\swan\ws.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ws"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ws"

!ENDIF 

# End Source File
# End Group
# Begin Group "nes"

# PROP Default_Filter ""
# Begin Group "apu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\nes\apu\2413tone.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\emutypes.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\exsound_tbl.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\fdsplugin.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\fdssnd.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\fdssnd.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\fmtone.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_apu.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_apu.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_apu_wrapper.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_apu_wrapper.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_exsound.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_extsound.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_fds.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_fme7.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_fme7.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_mmc5.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_mmc5.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_n106.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_n106.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_vrc6.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_vrc6.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_vrc7.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\nes_vrc7.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\apu\vrc7_tbl.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# End Group
# Begin Group "cpu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\nes\cpu\nes6502.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\cpu\nes6502.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\cpu\nes_6502.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\cpu\nes_6502.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# End Group
# Begin Group "ppu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\nes\ppu\nes_ppu.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\ppu\nes_ppu.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# End Group
# Begin Group "debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\nes\debug\debug.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# End Group
# Begin Group "libsnss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\nes\libsnss\libsnss.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\libsnss\libsnss.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\core\nes\nes.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_config.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_config.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_crc32.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_crc32.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_mapper.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_mapper.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_pad.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_pal.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_rom.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nes_rom.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\nintendulator_pal.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\snss.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\nes\snss.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\nes"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\nes"

!ENDIF 

# End Source File
# End Group
# Begin Group "ngp"

# PROP Default_Filter ""
# Begin Group "z80"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\ngp\z80\Codes.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\z80\CodesCB.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\z80\CodesED.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\z80\CodesXCB.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\z80\CodesXX.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\z80\Tables.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\z80\Z80.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\z80\Z80n.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# End Group
# Begin Group "tlcs-900h"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\core\ngp\tlcs-900h\interpret.c"

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\core\ngp\tlcs-900h\interpret.h"

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\core\ngp\bios.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\bios.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\biosHLE.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\dma.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\dma.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\flash.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\flash.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\gfx.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\gfx.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\gfx_scanline_colour.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\gfx_scanline_mono.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\interrupt.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\interrupt.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\mem.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\mem.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\neopop.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\neopop.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\rom.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\sound.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\sound.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\state.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\Z80_interface.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\ngp\Z80_interface.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\ngp"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\ngp"

!ENDIF 

# End Source File
# End Group
# Begin Group "pce"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\pce\cd.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"
# SUBTRACT CPP /I "sdl" /I "..\psp" /I "..\core\nes" /I "..\core\nes\debug" /I "..\core\nes\apu" /I "..\core\nes\ppu" /I "..\core\nes\cpu" /I "..\core\nes\libsnss" /I "..\core\sms" /I "..\libs\zlib" /I "..\libs\libzip" /I "..\core\meka\srcs" /I "..\core\meka\srcs\sound" /I "..\core\meka\srcs\sound\emu2413" /I "..\core\meka\srcs\z80marat"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\pce\cd_dat.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\pce\Codes.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\pce\Hucodes.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\pce\Hutables.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\pce\m6502.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\pce\m6502.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\pce\pce.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\pce\pce.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\pce\pceregs.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\pce"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\pce"

!ENDIF 

# End Source File
# End Group
# Begin Group "sms"

# PROP Default_Filter ""
# Begin Group "z80g"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\sms\cpu\cpuintrf.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\cpu\osd_cpu.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\cpu\z80.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\cpu\z80.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\cpu\z80daa.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# End Group
# Begin Group "sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\sms\sound\2413tone.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\sound\emu2413.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\sound\emu2413.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\sound\sn76496.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\sound\sn76496.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\sound\vrc7tone.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\core\sms\render.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\render.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\shared.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\sms.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\sms.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\system.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\system.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\vdp.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\sms\vdp.h

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\sms"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\sms"

!ENDIF 

# End Source File
# End Group
# Begin Group "gb"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\gb\cheat.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\gb"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\gb"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\gb\cpu.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\gb"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\gb"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\gb\gb.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\gb"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\gb"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\gb\gb_apu.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\gb"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\gb"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\gb\gbrom.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\gb"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\gb"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\gb\lcd.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\gb"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\gb"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\gb\mbc.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\gb"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\gb"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\core\gb\sgb.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\gb"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\gb"

!ENDIF 

# End Source File
# End Group
# Begin Group "lynx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\core\lynx\C6502mak.h
# End Source File
# Begin Source File

SOURCE=..\core\lynx\C65c02.c
# End Source File
# Begin Source File

SOURCE=..\core\lynx\C65c02.h
# End Source File
# Begin Source File

SOURCE=..\core\lynx\LYNXDEF.H
# End Source File
# Begin Source File

SOURCE=..\core\lynx\System.c
# End Source File
# Begin Source File

SOURCE=..\core\lynx\System.h
# End Source File
# End Group
# End Group
# Begin Group "com"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\com\common.c
# End Source File
# Begin Source File

SOURCE=..\com\common.h
# End Source File
# Begin Source File

SOURCE=..\com\cstring.c
# End Source File
# Begin Source File

SOURCE=..\com\cstring.h
# End Source File
# Begin Source File

SOURCE=..\com\file_io.c
# End Source File
# Begin Source File

SOURCE=..\com\hal.h
# End Source File
# Begin Source File

SOURCE=..\com\types.h
# End Source File
# End Group
# Begin Group "libs"

# PROP Default_Filter ""
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\libs\zlib\adler32.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\compress.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\crc32.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\deflate.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\gzio.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\infback.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\inffast.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\inflate.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\inftrees.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\trees.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\uncompr.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\zlib\zutil.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\zlib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\zlib"

!ENDIF 

# End Source File
# End Group
# Begin Group "libzip"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\libs\libzip\ioapi.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libzip"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libzip"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libzip\unzip.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libzip"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libzip"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libzip\zlibFileMemory.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libzip"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libzip"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libzip\zlibInterface.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libzip"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libzip"

!ENDIF 

# End Source File
# End Group
# Begin Group "libmad"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\libs\libmad\bit.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\decoder.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\fixed.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\frame.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\huffman.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\layer12.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\layer3.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\stream.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\synth.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\timer.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\libmad\version.c

!IF  "$(CFG)" == "win32 - Win32 Release"

# PROP Intermediate_Dir "D:\080\win32\Release\libmad"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

# PROP Intermediate_Dir "m:\080\win32\debug\libmad"

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
