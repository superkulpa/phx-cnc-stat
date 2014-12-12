TEMPLATE = app
INCLUDEPATH += ./src ./GeneratedFiles
QT      += xml network webkit
UI_DIR	+= ./GeneratedFiles

CONFIG += debug_and_release

_install-release.depends = $(SOURCES)
_install-debug.depends = $(SOURCES)
win32 {
_install-release.commands = cp ./release/phx-cnc-ui-mainform*  ../install
_install-debug.commands = cp ./debug/phx-cnc-ui-mainform*  ../install
}
unix {
_install-release.commands = cp ./phx-cnc-ui-mainform*  ../install
_install-debug.commands = cp ./phx-cnc-ui-mainform*  ../install
}
QMAKE_EXTRA_TARGETS += _install-release _install-debug

QMAKE_CXXFLAGS += -std=c++11
#QMAKE_CXXFLAGS += -std=c++0x

#QMAKE_CXXFLAGS += -pg
#QMAKE_LFLAGS += -pg


# Input
HEADERS	+=	src/CXSectionDialog.h \
			src/CXSettings.h \
			src/CXSettingsDialog.h \
			src/statistic.h \
			src/SXParamData.h \
			src/engine/engine.h \
			src/qttelnet.h

FORMS	+=	src/CXSectionDialog.ui \
			src/settings.ui \
			src/statistic.ui

SOURCES +=	src/CXSectionDialog.cpp \
			src/CXSettings.cpp \
			src/CXSettingsDialog.cpp \
			src/main.cpp \
			src/statistic.cpp \
			src/engine/engine.cpp \
			src/qttelnet.cpp

RESOURCES += src/statistic.qrc

win32:
{
   RC_FILE = src/icon.rc
}

#DEFINES += __GXX_EXPERIMENTAL_CXX0X__ 
#QMAKE_CXXFLAGS += -std=c++0x
#DESTDIR = install