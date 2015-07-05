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
           ../util/jsonwrapper.cpp
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
           ../util/jsonwrapper.h
CONFIG  += debug
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    CONFIG  += qtestlib 
} else {
    CONFIG  += qtestlib 
}
QT      += core network sql
LIBS = -lssl -lcrypto -lgcrypt -lnatpmp -lgcov -lminiupnpc
lessThan(QT_MAJOR_VERSION, 5) {
    LIBS += -lqjson
}
unix:INCLUDEPATH += /usr/include/miniupnpc
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage -DDEBUG
