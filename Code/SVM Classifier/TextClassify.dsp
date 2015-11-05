# Microsoft Developer Studio Project File - Name="TextClassify" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TextClassify - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TextClassify.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TextClassify.mak" CFG="TextClassify - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TextClassify - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TextClassify - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TextClassify - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "TextClassify - Win32 Release"
# Name "TextClassify - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="C:\Program Files\Microsoft Visual Studio\VC98\MFC\SRC\APPMODUL.CPP"

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BAYES.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CATALOGLIST.CPP

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Classifier.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CLASSIFIERPARAM.CPP

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Compute_Param.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Compute_Prompt.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Compute_Result.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Utility\ContextStat.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Utility\Dictionary.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Segment\DynamicArray.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LeftView.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Message.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Segment\NShortPath.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Segment\Queue.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SegDocDlg.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Segment\SegGraph.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Segment\Segment.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Tag\Span.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Stemmer.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SVM.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SVMParamsPage.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TestParamsDlg.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TextClassify.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TextClassify.rc
# End Source File
# Begin Source File

SOURCE=.\TextClassifyDoc.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TextClassifyView.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TrainParamsPage.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Unknown\UnknowWord.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Utility\Utility.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WORDLIST.CPP

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WordSegment.cpp

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WZDDRDLG.CPP

!IF  "$(CFG)" == "TextClassify - Win32 Release"

!ELSEIF  "$(CFG)" == "TextClassify - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=E:\MFC\Include\AFXTEMPL.H
# End Source File
# Begin Source File

SOURCE=.\BAYES.h
# End Source File
# Begin Source File

SOURCE=.\CATALOGLIST.H
# End Source File
# Begin Source File

SOURCE=.\Classifier.h
# End Source File
# Begin Source File

SOURCE=.\CLASSIFIERPARAM.H
# End Source File
# Begin Source File

SOURCE=.\Compute_Param.h
# End Source File
# Begin Source File

SOURCE=.\Compute_Prompt.h
# End Source File
# Begin Source File

SOURCE=.\Compute_Result.h
# End Source File
# Begin Source File

SOURCE=.\Utility\ContextStat.h
# End Source File
# Begin Source File

SOURCE=.\Utility\Dictionary.h
# End Source File
# Begin Source File

SOURCE=.\Segment\DynamicArray.h
# End Source File
# Begin Source File

SOURCE=.\LeftView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Message.h
# End Source File
# Begin Source File

SOURCE=.\Segment\NShortPath.h
# End Source File
# Begin Source File

SOURCE=.\Segment\Queue.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SegDocDlg.h
# End Source File
# Begin Source File

SOURCE=.\Segment\SegGraph.h
# End Source File
# Begin Source File

SOURCE=.\Segment\Segment.h
# End Source File
# Begin Source File

SOURCE=.\Tag\Span.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Stemmer.h
# End Source File
# Begin Source File

SOURCE=.\SVM.h
# End Source File
# Begin Source File

SOURCE=.\SVMParamsPage.h
# End Source File
# Begin Source File

SOURCE=.\TestParamsDlg.h
# End Source File
# Begin Source File

SOURCE=.\TextClassify.h
# End Source File
# Begin Source File

SOURCE=.\TextClassifyDoc.h
# End Source File
# Begin Source File

SOURCE=.\TextClassifyView.h
# End Source File
# Begin Source File

SOURCE=.\TrainParamsPage.h
# End Source File
# Begin Source File

SOURCE=.\Unknown\UnknowWord.h
# End Source File
# Begin Source File

SOURCE=.\Utility\Utility.h
# End Source File
# Begin Source File

SOURCE=.\WORDLIST.H
# End Source File
# Begin Source File

SOURCE=.\WordSegment.h
# End Source File
# Begin Source File

SOURCE=.\WZDDRDLG.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\TextClassify.ico
# End Source File
# Begin Source File

SOURCE=.\res\TextClassify.rc2
# End Source File
# Begin Source File

SOURCE=.\res\TextClassifyDoc.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
