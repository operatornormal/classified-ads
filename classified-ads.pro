QT     += core network sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
} 
CONFIG += release
win32.CONFIG += console
CODECFORTR = UTF-8
CODECFORSRC = UTF-8
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
	ui/searchdisplay.h	
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
	ui/searchdisplay.cpp	
FORMS = frontWidget.ui ui/profileReadersDialog.ui ui/passwordDialog.ui \
	ui/newClassifiedAd.ui 	ui/newPrivMsg.ui ui/editContact.ui \
        ui/newProfileComment.ui ui/profileCommentDisplay.ui \
        ui/attachmentListDialog.ui ui/settingsDialog.ui \
	ui/statusDialog.ui ui/manualConnectionDialog.ui \
	ui/aboutDialog.ui ui/searchDisplay.ui
RESOURCES     = ui_resources.qrc

unix:LIBS = -lssl -lcrypto -lnatpmp -lqjson
win32:LIBS += "c:\msys\1.0\local\lib\libssl.a" 
win32:LIBS += "c:\msys\1.0\local\lib\libcrypto.a" 
win32:LIBS += "..\libnatpmp-20140401\libnatpmp.a" 
win32:LIBS += "-L" 
win32:LIBS += "..\qjson-master\build\src"
win32:LIBS += "-lqjson"
win32:LIBS += "-lWs2_32" "-lGdi32" "-lIphlpapi"
win32:INCLUDEPATH += "C:\msys\1.0\local\include"
win32:INCLUDEPATH += "..\libnatpmp-20140401"
win32:INCLUDEPATH += "..\qjson-master\include"

target.path = /usr/bin
desktopfiles.path = /usr/share/applications
desktopfiles.files = ui/classified_ads.desktop
desktopicons.path = /usr/share/app-install/icons/
desktopicons.files = ui/turt-transparent-128x128.png
INSTALLS += target desktopfiles desktopicons
