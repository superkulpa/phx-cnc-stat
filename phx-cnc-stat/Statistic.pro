TEMPLATE = app
QT      += xml network webkit


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

DEFINES += __GXX_EXPERIMENTAL_CXX0X__ 
QMAKE_CXXFLAGS += -std=c++0x
DESTDIR = install