QT += core

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

SOURCES += main.cpp \
           zbarQRScanner.cpp \

# Force static linking to zbar, but any library concatenated to LIBS should be dynamic
LIBS += -Wl,-Bstatic -lzbar -Wl,-Bdynamic

HEADERS += QRScanner.h \
           zbarQRScanner.h \

#for satisfying Qt Creator
contains(BUILDROOT, yes): {
}
else {
    INCLUDEPATH += external
}
