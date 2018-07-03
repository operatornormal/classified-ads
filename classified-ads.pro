#
# Classified Ads is Copyright (c) Antti Järvinen 2013-2018.
#
# This file is part of Classified Ads.
#
# Classified Ads is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# Classified Ads is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with Classified Ads; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
QT     += core network sql multimedia
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
} 
#CONFIG += debug
#CONFIG -= release
CONFIG(release) {
message("hardening flags for release build")
unix:QMAKE_CPPFLAGS *= $$(CA_QMAKE_CPPFLAGS)
unix:QMAKE_CFLAGS   *= $$(CA_QMAKE_CFLAGS)
unix:QMAKE_CXXFLAGS *= $$(CA_QMAKE_CXXFLAGS)
unix:QMAKE_LFLAGS   *= $$(CA_QMAKE_LDFLAGS)
} else {
message("debug build, no hardening")
# enabled -DDEBUG to have console log via qDebug()
QMAKE_CXXFLAGS += -DDEBUG
win32.CONFIG += console
}
QMAKE_CXXFLAGS += $$(RPM_OPT_FLAGS)
QMAKE_LFLAGS   += $$(RPM_LD_FLAGS)
CODECFORTR = UTF-8
CODECFORSRC = UTF-8
QMAKE_EXTRA_TARGETS += translations_compile
PRE_TARGETDEPS += translations_compile
unix {
    translations_compile.commands = cd po ; $(MAKE)
    QMAKE_EXTRA_TARGETS += test
    test.target = check
    test.depends = testca/testca.cpp
    test.commands = cd testca ; $(QMAKE) ; $(MAKE) ; mkdir test_home ; export HOME=`pwd`/test_home ; ./testca ; rm -rf test_home ; cd .. ; touch check
    QMAKE_CLEAN += po/*.mo check testca/testca testca/Makefile
}
win32 {
    translations_compile.commands = $(MAKE) -C po MSGFMT_PATH=/msys32/usr/local/bin/
    QMAKE_CLEAN += po\*.mo
}
HEADERS = mcontroller.h controller.h FrontWidget.h net/node.h util/hash.h \
	net/connection.h datamodel/model.h \
	net/networklistener.h net/protocol_message_formatter.h \
	net/protocol_message_parser.h net/networkconnectorengine.h \
	datamodel/nodemodel.h datamodel/netrequestexecutor.h \
	datamodel/contentencryptionmodel.h ui/passwd_dialog.h \
	datamodel/profilemodel.h datamodel/profile.h \
	net/publishingengine.h datamodel/mmodelprotocolinterface.h \
        datamodel/mnodemodelprotocolinterface.h ui/profilereadersdialog.h \
	datamodel/profilesearchmodel.h datamodel/profilereaderslistingmodel.h \
	datamodel/binaryfilemodel.h datamodel/binaryfile.h \
	datamodel/binaryfilelistingmodel.h datamodel/camodel.h \
	ui/newclassifiedaddialog.h datamodel/ca.h \
	datamodel/calistingmodel.h net/retrievalengine.h datamodel/privmsg.h \
	datamodel/privmsgmodel.h ui/newprivmsgdialog.h \
	datamodel/privmsgsearchmodel.h 	datamodel/contact.h \
	datamodel/contactlistingmodel.h ui/editcontact.h \
	datamodel/datamodelbase.h datamodel/profilecomment.h \
	datamodel/profilecommentmodel.h ui/newprofilecommentdialog.h \
	datamodel/profilecommentlistingmodel.h ui/profilecommentdisplay.h \
	ui/profilecommentitemdelegate.h ui/dialogbase.h \
        ui/attachmentlistdialog.h ui/settings.h ui/status.h \
	datamodel/connectionlistingmodel.h ui/manualconnection.h \
	ui/aboutdialog.h textedit/textedit.h datamodel/searchmodel.h \
	ui/searchdisplay.h ui/insertlinkdialog.h ui/newtextdocument.h \
	datamodel/trusttreemodel.h ui/metadataQuery.h util/jsonwrapper.h \
        util/catranslator.h datamodel/voicecall.h net/voicecallengine.h \
        ui/callstatus.h ui/callbuttondelegate.h net/mvoicecallengine.h \
        call/audiosource.h call/audiomixer.h call/audioplayer.h \
        call/audioencoder.h call/audiodecoder.h call/ringtoneplayer.h \
        ui/tclPrograms.h ui/tclConsole.h tcl/tclWrapper.h \
        datamodel/tclprogram.h datamodel/tclmodel.h tcl/tclCallbacks.h \
        tcl/tclUtil.h datamodel/cadbrecord.h datamodel/cadbrecordmodel.h \
        net/dbretrievalengine.h util/ungzip.h
SOURCES = main.cpp controller.cpp FrontWidget.cpp net/node.cpp util/hash.cpp \
	net/connection.cpp datamodel/model.cpp \
        net/networklistener.cpp net/protocol_message_formatter.cpp \
	net/protocol_message_parser.cpp net/networkconnectorengine.cpp \
	datamodel/nodemodel.cpp datamodel/netrequestexecutor.cpp \
	datamodel/contentencryptionmodel.cpp ui/passwd_dialog.cpp \
	datamodel/profilemodel.cpp datamodel/profile.cpp \
	net/publishingengine.cpp ui/profilereadersdialog.cpp \
	datamodel/profilesearchmodel.cpp \
	datamodel/profilereaderslistingmodel.cpp \
	datamodel/binaryfilemodel.cpp datamodel/binaryfile.cpp \
	datamodel/binaryfilelistingmodel.cpp datamodel/camodel.cpp \
	ui/newclassifiedaddialog.cpp datamodel/ca.cpp \
	datamodel/calistingmodel.cpp net/retrievalengine.cpp \
	datamodel/privmsg.cpp datamodel/privmsgmodel.cpp \
        ui/newprivmsgdialog.cpp datamodel/privmsgsearchmodel.cpp \
	datamodel/contact.cpp datamodel/contactlistingmodel.cpp \
	ui/editcontact.cpp 	datamodel/datamodelbase.cpp \
	datamodel/profilecomment.cpp datamodel/profilecommentmodel.cpp \
        ui/newprofilecommentdialog.cpp 	\
	datamodel/profilecommentlistingmodel.cpp ui/profilecommentdisplay.cpp \
	ui/profilecommentitemdelegate.cpp ui/dialogbase.cpp \
        ui/attachmentlistdialog.cpp ui/settings.cpp ui/status.cpp \
	datamodel/connectionlistingmodel.cpp ui/manualconnection.cpp \
        ui/aboutdialog.cpp textedit/textedit.cpp datamodel/searchmodel.cpp \
	ui/searchdisplay.cpp ui/insertlinkdialog.cpp ui/newtextdocument.cpp \
	datamodel/trusttreemodel.cpp ui/metadataQuery.cpp \
        util/jsonwrapper.cpp util/catranslator.cpp datamodel/voicecall.cpp \
        net/voicecallengine.cpp ui/callstatus.cpp ui/callbuttondelegate.cpp \
        call/audiosource.cpp call/audiomixer.cpp \
        call/audioplayer.cpp call/audioencoder.cpp call/audiodecoder.cpp \
        call/ringtoneplayer.cpp ui/tclPrograms.cpp ui/tclConsole.cpp \
        tcl/tclWrapper.cpp datamodel/tclprogram.cpp datamodel/tclmodel.cpp \
        tcl/tclCallbacks.cpp tcl/tclUtil.cpp  datamodel/cadbrecord.cpp \
        datamodel/cadbrecordmodel.cpp net/dbretrievalengine.cpp \
        util/ungzip.cpp
FORMS = frontWidget.ui ui/profileReadersDialog.ui ui/passwordDialog.ui \
	ui/newClassifiedAd.ui 	ui/newPrivMsg.ui ui/editContact.ui \
        ui/newProfileComment.ui ui/profileCommentDisplay.ui \
        ui/attachmentListDialog.ui ui/settingsDialog.ui \
	ui/statusDialog.ui ui/manualConnectionDialog.ui \
	ui/aboutDialog.ui ui/searchDisplay.ui ui/insertLink.ui \
        ui/newTextDocument.ui ui/metadataQuery.ui ui/callStatusDialog.ui \
        ui/tclPrograms.ui ui/tclConsole.ui
RESOURCES     = ui_resources.qrc
TRANSLATIONS  = classified_ads_fi.ts \
                classified_ads_sv.ts
unix:LIBS = -lssl -lcrypto -lnatpmp -lminiupnpc -ltcl -ltk -lz -lbz2
win32:LIBS+=-ltcl86 -ltk86 -lz
lessThan(QT_MAJOR_VERSION, 5) {
     unix:LIBS +=  -lqjson -lmagic
} 
LIBS += -lopus
# following line is needed for fedora linux, natpnp needs miniupnpc
unix:INCLUDEPATH += /usr/include/miniupnpc
win32:LIBS += "-L..\openssl-1.0.2o"
win32:LIBS += "-lcrypto"
win32:LIBS += "-lssl"
win32:LIBS += "..\miniupnpc-2.1\miniupnpc.lib" 
win32:LIBS += "-Lc:\msys32\usr\local\lib"
win32:LIBS += "-lintl"
win32:LIBS += "-L..\opus-1.2\binary\lib"
win32:LIBS += "-Lc:\msys32\opt\tcl\lib"
lessThan(QT_MAJOR_VERSION, 5) {
    win32:LIBS += "-L" 
    win32:LIBS += "..\qjson-master\build\src"
    win32:LIBS += "-lqjson"
}
win32:LIBS += "-lWs2_32" "-lGdi32" "-lIphlpapi"
win32:INCLUDEPATH += "..\openssl-1.0.2o\include"
win32:INCLUDEPATH += "..\miniupnpc-2.1"
win32:INCLUDEPATH += "c:\msys32\usr\local\include"
win32:INCLUDEPATH += "..\opus-1.2\binary\include"
win32:INCLUDEPATH += "c:\msys32\opt\tcl\include"
lessThan(QT_MAJOR_VERSION, 5) {
    win32:INCLUDEPATH += "..\qjson-master\include"
}
unix {
        TCL_VERSION = $$system(echo \'puts $tcl_version;exit 0\' | tclsh)
        message(Tcl version $$TCL_VERSION)
} 
win32 {
        TCL_VERSION = 8.6
}
INCLUDEPATH += /usr/include/tcl$$TCL_VERSION /usr/include/tk /usr/include/tk$$TCL_VERSION
target.path = /usr/bin
desktopfiles.path = /usr/share/applications
desktopfiles.files = ui/classified-ads.desktop
appdata.files = ui/classified-ads.appdata.xml
appdata.path = /usr/share/metainfo/
unix {
    # in unix install translations as part of appdata:
    appdata.extra = cd po ; $(MAKE) install DESTDIR=$(DESTDIR)
}
desktopicons.files = ui/turt-transparent-128x128.png
desktopicons.path = /usr/share/app-install/icons/
manpages.path = /usr/share/man/man1
manpages.files = classified-ads.1
# note this example file path appears also in file tclmodel.cpp
examplefiles.path = /usr/share/doc/classified-ads/examples
examplefiles.files = doc/sysinfo.tcl doc/luikero.tcl doc/calendar.tcl
INSTALLS += target \
        desktopfiles \
        desktopicons \
        appdata
unix:INSTALLS += manpages
unix:INSTALLS += appdata
unix:INSTALLS += examplefiles
RC_FILE=classified-ads.rc
