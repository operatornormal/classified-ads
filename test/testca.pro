#
# Classified Ads is Copyright (c) Antti Järvinen 2013-2017.
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
FORMS = ../ui/tclPrograms.ui ../ui/tclConsole.ui
SOURCES = testca.cpp ../util/hash.cpp mockup_controller.cpp \
	   ../datamodel/model.cpp ../datamodel/nodemodel.cpp \
	   ../datamodel/contentencryptionmodel.cpp ../net/networklistener.cpp \
           ../net/protocol_message_formatter.cpp \
           ../net/protocol_message_parser.cpp \
           ../net/node.cpp ../net/connection.cpp \
           ../datamodel/netrequestexecutor.cpp \
	   ../datamodel/profile.cpp \
           ../datamodel/ca.cpp \
           ../datamodel/binaryfile.cpp \
           ../datamodel/profilemodel.cpp \
           mockup_model.cpp mockup_nodemodel.cpp \
           ../datamodel/camodel.cpp \
           ../datamodel/binaryfilemodel.cpp \
           ../datamodel/privmsgmodel.cpp \
           ../datamodel/privmsg.cpp \
           ../datamodel/profilecommentmodel.cpp \
           ../datamodel/searchmodel.cpp \
           ../datamodel/datamodelbase.cpp \
           ../datamodel/profilecomment.cpp ../datamodel/trusttreemodel.cpp \
           ../util/jsonwrapper.cpp mockup_voicecallengine.cpp \
           ../datamodel/voicecall.cpp ../datamodel/cadbrecord.cpp \
           ../datamodel/cadbrecordmodel.cpp ../tcl/tclWrapper.cpp \
           ../datamodel/tclprogram.cpp ../datamodel/tclmodel.cpp \
           ../tcl/tclCallbacks.cpp ../tcl/tclUtil.cpp ../ui/tclConsole.cpp \
           ../util/ungzip.cpp
HEADERS = ../util/hash.h ../mcontroller.h mockup_controller.h \
	   ../datamodel/model.h ../datamodel/nodemodel.h \
	   ../datamodel/contentencryptionmodel.h ../net/networklistener.h \
           ../net/protocol_message_formatter.h \
           ../net/protocol_message_parser.h \
           ../net/node.h  ../net/connection.h \
           ../datamodel/netrequestexecutor.h \
	   ../datamodel/profile.h \
           ../datamodel/camodel.h \
           ../datamodel/ca.h \
           ../datamodel/binaryfile.h \
           ../datamodel/binaryfilemodel.h \
           ../datamodel/profilemodel.h            mockup_model.h \
           ../datamodel/mmodelprotocolinterface.h \
           ../datamodel/mnodemodelprotocolinterface.h mockup_nodemodel.h \
           ../datamodel/privmsgmodel.h \
           ../datamodel/privmsg.h \
           ../datamodel/profilecommentmodel.h \
           ../datamodel/searchmodel.h \
           ../datamodel/datamodelbase.h \
           ../datamodel/profilecomment.h ../datamodel/trusttreemodel.h \
           ../util/jsonwrapper.h mockup_voicecallengine.h \
           ../net/mvoicecallengine.h ../datamodel/voicecall.h ../datamodel/cadbrecord.h \
           ../datamodel/cadbrecordmodel.h ../tcl/tclWrapper.h \
           ../datamodel/tclprogram.h ../datamodel/tclmodel.h ../tcl/tclCallbacks.h \
           ../tcl/tclUtil.h ../ui/tclConsole.h ../util/ungzip.h
CONFIG  += debug qtestlib 
QT      += core network sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}
LIBS = -lssl -lcrypto -lgcrypt -lnatpmp -lgcov -lminiupnpc -ltcl -ltk -lz -lbz2
lessThan(QT_MAJOR_VERSION, 5) {
    LIBS += -lqjson
}
unix:INCLUDEPATH += /usr/include/miniupnpc
unix {
        TCL_VERSION = $$system(echo \'puts $tcl_version;exit 0\' | tclsh)
        message(Tcl version $$TCL_VERSION)
} 
win32 {
        TCL_VERSION = 8.6
}
INCLUDEPATH += /usr/include/tcl$$TCL_VERSION
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage -DDEBUG
