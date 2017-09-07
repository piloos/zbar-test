QT += core gui widgets

CONFIG += c++14

TARGET = zbar-test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -Wextra -Werror -Weffc++

# Make sure that Qt headers are treated as system headers by the compiler.
# This suppresses the possible warnings the Qt headers.
QMAKE_CXXFLAGS += -isystem $$[QT_INSTALL_HEADERS]
QMAKE_CXXFLAGS += -isystem $$[QT_INSTALL_HEADERS]/QtCore

SOURCES += main.cpp

LIBS += -lunified-logger

HEADERS += 

#for satisfying Qt Creator
contains(BUILDROOT, yes): {
}
else {
    INCLUDEPATH += external/unified-logger/include
    INCLUDEPATH += external
}
