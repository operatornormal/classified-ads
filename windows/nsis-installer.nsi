#
# inteneded to be used with nullsoft NSIS tool.
#
# this file mostly stolen from 
# http://sourceforge.net/p/qtpfsgui/extra/ci/master/tree/NSIS/nsis-installer.nsi
# so thanks to all great guys doing Luminance HDR :)
#
# define name of installer
outFile "Classified-ads-Win32.exe"
!define MUI_ICON "turt-transparent-128x128.ico"
!define VERSION "0.09" 
# ask to be admin in order to create start menu shortcuts to all users
RequestExecutionLevel admin
Name "Classified ads ${VERSION}" 
# define installation directory
installDir $PROGRAMFILES\Classified-ads
# start default section
section
SetShellVarContext all
# set the installation directory as the destination for the following actions
setOutPath $INSTDIR
# create the uninstaller
writeUninstaller "$INSTDIR\uninstall.exe"
# create a shortcut named "new shortcut" in the start menu programs directory
# point the new shortcut at the program uninstaller
CreateDirectory "$SMPROGRAMS\Classified-ads"
CreateShortCut "$SMPROGRAMS\Classified-ads\uninstall.lnk" "$INSTDIR\uninstall.exe"
file /oname=classified-ads.exe ..\release\classified-ads.exe
CreateDirectory "$INSTDIR\fi\LC_MESSAGES"
file /oname=$INSTDIR\fi\LC_MESSAGES\classified-ads.mo ..\po\fi.mo
CreateDirectory "$INSTDIR\sv\LC_MESSAGES"
file /oname=$INSTDIR\sv\LC_MESSAGES\classified-ads.mo ..\po\sv.mo
CreateDirectory "$INSTDIR\da\LC_MESSAGES"
file /oname=$INSTDIR\da\LC_MESSAGES\classified-ads.mo ..\po\da.mo
file /oname=qt_ca.qm ..\release\translations\qt_ca.qm
file /oname=qt_cs.qm ..\release\translations\qt_cs.qm
file /oname=qt_de.qm ..\release\translations\qt_de.qm
file /oname=qt_fi.qm ..\release\translations\qt_fi.qm
file /oname=qt_hu.qm ..\release\translations\qt_hu.qm
file /oname=qt_it.qm ..\release\translations\qt_it.qm
file /oname=qt_ja.qm ..\release\translations\qt_ja.qm
file /oname=qt_lv.qm ..\release\translations\qt_lv.qm
file /oname=qt_ru.qm ..\release\translations\qt_ru.qm
file /oname=qt_sk.qm ..\release\translations\qt_sk.qm
file /oname=qt_uk.qm ..\release\translations\qt_uk.qm
CreateDirectory "$INSTDIR\bearer"
CreateDirectory "$INSTDIR\iconengines"
CreateDirectory "$INSTDIR\imageformats"
CreateDirectory "$INSTDIR\printsupport"
CreateDirectory "$INSTDIR\platforms"
CreateDirectory "$INSTDIR\sqldrivers"
file /oname=miniupnpc.dll ..\..\miniupnpc-1.9\miniupnpc.dll
CreateShortCut "$SMPROGRAMS\Classified-ads\Classified-ads.lnk" "$INSTDIR\Classified-ads.exe"
file /oname=D3Dcompiler_41.dll deps\D3Dcompiler_41.dll
file /oname=libEGL.dll deps\libEGL.dll
file /oname=libGLESV2.dll deps\libGLESV2.dll
file /oname=libgcc_s_dw2-1.dll deps\libgcc_s_dw2-1.dll
file /oname=libstdc++-6.dll deps\libstdc++-6.dll
file /oname=libwinpthread-1.dll deps\libwinpthread-1.dll
file /oname=Qt5Core.dll deps\Qt5Core.dll
file /oname=Qt5Gui.dll deps\Qt5Gui.dll
file /oname=Qt5Network.dll deps\Qt5Network.dll
file /oname=Qt5PrintSupport.dll deps\Qt5PrintSupport.dll
file /oname=Qt5Sql.dll deps\Qt5Sql.dll
file /oname=Qt5Svg.dll deps\Qt5Svg.dll
file /oname=Qt5Widgets.dll deps\Qt5Widgets.dll
file /oname=bearer\qgenericbearer.dll ..\release\bearer\qgenericbearer.dll
file /oname=bearer\qnativewifibearer.dll ..\release\bearer\qnativewifibearer.dll
file /oname=iconengines\qsvgicon.dll ..\release\iconengines\qsvgicon.dll
file /oname=imageformats\qdds.dll ..\release\imageformats\qdds.dll
file /oname=imageformats\qgif.dll ..\release\imageformats\qgif.dll
file /oname=imageformats\qicns.dll ..\release\imageformats\qicns.dll
file /oname=imageformats\qico.dll ..\release\imageformats\qico.dll
file /oname=imageformats\qjp2.dll ..\release\imageformats\qjp2.dll
file /oname=imageformats\qjpeg.dll ..\release\imageformats\qjpeg.dll
file /oname=imageformats\qmng.dll ..\release\imageformats\qmng.dll
file /oname=imageformats\qsvg.dll ..\release\imageformats\qsvg.dll
file /oname=imageformats\qtga.dll ..\release\imageformats\qtga.dll
file /oname=imageformats\qtiff.dll ..\release\imageformats\qtiff.dll
file /oname=imageformats\qwbmp.dll ..\release\imageformats\qwbmp.dll
file /oname=imageformats\qwebp.dll ..\release\imageformats\qwebp.dll
file /oname=platforms\qwindows.dll ..\release\platforms\qwindows.dll
file /oname=printsupport\windowsprintersupport.dll ..\release\printsupport\windowsprintersupport.dll
file /oname=sqldrivers\qsqlite.dll ..\release\sqldrivers\qsqlite.dll
file /oname=libeay32.dll ..\release\libeay32.dll
file /oname=ssleay32.dll ..\release\ssleay32.dll
file /oname=libiconv-2.dll ..\release\libiconv-2.dll
file /oname=libintl-8.dll ..\release\libintl-8.dll
file /oname=LICENSE ..\LICENSE
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Classified-ads" "DisplayName" "Classified-ads (remove only)"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Classified-ads" "UninstallString" "$INSTDIR\Uninstall.exe"
sectionEnd
# uninstaller section start
section "uninstall"
SetShellVarContext all
# first, delete the uninstaller
delete "$INSTDIR\uninstall.exe"
# second, remove the link from the start menu
delete "$SMPROGRAMS\Classified-ads\uninstall.lnk"
delete "$SMPROGRAMS\Classified-ads\Classified-ads.lnk"
RMDIR "$SMPROGRAMS\Classified-ads"
delete "$INSTDIR\classified-ads.exe"
delete "$INSTDIR\qt_ca.qm"
delete "$INSTDIR\qt_cs.qm"
delete "$INSTDIR\qt_de.qm"
delete "$INSTDIR\qt_fi.qm"
delete "$INSTDIR\qt_hu.qm"
delete "$INSTDIR\qt_it.qm"
delete "$INSTDIR\qt_ja.qm"
delete "$INSTDIR\qt_lv.qm"
delete "$INSTDIR\qt_ru.qm"
delete "$INSTDIR\qt_sk.qm"
delete "$INSTDIR\qt_uk.qm"
delete "$INSTDIR\D3Dcompiler_41.dll"
delete "$INSTDIR\libEGL.dll"
delete "$INSTDIR\libGLESV2.dll"
delete "$INSTDIR\libgcc_s_dw2-1.dll"
delete "$INSTDIR\libstdc++-6.dll"
delete "$INSTDIR\libwinpthread-1.dll"
delete "$INSTDIR\Qt5Core.dll"
delete "$INSTDIR\Qt5Gui.dll"
delete "$INSTDIR\Qt5Network.dll"
delete "$INSTDIR\Qt5PrintSupport.dll"
delete "$INSTDIR\Qt5Sql.dll"
delete "$INSTDIR\Qt5Svg.dll"
delete "$INSTDIR\Qt5Widgets.dll"
delete "$INSTDIR\LICENSE"
delete "$INSTDIR\bearer\qgenericbearer.dll"
delete "$INSTDIR\bearer\qnativewifibearer.dll"
delete "$INSTDIR\iconengines\qsvgicon.dll"
delete "$INSTDIR\imageformats\qdds.dll"
delete "$INSTDIR\imageformats\qgif.dll"
delete "$INSTDIR\imageformats\qicns.dll"
delete "$INSTDIR\imageformats\qico.dll"
delete "$INSTDIR\imageformats\qjp2.dll"
delete "$INSTDIR\imageformats\qjpeg.dll"
delete "$INSTDIR\imageformats\qmng.dll"
delete "$INSTDIR\imageformats\qsvg.dll"
delete "$INSTDIR\imageformats\qtga.dll"
delete "$INSTDIR\imageformats\qtiff.dll"
delete "$INSTDIR\imageformats\qwbmp.dll"
delete "$INSTDIR\imageformats\qwebp.dll"
delete "$INSTDIR\platforms\qwindows.dll"
delete "$INSTDIR\printsupport\windowsprintersupport.dll"
delete "$INSTDIR\sqldrivers\qsqlite.dll"
delete "$INSTDIR\libeay32.dll"
delete "$INSTDIR\ssleay32.dll"
delete "$INSTDIR\miniupnpc.dll"
delete "$INSTDIR\libiconv-2.dll"
delete "$INSTDIR\libintl-8.dll"
delete "$INSTDIR\fi\LC_MESSAGES\classified-ads.mo"
delete "$INSTDIR\sv\LC_MESSAGES\classified-ads.mo"
delete "$INSTDIR\da\LC_MESSAGES\classified-ads.mo"
RMDIR "$INSTDIR\sv\LC_MESSAGES"
RMDIR "$INSTDIR\sv"
RMDIR "$INSTDIR\fi\LC_MESSAGES"
RMDIR "$INSTDIR\fi"
RMDIR "$INSTDIR\da\LC_MESSAGES"
RMDIR "$INSTDIR\da"
RMDIR "$INSTDIR\bearer"
RMDIR "$INSTDIR\iconengines"
RMDIR "$INSTDIR\imageformats"
RMDIR "$INSTDIR\printsupport"
RMDIR "$INSTDIR\platforms"
RMDIR "$INSTDIR\sqldrivers"
RMDIR "$INSTDIR"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classified-ads"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Classified-ads"
# uninstaller section end
sectionEnd 
