CONFIG += plugin
TARGET = im-scim
INCLUDEPATH += . ../common ../client-common
QT += gui-private widgets x11extras

HEADERS += scim-bridge-client-common-qt.h \
           scim-bridge-client-imcontext-qt.h \
           scim-bridge-client-key-event-utility-qt.h \
           scim-bridge-client-qt.h

SOURCES += im-scim-bridge-qt.cpp \
           scim-bridge-client-imcontext-qt.cpp \
           scim-bridge-client-key-event-utility-qt.cpp \
           scim-bridge-client-qt.cpp

QMAKE_LIBDIR += ../client-common/.libs ../common/.libs
LIBS += -lscimbridgeclientcommon -lscimbridgecommon -lX11

OTHER_FILES += scim.json

PLUGIN_TYPE = platforminputcontexts
PLUGIN_EXTENDS = -
PLUGIN_CLASS_NAME = ScimBridgeInputContextPlugin
target.path += $$[QT_INSTALL_PLUGINS]/platforminputcontexts
load(qt_plugin)
