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
!define VERSION "0.12" 
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
CreateDirectory "$INSTDIR\uk\LC_MESSAGES"
file /oname=$INSTDIR\uk\LC_MESSAGES\classified-ads.mo ..\po\uk.mo
CreateDirectory "$INSTDIR\de\LC_MESSAGES"
file /oname=$INSTDIR\de\LC_MESSAGES\classified-ads.mo ..\po\de.mo
CreateDirectory "$INSTDIR\es\LC_MESSAGES"
file /oname=$INSTDIR\es\LC_MESSAGES\classified-ads.mo ..\po\es.mo
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
CreateDirectory "$INSTDIR\examples"
file /oname=$INSTDIR\examples\sysinfo.tcl ..\doc\sysinfo.tcl
file /oname=$INSTDIR\examples\luikero.tcl ..\doc\luikero.tcl
file /oname=$INSTDIR\examples\calendar.tcl ..\doc\calendar.tcl
CreateDirectory "$INSTDIR\bearer"
CreateDirectory "$INSTDIR\iconengines"
CreateDirectory "$INSTDIR\imageformats"
CreateDirectory "$INSTDIR\printsupport"
CreateDirectory "$INSTDIR\platforms"
CreateDirectory "$INSTDIR\sqldrivers"
CreateDirectory "$INSTDIR\audio"
file /oname=miniupnpc.dll ..\..\miniupnpc-1.9\miniupnpc.dll
CreateShortCut "$SMPROGRAMS\Classified-ads\Classified-ads.lnk" "$INSTDIR\Classified-ads.exe"
file /oname=D3Dcompiler_47.dll deps\D3Dcompiler_47.dll
file /oname=libEGL.dll deps\libEGL.dll
file /oname=libGLESV2.dll deps\libGLESV2.dll
file /oname=libgcc_s_dw2-1.dll deps\libgcc_s_dw2-1.dll
file /oname=libstdc++-6.dll deps\libstdc++-6.dll
file /oname=libwinpthread-1.dll deps\libwinpthread-1.dll
file /oname=opengl32sw.dll deps\opengl32sw.dll
file /oname=Qt5Core.dll deps\Qt5Core.dll
file /oname=Qt5Gui.dll deps\Qt5Gui.dll
file /oname=Qt5Multimedia.dll deps\Qt5Multimedia.dll
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
file /oname=audio\qtaudio_windows.dll ..\release\audio\qtaudio_windows.dll
file /oname=libeay32.dll ..\release\libeay32.dll
file /oname=ssleay32.dll ..\release\ssleay32.dll
file /oname=libiconv-2.dll ..\release\libiconv-2.dll
file /oname=libintl-8.dll ..\release\libintl-8.dll
file /oname=LICENSE ..\LICENSE
#
# TCL-related directories and files
#
CreateDirectory "$INSTDIR\tcl8.6"
CreateDirectory "$INSTDIR\tcl8.6\encoding"
CreateDirectory "$INSTDIR\tcl8.6\http1.0"
CreateDirectory "$INSTDIR\tcl8.6\msgs"
CreateDirectory "$INSTDIR\tcl8.6\opt0.4"
CreateDirectory "$INSTDIR\tcl8.6\tzdata"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Africa"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\America"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\America\Argentina"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\America\Indiana"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\America\Kentucky"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\America\North_Dakota"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Antarctica"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Arctic"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Asia"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Atlantic"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Australia"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Brazil"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Canada"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Chile"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Etc"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Europe"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Indian"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Mexico"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\Pacific"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\SystemV"
CreateDirectory "$INSTDIR\tcl8.6\tzdata\US"
CreateDirectory "$INSTDIR\tk8.6"
CreateDirectory "$INSTDIR\tk8.6\images"
CreateDirectory "$INSTDIR\tk8.6\msgs"
CreateDirectory "$INSTDIR\tk8.6\ttk"
CreateDirectory "$INSTDIR\tcl8"
CreateDirectory "$INSTDIR\tcl8\8.4"
CreateDirectory "$INSTDIR\tcl8\8.4\platform"
CreateDirectory "$INSTDIR\tcl8\8.5"
CreateDirectory "$INSTDIR\tcl8\8.6"
CreateDirectory "$INSTDIR\tcl8\8.6\tdbc"
# and files
file /oname=$INSTDIR\tcl8.6\auto.tcl ..\release\tcl8.6\auto.tcl
file /oname=$INSTDIR\tcl8.6\clock.tcl ..\release\tcl8.6\clock.tcl
file /oname=$INSTDIR\tcl8.6\encoding\ascii.enc ..\release\tcl8.6\encoding\ascii.enc
file /oname=$INSTDIR\tcl8.6\encoding\big5.enc ..\release\tcl8.6\encoding\big5.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp1250.enc ..\release\tcl8.6\encoding\cp1250.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp1251.enc ..\release\tcl8.6\encoding\cp1251.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp1252.enc ..\release\tcl8.6\encoding\cp1252.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp1253.enc ..\release\tcl8.6\encoding\cp1253.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp1254.enc ..\release\tcl8.6\encoding\cp1254.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp1255.enc ..\release\tcl8.6\encoding\cp1255.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp1256.enc ..\release\tcl8.6\encoding\cp1256.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp1257.enc ..\release\tcl8.6\encoding\cp1257.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp1258.enc ..\release\tcl8.6\encoding\cp1258.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp437.enc ..\release\tcl8.6\encoding\cp437.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp737.enc ..\release\tcl8.6\encoding\cp737.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp775.enc ..\release\tcl8.6\encoding\cp775.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp850.enc ..\release\tcl8.6\encoding\cp850.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp852.enc ..\release\tcl8.6\encoding\cp852.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp855.enc ..\release\tcl8.6\encoding\cp855.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp857.enc ..\release\tcl8.6\encoding\cp857.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp860.enc ..\release\tcl8.6\encoding\cp860.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp861.enc ..\release\tcl8.6\encoding\cp861.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp862.enc ..\release\tcl8.6\encoding\cp862.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp863.enc ..\release\tcl8.6\encoding\cp863.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp864.enc ..\release\tcl8.6\encoding\cp864.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp865.enc ..\release\tcl8.6\encoding\cp865.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp866.enc ..\release\tcl8.6\encoding\cp866.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp869.enc ..\release\tcl8.6\encoding\cp869.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp874.enc ..\release\tcl8.6\encoding\cp874.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp932.enc ..\release\tcl8.6\encoding\cp932.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp936.enc ..\release\tcl8.6\encoding\cp936.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp949.enc ..\release\tcl8.6\encoding\cp949.enc
file /oname=$INSTDIR\tcl8.6\encoding\cp950.enc ..\release\tcl8.6\encoding\cp950.enc
file /oname=$INSTDIR\tcl8.6\encoding\dingbats.enc ..\release\tcl8.6\encoding\dingbats.enc
file /oname=$INSTDIR\tcl8.6\encoding\ebcdic.enc ..\release\tcl8.6\encoding\ebcdic.enc
file /oname=$INSTDIR\tcl8.6\encoding\euc-cn.enc ..\release\tcl8.6\encoding\euc-cn.enc
file /oname=$INSTDIR\tcl8.6\encoding\euc-jp.enc ..\release\tcl8.6\encoding\euc-jp.enc
file /oname=$INSTDIR\tcl8.6\encoding\euc-kr.enc ..\release\tcl8.6\encoding\euc-kr.enc
file /oname=$INSTDIR\tcl8.6\encoding\gb12345.enc ..\release\tcl8.6\encoding\gb12345.enc
file /oname=$INSTDIR\tcl8.6\encoding\gb1988.enc ..\release\tcl8.6\encoding\gb1988.enc
file /oname=$INSTDIR\tcl8.6\encoding\gb2312-raw.enc ..\release\tcl8.6\encoding\gb2312-raw.enc
file /oname=$INSTDIR\tcl8.6\encoding\gb2312.enc ..\release\tcl8.6\encoding\gb2312.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso2022-jp.enc ..\release\tcl8.6\encoding\iso2022-jp.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso2022-kr.enc ..\release\tcl8.6\encoding\iso2022-kr.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso2022.enc ..\release\tcl8.6\encoding\iso2022.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-1.enc ..\release\tcl8.6\encoding\iso8859-1.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-10.enc ..\release\tcl8.6\encoding\iso8859-10.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-13.enc ..\release\tcl8.6\encoding\iso8859-13.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-14.enc ..\release\tcl8.6\encoding\iso8859-14.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-15.enc ..\release\tcl8.6\encoding\iso8859-15.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-16.enc ..\release\tcl8.6\encoding\iso8859-16.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-2.enc ..\release\tcl8.6\encoding\iso8859-2.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-3.enc ..\release\tcl8.6\encoding\iso8859-3.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-4.enc ..\release\tcl8.6\encoding\iso8859-4.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-5.enc ..\release\tcl8.6\encoding\iso8859-5.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-6.enc ..\release\tcl8.6\encoding\iso8859-6.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-7.enc ..\release\tcl8.6\encoding\iso8859-7.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-8.enc ..\release\tcl8.6\encoding\iso8859-8.enc
file /oname=$INSTDIR\tcl8.6\encoding\iso8859-9.enc ..\release\tcl8.6\encoding\iso8859-9.enc
file /oname=$INSTDIR\tcl8.6\encoding\jis0201.enc ..\release\tcl8.6\encoding\jis0201.enc
file /oname=$INSTDIR\tcl8.6\encoding\jis0208.enc ..\release\tcl8.6\encoding\jis0208.enc
file /oname=$INSTDIR\tcl8.6\encoding\jis0212.enc ..\release\tcl8.6\encoding\jis0212.enc
file /oname=$INSTDIR\tcl8.6\encoding\koi8-r.enc ..\release\tcl8.6\encoding\koi8-r.enc
file /oname=$INSTDIR\tcl8.6\encoding\koi8-u.enc ..\release\tcl8.6\encoding\koi8-u.enc
file /oname=$INSTDIR\tcl8.6\encoding\ksc5601.enc ..\release\tcl8.6\encoding\ksc5601.enc
file /oname=$INSTDIR\tcl8.6\encoding\macCentEuro.enc ..\release\tcl8.6\encoding\macCentEuro.enc
file /oname=$INSTDIR\tcl8.6\encoding\macCroatian.enc ..\release\tcl8.6\encoding\macCroatian.enc
file /oname=$INSTDIR\tcl8.6\encoding\macCyrillic.enc ..\release\tcl8.6\encoding\macCyrillic.enc
file /oname=$INSTDIR\tcl8.6\encoding\macDingbats.enc ..\release\tcl8.6\encoding\macDingbats.enc
file /oname=$INSTDIR\tcl8.6\encoding\macGreek.enc ..\release\tcl8.6\encoding\macGreek.enc
file /oname=$INSTDIR\tcl8.6\encoding\macIceland.enc ..\release\tcl8.6\encoding\macIceland.enc
file /oname=$INSTDIR\tcl8.6\encoding\macJapan.enc ..\release\tcl8.6\encoding\macJapan.enc
file /oname=$INSTDIR\tcl8.6\encoding\macRoman.enc ..\release\tcl8.6\encoding\macRoman.enc
file /oname=$INSTDIR\tcl8.6\encoding\macRomania.enc ..\release\tcl8.6\encoding\macRomania.enc
file /oname=$INSTDIR\tcl8.6\encoding\macThai.enc ..\release\tcl8.6\encoding\macThai.enc
file /oname=$INSTDIR\tcl8.6\encoding\macTurkish.enc ..\release\tcl8.6\encoding\macTurkish.enc
file /oname=$INSTDIR\tcl8.6\encoding\macUkraine.enc ..\release\tcl8.6\encoding\macUkraine.enc
file /oname=$INSTDIR\tcl8.6\encoding\shiftjis.enc ..\release\tcl8.6\encoding\shiftjis.enc
file /oname=$INSTDIR\tcl8.6\encoding\symbol.enc ..\release\tcl8.6\encoding\symbol.enc
file /oname=$INSTDIR\tcl8.6\encoding\tis-620.enc ..\release\tcl8.6\encoding\tis-620.enc
file /oname=$INSTDIR\tcl8.6\history.tcl ..\release\tcl8.6\history.tcl
file /oname=$INSTDIR\tcl8.6\http1.0\http.tcl ..\release\tcl8.6\http1.0\http.tcl
file /oname=$INSTDIR\tcl8.6\http1.0\pkgIndex.tcl ..\release\tcl8.6\http1.0\pkgIndex.tcl
file /oname=$INSTDIR\tcl8.6\init.tcl ..\release\tcl8.6\init.tcl
file /oname=$INSTDIR\tcl8.6\msgs\af.msg ..\release\tcl8.6\msgs\af.msg
file /oname=$INSTDIR\tcl8.6\msgs\af_za.msg ..\release\tcl8.6\msgs\af_za.msg
file /oname=$INSTDIR\tcl8.6\msgs\ar.msg ..\release\tcl8.6\msgs\ar.msg
file /oname=$INSTDIR\tcl8.6\msgs\ar_in.msg ..\release\tcl8.6\msgs\ar_in.msg
file /oname=$INSTDIR\tcl8.6\msgs\ar_jo.msg ..\release\tcl8.6\msgs\ar_jo.msg
file /oname=$INSTDIR\tcl8.6\msgs\ar_lb.msg ..\release\tcl8.6\msgs\ar_lb.msg
file /oname=$INSTDIR\tcl8.6\msgs\ar_sy.msg ..\release\tcl8.6\msgs\ar_sy.msg
file /oname=$INSTDIR\tcl8.6\msgs\be.msg ..\release\tcl8.6\msgs\be.msg
file /oname=$INSTDIR\tcl8.6\msgs\bg.msg ..\release\tcl8.6\msgs\bg.msg
file /oname=$INSTDIR\tcl8.6\msgs\bn.msg ..\release\tcl8.6\msgs\bn.msg
file /oname=$INSTDIR\tcl8.6\msgs\bn_in.msg ..\release\tcl8.6\msgs\bn_in.msg
file /oname=$INSTDIR\tcl8.6\msgs\ca.msg ..\release\tcl8.6\msgs\ca.msg
file /oname=$INSTDIR\tcl8.6\msgs\cs.msg ..\release\tcl8.6\msgs\cs.msg
file /oname=$INSTDIR\tcl8.6\msgs\da.msg ..\release\tcl8.6\msgs\da.msg
file /oname=$INSTDIR\tcl8.6\msgs\de.msg ..\release\tcl8.6\msgs\de.msg
file /oname=$INSTDIR\tcl8.6\msgs\de_at.msg ..\release\tcl8.6\msgs\de_at.msg
file /oname=$INSTDIR\tcl8.6\msgs\de_be.msg ..\release\tcl8.6\msgs\de_be.msg
file /oname=$INSTDIR\tcl8.6\msgs\el.msg ..\release\tcl8.6\msgs\el.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_au.msg ..\release\tcl8.6\msgs\en_au.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_be.msg ..\release\tcl8.6\msgs\en_be.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_bw.msg ..\release\tcl8.6\msgs\en_bw.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_ca.msg ..\release\tcl8.6\msgs\en_ca.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_gb.msg ..\release\tcl8.6\msgs\en_gb.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_hk.msg ..\release\tcl8.6\msgs\en_hk.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_ie.msg ..\release\tcl8.6\msgs\en_ie.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_in.msg ..\release\tcl8.6\msgs\en_in.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_nz.msg ..\release\tcl8.6\msgs\en_nz.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_ph.msg ..\release\tcl8.6\msgs\en_ph.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_sg.msg ..\release\tcl8.6\msgs\en_sg.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_za.msg ..\release\tcl8.6\msgs\en_za.msg
file /oname=$INSTDIR\tcl8.6\msgs\en_zw.msg ..\release\tcl8.6\msgs\en_zw.msg
file /oname=$INSTDIR\tcl8.6\msgs\eo.msg ..\release\tcl8.6\msgs\eo.msg
file /oname=$INSTDIR\tcl8.6\msgs\es.msg ..\release\tcl8.6\msgs\es.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_ar.msg ..\release\tcl8.6\msgs\es_ar.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_bo.msg ..\release\tcl8.6\msgs\es_bo.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_cl.msg ..\release\tcl8.6\msgs\es_cl.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_co.msg ..\release\tcl8.6\msgs\es_co.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_cr.msg ..\release\tcl8.6\msgs\es_cr.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_do.msg ..\release\tcl8.6\msgs\es_do.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_ec.msg ..\release\tcl8.6\msgs\es_ec.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_gt.msg ..\release\tcl8.6\msgs\es_gt.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_hn.msg ..\release\tcl8.6\msgs\es_hn.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_mx.msg ..\release\tcl8.6\msgs\es_mx.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_ni.msg ..\release\tcl8.6\msgs\es_ni.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_pa.msg ..\release\tcl8.6\msgs\es_pa.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_pe.msg ..\release\tcl8.6\msgs\es_pe.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_pr.msg ..\release\tcl8.6\msgs\es_pr.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_py.msg ..\release\tcl8.6\msgs\es_py.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_sv.msg ..\release\tcl8.6\msgs\es_sv.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_uy.msg ..\release\tcl8.6\msgs\es_uy.msg
file /oname=$INSTDIR\tcl8.6\msgs\es_ve.msg ..\release\tcl8.6\msgs\es_ve.msg
file /oname=$INSTDIR\tcl8.6\msgs\et.msg ..\release\tcl8.6\msgs\et.msg
file /oname=$INSTDIR\tcl8.6\msgs\eu.msg ..\release\tcl8.6\msgs\eu.msg
file /oname=$INSTDIR\tcl8.6\msgs\eu_es.msg ..\release\tcl8.6\msgs\eu_es.msg
file /oname=$INSTDIR\tcl8.6\msgs\fa.msg ..\release\tcl8.6\msgs\fa.msg
file /oname=$INSTDIR\tcl8.6\msgs\fa_in.msg ..\release\tcl8.6\msgs\fa_in.msg
file /oname=$INSTDIR\tcl8.6\msgs\fa_ir.msg ..\release\tcl8.6\msgs\fa_ir.msg
file /oname=$INSTDIR\tcl8.6\msgs\fi.msg ..\release\tcl8.6\msgs\fi.msg
file /oname=$INSTDIR\tcl8.6\msgs\fo.msg ..\release\tcl8.6\msgs\fo.msg
file /oname=$INSTDIR\tcl8.6\msgs\fo_fo.msg ..\release\tcl8.6\msgs\fo_fo.msg
file /oname=$INSTDIR\tcl8.6\msgs\fr.msg ..\release\tcl8.6\msgs\fr.msg
file /oname=$INSTDIR\tcl8.6\msgs\fr_be.msg ..\release\tcl8.6\msgs\fr_be.msg
file /oname=$INSTDIR\tcl8.6\msgs\fr_ca.msg ..\release\tcl8.6\msgs\fr_ca.msg
file /oname=$INSTDIR\tcl8.6\msgs\fr_ch.msg ..\release\tcl8.6\msgs\fr_ch.msg
file /oname=$INSTDIR\tcl8.6\msgs\ga.msg ..\release\tcl8.6\msgs\ga.msg
file /oname=$INSTDIR\tcl8.6\msgs\ga_ie.msg ..\release\tcl8.6\msgs\ga_ie.msg
file /oname=$INSTDIR\tcl8.6\msgs\gl.msg ..\release\tcl8.6\msgs\gl.msg
file /oname=$INSTDIR\tcl8.6\msgs\gl_es.msg ..\release\tcl8.6\msgs\gl_es.msg
file /oname=$INSTDIR\tcl8.6\msgs\gv.msg ..\release\tcl8.6\msgs\gv.msg
file /oname=$INSTDIR\tcl8.6\msgs\gv_gb.msg ..\release\tcl8.6\msgs\gv_gb.msg
file /oname=$INSTDIR\tcl8.6\msgs\he.msg ..\release\tcl8.6\msgs\he.msg
file /oname=$INSTDIR\tcl8.6\msgs\hi.msg ..\release\tcl8.6\msgs\hi.msg
file /oname=$INSTDIR\tcl8.6\msgs\hi_in.msg ..\release\tcl8.6\msgs\hi_in.msg
file /oname=$INSTDIR\tcl8.6\msgs\hr.msg ..\release\tcl8.6\msgs\hr.msg
file /oname=$INSTDIR\tcl8.6\msgs\hu.msg ..\release\tcl8.6\msgs\hu.msg
file /oname=$INSTDIR\tcl8.6\msgs\id.msg ..\release\tcl8.6\msgs\id.msg
file /oname=$INSTDIR\tcl8.6\msgs\id_id.msg ..\release\tcl8.6\msgs\id_id.msg
file /oname=$INSTDIR\tcl8.6\msgs\is.msg ..\release\tcl8.6\msgs\is.msg
file /oname=$INSTDIR\tcl8.6\msgs\it.msg ..\release\tcl8.6\msgs\it.msg
file /oname=$INSTDIR\tcl8.6\msgs\it_ch.msg ..\release\tcl8.6\msgs\it_ch.msg
file /oname=$INSTDIR\tcl8.6\msgs\ja.msg ..\release\tcl8.6\msgs\ja.msg
file /oname=$INSTDIR\tcl8.6\msgs\kl.msg ..\release\tcl8.6\msgs\kl.msg
file /oname=$INSTDIR\tcl8.6\msgs\kl_gl.msg ..\release\tcl8.6\msgs\kl_gl.msg
file /oname=$INSTDIR\tcl8.6\msgs\ko.msg ..\release\tcl8.6\msgs\ko.msg
file /oname=$INSTDIR\tcl8.6\msgs\kok.msg ..\release\tcl8.6\msgs\kok.msg
file /oname=$INSTDIR\tcl8.6\msgs\kok_in.msg ..\release\tcl8.6\msgs\kok_in.msg
file /oname=$INSTDIR\tcl8.6\msgs\ko_kr.msg ..\release\tcl8.6\msgs\ko_kr.msg
file /oname=$INSTDIR\tcl8.6\msgs\kw.msg ..\release\tcl8.6\msgs\kw.msg
file /oname=$INSTDIR\tcl8.6\msgs\kw_gb.msg ..\release\tcl8.6\msgs\kw_gb.msg
file /oname=$INSTDIR\tcl8.6\msgs\lt.msg ..\release\tcl8.6\msgs\lt.msg
file /oname=$INSTDIR\tcl8.6\msgs\lv.msg ..\release\tcl8.6\msgs\lv.msg
file /oname=$INSTDIR\tcl8.6\msgs\mk.msg ..\release\tcl8.6\msgs\mk.msg
file /oname=$INSTDIR\tcl8.6\msgs\mr.msg ..\release\tcl8.6\msgs\mr.msg
file /oname=$INSTDIR\tcl8.6\msgs\mr_in.msg ..\release\tcl8.6\msgs\mr_in.msg
file /oname=$INSTDIR\tcl8.6\msgs\ms.msg ..\release\tcl8.6\msgs\ms.msg
file /oname=$INSTDIR\tcl8.6\msgs\ms_my.msg ..\release\tcl8.6\msgs\ms_my.msg
file /oname=$INSTDIR\tcl8.6\msgs\mt.msg ..\release\tcl8.6\msgs\mt.msg
file /oname=$INSTDIR\tcl8.6\msgs\nb.msg ..\release\tcl8.6\msgs\nb.msg
file /oname=$INSTDIR\tcl8.6\msgs\nl.msg ..\release\tcl8.6\msgs\nl.msg
file /oname=$INSTDIR\tcl8.6\msgs\nl_be.msg ..\release\tcl8.6\msgs\nl_be.msg
file /oname=$INSTDIR\tcl8.6\msgs\nn.msg ..\release\tcl8.6\msgs\nn.msg
file /oname=$INSTDIR\tcl8.6\msgs\pl.msg ..\release\tcl8.6\msgs\pl.msg
file /oname=$INSTDIR\tcl8.6\msgs\pt.msg ..\release\tcl8.6\msgs\pt.msg
file /oname=$INSTDIR\tcl8.6\msgs\pt_br.msg ..\release\tcl8.6\msgs\pt_br.msg
file /oname=$INSTDIR\tcl8.6\msgs\ro.msg ..\release\tcl8.6\msgs\ro.msg
file /oname=$INSTDIR\tcl8.6\msgs\ru.msg ..\release\tcl8.6\msgs\ru.msg
file /oname=$INSTDIR\tcl8.6\msgs\ru_ua.msg ..\release\tcl8.6\msgs\ru_ua.msg
file /oname=$INSTDIR\tcl8.6\msgs\sh.msg ..\release\tcl8.6\msgs\sh.msg
file /oname=$INSTDIR\tcl8.6\msgs\sk.msg ..\release\tcl8.6\msgs\sk.msg
file /oname=$INSTDIR\tcl8.6\msgs\sl.msg ..\release\tcl8.6\msgs\sl.msg
file /oname=$INSTDIR\tcl8.6\msgs\sq.msg ..\release\tcl8.6\msgs\sq.msg
file /oname=$INSTDIR\tcl8.6\msgs\sr.msg ..\release\tcl8.6\msgs\sr.msg
file /oname=$INSTDIR\tcl8.6\msgs\sv.msg ..\release\tcl8.6\msgs\sv.msg
file /oname=$INSTDIR\tcl8.6\msgs\sw.msg ..\release\tcl8.6\msgs\sw.msg
file /oname=$INSTDIR\tcl8.6\msgs\ta.msg ..\release\tcl8.6\msgs\ta.msg
file /oname=$INSTDIR\tcl8.6\msgs\ta_in.msg ..\release\tcl8.6\msgs\ta_in.msg
file /oname=$INSTDIR\tcl8.6\msgs\te.msg ..\release\tcl8.6\msgs\te.msg
file /oname=$INSTDIR\tcl8.6\msgs\te_in.msg ..\release\tcl8.6\msgs\te_in.msg
file /oname=$INSTDIR\tcl8.6\msgs\th.msg ..\release\tcl8.6\msgs\th.msg
file /oname=$INSTDIR\tcl8.6\msgs\tr.msg ..\release\tcl8.6\msgs\tr.msg
file /oname=$INSTDIR\tcl8.6\msgs\uk.msg ..\release\tcl8.6\msgs\uk.msg
file /oname=$INSTDIR\tcl8.6\msgs\vi.msg ..\release\tcl8.6\msgs\vi.msg
file /oname=$INSTDIR\tcl8.6\msgs\zh.msg ..\release\tcl8.6\msgs\zh.msg
file /oname=$INSTDIR\tcl8.6\msgs\zh_cn.msg ..\release\tcl8.6\msgs\zh_cn.msg
file /oname=$INSTDIR\tcl8.6\msgs\zh_hk.msg ..\release\tcl8.6\msgs\zh_hk.msg
file /oname=$INSTDIR\tcl8.6\msgs\zh_sg.msg ..\release\tcl8.6\msgs\zh_sg.msg
file /oname=$INSTDIR\tcl8.6\msgs\zh_tw.msg ..\release\tcl8.6\msgs\zh_tw.msg
file /oname=$INSTDIR\tcl8.6\opt0.4\optparse.tcl ..\release\tcl8.6\opt0.4\optparse.tcl
file /oname=$INSTDIR\tcl8.6\opt0.4\pkgIndex.tcl ..\release\tcl8.6\opt0.4\pkgIndex.tcl
file /oname=$INSTDIR\tcl8.6\package.tcl ..\release\tcl8.6\package.tcl
file /oname=$INSTDIR\tcl8.6\parray.tcl ..\release\tcl8.6\parray.tcl
file /oname=$INSTDIR\tcl8.6\safe.tcl ..\release\tcl8.6\safe.tcl
file /oname=$INSTDIR\tcl8.6\tclIndex ..\release\tcl8.6\tclIndex
file /oname=$INSTDIR\tcl8.6\tm.tcl ..\release\tcl8.6\tm.tcl
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Abidjan ..\release\tcl8.6\tzdata\Africa\Abidjan
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Accra ..\release\tcl8.6\tzdata\Africa\Accra
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Addis_Ababa ..\release\tcl8.6\tzdata\Africa\Addis_Ababa
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Algiers ..\release\tcl8.6\tzdata\Africa\Algiers
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Asmara ..\release\tcl8.6\tzdata\Africa\Asmara
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Asmera ..\release\tcl8.6\tzdata\Africa\Asmera
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Bamako ..\release\tcl8.6\tzdata\Africa\Bamako
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Bangui ..\release\tcl8.6\tzdata\Africa\Bangui
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Banjul ..\release\tcl8.6\tzdata\Africa\Banjul
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Bissau ..\release\tcl8.6\tzdata\Africa\Bissau
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Blantyre ..\release\tcl8.6\tzdata\Africa\Blantyre
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Brazzaville ..\release\tcl8.6\tzdata\Africa\Brazzaville
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Bujumbura ..\release\tcl8.6\tzdata\Africa\Bujumbura
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Cairo ..\release\tcl8.6\tzdata\Africa\Cairo
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Casablanca ..\release\tcl8.6\tzdata\Africa\Casablanca
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Ceuta ..\release\tcl8.6\tzdata\Africa\Ceuta
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Conakry ..\release\tcl8.6\tzdata\Africa\Conakry
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Dakar ..\release\tcl8.6\tzdata\Africa\Dakar
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Dar_es_Salaam ..\release\tcl8.6\tzdata\Africa\Dar_es_Salaam
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Djibouti ..\release\tcl8.6\tzdata\Africa\Djibouti
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Douala ..\release\tcl8.6\tzdata\Africa\Douala
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\El_Aaiun ..\release\tcl8.6\tzdata\Africa\El_Aaiun
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Freetown ..\release\tcl8.6\tzdata\Africa\Freetown
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Gaborone ..\release\tcl8.6\tzdata\Africa\Gaborone
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Harare ..\release\tcl8.6\tzdata\Africa\Harare
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Johannesburg ..\release\tcl8.6\tzdata\Africa\Johannesburg
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Juba ..\release\tcl8.6\tzdata\Africa\Juba
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Kampala ..\release\tcl8.6\tzdata\Africa\Kampala
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Khartoum ..\release\tcl8.6\tzdata\Africa\Khartoum
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Kigali ..\release\tcl8.6\tzdata\Africa\Kigali
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Kinshasa ..\release\tcl8.6\tzdata\Africa\Kinshasa
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Lagos ..\release\tcl8.6\tzdata\Africa\Lagos
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Libreville ..\release\tcl8.6\tzdata\Africa\Libreville
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Lome ..\release\tcl8.6\tzdata\Africa\Lome
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Luanda ..\release\tcl8.6\tzdata\Africa\Luanda
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Lubumbashi ..\release\tcl8.6\tzdata\Africa\Lubumbashi
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Lusaka ..\release\tcl8.6\tzdata\Africa\Lusaka
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Malabo ..\release\tcl8.6\tzdata\Africa\Malabo
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Maputo ..\release\tcl8.6\tzdata\Africa\Maputo
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Maseru ..\release\tcl8.6\tzdata\Africa\Maseru
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Mbabane ..\release\tcl8.6\tzdata\Africa\Mbabane
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Mogadishu ..\release\tcl8.6\tzdata\Africa\Mogadishu
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Monrovia ..\release\tcl8.6\tzdata\Africa\Monrovia
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Nairobi ..\release\tcl8.6\tzdata\Africa\Nairobi
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Ndjamena ..\release\tcl8.6\tzdata\Africa\Ndjamena
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Niamey ..\release\tcl8.6\tzdata\Africa\Niamey
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Nouakchott ..\release\tcl8.6\tzdata\Africa\Nouakchott
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Ouagadougou ..\release\tcl8.6\tzdata\Africa\Ouagadougou
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Porto-Novo ..\release\tcl8.6\tzdata\Africa\Porto-Novo
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Sao_Tome ..\release\tcl8.6\tzdata\Africa\Sao_Tome
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Timbuktu ..\release\tcl8.6\tzdata\Africa\Timbuktu
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Tripoli ..\release\tcl8.6\tzdata\Africa\Tripoli
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Tunis ..\release\tcl8.6\tzdata\Africa\Tunis
file /oname=$INSTDIR\tcl8.6\tzdata\Africa\Windhoek ..\release\tcl8.6\tzdata\Africa\Windhoek
file /oname=$INSTDIR\tcl8.6\tzdata\America\Adak ..\release\tcl8.6\tzdata\America\Adak
file /oname=$INSTDIR\tcl8.6\tzdata\America\Anchorage ..\release\tcl8.6\tzdata\America\Anchorage
file /oname=$INSTDIR\tcl8.6\tzdata\America\Anguilla ..\release\tcl8.6\tzdata\America\Anguilla
file /oname=$INSTDIR\tcl8.6\tzdata\America\Antigua ..\release\tcl8.6\tzdata\America\Antigua
file /oname=$INSTDIR\tcl8.6\tzdata\America\Araguaina ..\release\tcl8.6\tzdata\America\Araguaina
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\Buenos_Aires ..\release\tcl8.6\tzdata\America\Argentina\Buenos_Aires
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\Catamarca ..\release\tcl8.6\tzdata\America\Argentina\Catamarca
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\ComodRivadavia ..\release\tcl8.6\tzdata\America\Argentina\ComodRivadavia
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\Cordoba ..\release\tcl8.6\tzdata\America\Argentina\Cordoba
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\Jujuy ..\release\tcl8.6\tzdata\America\Argentina\Jujuy
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\La_Rioja ..\release\tcl8.6\tzdata\America\Argentina\La_Rioja
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\Mendoza ..\release\tcl8.6\tzdata\America\Argentina\Mendoza
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\Rio_Gallegos ..\release\tcl8.6\tzdata\America\Argentina\Rio_Gallegos
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\Salta ..\release\tcl8.6\tzdata\America\Argentina\Salta
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\San_Juan ..\release\tcl8.6\tzdata\America\Argentina\San_Juan
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\San_Luis ..\release\tcl8.6\tzdata\America\Argentina\San_Luis
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\Tucuman ..\release\tcl8.6\tzdata\America\Argentina\Tucuman
file /oname=$INSTDIR\tcl8.6\tzdata\America\Argentina\Ushuaia ..\release\tcl8.6\tzdata\America\Argentina\Ushuaia
file /oname=$INSTDIR\tcl8.6\tzdata\America\Aruba ..\release\tcl8.6\tzdata\America\Aruba
file /oname=$INSTDIR\tcl8.6\tzdata\America\Asuncion ..\release\tcl8.6\tzdata\America\Asuncion
file /oname=$INSTDIR\tcl8.6\tzdata\America\Atikokan ..\release\tcl8.6\tzdata\America\Atikokan
file /oname=$INSTDIR\tcl8.6\tzdata\America\Atka ..\release\tcl8.6\tzdata\America\Atka
file /oname=$INSTDIR\tcl8.6\tzdata\America\Bahia ..\release\tcl8.6\tzdata\America\Bahia
file /oname=$INSTDIR\tcl8.6\tzdata\America\Bahia_Banderas ..\release\tcl8.6\tzdata\America\Bahia_Banderas
file /oname=$INSTDIR\tcl8.6\tzdata\America\Barbados ..\release\tcl8.6\tzdata\America\Barbados
file /oname=$INSTDIR\tcl8.6\tzdata\America\Belem ..\release\tcl8.6\tzdata\America\Belem
file /oname=$INSTDIR\tcl8.6\tzdata\America\Belize ..\release\tcl8.6\tzdata\America\Belize
file /oname=$INSTDIR\tcl8.6\tzdata\America\Blanc-Sablon ..\release\tcl8.6\tzdata\America\Blanc-Sablon
file /oname=$INSTDIR\tcl8.6\tzdata\America\Boa_Vista ..\release\tcl8.6\tzdata\America\Boa_Vista
file /oname=$INSTDIR\tcl8.6\tzdata\America\Bogota ..\release\tcl8.6\tzdata\America\Bogota
file /oname=$INSTDIR\tcl8.6\tzdata\America\Boise ..\release\tcl8.6\tzdata\America\Boise
file /oname=$INSTDIR\tcl8.6\tzdata\America\Buenos_Aires ..\release\tcl8.6\tzdata\America\Buenos_Aires
file /oname=$INSTDIR\tcl8.6\tzdata\America\Cambridge_Bay ..\release\tcl8.6\tzdata\America\Cambridge_Bay
file /oname=$INSTDIR\tcl8.6\tzdata\America\Campo_Grande ..\release\tcl8.6\tzdata\America\Campo_Grande
file /oname=$INSTDIR\tcl8.6\tzdata\America\Cancun ..\release\tcl8.6\tzdata\America\Cancun
file /oname=$INSTDIR\tcl8.6\tzdata\America\Caracas ..\release\tcl8.6\tzdata\America\Caracas
file /oname=$INSTDIR\tcl8.6\tzdata\America\Catamarca ..\release\tcl8.6\tzdata\America\Catamarca
file /oname=$INSTDIR\tcl8.6\tzdata\America\Cayenne ..\release\tcl8.6\tzdata\America\Cayenne
file /oname=$INSTDIR\tcl8.6\tzdata\America\Cayman ..\release\tcl8.6\tzdata\America\Cayman
file /oname=$INSTDIR\tcl8.6\tzdata\America\Chicago ..\release\tcl8.6\tzdata\America\Chicago
file /oname=$INSTDIR\tcl8.6\tzdata\America\Chihuahua ..\release\tcl8.6\tzdata\America\Chihuahua
file /oname=$INSTDIR\tcl8.6\tzdata\America\Coral_Harbour ..\release\tcl8.6\tzdata\America\Coral_Harbour
file /oname=$INSTDIR\tcl8.6\tzdata\America\Cordoba ..\release\tcl8.6\tzdata\America\Cordoba
file /oname=$INSTDIR\tcl8.6\tzdata\America\Costa_Rica ..\release\tcl8.6\tzdata\America\Costa_Rica
file /oname=$INSTDIR\tcl8.6\tzdata\America\Creston ..\release\tcl8.6\tzdata\America\Creston
file /oname=$INSTDIR\tcl8.6\tzdata\America\Cuiaba ..\release\tcl8.6\tzdata\America\Cuiaba
file /oname=$INSTDIR\tcl8.6\tzdata\America\Curacao ..\release\tcl8.6\tzdata\America\Curacao
file /oname=$INSTDIR\tcl8.6\tzdata\America\Danmarkshavn ..\release\tcl8.6\tzdata\America\Danmarkshavn
file /oname=$INSTDIR\tcl8.6\tzdata\America\Dawson ..\release\tcl8.6\tzdata\America\Dawson
file /oname=$INSTDIR\tcl8.6\tzdata\America\Dawson_Creek ..\release\tcl8.6\tzdata\America\Dawson_Creek
file /oname=$INSTDIR\tcl8.6\tzdata\America\Denver ..\release\tcl8.6\tzdata\America\Denver
file /oname=$INSTDIR\tcl8.6\tzdata\America\Detroit ..\release\tcl8.6\tzdata\America\Detroit
file /oname=$INSTDIR\tcl8.6\tzdata\America\Dominica ..\release\tcl8.6\tzdata\America\Dominica
file /oname=$INSTDIR\tcl8.6\tzdata\America\Edmonton ..\release\tcl8.6\tzdata\America\Edmonton
file /oname=$INSTDIR\tcl8.6\tzdata\America\Eirunepe ..\release\tcl8.6\tzdata\America\Eirunepe
file /oname=$INSTDIR\tcl8.6\tzdata\America\El_Salvador ..\release\tcl8.6\tzdata\America\El_Salvador
file /oname=$INSTDIR\tcl8.6\tzdata\America\Ensenada ..\release\tcl8.6\tzdata\America\Ensenada
file /oname=$INSTDIR\tcl8.6\tzdata\America\Fortaleza ..\release\tcl8.6\tzdata\America\Fortaleza
file /oname=$INSTDIR\tcl8.6\tzdata\America\Fort_Nelson ..\release\tcl8.6\tzdata\America\Fort_Nelson
file /oname=$INSTDIR\tcl8.6\tzdata\America\Fort_Wayne ..\release\tcl8.6\tzdata\America\Fort_Wayne
file /oname=$INSTDIR\tcl8.6\tzdata\America\Glace_Bay ..\release\tcl8.6\tzdata\America\Glace_Bay
file /oname=$INSTDIR\tcl8.6\tzdata\America\Godthab ..\release\tcl8.6\tzdata\America\Godthab
file /oname=$INSTDIR\tcl8.6\tzdata\America\Goose_Bay ..\release\tcl8.6\tzdata\America\Goose_Bay
file /oname=$INSTDIR\tcl8.6\tzdata\America\Grand_Turk ..\release\tcl8.6\tzdata\America\Grand_Turk
file /oname=$INSTDIR\tcl8.6\tzdata\America\Grenada ..\release\tcl8.6\tzdata\America\Grenada
file /oname=$INSTDIR\tcl8.6\tzdata\America\Guadeloupe ..\release\tcl8.6\tzdata\America\Guadeloupe
file /oname=$INSTDIR\tcl8.6\tzdata\America\Guatemala ..\release\tcl8.6\tzdata\America\Guatemala
file /oname=$INSTDIR\tcl8.6\tzdata\America\Guayaquil ..\release\tcl8.6\tzdata\America\Guayaquil
file /oname=$INSTDIR\tcl8.6\tzdata\America\Guyana ..\release\tcl8.6\tzdata\America\Guyana
file /oname=$INSTDIR\tcl8.6\tzdata\America\Halifax ..\release\tcl8.6\tzdata\America\Halifax
file /oname=$INSTDIR\tcl8.6\tzdata\America\Havana ..\release\tcl8.6\tzdata\America\Havana
file /oname=$INSTDIR\tcl8.6\tzdata\America\Hermosillo ..\release\tcl8.6\tzdata\America\Hermosillo
file /oname=$INSTDIR\tcl8.6\tzdata\America\Indiana\Indianapolis ..\release\tcl8.6\tzdata\America\Indiana\Indianapolis
file /oname=$INSTDIR\tcl8.6\tzdata\America\Indiana\Knox ..\release\tcl8.6\tzdata\America\Indiana\Knox
file /oname=$INSTDIR\tcl8.6\tzdata\America\Indiana\Marengo ..\release\tcl8.6\tzdata\America\Indiana\Marengo
file /oname=$INSTDIR\tcl8.6\tzdata\America\Indiana\Petersburg ..\release\tcl8.6\tzdata\America\Indiana\Petersburg
file /oname=$INSTDIR\tcl8.6\tzdata\America\Indiana\Tell_City ..\release\tcl8.6\tzdata\America\Indiana\Tell_City
file /oname=$INSTDIR\tcl8.6\tzdata\America\Indiana\Vevay ..\release\tcl8.6\tzdata\America\Indiana\Vevay
file /oname=$INSTDIR\tcl8.6\tzdata\America\Indiana\Vincennes ..\release\tcl8.6\tzdata\America\Indiana\Vincennes
file /oname=$INSTDIR\tcl8.6\tzdata\America\Indiana\Winamac ..\release\tcl8.6\tzdata\America\Indiana\Winamac
file /oname=$INSTDIR\tcl8.6\tzdata\America\Indianapolis ..\release\tcl8.6\tzdata\America\Indianapolis
file /oname=$INSTDIR\tcl8.6\tzdata\America\Inuvik ..\release\tcl8.6\tzdata\America\Inuvik
file /oname=$INSTDIR\tcl8.6\tzdata\America\Iqaluit ..\release\tcl8.6\tzdata\America\Iqaluit
file /oname=$INSTDIR\tcl8.6\tzdata\America\Jamaica ..\release\tcl8.6\tzdata\America\Jamaica
file /oname=$INSTDIR\tcl8.6\tzdata\America\Jujuy ..\release\tcl8.6\tzdata\America\Jujuy
file /oname=$INSTDIR\tcl8.6\tzdata\America\Juneau ..\release\tcl8.6\tzdata\America\Juneau
file /oname=$INSTDIR\tcl8.6\tzdata\America\Kentucky\Louisville ..\release\tcl8.6\tzdata\America\Kentucky\Louisville
file /oname=$INSTDIR\tcl8.6\tzdata\America\Kentucky\Monticello ..\release\tcl8.6\tzdata\America\Kentucky\Monticello
file /oname=$INSTDIR\tcl8.6\tzdata\America\Knox_IN ..\release\tcl8.6\tzdata\America\Knox_IN
file /oname=$INSTDIR\tcl8.6\tzdata\America\Kralendijk ..\release\tcl8.6\tzdata\America\Kralendijk
file /oname=$INSTDIR\tcl8.6\tzdata\America\La_Paz ..\release\tcl8.6\tzdata\America\La_Paz
file /oname=$INSTDIR\tcl8.6\tzdata\America\Lima ..\release\tcl8.6\tzdata\America\Lima
file /oname=$INSTDIR\tcl8.6\tzdata\America\Los_Angeles ..\release\tcl8.6\tzdata\America\Los_Angeles
file /oname=$INSTDIR\tcl8.6\tzdata\America\Louisville ..\release\tcl8.6\tzdata\America\Louisville
file /oname=$INSTDIR\tcl8.6\tzdata\America\Lower_Princes ..\release\tcl8.6\tzdata\America\Lower_Princes
file /oname=$INSTDIR\tcl8.6\tzdata\America\Maceio ..\release\tcl8.6\tzdata\America\Maceio
file /oname=$INSTDIR\tcl8.6\tzdata\America\Managua ..\release\tcl8.6\tzdata\America\Managua
file /oname=$INSTDIR\tcl8.6\tzdata\America\Manaus ..\release\tcl8.6\tzdata\America\Manaus
file /oname=$INSTDIR\tcl8.6\tzdata\America\Marigot ..\release\tcl8.6\tzdata\America\Marigot
file /oname=$INSTDIR\tcl8.6\tzdata\America\Martinique ..\release\tcl8.6\tzdata\America\Martinique
file /oname=$INSTDIR\tcl8.6\tzdata\America\Matamoros ..\release\tcl8.6\tzdata\America\Matamoros
file /oname=$INSTDIR\tcl8.6\tzdata\America\Mazatlan ..\release\tcl8.6\tzdata\America\Mazatlan
file /oname=$INSTDIR\tcl8.6\tzdata\America\Mendoza ..\release\tcl8.6\tzdata\America\Mendoza
file /oname=$INSTDIR\tcl8.6\tzdata\America\Menominee ..\release\tcl8.6\tzdata\America\Menominee
file /oname=$INSTDIR\tcl8.6\tzdata\America\Merida ..\release\tcl8.6\tzdata\America\Merida
file /oname=$INSTDIR\tcl8.6\tzdata\America\Metlakatla ..\release\tcl8.6\tzdata\America\Metlakatla
file /oname=$INSTDIR\tcl8.6\tzdata\America\Mexico_City ..\release\tcl8.6\tzdata\America\Mexico_City
file /oname=$INSTDIR\tcl8.6\tzdata\America\Miquelon ..\release\tcl8.6\tzdata\America\Miquelon
file /oname=$INSTDIR\tcl8.6\tzdata\America\Moncton ..\release\tcl8.6\tzdata\America\Moncton
file /oname=$INSTDIR\tcl8.6\tzdata\America\Monterrey ..\release\tcl8.6\tzdata\America\Monterrey
file /oname=$INSTDIR\tcl8.6\tzdata\America\Montevideo ..\release\tcl8.6\tzdata\America\Montevideo
file /oname=$INSTDIR\tcl8.6\tzdata\America\Montreal ..\release\tcl8.6\tzdata\America\Montreal
file /oname=$INSTDIR\tcl8.6\tzdata\America\Montserrat ..\release\tcl8.6\tzdata\America\Montserrat
file /oname=$INSTDIR\tcl8.6\tzdata\America\Nassau ..\release\tcl8.6\tzdata\America\Nassau
file /oname=$INSTDIR\tcl8.6\tzdata\America\New_York ..\release\tcl8.6\tzdata\America\New_York
file /oname=$INSTDIR\tcl8.6\tzdata\America\Nipigon ..\release\tcl8.6\tzdata\America\Nipigon
file /oname=$INSTDIR\tcl8.6\tzdata\America\Nome ..\release\tcl8.6\tzdata\America\Nome
file /oname=$INSTDIR\tcl8.6\tzdata\America\Noronha ..\release\tcl8.6\tzdata\America\Noronha
file /oname=$INSTDIR\tcl8.6\tzdata\America\North_Dakota\Beulah ..\release\tcl8.6\tzdata\America\North_Dakota\Beulah
file /oname=$INSTDIR\tcl8.6\tzdata\America\North_Dakota\Center ..\release\tcl8.6\tzdata\America\North_Dakota\Center
file /oname=$INSTDIR\tcl8.6\tzdata\America\North_Dakota\New_Salem ..\release\tcl8.6\tzdata\America\North_Dakota\New_Salem
file /oname=$INSTDIR\tcl8.6\tzdata\America\Ojinaga ..\release\tcl8.6\tzdata\America\Ojinaga
file /oname=$INSTDIR\tcl8.6\tzdata\America\Panama ..\release\tcl8.6\tzdata\America\Panama
file /oname=$INSTDIR\tcl8.6\tzdata\America\Pangnirtung ..\release\tcl8.6\tzdata\America\Pangnirtung
file /oname=$INSTDIR\tcl8.6\tzdata\America\Paramaribo ..\release\tcl8.6\tzdata\America\Paramaribo
file /oname=$INSTDIR\tcl8.6\tzdata\America\Phoenix ..\release\tcl8.6\tzdata\America\Phoenix
file /oname=$INSTDIR\tcl8.6\tzdata\America\Port-au-Prince ..\release\tcl8.6\tzdata\America\Port-au-Prince
file /oname=$INSTDIR\tcl8.6\tzdata\America\Porto_Acre ..\release\tcl8.6\tzdata\America\Porto_Acre
file /oname=$INSTDIR\tcl8.6\tzdata\America\Porto_Velho ..\release\tcl8.6\tzdata\America\Porto_Velho
file /oname=$INSTDIR\tcl8.6\tzdata\America\Port_of_Spain ..\release\tcl8.6\tzdata\America\Port_of_Spain
file /oname=$INSTDIR\tcl8.6\tzdata\America\Puerto_Rico ..\release\tcl8.6\tzdata\America\Puerto_Rico
file /oname=$INSTDIR\tcl8.6\tzdata\America\Rainy_River ..\release\tcl8.6\tzdata\America\Rainy_River
file /oname=$INSTDIR\tcl8.6\tzdata\America\Rankin_Inlet ..\release\tcl8.6\tzdata\America\Rankin_Inlet
file /oname=$INSTDIR\tcl8.6\tzdata\America\Recife ..\release\tcl8.6\tzdata\America\Recife
file /oname=$INSTDIR\tcl8.6\tzdata\America\Regina ..\release\tcl8.6\tzdata\America\Regina
file /oname=$INSTDIR\tcl8.6\tzdata\America\Resolute ..\release\tcl8.6\tzdata\America\Resolute
file /oname=$INSTDIR\tcl8.6\tzdata\America\Rio_Branco ..\release\tcl8.6\tzdata\America\Rio_Branco
file /oname=$INSTDIR\tcl8.6\tzdata\America\Rosario ..\release\tcl8.6\tzdata\America\Rosario
file /oname=$INSTDIR\tcl8.6\tzdata\America\Santarem ..\release\tcl8.6\tzdata\America\Santarem
file /oname=$INSTDIR\tcl8.6\tzdata\America\Santa_Isabel ..\release\tcl8.6\tzdata\America\Santa_Isabel
file /oname=$INSTDIR\tcl8.6\tzdata\America\Santiago ..\release\tcl8.6\tzdata\America\Santiago
file /oname=$INSTDIR\tcl8.6\tzdata\America\Santo_Domingo ..\release\tcl8.6\tzdata\America\Santo_Domingo
file /oname=$INSTDIR\tcl8.6\tzdata\America\Sao_Paulo ..\release\tcl8.6\tzdata\America\Sao_Paulo
file /oname=$INSTDIR\tcl8.6\tzdata\America\Scoresbysund ..\release\tcl8.6\tzdata\America\Scoresbysund
file /oname=$INSTDIR\tcl8.6\tzdata\America\Shiprock ..\release\tcl8.6\tzdata\America\Shiprock
file /oname=$INSTDIR\tcl8.6\tzdata\America\Sitka ..\release\tcl8.6\tzdata\America\Sitka
file /oname=$INSTDIR\tcl8.6\tzdata\America\St_Barthelemy ..\release\tcl8.6\tzdata\America\St_Barthelemy
file /oname=$INSTDIR\tcl8.6\tzdata\America\St_Johns ..\release\tcl8.6\tzdata\America\St_Johns
file /oname=$INSTDIR\tcl8.6\tzdata\America\St_Kitts ..\release\tcl8.6\tzdata\America\St_Kitts
file /oname=$INSTDIR\tcl8.6\tzdata\America\St_Lucia ..\release\tcl8.6\tzdata\America\St_Lucia
file /oname=$INSTDIR\tcl8.6\tzdata\America\St_Thomas ..\release\tcl8.6\tzdata\America\St_Thomas
file /oname=$INSTDIR\tcl8.6\tzdata\America\St_Vincent ..\release\tcl8.6\tzdata\America\St_Vincent
file /oname=$INSTDIR\tcl8.6\tzdata\America\Swift_Current ..\release\tcl8.6\tzdata\America\Swift_Current
file /oname=$INSTDIR\tcl8.6\tzdata\America\Tegucigalpa ..\release\tcl8.6\tzdata\America\Tegucigalpa
file /oname=$INSTDIR\tcl8.6\tzdata\America\Thule ..\release\tcl8.6\tzdata\America\Thule
file /oname=$INSTDIR\tcl8.6\tzdata\America\Thunder_Bay ..\release\tcl8.6\tzdata\America\Thunder_Bay
file /oname=$INSTDIR\tcl8.6\tzdata\America\Tijuana ..\release\tcl8.6\tzdata\America\Tijuana
file /oname=$INSTDIR\tcl8.6\tzdata\America\Toronto ..\release\tcl8.6\tzdata\America\Toronto
file /oname=$INSTDIR\tcl8.6\tzdata\America\Tortola ..\release\tcl8.6\tzdata\America\Tortola
file /oname=$INSTDIR\tcl8.6\tzdata\America\Vancouver ..\release\tcl8.6\tzdata\America\Vancouver
file /oname=$INSTDIR\tcl8.6\tzdata\America\Virgin ..\release\tcl8.6\tzdata\America\Virgin
file /oname=$INSTDIR\tcl8.6\tzdata\America\Whitehorse ..\release\tcl8.6\tzdata\America\Whitehorse
file /oname=$INSTDIR\tcl8.6\tzdata\America\Winnipeg ..\release\tcl8.6\tzdata\America\Winnipeg
file /oname=$INSTDIR\tcl8.6\tzdata\America\Yakutat ..\release\tcl8.6\tzdata\America\Yakutat
file /oname=$INSTDIR\tcl8.6\tzdata\America\Yellowknife ..\release\tcl8.6\tzdata\America\Yellowknife
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\Casey ..\release\tcl8.6\tzdata\Antarctica\Casey
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\Davis ..\release\tcl8.6\tzdata\Antarctica\Davis
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\DumontDUrville ..\release\tcl8.6\tzdata\Antarctica\DumontDUrville
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\Macquarie ..\release\tcl8.6\tzdata\Antarctica\Macquarie
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\Mawson ..\release\tcl8.6\tzdata\Antarctica\Mawson
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\McMurdo ..\release\tcl8.6\tzdata\Antarctica\McMurdo
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\Palmer ..\release\tcl8.6\tzdata\Antarctica\Palmer
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\Rothera ..\release\tcl8.6\tzdata\Antarctica\Rothera
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\South_Pole ..\release\tcl8.6\tzdata\Antarctica\South_Pole
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\Syowa ..\release\tcl8.6\tzdata\Antarctica\Syowa
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\Troll ..\release\tcl8.6\tzdata\Antarctica\Troll
file /oname=$INSTDIR\tcl8.6\tzdata\Antarctica\Vostok ..\release\tcl8.6\tzdata\Antarctica\Vostok
file /oname=$INSTDIR\tcl8.6\tzdata\Arctic\Longyearbyen ..\release\tcl8.6\tzdata\Arctic\Longyearbyen
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Aden ..\release\tcl8.6\tzdata\Asia\Aden
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Almaty ..\release\tcl8.6\tzdata\Asia\Almaty
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Amman ..\release\tcl8.6\tzdata\Asia\Amman
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Anadyr ..\release\tcl8.6\tzdata\Asia\Anadyr
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Aqtau ..\release\tcl8.6\tzdata\Asia\Aqtau
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Aqtobe ..\release\tcl8.6\tzdata\Asia\Aqtobe
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Ashgabat ..\release\tcl8.6\tzdata\Asia\Ashgabat
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Ashkhabad ..\release\tcl8.6\tzdata\Asia\Ashkhabad
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Baghdad ..\release\tcl8.6\tzdata\Asia\Baghdad
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Bahrain ..\release\tcl8.6\tzdata\Asia\Bahrain
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Baku ..\release\tcl8.6\tzdata\Asia\Baku
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Bangkok ..\release\tcl8.6\tzdata\Asia\Bangkok
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Barnaul ..\release\tcl8.6\tzdata\Asia\Barnaul
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Beirut ..\release\tcl8.6\tzdata\Asia\Beirut
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Bishkek ..\release\tcl8.6\tzdata\Asia\Bishkek
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Brunei ..\release\tcl8.6\tzdata\Asia\Brunei
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Calcutta ..\release\tcl8.6\tzdata\Asia\Calcutta
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Chita ..\release\tcl8.6\tzdata\Asia\Chita
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Choibalsan ..\release\tcl8.6\tzdata\Asia\Choibalsan
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Chongqing ..\release\tcl8.6\tzdata\Asia\Chongqing
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Chungking ..\release\tcl8.6\tzdata\Asia\Chungking
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Colombo ..\release\tcl8.6\tzdata\Asia\Colombo
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Dacca ..\release\tcl8.6\tzdata\Asia\Dacca
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Damascus ..\release\tcl8.6\tzdata\Asia\Damascus
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Dhaka ..\release\tcl8.6\tzdata\Asia\Dhaka
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Dili ..\release\tcl8.6\tzdata\Asia\Dili
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Dubai ..\release\tcl8.6\tzdata\Asia\Dubai
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Dushanbe ..\release\tcl8.6\tzdata\Asia\Dushanbe
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Gaza ..\release\tcl8.6\tzdata\Asia\Gaza
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Harbin ..\release\tcl8.6\tzdata\Asia\Harbin
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Hebron ..\release\tcl8.6\tzdata\Asia\Hebron
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Hong_Kong ..\release\tcl8.6\tzdata\Asia\Hong_Kong
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Hovd ..\release\tcl8.6\tzdata\Asia\Hovd
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Ho_Chi_Minh ..\release\tcl8.6\tzdata\Asia\Ho_Chi_Minh
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Irkutsk ..\release\tcl8.6\tzdata\Asia\Irkutsk
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Istanbul ..\release\tcl8.6\tzdata\Asia\Istanbul
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Jakarta ..\release\tcl8.6\tzdata\Asia\Jakarta
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Jayapura ..\release\tcl8.6\tzdata\Asia\Jayapura
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Jerusalem ..\release\tcl8.6\tzdata\Asia\Jerusalem
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Kabul ..\release\tcl8.6\tzdata\Asia\Kabul
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Kamchatka ..\release\tcl8.6\tzdata\Asia\Kamchatka
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Karachi ..\release\tcl8.6\tzdata\Asia\Karachi
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Kashgar ..\release\tcl8.6\tzdata\Asia\Kashgar
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Kathmandu ..\release\tcl8.6\tzdata\Asia\Kathmandu
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Katmandu ..\release\tcl8.6\tzdata\Asia\Katmandu
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Khandyga ..\release\tcl8.6\tzdata\Asia\Khandyga
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Kolkata ..\release\tcl8.6\tzdata\Asia\Kolkata
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Krasnoyarsk ..\release\tcl8.6\tzdata\Asia\Krasnoyarsk
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Kuala_Lumpur ..\release\tcl8.6\tzdata\Asia\Kuala_Lumpur
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Kuching ..\release\tcl8.6\tzdata\Asia\Kuching
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Kuwait ..\release\tcl8.6\tzdata\Asia\Kuwait
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Macao ..\release\tcl8.6\tzdata\Asia\Macao
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Macau ..\release\tcl8.6\tzdata\Asia\Macau
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Magadan ..\release\tcl8.6\tzdata\Asia\Magadan
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Makassar ..\release\tcl8.6\tzdata\Asia\Makassar
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Manila ..\release\tcl8.6\tzdata\Asia\Manila
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Muscat ..\release\tcl8.6\tzdata\Asia\Muscat
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Nicosia ..\release\tcl8.6\tzdata\Asia\Nicosia
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Novokuznetsk ..\release\tcl8.6\tzdata\Asia\Novokuznetsk
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Novosibirsk ..\release\tcl8.6\tzdata\Asia\Novosibirsk
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Omsk ..\release\tcl8.6\tzdata\Asia\Omsk
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Oral ..\release\tcl8.6\tzdata\Asia\Oral
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Phnom_Penh ..\release\tcl8.6\tzdata\Asia\Phnom_Penh
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Pontianak ..\release\tcl8.6\tzdata\Asia\Pontianak
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Pyongyang ..\release\tcl8.6\tzdata\Asia\Pyongyang
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Qatar ..\release\tcl8.6\tzdata\Asia\Qatar
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Qyzylorda ..\release\tcl8.6\tzdata\Asia\Qyzylorda
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Rangoon ..\release\tcl8.6\tzdata\Asia\Rangoon
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Riyadh ..\release\tcl8.6\tzdata\Asia\Riyadh
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Saigon ..\release\tcl8.6\tzdata\Asia\Saigon
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Sakhalin ..\release\tcl8.6\tzdata\Asia\Sakhalin
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Samarkand ..\release\tcl8.6\tzdata\Asia\Samarkand
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Seoul ..\release\tcl8.6\tzdata\Asia\Seoul
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Shanghai ..\release\tcl8.6\tzdata\Asia\Shanghai
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Singapore ..\release\tcl8.6\tzdata\Asia\Singapore
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Srednekolymsk ..\release\tcl8.6\tzdata\Asia\Srednekolymsk
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Taipei ..\release\tcl8.6\tzdata\Asia\Taipei
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Tashkent ..\release\tcl8.6\tzdata\Asia\Tashkent
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Tbilisi ..\release\tcl8.6\tzdata\Asia\Tbilisi
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Tehran ..\release\tcl8.6\tzdata\Asia\Tehran
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Tel_Aviv ..\release\tcl8.6\tzdata\Asia\Tel_Aviv
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Thimbu ..\release\tcl8.6\tzdata\Asia\Thimbu
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Thimphu ..\release\tcl8.6\tzdata\Asia\Thimphu
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Tokyo ..\release\tcl8.6\tzdata\Asia\Tokyo
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Tomsk ..\release\tcl8.6\tzdata\Asia\Tomsk
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Ujung_Pandang ..\release\tcl8.6\tzdata\Asia\Ujung_Pandang
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Ulaanbaatar ..\release\tcl8.6\tzdata\Asia\Ulaanbaatar
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Ulan_Bator ..\release\tcl8.6\tzdata\Asia\Ulan_Bator
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Urumqi ..\release\tcl8.6\tzdata\Asia\Urumqi
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Ust-Nera ..\release\tcl8.6\tzdata\Asia\Ust-Nera
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Vientiane ..\release\tcl8.6\tzdata\Asia\Vientiane
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Vladivostok ..\release\tcl8.6\tzdata\Asia\Vladivostok
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Yakutsk ..\release\tcl8.6\tzdata\Asia\Yakutsk
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Yekaterinburg ..\release\tcl8.6\tzdata\Asia\Yekaterinburg
file /oname=$INSTDIR\tcl8.6\tzdata\Asia\Yerevan ..\release\tcl8.6\tzdata\Asia\Yerevan
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Azores ..\release\tcl8.6\tzdata\Atlantic\Azores
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Bermuda ..\release\tcl8.6\tzdata\Atlantic\Bermuda
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Canary ..\release\tcl8.6\tzdata\Atlantic\Canary
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Cape_Verde ..\release\tcl8.6\tzdata\Atlantic\Cape_Verde
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Faeroe ..\release\tcl8.6\tzdata\Atlantic\Faeroe
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Faroe ..\release\tcl8.6\tzdata\Atlantic\Faroe
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Jan_Mayen ..\release\tcl8.6\tzdata\Atlantic\Jan_Mayen
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Madeira ..\release\tcl8.6\tzdata\Atlantic\Madeira
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Reykjavik ..\release\tcl8.6\tzdata\Atlantic\Reykjavik
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\South_Georgia ..\release\tcl8.6\tzdata\Atlantic\South_Georgia
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\Stanley ..\release\tcl8.6\tzdata\Atlantic\Stanley
file /oname=$INSTDIR\tcl8.6\tzdata\Atlantic\St_Helena ..\release\tcl8.6\tzdata\Atlantic\St_Helena
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\ACT ..\release\tcl8.6\tzdata\Australia\ACT
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Adelaide ..\release\tcl8.6\tzdata\Australia\Adelaide
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Brisbane ..\release\tcl8.6\tzdata\Australia\Brisbane
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Broken_Hill ..\release\tcl8.6\tzdata\Australia\Broken_Hill
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Canberra ..\release\tcl8.6\tzdata\Australia\Canberra
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Currie ..\release\tcl8.6\tzdata\Australia\Currie
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Darwin ..\release\tcl8.6\tzdata\Australia\Darwin
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Eucla ..\release\tcl8.6\tzdata\Australia\Eucla
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Hobart ..\release\tcl8.6\tzdata\Australia\Hobart
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\LHI ..\release\tcl8.6\tzdata\Australia\LHI
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Lindeman ..\release\tcl8.6\tzdata\Australia\Lindeman
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Lord_Howe ..\release\tcl8.6\tzdata\Australia\Lord_Howe
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Melbourne ..\release\tcl8.6\tzdata\Australia\Melbourne
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\North ..\release\tcl8.6\tzdata\Australia\North
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\NSW ..\release\tcl8.6\tzdata\Australia\NSW
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Perth ..\release\tcl8.6\tzdata\Australia\Perth
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Queensland ..\release\tcl8.6\tzdata\Australia\Queensland
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\South ..\release\tcl8.6\tzdata\Australia\South
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Sydney ..\release\tcl8.6\tzdata\Australia\Sydney
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Tasmania ..\release\tcl8.6\tzdata\Australia\Tasmania
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Victoria ..\release\tcl8.6\tzdata\Australia\Victoria
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\West ..\release\tcl8.6\tzdata\Australia\West
file /oname=$INSTDIR\tcl8.6\tzdata\Australia\Yancowinna ..\release\tcl8.6\tzdata\Australia\Yancowinna
file /oname=$INSTDIR\tcl8.6\tzdata\Brazil\Acre ..\release\tcl8.6\tzdata\Brazil\Acre
file /oname=$INSTDIR\tcl8.6\tzdata\Brazil\DeNoronha ..\release\tcl8.6\tzdata\Brazil\DeNoronha
file /oname=$INSTDIR\tcl8.6\tzdata\Brazil\East ..\release\tcl8.6\tzdata\Brazil\East
file /oname=$INSTDIR\tcl8.6\tzdata\Brazil\West ..\release\tcl8.6\tzdata\Brazil\West
file /oname=$INSTDIR\tcl8.6\tzdata\Canada\Atlantic ..\release\tcl8.6\tzdata\Canada\Atlantic
file /oname=$INSTDIR\tcl8.6\tzdata\Canada\Central ..\release\tcl8.6\tzdata\Canada\Central
file /oname=$INSTDIR\tcl8.6\tzdata\Canada\East-Saskatchewan ..\release\tcl8.6\tzdata\Canada\East-Saskatchewan
file /oname=$INSTDIR\tcl8.6\tzdata\Canada\Eastern ..\release\tcl8.6\tzdata\Canada\Eastern
file /oname=$INSTDIR\tcl8.6\tzdata\Canada\Mountain ..\release\tcl8.6\tzdata\Canada\Mountain
file /oname=$INSTDIR\tcl8.6\tzdata\Canada\Newfoundland ..\release\tcl8.6\tzdata\Canada\Newfoundland
file /oname=$INSTDIR\tcl8.6\tzdata\Canada\Pacific ..\release\tcl8.6\tzdata\Canada\Pacific
file /oname=$INSTDIR\tcl8.6\tzdata\Canada\Saskatchewan ..\release\tcl8.6\tzdata\Canada\Saskatchewan
file /oname=$INSTDIR\tcl8.6\tzdata\Canada\Yukon ..\release\tcl8.6\tzdata\Canada\Yukon
file /oname=$INSTDIR\tcl8.6\tzdata\CET ..\release\tcl8.6\tzdata\CET
file /oname=$INSTDIR\tcl8.6\tzdata\Chile\Continental ..\release\tcl8.6\tzdata\Chile\Continental
file /oname=$INSTDIR\tcl8.6\tzdata\Chile\EasterIsland ..\release\tcl8.6\tzdata\Chile\EasterIsland
file /oname=$INSTDIR\tcl8.6\tzdata\CST6CDT ..\release\tcl8.6\tzdata\CST6CDT
file /oname=$INSTDIR\tcl8.6\tzdata\Cuba ..\release\tcl8.6\tzdata\Cuba
file /oname=$INSTDIR\tcl8.6\tzdata\EET ..\release\tcl8.6\tzdata\EET
file /oname=$INSTDIR\tcl8.6\tzdata\Egypt ..\release\tcl8.6\tzdata\Egypt
file /oname=$INSTDIR\tcl8.6\tzdata\Eire ..\release\tcl8.6\tzdata\Eire
file /oname=$INSTDIR\tcl8.6\tzdata\EST ..\release\tcl8.6\tzdata\EST
file /oname=$INSTDIR\tcl8.6\tzdata\EST5EDT ..\release\tcl8.6\tzdata\EST5EDT
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT ..\release\tcl8.6\tzdata\Etc\GMT
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+0 ..\release\tcl8.6\tzdata\Etc\GMT+0
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+1 ..\release\tcl8.6\tzdata\Etc\GMT+1
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+10 ..\release\tcl8.6\tzdata\Etc\GMT+10
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+11 ..\release\tcl8.6\tzdata\Etc\GMT+11
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+12 ..\release\tcl8.6\tzdata\Etc\GMT+12
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+2 ..\release\tcl8.6\tzdata\Etc\GMT+2
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+3 ..\release\tcl8.6\tzdata\Etc\GMT+3
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+4 ..\release\tcl8.6\tzdata\Etc\GMT+4
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+5 ..\release\tcl8.6\tzdata\Etc\GMT+5
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+6 ..\release\tcl8.6\tzdata\Etc\GMT+6
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+7 ..\release\tcl8.6\tzdata\Etc\GMT+7
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+8 ..\release\tcl8.6\tzdata\Etc\GMT+8
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT+9 ..\release\tcl8.6\tzdata\Etc\GMT+9
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-0 ..\release\tcl8.6\tzdata\Etc\GMT-0
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-1 ..\release\tcl8.6\tzdata\Etc\GMT-1
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-10 ..\release\tcl8.6\tzdata\Etc\GMT-10
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-11 ..\release\tcl8.6\tzdata\Etc\GMT-11
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-12 ..\release\tcl8.6\tzdata\Etc\GMT-12
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-13 ..\release\tcl8.6\tzdata\Etc\GMT-13
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-14 ..\release\tcl8.6\tzdata\Etc\GMT-14
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-2 ..\release\tcl8.6\tzdata\Etc\GMT-2
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-3 ..\release\tcl8.6\tzdata\Etc\GMT-3
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-4 ..\release\tcl8.6\tzdata\Etc\GMT-4
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-5 ..\release\tcl8.6\tzdata\Etc\GMT-5
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-6 ..\release\tcl8.6\tzdata\Etc\GMT-6
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-7 ..\release\tcl8.6\tzdata\Etc\GMT-7
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-8 ..\release\tcl8.6\tzdata\Etc\GMT-8
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT-9 ..\release\tcl8.6\tzdata\Etc\GMT-9
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\GMT0 ..\release\tcl8.6\tzdata\Etc\GMT0
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\Greenwich ..\release\tcl8.6\tzdata\Etc\Greenwich
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\UCT ..\release\tcl8.6\tzdata\Etc\UCT
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\Universal ..\release\tcl8.6\tzdata\Etc\Universal
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\UTC ..\release\tcl8.6\tzdata\Etc\UTC
file /oname=$INSTDIR\tcl8.6\tzdata\Etc\Zulu ..\release\tcl8.6\tzdata\Etc\Zulu
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Amsterdam ..\release\tcl8.6\tzdata\Europe\Amsterdam
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Andorra ..\release\tcl8.6\tzdata\Europe\Andorra
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Astrakhan ..\release\tcl8.6\tzdata\Europe\Astrakhan
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Athens ..\release\tcl8.6\tzdata\Europe\Athens
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Belfast ..\release\tcl8.6\tzdata\Europe\Belfast
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Belgrade ..\release\tcl8.6\tzdata\Europe\Belgrade
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Berlin ..\release\tcl8.6\tzdata\Europe\Berlin
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Bratislava ..\release\tcl8.6\tzdata\Europe\Bratislava
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Brussels ..\release\tcl8.6\tzdata\Europe\Brussels
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Bucharest ..\release\tcl8.6\tzdata\Europe\Bucharest
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Budapest ..\release\tcl8.6\tzdata\Europe\Budapest
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Busingen ..\release\tcl8.6\tzdata\Europe\Busingen
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Chisinau ..\release\tcl8.6\tzdata\Europe\Chisinau
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Copenhagen ..\release\tcl8.6\tzdata\Europe\Copenhagen
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Dublin ..\release\tcl8.6\tzdata\Europe\Dublin
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Gibraltar ..\release\tcl8.6\tzdata\Europe\Gibraltar
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Guernsey ..\release\tcl8.6\tzdata\Europe\Guernsey
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Helsinki ..\release\tcl8.6\tzdata\Europe\Helsinki
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Isle_of_Man ..\release\tcl8.6\tzdata\Europe\Isle_of_Man
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Istanbul ..\release\tcl8.6\tzdata\Europe\Istanbul
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Jersey ..\release\tcl8.6\tzdata\Europe\Jersey
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Kaliningrad ..\release\tcl8.6\tzdata\Europe\Kaliningrad
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Kiev ..\release\tcl8.6\tzdata\Europe\Kiev
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Kirov ..\release\tcl8.6\tzdata\Europe\Kirov
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Lisbon ..\release\tcl8.6\tzdata\Europe\Lisbon
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Ljubljana ..\release\tcl8.6\tzdata\Europe\Ljubljana
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\London ..\release\tcl8.6\tzdata\Europe\London
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Luxembourg ..\release\tcl8.6\tzdata\Europe\Luxembourg
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Madrid ..\release\tcl8.6\tzdata\Europe\Madrid
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Malta ..\release\tcl8.6\tzdata\Europe\Malta
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Mariehamn ..\release\tcl8.6\tzdata\Europe\Mariehamn
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Minsk ..\release\tcl8.6\tzdata\Europe\Minsk
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Monaco ..\release\tcl8.6\tzdata\Europe\Monaco
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Moscow ..\release\tcl8.6\tzdata\Europe\Moscow
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Nicosia ..\release\tcl8.6\tzdata\Europe\Nicosia
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Oslo ..\release\tcl8.6\tzdata\Europe\Oslo
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Paris ..\release\tcl8.6\tzdata\Europe\Paris
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Podgorica ..\release\tcl8.6\tzdata\Europe\Podgorica
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Prague ..\release\tcl8.6\tzdata\Europe\Prague
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Riga ..\release\tcl8.6\tzdata\Europe\Riga
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Rome ..\release\tcl8.6\tzdata\Europe\Rome
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Samara ..\release\tcl8.6\tzdata\Europe\Samara
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\San_Marino ..\release\tcl8.6\tzdata\Europe\San_Marino
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Sarajevo ..\release\tcl8.6\tzdata\Europe\Sarajevo
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Simferopol ..\release\tcl8.6\tzdata\Europe\Simferopol
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Skopje ..\release\tcl8.6\tzdata\Europe\Skopje
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Sofia ..\release\tcl8.6\tzdata\Europe\Sofia
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Stockholm ..\release\tcl8.6\tzdata\Europe\Stockholm
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Tallinn ..\release\tcl8.6\tzdata\Europe\Tallinn
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Tirane ..\release\tcl8.6\tzdata\Europe\Tirane
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Tiraspol ..\release\tcl8.6\tzdata\Europe\Tiraspol
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Ulyanovsk ..\release\tcl8.6\tzdata\Europe\Ulyanovsk
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Uzhgorod ..\release\tcl8.6\tzdata\Europe\Uzhgorod
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Vaduz ..\release\tcl8.6\tzdata\Europe\Vaduz
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Vatican ..\release\tcl8.6\tzdata\Europe\Vatican
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Vienna ..\release\tcl8.6\tzdata\Europe\Vienna
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Vilnius ..\release\tcl8.6\tzdata\Europe\Vilnius
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Volgograd ..\release\tcl8.6\tzdata\Europe\Volgograd
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Warsaw ..\release\tcl8.6\tzdata\Europe\Warsaw
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Zagreb ..\release\tcl8.6\tzdata\Europe\Zagreb
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Zaporozhye ..\release\tcl8.6\tzdata\Europe\Zaporozhye
file /oname=$INSTDIR\tcl8.6\tzdata\Europe\Zurich ..\release\tcl8.6\tzdata\Europe\Zurich
file /oname=$INSTDIR\tcl8.6\tzdata\GB ..\release\tcl8.6\tzdata\GB
file /oname=$INSTDIR\tcl8.6\tzdata\GB-Eire ..\release\tcl8.6\tzdata\GB-Eire
file /oname=$INSTDIR\tcl8.6\tzdata\GMT ..\release\tcl8.6\tzdata\GMT
file /oname=$INSTDIR\tcl8.6\tzdata\GMT+0 ..\release\tcl8.6\tzdata\GMT+0
file /oname=$INSTDIR\tcl8.6\tzdata\GMT-0 ..\release\tcl8.6\tzdata\GMT-0
file /oname=$INSTDIR\tcl8.6\tzdata\GMT0 ..\release\tcl8.6\tzdata\GMT0
file /oname=$INSTDIR\tcl8.6\tzdata\Greenwich ..\release\tcl8.6\tzdata\Greenwich
file /oname=$INSTDIR\tcl8.6\tzdata\Hongkong ..\release\tcl8.6\tzdata\Hongkong
file /oname=$INSTDIR\tcl8.6\tzdata\HST ..\release\tcl8.6\tzdata\HST
file /oname=$INSTDIR\tcl8.6\tzdata\Iceland ..\release\tcl8.6\tzdata\Iceland
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Antananarivo ..\release\tcl8.6\tzdata\Indian\Antananarivo
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Chagos ..\release\tcl8.6\tzdata\Indian\Chagos
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Christmas ..\release\tcl8.6\tzdata\Indian\Christmas
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Cocos ..\release\tcl8.6\tzdata\Indian\Cocos
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Comoro ..\release\tcl8.6\tzdata\Indian\Comoro
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Kerguelen ..\release\tcl8.6\tzdata\Indian\Kerguelen
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Mahe ..\release\tcl8.6\tzdata\Indian\Mahe
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Maldives ..\release\tcl8.6\tzdata\Indian\Maldives
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Mauritius ..\release\tcl8.6\tzdata\Indian\Mauritius
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Mayotte ..\release\tcl8.6\tzdata\Indian\Mayotte
file /oname=$INSTDIR\tcl8.6\tzdata\Indian\Reunion ..\release\tcl8.6\tzdata\Indian\Reunion
file /oname=$INSTDIR\tcl8.6\tzdata\Iran ..\release\tcl8.6\tzdata\Iran
file /oname=$INSTDIR\tcl8.6\tzdata\Israel ..\release\tcl8.6\tzdata\Israel
file /oname=$INSTDIR\tcl8.6\tzdata\Jamaica ..\release\tcl8.6\tzdata\Jamaica
file /oname=$INSTDIR\tcl8.6\tzdata\Japan ..\release\tcl8.6\tzdata\Japan
file /oname=$INSTDIR\tcl8.6\tzdata\Kwajalein ..\release\tcl8.6\tzdata\Kwajalein
file /oname=$INSTDIR\tcl8.6\tzdata\Libya ..\release\tcl8.6\tzdata\Libya
file /oname=$INSTDIR\tcl8.6\tzdata\MET ..\release\tcl8.6\tzdata\MET
file /oname=$INSTDIR\tcl8.6\tzdata\Mexico\BajaNorte ..\release\tcl8.6\tzdata\Mexico\BajaNorte
file /oname=$INSTDIR\tcl8.6\tzdata\Mexico\BajaSur ..\release\tcl8.6\tzdata\Mexico\BajaSur
file /oname=$INSTDIR\tcl8.6\tzdata\Mexico\General ..\release\tcl8.6\tzdata\Mexico\General
file /oname=$INSTDIR\tcl8.6\tzdata\MST ..\release\tcl8.6\tzdata\MST
file /oname=$INSTDIR\tcl8.6\tzdata\MST7MDT ..\release\tcl8.6\tzdata\MST7MDT
file /oname=$INSTDIR\tcl8.6\tzdata\Navajo ..\release\tcl8.6\tzdata\Navajo
file /oname=$INSTDIR\tcl8.6\tzdata\NZ ..\release\tcl8.6\tzdata\NZ
file /oname=$INSTDIR\tcl8.6\tzdata\NZ-CHAT ..\release\tcl8.6\tzdata\NZ-CHAT
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Apia ..\release\tcl8.6\tzdata\Pacific\Apia
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Auckland ..\release\tcl8.6\tzdata\Pacific\Auckland
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Bougainville ..\release\tcl8.6\tzdata\Pacific\Bougainville
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Chatham ..\release\tcl8.6\tzdata\Pacific\Chatham
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Chuuk ..\release\tcl8.6\tzdata\Pacific\Chuuk
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Easter ..\release\tcl8.6\tzdata\Pacific\Easter
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Efate ..\release\tcl8.6\tzdata\Pacific\Efate
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Enderbury ..\release\tcl8.6\tzdata\Pacific\Enderbury
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Fakaofo ..\release\tcl8.6\tzdata\Pacific\Fakaofo
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Fiji ..\release\tcl8.6\tzdata\Pacific\Fiji
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Funafuti ..\release\tcl8.6\tzdata\Pacific\Funafuti
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Galapagos ..\release\tcl8.6\tzdata\Pacific\Galapagos
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Gambier ..\release\tcl8.6\tzdata\Pacific\Gambier
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Guadalcanal ..\release\tcl8.6\tzdata\Pacific\Guadalcanal
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Guam ..\release\tcl8.6\tzdata\Pacific\Guam
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Honolulu ..\release\tcl8.6\tzdata\Pacific\Honolulu
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Johnston ..\release\tcl8.6\tzdata\Pacific\Johnston
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Kiritimati ..\release\tcl8.6\tzdata\Pacific\Kiritimati
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Kosrae ..\release\tcl8.6\tzdata\Pacific\Kosrae
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Kwajalein ..\release\tcl8.6\tzdata\Pacific\Kwajalein
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Majuro ..\release\tcl8.6\tzdata\Pacific\Majuro
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Marquesas ..\release\tcl8.6\tzdata\Pacific\Marquesas
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Midway ..\release\tcl8.6\tzdata\Pacific\Midway
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Nauru ..\release\tcl8.6\tzdata\Pacific\Nauru
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Niue ..\release\tcl8.6\tzdata\Pacific\Niue
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Norfolk ..\release\tcl8.6\tzdata\Pacific\Norfolk
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Noumea ..\release\tcl8.6\tzdata\Pacific\Noumea
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Pago_Pago ..\release\tcl8.6\tzdata\Pacific\Pago_Pago
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Palau ..\release\tcl8.6\tzdata\Pacific\Palau
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Pitcairn ..\release\tcl8.6\tzdata\Pacific\Pitcairn
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Pohnpei ..\release\tcl8.6\tzdata\Pacific\Pohnpei
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Ponape ..\release\tcl8.6\tzdata\Pacific\Ponape
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Port_Moresby ..\release\tcl8.6\tzdata\Pacific\Port_Moresby
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Rarotonga ..\release\tcl8.6\tzdata\Pacific\Rarotonga
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Saipan ..\release\tcl8.6\tzdata\Pacific\Saipan
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Samoa ..\release\tcl8.6\tzdata\Pacific\Samoa
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Tahiti ..\release\tcl8.6\tzdata\Pacific\Tahiti
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Tarawa ..\release\tcl8.6\tzdata\Pacific\Tarawa
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Tongatapu ..\release\tcl8.6\tzdata\Pacific\Tongatapu
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Truk ..\release\tcl8.6\tzdata\Pacific\Truk
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Wake ..\release\tcl8.6\tzdata\Pacific\Wake
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Wallis ..\release\tcl8.6\tzdata\Pacific\Wallis
file /oname=$INSTDIR\tcl8.6\tzdata\Pacific\Yap ..\release\tcl8.6\tzdata\Pacific\Yap
file /oname=$INSTDIR\tcl8.6\tzdata\Poland ..\release\tcl8.6\tzdata\Poland
file /oname=$INSTDIR\tcl8.6\tzdata\Portugal ..\release\tcl8.6\tzdata\Portugal
file /oname=$INSTDIR\tcl8.6\tzdata\PRC ..\release\tcl8.6\tzdata\PRC
file /oname=$INSTDIR\tcl8.6\tzdata\PST8PDT ..\release\tcl8.6\tzdata\PST8PDT
file /oname=$INSTDIR\tcl8.6\tzdata\ROC ..\release\tcl8.6\tzdata\ROC
file /oname=$INSTDIR\tcl8.6\tzdata\ROK ..\release\tcl8.6\tzdata\ROK
file /oname=$INSTDIR\tcl8.6\tzdata\Singapore ..\release\tcl8.6\tzdata\Singapore
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\AST4 ..\release\tcl8.6\tzdata\SystemV\AST4
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\AST4ADT ..\release\tcl8.6\tzdata\SystemV\AST4ADT
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\CST6 ..\release\tcl8.6\tzdata\SystemV\CST6
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\CST6CDT ..\release\tcl8.6\tzdata\SystemV\CST6CDT
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\EST5 ..\release\tcl8.6\tzdata\SystemV\EST5
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\EST5EDT ..\release\tcl8.6\tzdata\SystemV\EST5EDT
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\HST10 ..\release\tcl8.6\tzdata\SystemV\HST10
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\MST7 ..\release\tcl8.6\tzdata\SystemV\MST7
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\MST7MDT ..\release\tcl8.6\tzdata\SystemV\MST7MDT
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\PST8 ..\release\tcl8.6\tzdata\SystemV\PST8
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\PST8PDT ..\release\tcl8.6\tzdata\SystemV\PST8PDT
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\YST9 ..\release\tcl8.6\tzdata\SystemV\YST9
file /oname=$INSTDIR\tcl8.6\tzdata\SystemV\YST9YDT ..\release\tcl8.6\tzdata\SystemV\YST9YDT
file /oname=$INSTDIR\tcl8.6\tzdata\Turkey ..\release\tcl8.6\tzdata\Turkey
file /oname=$INSTDIR\tcl8.6\tzdata\UCT ..\release\tcl8.6\tzdata\UCT
file /oname=$INSTDIR\tcl8.6\tzdata\Universal ..\release\tcl8.6\tzdata\Universal
file /oname=$INSTDIR\tcl8.6\tzdata\US\Alaska ..\release\tcl8.6\tzdata\US\Alaska
file /oname=$INSTDIR\tcl8.6\tzdata\US\Aleutian ..\release\tcl8.6\tzdata\US\Aleutian
file /oname=$INSTDIR\tcl8.6\tzdata\US\Arizona ..\release\tcl8.6\tzdata\US\Arizona
file /oname=$INSTDIR\tcl8.6\tzdata\US\Central ..\release\tcl8.6\tzdata\US\Central
file /oname=$INSTDIR\tcl8.6\tzdata\US\East-Indiana ..\release\tcl8.6\tzdata\US\East-Indiana
file /oname=$INSTDIR\tcl8.6\tzdata\US\Eastern ..\release\tcl8.6\tzdata\US\Eastern
file /oname=$INSTDIR\tcl8.6\tzdata\US\Hawaii ..\release\tcl8.6\tzdata\US\Hawaii
file /oname=$INSTDIR\tcl8.6\tzdata\US\Indiana-Starke ..\release\tcl8.6\tzdata\US\Indiana-Starke
file /oname=$INSTDIR\tcl8.6\tzdata\US\Michigan ..\release\tcl8.6\tzdata\US\Michigan
file /oname=$INSTDIR\tcl8.6\tzdata\US\Mountain ..\release\tcl8.6\tzdata\US\Mountain
file /oname=$INSTDIR\tcl8.6\tzdata\US\Pacific ..\release\tcl8.6\tzdata\US\Pacific
file /oname=$INSTDIR\tcl8.6\tzdata\US\Pacific-New ..\release\tcl8.6\tzdata\US\Pacific-New
file /oname=$INSTDIR\tcl8.6\tzdata\US\Samoa ..\release\tcl8.6\tzdata\US\Samoa
file /oname=$INSTDIR\tcl8.6\tzdata\UTC ..\release\tcl8.6\tzdata\UTC
file /oname=$INSTDIR\tcl8.6\tzdata\W-SU ..\release\tcl8.6\tzdata\W-SU
file /oname=$INSTDIR\tcl8.6\tzdata\WET ..\release\tcl8.6\tzdata\WET
file /oname=$INSTDIR\tcl8.6\tzdata\Zulu ..\release\tcl8.6\tzdata\Zulu
file /oname=$INSTDIR\tcl8.6\word.tcl ..\release\tcl8.6\word.tcl
file /oname=$INSTDIR\tk8.6\bgerror.tcl ..\release\tk8.6\bgerror.tcl
file /oname=$INSTDIR\tk8.6\button.tcl ..\release\tk8.6\button.tcl
file /oname=$INSTDIR\tk8.6\choosedir.tcl ..\release\tk8.6\choosedir.tcl
file /oname=$INSTDIR\tk8.6\clrpick.tcl ..\release\tk8.6\clrpick.tcl
file /oname=$INSTDIR\tk8.6\comdlg.tcl ..\release\tk8.6\comdlg.tcl
file /oname=$INSTDIR\tk8.6\console.tcl ..\release\tk8.6\console.tcl
file /oname=$INSTDIR\tk8.6\dialog.tcl ..\release\tk8.6\dialog.tcl
file /oname=$INSTDIR\tk8.6\entry.tcl ..\release\tk8.6\entry.tcl
file /oname=$INSTDIR\tk8.6\focus.tcl ..\release\tk8.6\focus.tcl
file /oname=$INSTDIR\tk8.6\fontchooser.tcl ..\release\tk8.6\fontchooser.tcl
file /oname=$INSTDIR\tk8.6\iconlist.tcl ..\release\tk8.6\iconlist.tcl
file /oname=$INSTDIR\tk8.6\icons.tcl ..\release\tk8.6\icons.tcl
file /oname=$INSTDIR\tk8.6\images\logo.eps ..\release\tk8.6\images\logo.eps
file /oname=$INSTDIR\tk8.6\images\logo100.gif ..\release\tk8.6\images\logo100.gif
file /oname=$INSTDIR\tk8.6\images\logo64.gif ..\release\tk8.6\images\logo64.gif
file /oname=$INSTDIR\tk8.6\images\logoLarge.gif ..\release\tk8.6\images\logoLarge.gif
file /oname=$INSTDIR\tk8.6\images\logoMed.gif ..\release\tk8.6\images\logoMed.gif
file /oname=$INSTDIR\tk8.6\images\pwrdLogo.eps ..\release\tk8.6\images\pwrdLogo.eps
file /oname=$INSTDIR\tk8.6\images\pwrdLogo100.gif ..\release\tk8.6\images\pwrdLogo100.gif
file /oname=$INSTDIR\tk8.6\images\pwrdLogo150.gif ..\release\tk8.6\images\pwrdLogo150.gif
file /oname=$INSTDIR\tk8.6\images\pwrdLogo175.gif ..\release\tk8.6\images\pwrdLogo175.gif
file /oname=$INSTDIR\tk8.6\images\pwrdLogo200.gif ..\release\tk8.6\images\pwrdLogo200.gif
file /oname=$INSTDIR\tk8.6\images\pwrdLogo75.gif ..\release\tk8.6\images\pwrdLogo75.gif
file /oname=$INSTDIR\tk8.6\images\README ..\release\tk8.6\images\README
file /oname=$INSTDIR\tk8.6\images\tai-ku.gif ..\release\tk8.6\images\tai-ku.gif
file /oname=$INSTDIR\tk8.6\listbox.tcl ..\release\tk8.6\listbox.tcl
file /oname=$INSTDIR\tk8.6\megawidget.tcl ..\release\tk8.6\megawidget.tcl
file /oname=$INSTDIR\tk8.6\menu.tcl ..\release\tk8.6\menu.tcl
file /oname=$INSTDIR\tk8.6\mkpsenc.tcl ..\release\tk8.6\mkpsenc.tcl
file /oname=$INSTDIR\tk8.6\msgbox.tcl ..\release\tk8.6\msgbox.tcl
file /oname=$INSTDIR\tk8.6\msgs\cs.msg ..\release\tk8.6\msgs\cs.msg
file /oname=$INSTDIR\tk8.6\msgs\da.msg ..\release\tk8.6\msgs\da.msg
file /oname=$INSTDIR\tk8.6\msgs\de.msg ..\release\tk8.6\msgs\de.msg
file /oname=$INSTDIR\tk8.6\msgs\el.msg ..\release\tk8.6\msgs\el.msg
file /oname=$INSTDIR\tk8.6\msgs\en.msg ..\release\tk8.6\msgs\en.msg
file /oname=$INSTDIR\tk8.6\msgs\en_gb.msg ..\release\tk8.6\msgs\en_gb.msg
file /oname=$INSTDIR\tk8.6\msgs\eo.msg ..\release\tk8.6\msgs\eo.msg
file /oname=$INSTDIR\tk8.6\msgs\es.msg ..\release\tk8.6\msgs\es.msg
file /oname=$INSTDIR\tk8.6\msgs\fr.msg ..\release\tk8.6\msgs\fr.msg
file /oname=$INSTDIR\tk8.6\msgs\hu.msg ..\release\tk8.6\msgs\hu.msg
file /oname=$INSTDIR\tk8.6\msgs\it.msg ..\release\tk8.6\msgs\it.msg
file /oname=$INSTDIR\tk8.6\msgs\nl.msg ..\release\tk8.6\msgs\nl.msg
file /oname=$INSTDIR\tk8.6\msgs\pl.msg ..\release\tk8.6\msgs\pl.msg
file /oname=$INSTDIR\tk8.6\msgs\pt.msg ..\release\tk8.6\msgs\pt.msg
file /oname=$INSTDIR\tk8.6\msgs\ru.msg ..\release\tk8.6\msgs\ru.msg
file /oname=$INSTDIR\tk8.6\msgs\sv.msg ..\release\tk8.6\msgs\sv.msg
file /oname=$INSTDIR\tk8.6\obsolete.tcl ..\release\tk8.6\obsolete.tcl
file /oname=$INSTDIR\tk8.6\optMenu.tcl ..\release\tk8.6\optMenu.tcl
file /oname=$INSTDIR\tk8.6\palette.tcl ..\release\tk8.6\palette.tcl
file /oname=$INSTDIR\tk8.6\panedwindow.tcl ..\release\tk8.6\panedwindow.tcl
file /oname=$INSTDIR\tk8.6\pkgIndex.tcl ..\release\tk8.6\pkgIndex.tcl
file /oname=$INSTDIR\tk8.6\safetk.tcl ..\release\tk8.6\safetk.tcl
file /oname=$INSTDIR\tk8.6\scale.tcl ..\release\tk8.6\scale.tcl
file /oname=$INSTDIR\tk8.6\scrlbar.tcl ..\release\tk8.6\scrlbar.tcl
file /oname=$INSTDIR\tk8.6\spinbox.tcl ..\release\tk8.6\spinbox.tcl
file /oname=$INSTDIR\tk8.6\tclIndex ..\release\tk8.6\tclIndex
file /oname=$INSTDIR\tk8.6\tearoff.tcl ..\release\tk8.6\tearoff.tcl
file /oname=$INSTDIR\tk8.6\text.tcl ..\release\tk8.6\text.tcl
file /oname=$INSTDIR\tk8.6\tk.tcl ..\release\tk8.6\tk.tcl
file /oname=$INSTDIR\tk8.6\tkAppInit.c ..\release\tk8.6\tkAppInit.c
file /oname=$INSTDIR\tk8.6\tkfbox.tcl ..\release\tk8.6\tkfbox.tcl
file /oname=$INSTDIR\tk8.6\ttk\altTheme.tcl ..\release\tk8.6\ttk\altTheme.tcl
file /oname=$INSTDIR\tk8.6\ttk\aquaTheme.tcl ..\release\tk8.6\ttk\aquaTheme.tcl
file /oname=$INSTDIR\tk8.6\ttk\button.tcl ..\release\tk8.6\ttk\button.tcl
file /oname=$INSTDIR\tk8.6\ttk\clamTheme.tcl ..\release\tk8.6\ttk\clamTheme.tcl
file /oname=$INSTDIR\tk8.6\ttk\classicTheme.tcl ..\release\tk8.6\ttk\classicTheme.tcl
file /oname=$INSTDIR\tk8.6\ttk\combobox.tcl ..\release\tk8.6\ttk\combobox.tcl
file /oname=$INSTDIR\tk8.6\ttk\cursors.tcl ..\release\tk8.6\ttk\cursors.tcl
file /oname=$INSTDIR\tk8.6\ttk\defaults.tcl ..\release\tk8.6\ttk\defaults.tcl
file /oname=$INSTDIR\tk8.6\ttk\entry.tcl ..\release\tk8.6\ttk\entry.tcl
file /oname=$INSTDIR\tk8.6\ttk\fonts.tcl ..\release\tk8.6\ttk\fonts.tcl
file /oname=$INSTDIR\tk8.6\ttk\menubutton.tcl ..\release\tk8.6\ttk\menubutton.tcl
file /oname=$INSTDIR\tk8.6\ttk\notebook.tcl ..\release\tk8.6\ttk\notebook.tcl
file /oname=$INSTDIR\tk8.6\ttk\panedwindow.tcl ..\release\tk8.6\ttk\panedwindow.tcl
file /oname=$INSTDIR\tk8.6\ttk\progress.tcl ..\release\tk8.6\ttk\progress.tcl
file /oname=$INSTDIR\tk8.6\ttk\scale.tcl ..\release\tk8.6\ttk\scale.tcl
file /oname=$INSTDIR\tk8.6\ttk\scrollbar.tcl ..\release\tk8.6\ttk\scrollbar.tcl
file /oname=$INSTDIR\tk8.6\ttk\sizegrip.tcl ..\release\tk8.6\ttk\sizegrip.tcl
file /oname=$INSTDIR\tk8.6\ttk\spinbox.tcl ..\release\tk8.6\ttk\spinbox.tcl
file /oname=$INSTDIR\tk8.6\ttk\treeview.tcl ..\release\tk8.6\ttk\treeview.tcl
file /oname=$INSTDIR\tk8.6\ttk\ttk.tcl ..\release\tk8.6\ttk\ttk.tcl
file /oname=$INSTDIR\tk8.6\ttk\utils.tcl ..\release\tk8.6\ttk\utils.tcl
file /oname=$INSTDIR\tk8.6\ttk\vistaTheme.tcl ..\release\tk8.6\ttk\vistaTheme.tcl
file /oname=$INSTDIR\tk8.6\ttk\winTheme.tcl ..\release\tk8.6\ttk\winTheme.tcl
file /oname=$INSTDIR\tk8.6\ttk\xpTheme.tcl ..\release\tk8.6\ttk\xpTheme.tcl
file /oname=$INSTDIR\tk8.6\unsupported.tcl ..\release\tk8.6\unsupported.tcl
file /oname=$INSTDIR\tk8.6\xmfbox.tcl ..\release\tk8.6\xmfbox.tcl
file /oname=$INSTDIR\zlib1.dll ..\release\zlib1.dll
file /oname=$INSTDIR\tcl86.dll ..\release\tcl86.dll
file /oname=$INSTDIR\tk86.dll ..\release\tk86.dll
file /oname=$INSTDIR\tcl8\8.4\platform\shell-1.1.4.tm ..\release\tcl8\8.4\platform\shell-1.1.4.tm 
file /oname=$INSTDIR\tcl8\8.4\platform-1.0.14.tm ..\release\tcl8\8.4\platform-1.0.14.tm 
file /oname=$INSTDIR\tcl8\8.5\msgcat-1.6.0.tm ..\release\tcl8\8.5\msgcat-1.6.0.tm
file /oname=$INSTDIR\tcl8\8.5\tcltest-2.4.0.tm ..\release\tcl8\8.5\tcltest-2.4.0.tm
file /oname=$INSTDIR\tcl8\8.6\http-2.8.9.tm ..\release\tcl8\8.6\http-2.8.9.tm
file /oname=$INSTDIR\tcl8\8.6\tdbc\sqlite3-1.0.4.tm ..\release\tcl8\8.6\tdbc\sqlite3-1.0.4.tm
# end of TCL-related files and directories
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Classified-ads" "DisplayName" "Classified-ads (remove only)"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Classified-ads" "UninstallString" "$INSTDIR\Uninstall.exe"
DetailPrint "Register Classified Ad profile URI Handler"
DeleteRegKey HKCR "caprofile"
WriteRegStr HKCR "caprofile" "" "URL:caprofile"
WriteRegStr HKCR "caprofile" "URL Protocol" ""
WriteRegStr HKCR "caprofile\DefaultIcon" "" "$INSTDIR\classified-ads.exe"
WriteRegStr HKCR "caprofile\shell" "" ""
WriteRegStr HKCR "caprofile\shell\Open" "" ""
WriteRegStr HKCR "caprofile\shell\Open\command" "" "$INSTDIR\classified-ads.exe %l"
DetailPrint "Register Classified Ad ad URI Handler"
DeleteRegKey HKCR "caad"
WriteRegStr HKCR "caad" "" "URL:caad"
WriteRegStr HKCR "caad" "URL Protocol" ""
WriteRegStr HKCR "caad\DefaultIcon" "" "$INSTDIR\classified-ads.exe"
WriteRegStr HKCR "caad\shell" "" ""
WriteRegStr HKCR "caad\shell\Open" "" ""
WriteRegStr HKCR "caad\shell\Open\command" "" "$INSTDIR\classified-ads.exe %l"
DetailPrint "Register Classified Ad comment URI Handler"
DeleteRegKey HKCR "cacomment"
WriteRegStr HKCR "cacomment" "" "URL:cacomment"
WriteRegStr HKCR "cacomment" "URL Protocol" ""
WriteRegStr HKCR "cacomment\DefaultIcon" "" "$INSTDIR\classified-ads.exe"
WriteRegStr HKCR "cacomment\shell" "" ""
WriteRegStr HKCR "cacomment\shell\Open" "" ""
WriteRegStr HKCR "cacomment\shell\Open\command" "" "$INSTDIR\classified-ads.exe %l"
DetailPrint "Register Classified Ad blob URI Handler"
DeleteRegKey HKCR "cablob"
WriteRegStr HKCR "cablob" "" "URL:cablob"
WriteRegStr HKCR "cablob" "URL Protocol" ""
WriteRegStr HKCR "cablob\DefaultIcon" "" "$INSTDIR\classified-ads.exe"
WriteRegStr HKCR "cablob\shell" "" ""
WriteRegStr HKCR "cablob\shell\Open" "" ""
WriteRegStr HKCR "cablob\shell\Open\command" "" "$INSTDIR\classified-ads.exe %l"
sectionEnd
#
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
delete "$INSTDIR\examples\sysinfo.tcl"
delete "$INSTDIR\examples\luikero.tcl"
delete "$INSTDIR\examples\calendar.tcl"
RMDIR "$INSTDIR\examples"
delete "$INSTDIR\D3Dcompiler_47.dll"
delete "$INSTDIR\libEGL.dll"
delete "$INSTDIR\libGLESV2.dll"
delete "$INSTDIR\libgcc_s_dw2-1.dll"
delete "$INSTDIR\libstdc++-6.dll"
delete "$INSTDIR\libwinpthread-1.dll"
delete "$INSTDIR\opengl32sw.dll"
delete "$INSTDIR\Qt5Multimedia.dll"
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
delete "$INSTDIR\audio\qtaudio_windows.dll"
delete "$INSTDIR\libeay32.dll"
delete "$INSTDIR\ssleay32.dll"
delete "$INSTDIR\miniupnpc.dll"
delete "$INSTDIR\libiconv-2.dll"
delete "$INSTDIR\libintl-8.dll"
delete "$INSTDIR\fi\LC_MESSAGES\classified-ads.mo"
delete "$INSTDIR\sv\LC_MESSAGES\classified-ads.mo"
delete "$INSTDIR\da\LC_MESSAGES\classified-ads.mo"
delete "$INSTDIR\uk\LC_MESSAGES\classified-ads.mo"
delete "$INSTDIR\de\LC_MESSAGES\classified-ads.mo"
delete "$INSTDIR\es\LC_MESSAGES\classified-ads.mo"
RMDIR "$INSTDIR\sv\LC_MESSAGES"
RMDIR "$INSTDIR\sv"
RMDIR "$INSTDIR\fi\LC_MESSAGES"
RMDIR "$INSTDIR\fi"
RMDIR "$INSTDIR\da\LC_MESSAGES"
RMDIR "$INSTDIR\da"
RMDIR "$INSTDIR\uk\LC_MESSAGES"
RMDIR "$INSTDIR\uk"
RMDIR "$INSTDIR\de\LC_MESSAGES"
RMDIR "$INSTDIR\de"
RMDIR "$INSTDIR\es\LC_MESSAGES"
RMDIR "$INSTDIR\es"
RMDIR "$INSTDIR\bearer"
RMDIR "$INSTDIR\iconengines"
RMDIR "$INSTDIR\imageformats"
RMDIR "$INSTDIR\printsupport"
RMDIR "$INSTDIR\platforms"
RMDIR "$INSTDIR\sqldrivers"
RMDIR "$INSTDIR\audio"
#
# Removal of TCL-related files and directories
#
# files first
delete "$INSTDIR\tcl8.6\auto.tcl"
delete "$INSTDIR\tcl8.6\clock.tcl"
delete "$INSTDIR\tcl8.6\encoding\ascii.enc"
delete "$INSTDIR\tcl8.6\encoding\big5.enc"
delete "$INSTDIR\tcl8.6\encoding\cp1250.enc"
delete "$INSTDIR\tcl8.6\encoding\cp1251.enc"
delete "$INSTDIR\tcl8.6\encoding\cp1252.enc"
delete "$INSTDIR\tcl8.6\encoding\cp1253.enc"
delete "$INSTDIR\tcl8.6\encoding\cp1254.enc"
delete "$INSTDIR\tcl8.6\encoding\cp1255.enc"
delete "$INSTDIR\tcl8.6\encoding\cp1256.enc"
delete "$INSTDIR\tcl8.6\encoding\cp1257.enc"
delete "$INSTDIR\tcl8.6\encoding\cp1258.enc"
delete "$INSTDIR\tcl8.6\encoding\cp437.enc"
delete "$INSTDIR\tcl8.6\encoding\cp737.enc"
delete "$INSTDIR\tcl8.6\encoding\cp775.enc"
delete "$INSTDIR\tcl8.6\encoding\cp850.enc"
delete "$INSTDIR\tcl8.6\encoding\cp852.enc"
delete "$INSTDIR\tcl8.6\encoding\cp855.enc"
delete "$INSTDIR\tcl8.6\encoding\cp857.enc"
delete "$INSTDIR\tcl8.6\encoding\cp860.enc"
delete "$INSTDIR\tcl8.6\encoding\cp861.enc"
delete "$INSTDIR\tcl8.6\encoding\cp862.enc"
delete "$INSTDIR\tcl8.6\encoding\cp863.enc"
delete "$INSTDIR\tcl8.6\encoding\cp864.enc"
delete "$INSTDIR\tcl8.6\encoding\cp865.enc"
delete "$INSTDIR\tcl8.6\encoding\cp866.enc"
delete "$INSTDIR\tcl8.6\encoding\cp869.enc"
delete "$INSTDIR\tcl8.6\encoding\cp874.enc"
delete "$INSTDIR\tcl8.6\encoding\cp932.enc"
delete "$INSTDIR\tcl8.6\encoding\cp936.enc"
delete "$INSTDIR\tcl8.6\encoding\cp949.enc"
delete "$INSTDIR\tcl8.6\encoding\cp950.enc"
delete "$INSTDIR\tcl8.6\encoding\dingbats.enc"
delete "$INSTDIR\tcl8.6\encoding\ebcdic.enc"
delete "$INSTDIR\tcl8.6\encoding\euc-cn.enc"
delete "$INSTDIR\tcl8.6\encoding\euc-jp.enc"
delete "$INSTDIR\tcl8.6\encoding\euc-kr.enc"
delete "$INSTDIR\tcl8.6\encoding\gb12345.enc"
delete "$INSTDIR\tcl8.6\encoding\gb1988.enc"
delete "$INSTDIR\tcl8.6\encoding\gb2312-raw.enc"
delete "$INSTDIR\tcl8.6\encoding\gb2312.enc"
delete "$INSTDIR\tcl8.6\encoding\iso2022-jp.enc"
delete "$INSTDIR\tcl8.6\encoding\iso2022-kr.enc"
delete "$INSTDIR\tcl8.6\encoding\iso2022.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-1.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-10.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-13.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-14.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-15.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-16.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-2.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-3.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-4.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-5.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-6.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-7.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-8.enc"
delete "$INSTDIR\tcl8.6\encoding\iso8859-9.enc"
delete "$INSTDIR\tcl8.6\encoding\jis0201.enc"
delete "$INSTDIR\tcl8.6\encoding\jis0208.enc"
delete "$INSTDIR\tcl8.6\encoding\jis0212.enc"
delete "$INSTDIR\tcl8.6\encoding\koi8-r.enc"
delete "$INSTDIR\tcl8.6\encoding\koi8-u.enc"
delete "$INSTDIR\tcl8.6\encoding\ksc5601.enc"
delete "$INSTDIR\tcl8.6\encoding\macCentEuro.enc"
delete "$INSTDIR\tcl8.6\encoding\macCroatian.enc"
delete "$INSTDIR\tcl8.6\encoding\macCyrillic.enc"
delete "$INSTDIR\tcl8.6\encoding\macDingbats.enc"
delete "$INSTDIR\tcl8.6\encoding\macGreek.enc"
delete "$INSTDIR\tcl8.6\encoding\macIceland.enc"
delete "$INSTDIR\tcl8.6\encoding\macJapan.enc"
delete "$INSTDIR\tcl8.6\encoding\macRoman.enc"
delete "$INSTDIR\tcl8.6\encoding\macRomania.enc"
delete "$INSTDIR\tcl8.6\encoding\macThai.enc"
delete "$INSTDIR\tcl8.6\encoding\macTurkish.enc"
delete "$INSTDIR\tcl8.6\encoding\macUkraine.enc"
delete "$INSTDIR\tcl8.6\encoding\shiftjis.enc"
delete "$INSTDIR\tcl8.6\encoding\symbol.enc"
delete "$INSTDIR\tcl8.6\encoding\tis-620.enc"
delete "$INSTDIR\tcl8.6\history.tcl"
delete "$INSTDIR\tcl8.6\http1.0\http.tcl"
delete "$INSTDIR\tcl8.6\http1.0\pkgIndex.tcl"
delete "$INSTDIR\tcl8.6\init.tcl"
delete "$INSTDIR\tcl8.6\msgs\af.msg"
delete "$INSTDIR\tcl8.6\msgs\af_za.msg"
delete "$INSTDIR\tcl8.6\msgs\ar.msg"
delete "$INSTDIR\tcl8.6\msgs\ar_in.msg"
delete "$INSTDIR\tcl8.6\msgs\ar_jo.msg"
delete "$INSTDIR\tcl8.6\msgs\ar_lb.msg"
delete "$INSTDIR\tcl8.6\msgs\ar_sy.msg"
delete "$INSTDIR\tcl8.6\msgs\be.msg"
delete "$INSTDIR\tcl8.6\msgs\bg.msg"
delete "$INSTDIR\tcl8.6\msgs\bn.msg"
delete "$INSTDIR\tcl8.6\msgs\bn_in.msg"
delete "$INSTDIR\tcl8.6\msgs\ca.msg"
delete "$INSTDIR\tcl8.6\msgs\cs.msg"
delete "$INSTDIR\tcl8.6\msgs\da.msg"
delete "$INSTDIR\tcl8.6\msgs\de.msg"
delete "$INSTDIR\tcl8.6\msgs\de_at.msg"
delete "$INSTDIR\tcl8.6\msgs\de_be.msg"
delete "$INSTDIR\tcl8.6\msgs\el.msg"
delete "$INSTDIR\tcl8.6\msgs\en_au.msg"
delete "$INSTDIR\tcl8.6\msgs\en_be.msg"
delete "$INSTDIR\tcl8.6\msgs\en_bw.msg"
delete "$INSTDIR\tcl8.6\msgs\en_ca.msg"
delete "$INSTDIR\tcl8.6\msgs\en_gb.msg"
delete "$INSTDIR\tcl8.6\msgs\en_hk.msg"
delete "$INSTDIR\tcl8.6\msgs\en_ie.msg"
delete "$INSTDIR\tcl8.6\msgs\en_in.msg"
delete "$INSTDIR\tcl8.6\msgs\en_nz.msg"
delete "$INSTDIR\tcl8.6\msgs\en_ph.msg"
delete "$INSTDIR\tcl8.6\msgs\en_sg.msg"
delete "$INSTDIR\tcl8.6\msgs\en_za.msg"
delete "$INSTDIR\tcl8.6\msgs\en_zw.msg"
delete "$INSTDIR\tcl8.6\msgs\eo.msg"
delete "$INSTDIR\tcl8.6\msgs\es.msg"
delete "$INSTDIR\tcl8.6\msgs\es_ar.msg"
delete "$INSTDIR\tcl8.6\msgs\es_bo.msg"
delete "$INSTDIR\tcl8.6\msgs\es_cl.msg"
delete "$INSTDIR\tcl8.6\msgs\es_co.msg"
delete "$INSTDIR\tcl8.6\msgs\es_cr.msg"
delete "$INSTDIR\tcl8.6\msgs\es_do.msg"
delete "$INSTDIR\tcl8.6\msgs\es_ec.msg"
delete "$INSTDIR\tcl8.6\msgs\es_gt.msg"
delete "$INSTDIR\tcl8.6\msgs\es_hn.msg"
delete "$INSTDIR\tcl8.6\msgs\es_mx.msg"
delete "$INSTDIR\tcl8.6\msgs\es_ni.msg"
delete "$INSTDIR\tcl8.6\msgs\es_pa.msg"
delete "$INSTDIR\tcl8.6\msgs\es_pe.msg"
delete "$INSTDIR\tcl8.6\msgs\es_pr.msg"
delete "$INSTDIR\tcl8.6\msgs\es_py.msg"
delete "$INSTDIR\tcl8.6\msgs\es_sv.msg"
delete "$INSTDIR\tcl8.6\msgs\es_uy.msg"
delete "$INSTDIR\tcl8.6\msgs\es_ve.msg"
delete "$INSTDIR\tcl8.6\msgs\et.msg"
delete "$INSTDIR\tcl8.6\msgs\eu.msg"
delete "$INSTDIR\tcl8.6\msgs\eu_es.msg"
delete "$INSTDIR\tcl8.6\msgs\fa.msg"
delete "$INSTDIR\tcl8.6\msgs\fa_in.msg"
delete "$INSTDIR\tcl8.6\msgs\fa_ir.msg"
delete "$INSTDIR\tcl8.6\msgs\fi.msg"
delete "$INSTDIR\tcl8.6\msgs\fo.msg"
delete "$INSTDIR\tcl8.6\msgs\fo_fo.msg"
delete "$INSTDIR\tcl8.6\msgs\fr.msg"
delete "$INSTDIR\tcl8.6\msgs\fr_be.msg"
delete "$INSTDIR\tcl8.6\msgs\fr_ca.msg"
delete "$INSTDIR\tcl8.6\msgs\fr_ch.msg"
delete "$INSTDIR\tcl8.6\msgs\ga.msg"
delete "$INSTDIR\tcl8.6\msgs\ga_ie.msg"
delete "$INSTDIR\tcl8.6\msgs\gl.msg"
delete "$INSTDIR\tcl8.6\msgs\gl_es.msg"
delete "$INSTDIR\tcl8.6\msgs\gv.msg"
delete "$INSTDIR\tcl8.6\msgs\gv_gb.msg"
delete "$INSTDIR\tcl8.6\msgs\he.msg"
delete "$INSTDIR\tcl8.6\msgs\hi.msg"
delete "$INSTDIR\tcl8.6\msgs\hi_in.msg"
delete "$INSTDIR\tcl8.6\msgs\hr.msg"
delete "$INSTDIR\tcl8.6\msgs\hu.msg"
delete "$INSTDIR\tcl8.6\msgs\id.msg"
delete "$INSTDIR\tcl8.6\msgs\id_id.msg"
delete "$INSTDIR\tcl8.6\msgs\is.msg"
delete "$INSTDIR\tcl8.6\msgs\it.msg"
delete "$INSTDIR\tcl8.6\msgs\it_ch.msg"
delete "$INSTDIR\tcl8.6\msgs\ja.msg"
delete "$INSTDIR\tcl8.6\msgs\kl.msg"
delete "$INSTDIR\tcl8.6\msgs\kl_gl.msg"
delete "$INSTDIR\tcl8.6\msgs\ko.msg"
delete "$INSTDIR\tcl8.6\msgs\kok.msg"
delete "$INSTDIR\tcl8.6\msgs\kok_in.msg"
delete "$INSTDIR\tcl8.6\msgs\ko_kr.msg"
delete "$INSTDIR\tcl8.6\msgs\kw.msg"
delete "$INSTDIR\tcl8.6\msgs\kw_gb.msg"
delete "$INSTDIR\tcl8.6\msgs\lt.msg"
delete "$INSTDIR\tcl8.6\msgs\lv.msg"
delete "$INSTDIR\tcl8.6\msgs\mk.msg"
delete "$INSTDIR\tcl8.6\msgs\mr.msg"
delete "$INSTDIR\tcl8.6\msgs\mr_in.msg"
delete "$INSTDIR\tcl8.6\msgs\ms.msg"
delete "$INSTDIR\tcl8.6\msgs\ms_my.msg"
delete "$INSTDIR\tcl8.6\msgs\mt.msg"
delete "$INSTDIR\tcl8.6\msgs\nb.msg"
delete "$INSTDIR\tcl8.6\msgs\nl.msg"
delete "$INSTDIR\tcl8.6\msgs\nl_be.msg"
delete "$INSTDIR\tcl8.6\msgs\nn.msg"
delete "$INSTDIR\tcl8.6\msgs\pl.msg"
delete "$INSTDIR\tcl8.6\msgs\pt.msg"
delete "$INSTDIR\tcl8.6\msgs\pt_br.msg"
delete "$INSTDIR\tcl8.6\msgs\ro.msg"
delete "$INSTDIR\tcl8.6\msgs\ru.msg"
delete "$INSTDIR\tcl8.6\msgs\ru_ua.msg"
delete "$INSTDIR\tcl8.6\msgs\sh.msg"
delete "$INSTDIR\tcl8.6\msgs\sk.msg"
delete "$INSTDIR\tcl8.6\msgs\sl.msg"
delete "$INSTDIR\tcl8.6\msgs\sq.msg"
delete "$INSTDIR\tcl8.6\msgs\sr.msg"
delete "$INSTDIR\tcl8.6\msgs\sv.msg"
delete "$INSTDIR\tcl8.6\msgs\sw.msg"
delete "$INSTDIR\tcl8.6\msgs\ta.msg"
delete "$INSTDIR\tcl8.6\msgs\ta_in.msg"
delete "$INSTDIR\tcl8.6\msgs\te.msg"
delete "$INSTDIR\tcl8.6\msgs\te_in.msg"
delete "$INSTDIR\tcl8.6\msgs\th.msg"
delete "$INSTDIR\tcl8.6\msgs\tr.msg"
delete "$INSTDIR\tcl8.6\msgs\uk.msg"
delete "$INSTDIR\tcl8.6\msgs\vi.msg"
delete "$INSTDIR\tcl8.6\msgs\zh.msg"
delete "$INSTDIR\tcl8.6\msgs\zh_cn.msg"
delete "$INSTDIR\tcl8.6\msgs\zh_hk.msg"
delete "$INSTDIR\tcl8.6\msgs\zh_sg.msg"
delete "$INSTDIR\tcl8.6\msgs\zh_tw.msg"
delete "$INSTDIR\tcl8.6\opt0.4\optparse.tcl"
delete "$INSTDIR\tcl8.6\opt0.4\pkgIndex.tcl"
delete "$INSTDIR\tcl8.6\package.tcl"
delete "$INSTDIR\tcl8.6\parray.tcl"
delete "$INSTDIR\tcl8.6\safe.tcl"
delete "$INSTDIR\tcl8.6\tclIndex"
delete "$INSTDIR\tcl8.6\tm.tcl"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Abidjan"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Accra"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Addis_Ababa"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Algiers"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Asmara"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Asmera"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Bamako"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Bangui"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Banjul"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Bissau"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Blantyre"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Brazzaville"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Bujumbura"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Cairo"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Casablanca"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Ceuta"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Conakry"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Dakar"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Dar_es_Salaam"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Djibouti"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Douala"
delete "$INSTDIR\tcl8.6\tzdata\Africa\El_Aaiun"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Freetown"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Gaborone"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Harare"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Johannesburg"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Juba"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Kampala"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Khartoum"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Kigali"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Kinshasa"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Lagos"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Libreville"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Lome"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Luanda"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Lubumbashi"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Lusaka"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Malabo"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Maputo"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Maseru"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Mbabane"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Mogadishu"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Monrovia"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Nairobi"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Ndjamena"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Niamey"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Nouakchott"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Ouagadougou"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Porto-Novo"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Sao_Tome"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Timbuktu"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Tripoli"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Tunis"
delete "$INSTDIR\tcl8.6\tzdata\Africa\Windhoek"
delete "$INSTDIR\tcl8.6\tzdata\America\Adak"
delete "$INSTDIR\tcl8.6\tzdata\America\Anchorage"
delete "$INSTDIR\tcl8.6\tzdata\America\Anguilla"
delete "$INSTDIR\tcl8.6\tzdata\America\Antigua"
delete "$INSTDIR\tcl8.6\tzdata\America\Araguaina"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\Buenos_Aires"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\Catamarca"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\ComodRivadavia"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\Cordoba"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\Jujuy"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\La_Rioja"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\Mendoza"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\Rio_Gallegos"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\Salta"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\San_Juan"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\San_Luis"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\Tucuman"
delete "$INSTDIR\tcl8.6\tzdata\America\Argentina\Ushuaia"
delete "$INSTDIR\tcl8.6\tzdata\America\Aruba"
delete "$INSTDIR\tcl8.6\tzdata\America\Asuncion"
delete "$INSTDIR\tcl8.6\tzdata\America\Atikokan"
delete "$INSTDIR\tcl8.6\tzdata\America\Atka"
delete "$INSTDIR\tcl8.6\tzdata\America\Bahia"
delete "$INSTDIR\tcl8.6\tzdata\America\Bahia_Banderas"
delete "$INSTDIR\tcl8.6\tzdata\America\Barbados"
delete "$INSTDIR\tcl8.6\tzdata\America\Belem"
delete "$INSTDIR\tcl8.6\tzdata\America\Belize"
delete "$INSTDIR\tcl8.6\tzdata\America\Blanc-Sablon"
delete "$INSTDIR\tcl8.6\tzdata\America\Boa_Vista"
delete "$INSTDIR\tcl8.6\tzdata\America\Bogota"
delete "$INSTDIR\tcl8.6\tzdata\America\Boise"
delete "$INSTDIR\tcl8.6\tzdata\America\Buenos_Aires"
delete "$INSTDIR\tcl8.6\tzdata\America\Cambridge_Bay"
delete "$INSTDIR\tcl8.6\tzdata\America\Campo_Grande"
delete "$INSTDIR\tcl8.6\tzdata\America\Cancun"
delete "$INSTDIR\tcl8.6\tzdata\America\Caracas"
delete "$INSTDIR\tcl8.6\tzdata\America\Catamarca"
delete "$INSTDIR\tcl8.6\tzdata\America\Cayenne"
delete "$INSTDIR\tcl8.6\tzdata\America\Cayman"
delete "$INSTDIR\tcl8.6\tzdata\America\Chicago"
delete "$INSTDIR\tcl8.6\tzdata\America\Chihuahua"
delete "$INSTDIR\tcl8.6\tzdata\America\Coral_Harbour"
delete "$INSTDIR\tcl8.6\tzdata\America\Cordoba"
delete "$INSTDIR\tcl8.6\tzdata\America\Costa_Rica"
delete "$INSTDIR\tcl8.6\tzdata\America\Creston"
delete "$INSTDIR\tcl8.6\tzdata\America\Cuiaba"
delete "$INSTDIR\tcl8.6\tzdata\America\Curacao"
delete "$INSTDIR\tcl8.6\tzdata\America\Danmarkshavn"
delete "$INSTDIR\tcl8.6\tzdata\America\Dawson"
delete "$INSTDIR\tcl8.6\tzdata\America\Dawson_Creek"
delete "$INSTDIR\tcl8.6\tzdata\America\Denver"
delete "$INSTDIR\tcl8.6\tzdata\America\Detroit"
delete "$INSTDIR\tcl8.6\tzdata\America\Dominica"
delete "$INSTDIR\tcl8.6\tzdata\America\Edmonton"
delete "$INSTDIR\tcl8.6\tzdata\America\Eirunepe"
delete "$INSTDIR\tcl8.6\tzdata\America\El_Salvador"
delete "$INSTDIR\tcl8.6\tzdata\America\Ensenada"
delete "$INSTDIR\tcl8.6\tzdata\America\Fortaleza"
delete "$INSTDIR\tcl8.6\tzdata\America\Fort_Nelson"
delete "$INSTDIR\tcl8.6\tzdata\America\Fort_Wayne"
delete "$INSTDIR\tcl8.6\tzdata\America\Glace_Bay"
delete "$INSTDIR\tcl8.6\tzdata\America\Godthab"
delete "$INSTDIR\tcl8.6\tzdata\America\Goose_Bay"
delete "$INSTDIR\tcl8.6\tzdata\America\Grand_Turk"
delete "$INSTDIR\tcl8.6\tzdata\America\Grenada"
delete "$INSTDIR\tcl8.6\tzdata\America\Guadeloupe"
delete "$INSTDIR\tcl8.6\tzdata\America\Guatemala"
delete "$INSTDIR\tcl8.6\tzdata\America\Guayaquil"
delete "$INSTDIR\tcl8.6\tzdata\America\Guyana"
delete "$INSTDIR\tcl8.6\tzdata\America\Halifax"
delete "$INSTDIR\tcl8.6\tzdata\America\Havana"
delete "$INSTDIR\tcl8.6\tzdata\America\Hermosillo"
delete "$INSTDIR\tcl8.6\tzdata\America\Indiana\Indianapolis"
delete "$INSTDIR\tcl8.6\tzdata\America\Indiana\Knox"
delete "$INSTDIR\tcl8.6\tzdata\America\Indiana\Marengo"
delete "$INSTDIR\tcl8.6\tzdata\America\Indiana\Petersburg"
delete "$INSTDIR\tcl8.6\tzdata\America\Indiana\Tell_City"
delete "$INSTDIR\tcl8.6\tzdata\America\Indiana\Vevay"
delete "$INSTDIR\tcl8.6\tzdata\America\Indiana\Vincennes"
delete "$INSTDIR\tcl8.6\tzdata\America\Indiana\Winamac"
delete "$INSTDIR\tcl8.6\tzdata\America\Indianapolis"
delete "$INSTDIR\tcl8.6\tzdata\America\Inuvik"
delete "$INSTDIR\tcl8.6\tzdata\America\Iqaluit"
delete "$INSTDIR\tcl8.6\tzdata\America\Jamaica"
delete "$INSTDIR\tcl8.6\tzdata\America\Jujuy"
delete "$INSTDIR\tcl8.6\tzdata\America\Juneau"
delete "$INSTDIR\tcl8.6\tzdata\America\Kentucky\Louisville"
delete "$INSTDIR\tcl8.6\tzdata\America\Kentucky\Monticello"
delete "$INSTDIR\tcl8.6\tzdata\America\Knox_IN"
delete "$INSTDIR\tcl8.6\tzdata\America\Kralendijk"
delete "$INSTDIR\tcl8.6\tzdata\America\La_Paz"
delete "$INSTDIR\tcl8.6\tzdata\America\Lima"
delete "$INSTDIR\tcl8.6\tzdata\America\Los_Angeles"
delete "$INSTDIR\tcl8.6\tzdata\America\Louisville"
delete "$INSTDIR\tcl8.6\tzdata\America\Lower_Princes"
delete "$INSTDIR\tcl8.6\tzdata\America\Maceio"
delete "$INSTDIR\tcl8.6\tzdata\America\Managua"
delete "$INSTDIR\tcl8.6\tzdata\America\Manaus"
delete "$INSTDIR\tcl8.6\tzdata\America\Marigot"
delete "$INSTDIR\tcl8.6\tzdata\America\Martinique"
delete "$INSTDIR\tcl8.6\tzdata\America\Matamoros"
delete "$INSTDIR\tcl8.6\tzdata\America\Mazatlan"
delete "$INSTDIR\tcl8.6\tzdata\America\Mendoza"
delete "$INSTDIR\tcl8.6\tzdata\America\Menominee"
delete "$INSTDIR\tcl8.6\tzdata\America\Merida"
delete "$INSTDIR\tcl8.6\tzdata\America\Metlakatla"
delete "$INSTDIR\tcl8.6\tzdata\America\Mexico_City"
delete "$INSTDIR\tcl8.6\tzdata\America\Miquelon"
delete "$INSTDIR\tcl8.6\tzdata\America\Moncton"
delete "$INSTDIR\tcl8.6\tzdata\America\Monterrey"
delete "$INSTDIR\tcl8.6\tzdata\America\Montevideo"
delete "$INSTDIR\tcl8.6\tzdata\America\Montreal"
delete "$INSTDIR\tcl8.6\tzdata\America\Montserrat"
delete "$INSTDIR\tcl8.6\tzdata\America\Nassau"
delete "$INSTDIR\tcl8.6\tzdata\America\New_York"
delete "$INSTDIR\tcl8.6\tzdata\America\Nipigon"
delete "$INSTDIR\tcl8.6\tzdata\America\Nome"
delete "$INSTDIR\tcl8.6\tzdata\America\Noronha"
delete "$INSTDIR\tcl8.6\tzdata\America\North_Dakota\Beulah"
delete "$INSTDIR\tcl8.6\tzdata\America\North_Dakota\Center"
delete "$INSTDIR\tcl8.6\tzdata\America\North_Dakota\New_Salem"
delete "$INSTDIR\tcl8.6\tzdata\America\Ojinaga"
delete "$INSTDIR\tcl8.6\tzdata\America\Panama"
delete "$INSTDIR\tcl8.6\tzdata\America\Pangnirtung"
delete "$INSTDIR\tcl8.6\tzdata\America\Paramaribo"
delete "$INSTDIR\tcl8.6\tzdata\America\Phoenix"
delete "$INSTDIR\tcl8.6\tzdata\America\Port-au-Prince"
delete "$INSTDIR\tcl8.6\tzdata\America\Porto_Acre"
delete "$INSTDIR\tcl8.6\tzdata\America\Porto_Velho"
delete "$INSTDIR\tcl8.6\tzdata\America\Port_of_Spain"
delete "$INSTDIR\tcl8.6\tzdata\America\Puerto_Rico"
delete "$INSTDIR\tcl8.6\tzdata\America\Rainy_River"
delete "$INSTDIR\tcl8.6\tzdata\America\Rankin_Inlet"
delete "$INSTDIR\tcl8.6\tzdata\America\Recife"
delete "$INSTDIR\tcl8.6\tzdata\America\Regina"
delete "$INSTDIR\tcl8.6\tzdata\America\Resolute"
delete "$INSTDIR\tcl8.6\tzdata\America\Rio_Branco"
delete "$INSTDIR\tcl8.6\tzdata\America\Rosario"
delete "$INSTDIR\tcl8.6\tzdata\America\Santarem"
delete "$INSTDIR\tcl8.6\tzdata\America\Santa_Isabel"
delete "$INSTDIR\tcl8.6\tzdata\America\Santiago"
delete "$INSTDIR\tcl8.6\tzdata\America\Santo_Domingo"
delete "$INSTDIR\tcl8.6\tzdata\America\Sao_Paulo"
delete "$INSTDIR\tcl8.6\tzdata\America\Scoresbysund"
delete "$INSTDIR\tcl8.6\tzdata\America\Shiprock"
delete "$INSTDIR\tcl8.6\tzdata\America\Sitka"
delete "$INSTDIR\tcl8.6\tzdata\America\St_Barthelemy"
delete "$INSTDIR\tcl8.6\tzdata\America\St_Johns"
delete "$INSTDIR\tcl8.6\tzdata\America\St_Kitts"
delete "$INSTDIR\tcl8.6\tzdata\America\St_Lucia"
delete "$INSTDIR\tcl8.6\tzdata\America\St_Thomas"
delete "$INSTDIR\tcl8.6\tzdata\America\St_Vincent"
delete "$INSTDIR\tcl8.6\tzdata\America\Swift_Current"
delete "$INSTDIR\tcl8.6\tzdata\America\Tegucigalpa"
delete "$INSTDIR\tcl8.6\tzdata\America\Thule"
delete "$INSTDIR\tcl8.6\tzdata\America\Thunder_Bay"
delete "$INSTDIR\tcl8.6\tzdata\America\Tijuana"
delete "$INSTDIR\tcl8.6\tzdata\America\Toronto"
delete "$INSTDIR\tcl8.6\tzdata\America\Tortola"
delete "$INSTDIR\tcl8.6\tzdata\America\Vancouver"
delete "$INSTDIR\tcl8.6\tzdata\America\Virgin"
delete "$INSTDIR\tcl8.6\tzdata\America\Whitehorse"
delete "$INSTDIR\tcl8.6\tzdata\America\Winnipeg"
delete "$INSTDIR\tcl8.6\tzdata\America\Yakutat"
delete "$INSTDIR\tcl8.6\tzdata\America\Yellowknife"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\Casey"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\Davis"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\DumontDUrville"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\Macquarie"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\Mawson"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\McMurdo"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\Palmer"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\Rothera"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\South_Pole"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\Syowa"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\Troll"
delete "$INSTDIR\tcl8.6\tzdata\Antarctica\Vostok"
delete "$INSTDIR\tcl8.6\tzdata\Arctic\Longyearbyen"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Aden"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Almaty"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Amman"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Anadyr"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Aqtau"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Aqtobe"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Ashgabat"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Ashkhabad"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Baghdad"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Bahrain"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Baku"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Bangkok"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Barnaul"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Beirut"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Bishkek"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Brunei"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Calcutta"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Chita"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Choibalsan"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Chongqing"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Chungking"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Colombo"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Dacca"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Damascus"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Dhaka"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Dili"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Dubai"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Dushanbe"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Gaza"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Harbin"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Hebron"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Hong_Kong"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Hovd"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Ho_Chi_Minh"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Irkutsk"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Istanbul"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Jakarta"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Jayapura"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Jerusalem"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Kabul"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Kamchatka"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Karachi"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Kashgar"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Kathmandu"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Katmandu"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Khandyga"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Kolkata"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Krasnoyarsk"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Kuala_Lumpur"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Kuching"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Kuwait"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Macao"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Macau"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Magadan"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Makassar"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Manila"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Muscat"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Nicosia"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Novokuznetsk"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Novosibirsk"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Omsk"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Oral"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Phnom_Penh"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Pontianak"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Pyongyang"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Qatar"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Qyzylorda"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Rangoon"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Riyadh"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Saigon"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Sakhalin"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Samarkand"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Seoul"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Shanghai"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Singapore"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Srednekolymsk"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Taipei"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Tashkent"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Tbilisi"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Tehran"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Tel_Aviv"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Thimbu"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Thimphu"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Tokyo"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Tomsk"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Ujung_Pandang"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Ulaanbaatar"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Ulan_Bator"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Urumqi"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Ust-Nera"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Vientiane"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Vladivostok"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Yakutsk"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Yekaterinburg"
delete "$INSTDIR\tcl8.6\tzdata\Asia\Yerevan"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Azores"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Bermuda"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Canary"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Cape_Verde"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Faeroe"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Faroe"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Jan_Mayen"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Madeira"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Reykjavik"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\South_Georgia"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\Stanley"
delete "$INSTDIR\tcl8.6\tzdata\Atlantic\St_Helena"
delete "$INSTDIR\tcl8.6\tzdata\Australia\ACT"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Adelaide"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Brisbane"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Broken_Hill"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Canberra"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Currie"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Darwin"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Eucla"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Hobart"
delete "$INSTDIR\tcl8.6\tzdata\Australia\LHI"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Lindeman"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Lord_Howe"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Melbourne"
delete "$INSTDIR\tcl8.6\tzdata\Australia\North"
delete "$INSTDIR\tcl8.6\tzdata\Australia\NSW"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Perth"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Queensland"
delete "$INSTDIR\tcl8.6\tzdata\Australia\South"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Sydney"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Tasmania"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Victoria"
delete "$INSTDIR\tcl8.6\tzdata\Australia\West"
delete "$INSTDIR\tcl8.6\tzdata\Australia\Yancowinna"
delete "$INSTDIR\tcl8.6\tzdata\Brazil\Acre"
delete "$INSTDIR\tcl8.6\tzdata\Brazil\DeNoronha"
delete "$INSTDIR\tcl8.6\tzdata\Brazil\East"
delete "$INSTDIR\tcl8.6\tzdata\Brazil\West"
delete "$INSTDIR\tcl8.6\tzdata\Canada\Atlantic"
delete "$INSTDIR\tcl8.6\tzdata\Canada\Central"
delete "$INSTDIR\tcl8.6\tzdata\Canada\East-Saskatchewan"
delete "$INSTDIR\tcl8.6\tzdata\Canada\Eastern"
delete "$INSTDIR\tcl8.6\tzdata\Canada\Mountain"
delete "$INSTDIR\tcl8.6\tzdata\Canada\Newfoundland"
delete "$INSTDIR\tcl8.6\tzdata\Canada\Pacific"
delete "$INSTDIR\tcl8.6\tzdata\Canada\Saskatchewan"
delete "$INSTDIR\tcl8.6\tzdata\Canada\Yukon"
delete "$INSTDIR\tcl8.6\tzdata\CET"
delete "$INSTDIR\tcl8.6\tzdata\Chile\Continental"
delete "$INSTDIR\tcl8.6\tzdata\Chile\EasterIsland"
delete "$INSTDIR\tcl8.6\tzdata\CST6CDT"
delete "$INSTDIR\tcl8.6\tzdata\Cuba"
delete "$INSTDIR\tcl8.6\tzdata\EET"
delete "$INSTDIR\tcl8.6\tzdata\Egypt"
delete "$INSTDIR\tcl8.6\tzdata\Eire"
delete "$INSTDIR\tcl8.6\tzdata\EST"
delete "$INSTDIR\tcl8.6\tzdata\EST5EDT"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+0"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+1"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+10"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+11"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+12"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+2"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+3"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+4"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+5"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+6"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+7"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+8"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT+9"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-0"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-1"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-10"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-11"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-12"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-13"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-14"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-2"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-3"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-4"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-5"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-6"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-7"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-8"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT-9"
delete "$INSTDIR\tcl8.6\tzdata\Etc\GMT0"
delete "$INSTDIR\tcl8.6\tzdata\Etc\Greenwich"
delete "$INSTDIR\tcl8.6\tzdata\Etc\UCT"
delete "$INSTDIR\tcl8.6\tzdata\Etc\Universal"
delete "$INSTDIR\tcl8.6\tzdata\Etc\UTC"
delete "$INSTDIR\tcl8.6\tzdata\Etc\Zulu"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Amsterdam"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Andorra"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Astrakhan"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Athens"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Belfast"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Belgrade"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Berlin"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Bratislava"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Brussels"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Bucharest"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Budapest"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Busingen"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Chisinau"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Copenhagen"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Dublin"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Gibraltar"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Guernsey"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Helsinki"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Isle_of_Man"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Istanbul"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Jersey"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Kaliningrad"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Kiev"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Kirov"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Lisbon"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Ljubljana"
delete "$INSTDIR\tcl8.6\tzdata\Europe\London"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Luxembourg"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Madrid"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Malta"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Mariehamn"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Minsk"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Monaco"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Moscow"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Nicosia"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Oslo"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Paris"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Podgorica"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Prague"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Riga"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Rome"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Samara"
delete "$INSTDIR\tcl8.6\tzdata\Europe\San_Marino"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Sarajevo"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Simferopol"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Skopje"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Sofia"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Stockholm"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Tallinn"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Tirane"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Tiraspol"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Ulyanovsk"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Uzhgorod"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Vaduz"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Vatican"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Vienna"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Vilnius"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Volgograd"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Warsaw"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Zagreb"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Zaporozhye"
delete "$INSTDIR\tcl8.6\tzdata\Europe\Zurich"
delete "$INSTDIR\tcl8.6\tzdata\GB"
delete "$INSTDIR\tcl8.6\tzdata\GB-Eire"
delete "$INSTDIR\tcl8.6\tzdata\GMT"
delete "$INSTDIR\tcl8.6\tzdata\GMT+0"
delete "$INSTDIR\tcl8.6\tzdata\GMT-0"
delete "$INSTDIR\tcl8.6\tzdata\GMT0"
delete "$INSTDIR\tcl8.6\tzdata\Greenwich"
delete "$INSTDIR\tcl8.6\tzdata\Hongkong"
delete "$INSTDIR\tcl8.6\tzdata\HST"
delete "$INSTDIR\tcl8.6\tzdata\Iceland"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Antananarivo"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Chagos"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Christmas"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Cocos"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Comoro"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Kerguelen"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Mahe"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Maldives"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Mauritius"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Mayotte"
delete "$INSTDIR\tcl8.6\tzdata\Indian\Reunion"
delete "$INSTDIR\tcl8.6\tzdata\Iran"
delete "$INSTDIR\tcl8.6\tzdata\Israel"
delete "$INSTDIR\tcl8.6\tzdata\Jamaica"
delete "$INSTDIR\tcl8.6\tzdata\Japan"
delete "$INSTDIR\tcl8.6\tzdata\Kwajalein"
delete "$INSTDIR\tcl8.6\tzdata\Libya"
delete "$INSTDIR\tcl8.6\tzdata\MET"
delete "$INSTDIR\tcl8.6\tzdata\Mexico\BajaNorte"
delete "$INSTDIR\tcl8.6\tzdata\Mexico\BajaSur"
delete "$INSTDIR\tcl8.6\tzdata\Mexico\General"
delete "$INSTDIR\tcl8.6\tzdata\MST"
delete "$INSTDIR\tcl8.6\tzdata\MST7MDT"
delete "$INSTDIR\tcl8.6\tzdata\Navajo"
delete "$INSTDIR\tcl8.6\tzdata\NZ"
delete "$INSTDIR\tcl8.6\tzdata\NZ-CHAT"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Apia"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Auckland"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Bougainville"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Chatham"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Chuuk"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Easter"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Efate"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Enderbury"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Fakaofo"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Fiji"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Funafuti"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Galapagos"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Gambier"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Guadalcanal"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Guam"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Honolulu"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Johnston"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Kiritimati"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Kosrae"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Kwajalein"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Majuro"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Marquesas"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Midway"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Nauru"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Niue"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Norfolk"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Noumea"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Pago_Pago"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Palau"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Pitcairn"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Pohnpei"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Ponape"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Port_Moresby"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Rarotonga"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Saipan"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Samoa"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Tahiti"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Tarawa"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Tongatapu"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Truk"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Wake"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Wallis"
delete "$INSTDIR\tcl8.6\tzdata\Pacific\Yap"
delete "$INSTDIR\tcl8.6\tzdata\Poland"
delete "$INSTDIR\tcl8.6\tzdata\Portugal"
delete "$INSTDIR\tcl8.6\tzdata\PRC"
delete "$INSTDIR\tcl8.6\tzdata\PST8PDT"
delete "$INSTDIR\tcl8.6\tzdata\ROC"
delete "$INSTDIR\tcl8.6\tzdata\ROK"
delete "$INSTDIR\tcl8.6\tzdata\Singapore"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\AST4"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\AST4ADT"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\CST6"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\CST6CDT"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\EST5"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\EST5EDT"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\HST10"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\MST7"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\MST7MDT"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\PST8"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\PST8PDT"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\YST9"
delete "$INSTDIR\tcl8.6\tzdata\SystemV\YST9YDT"
delete "$INSTDIR\tcl8.6\tzdata\Turkey"
delete "$INSTDIR\tcl8.6\tzdata\UCT"
delete "$INSTDIR\tcl8.6\tzdata\Universal"
delete "$INSTDIR\tcl8.6\tzdata\US\Alaska"
delete "$INSTDIR\tcl8.6\tzdata\US\Aleutian"
delete "$INSTDIR\tcl8.6\tzdata\US\Arizona"
delete "$INSTDIR\tcl8.6\tzdata\US\Central"
delete "$INSTDIR\tcl8.6\tzdata\US\East-Indiana"
delete "$INSTDIR\tcl8.6\tzdata\US\Eastern"
delete "$INSTDIR\tcl8.6\tzdata\US\Hawaii"
delete "$INSTDIR\tcl8.6\tzdata\US\Indiana-Starke"
delete "$INSTDIR\tcl8.6\tzdata\US\Michigan"
delete "$INSTDIR\tcl8.6\tzdata\US\Mountain"
delete "$INSTDIR\tcl8.6\tzdata\US\Pacific"
delete "$INSTDIR\tcl8.6\tzdata\US\Pacific-New"
delete "$INSTDIR\tcl8.6\tzdata\US\Samoa"
delete "$INSTDIR\tcl8.6\tzdata\UTC"
delete "$INSTDIR\tcl8.6\tzdata\W-SU"
delete "$INSTDIR\tcl8.6\tzdata\WET"
delete "$INSTDIR\tcl8.6\tzdata\Zulu"
delete "$INSTDIR\tcl8.6\word.tcl"
delete "$INSTDIR\tk8.6\bgerror.tcl"
delete "$INSTDIR\tk8.6\button.tcl"
delete "$INSTDIR\tk8.6\choosedir.tcl"
delete "$INSTDIR\tk8.6\clrpick.tcl"
delete "$INSTDIR\tk8.6\comdlg.tcl"
delete "$INSTDIR\tk8.6\console.tcl"
delete "$INSTDIR\tk8.6\dialog.tcl"
delete "$INSTDIR\tk8.6\entry.tcl"
delete "$INSTDIR\tk8.6\focus.tcl"
delete "$INSTDIR\tk8.6\fontchooser.tcl"
delete "$INSTDIR\tk8.6\iconlist.tcl"
delete "$INSTDIR\tk8.6\icons.tcl"
delete "$INSTDIR\tk8.6\images\logo.eps"
delete "$INSTDIR\tk8.6\images\logo100.gif"
delete "$INSTDIR\tk8.6\images\logo64.gif"
delete "$INSTDIR\tk8.6\images\logoLarge.gif"
delete "$INSTDIR\tk8.6\images\logoMed.gif"
delete "$INSTDIR\tk8.6\images\pwrdLogo.eps"
delete "$INSTDIR\tk8.6\images\pwrdLogo100.gif"
delete "$INSTDIR\tk8.6\images\pwrdLogo150.gif"
delete "$INSTDIR\tk8.6\images\pwrdLogo175.gif"
delete "$INSTDIR\tk8.6\images\pwrdLogo200.gif"
delete "$INSTDIR\tk8.6\images\pwrdLogo75.gif"
delete "$INSTDIR\tk8.6\images\README"
delete "$INSTDIR\tk8.6\images\tai-ku.gif"
delete "$INSTDIR\tk8.6\listbox.tcl"
delete "$INSTDIR\tk8.6\megawidget.tcl"
delete "$INSTDIR\tk8.6\menu.tcl"
delete "$INSTDIR\tk8.6\mkpsenc.tcl"
delete "$INSTDIR\tk8.6\msgbox.tcl"
delete "$INSTDIR\tk8.6\msgs\cs.msg"
delete "$INSTDIR\tk8.6\msgs\da.msg"
delete "$INSTDIR\tk8.6\msgs\de.msg"
delete "$INSTDIR\tk8.6\msgs\el.msg"
delete "$INSTDIR\tk8.6\msgs\en.msg"
delete "$INSTDIR\tk8.6\msgs\en_gb.msg"
delete "$INSTDIR\tk8.6\msgs\eo.msg"
delete "$INSTDIR\tk8.6\msgs\es.msg"
delete "$INSTDIR\tk8.6\msgs\fr.msg"
delete "$INSTDIR\tk8.6\msgs\hu.msg"
delete "$INSTDIR\tk8.6\msgs\it.msg"
delete "$INSTDIR\tk8.6\msgs\nl.msg"
delete "$INSTDIR\tk8.6\msgs\pl.msg"
delete "$INSTDIR\tk8.6\msgs\pt.msg"
delete "$INSTDIR\tk8.6\msgs\ru.msg"
delete "$INSTDIR\tk8.6\msgs\sv.msg"
delete "$INSTDIR\tk8.6\obsolete.tcl"
delete "$INSTDIR\tk8.6\optMenu.tcl"
delete "$INSTDIR\tk8.6\palette.tcl"
delete "$INSTDIR\tk8.6\panedwindow.tcl"
delete "$INSTDIR\tk8.6\pkgIndex.tcl"
delete "$INSTDIR\tk8.6\safetk.tcl"
delete "$INSTDIR\tk8.6\scale.tcl"
delete "$INSTDIR\tk8.6\scrlbar.tcl"
delete "$INSTDIR\tk8.6\spinbox.tcl"
delete "$INSTDIR\tk8.6\tclIndex"
delete "$INSTDIR\tk8.6\tearoff.tcl"
delete "$INSTDIR\tk8.6\text.tcl"
delete "$INSTDIR\tk8.6\tk.tcl"
delete "$INSTDIR\tk8.6\tkAppInit.c"
delete "$INSTDIR\tk8.6\tkfbox.tcl"
delete "$INSTDIR\tk8.6\ttk\altTheme.tcl"
delete "$INSTDIR\tk8.6\ttk\aquaTheme.tcl"
delete "$INSTDIR\tk8.6\ttk\button.tcl"
delete "$INSTDIR\tk8.6\ttk\clamTheme.tcl"
delete "$INSTDIR\tk8.6\ttk\classicTheme.tcl"
delete "$INSTDIR\tk8.6\ttk\combobox.tcl"
delete "$INSTDIR\tk8.6\ttk\cursors.tcl"
delete "$INSTDIR\tk8.6\ttk\defaults.tcl"
delete "$INSTDIR\tk8.6\ttk\entry.tcl"
delete "$INSTDIR\tk8.6\ttk\fonts.tcl"
delete "$INSTDIR\tk8.6\ttk\menubutton.tcl"
delete "$INSTDIR\tk8.6\ttk\notebook.tcl"
delete "$INSTDIR\tk8.6\ttk\panedwindow.tcl"
delete "$INSTDIR\tk8.6\ttk\progress.tcl"
delete "$INSTDIR\tk8.6\ttk\scale.tcl"
delete "$INSTDIR\tk8.6\ttk\scrollbar.tcl"
delete "$INSTDIR\tk8.6\ttk\sizegrip.tcl"
delete "$INSTDIR\tk8.6\ttk\spinbox.tcl"
delete "$INSTDIR\tk8.6\ttk\treeview.tcl"
delete "$INSTDIR\tk8.6\ttk\ttk.tcl"
delete "$INSTDIR\tk8.6\ttk\utils.tcl"
delete "$INSTDIR\tk8.6\ttk\vistaTheme.tcl"
delete "$INSTDIR\tk8.6\ttk\winTheme.tcl"
delete "$INSTDIR\tk8.6\ttk\xpTheme.tcl"
delete "$INSTDIR\tk8.6\unsupported.tcl"
delete "$INSTDIR\tk8.6\xmfbox.tcl"
delete "$INSTDIR\zlib1.dll"
delete "$INSTDIR\tcl86.dll"
delete "$INSTDIR\tk86.dll"
delete "$INSTDIR\tcl8\8.4\platform\shell-1.1.4.tm"
delete "$INSTDIR\tcl8\8.4\platform-1.0.14.tm"
delete "$INSTDIR\tcl8\8.5\msgcat-1.6.0.tm"
delete "$INSTDIR\tcl8\8.5\tcltest-2.4.0.tm"
delete "$INSTDIR\tcl8\8.6\http-2.8.9.tm"
delete "$INSTDIR\tcl8\8.6\tdbc\sqlite3-1.0.4.tm"
# then directories
RMDIR "$INSTDIR\tk8.6\images"
RMDIR "$INSTDIR\tk8.6\msgs"
RMDIR "$INSTDIR\tk8.6\ttk"
RMDIR "$INSTDIR\tk8.6"
RMDIR "$INSTDIR\tcl8.6\tzdata\Africa"
RMDIR "$INSTDIR\tcl8.6\tzdata\America\Argentina"
RMDIR "$INSTDIR\tcl8.6\tzdata\America\Indiana"
RMDIR "$INSTDIR\tcl8.6\tzdata\America\Kentucky"
RMDIR "$INSTDIR\tcl8.6\tzdata\America\North_Dakota"
RMDIR "$INSTDIR\tcl8.6\tzdata\America"
RMDIR "$INSTDIR\tcl8.6\tzdata\Antarctica"
RMDIR "$INSTDIR\tcl8.6\tzdata\Arctic"
RMDIR "$INSTDIR\tcl8.6\tzdata\Asia"
RMDIR "$INSTDIR\tcl8.6\tzdata\Atlantic"
RMDIR "$INSTDIR\tcl8.6\tzdata\Australia"
RMDIR "$INSTDIR\tcl8.6\tzdata\Brazil"
RMDIR "$INSTDIR\tcl8.6\tzdata\Canada"
RMDIR "$INSTDIR\tcl8.6\tzdata\Chile"
RMDIR "$INSTDIR\tcl8.6\tzdata\Etc"
RMDIR "$INSTDIR\tcl8.6\tzdata\Europe"
RMDIR "$INSTDIR\tcl8.6\tzdata\Indian"
RMDIR "$INSTDIR\tcl8.6\tzdata\Mexico"
RMDIR "$INSTDIR\tcl8.6\tzdata\Pacific"
RMDIR "$INSTDIR\tcl8.6\tzdata\SystemV"
RMDIR "$INSTDIR\tcl8.6\tzdata\US"
RMDIR "$INSTDIR\tcl8.6\tzdata"
RMDIR "$INSTDIR\tcl8.6\encoding"
RMDIR "$INSTDIR\tcl8.6\http1.0"
RMDIR "$INSTDIR\tcl8.6\msgs"
RMDIR "$INSTDIR\tcl8.6\opt0.4"
RMDIR "$INSTDIR\tcl8.6"
RMDIR "$INSTDIR\tcl8\8.6\tdbc"
RMDIR "$INSTDIR\tcl8\8.6"
RMDIR "$INSTDIR\tcl8\8.4\platform"
RMDIR "$INSTDIR\tcl8\8.4"
RMDIR "$INSTDIR\tcl8\8.5"
RMDIR "$INSTDIR\tcl8"
# end of removal of TCL-related files
RMDIR "$INSTDIR"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classified-ads"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Classified-ads"
DetailPrint "De-Register Classified Ad URI Handlers"
DeleteRegKey HKCR "caprofile"
DeleteRegKey HKCR "caad"
DeleteRegKey HKCR "cacomment"
DeleteRegKey HKCR "cablob"
# uninstaller section end
sectionEnd 
 
