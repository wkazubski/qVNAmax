#-------------------------------------------------
#
# Project created by QtCreator 2012-12-01T22:06:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = qvnamax
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    settingsdialog.cpp \
    colordialog.cpp \
    commentdialog.cpp \
    vna_serial.cpp

HEADERS  += mainwindow.h \
    vna.h \
    settingsdialog.h \
    colordialog.h \
    commentdialog.h \
    vna_serial.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    colordialog.ui \
    commentdialog.ui

TRANSLATIONS = qvnamax_pl.ts

RESOURCES += icon.qrc

ad9851 {
    SOURCES += vna_ad9851.cpp
    HEADERS += vna_ad9851.h
    DEFINES += HAVE_AD9851
}

ad9951 {
    SOURCES += vna_ad9951.cpp
    HEADERS += vna_ad9951.h
    DEFINES += HAVE_AD9951
}

test {
    SOURCES += vna_test.cpp
    HEADERS += vna_test.h
    DEFINES += HAVE_TEST
}

qwt-rmb {
    DEFINES += HAVE_RMB
}

unix:CONFIG += qwt qtserialport
unix:INCLUDEPATH += /usr/include/qwt /usr/include/QtAddOnSerialPort
unix:LIBS += -lqwt -lSerialPort -lqwt-rmb

DISTFILES += qvnamax_pl.qm

# Install

isEmpty(PREFIX) {
    PREFIX = /usr/local
}
BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share
DEFINES += APP_DATA_DIR=\"$$PREFIX/share/qvnamax\"

bin.files = qvnamax
bin.path = $$PREFIX/bin

desktop.files = qvnamax.desktop
desktop.path = $$DATADIR/applications

language.files = qvnamax_pl.qm
language.path = $$DATADIR/qvnamax

icon16.files = icons/i16/qvnamax.png
icon16.path = $$DATADIR/icons/hicolor/16x16/apps

icon32.files = icons/i32/qvnamax.png
icon32.path = $$DATADIR/icons/hicolor/32x32/apps

icon48.files = icons/i48/qvnamax.png
icon48.path = $$DATADIR/icons/hicolor/48x48/apps

icon64.files = icons/i64/qvnamax.png
icon64.path = $$DATADIR/icons/hicolor/64x64/apps

INSTALLS += bin desktop icon16 icon32 icon48 icon64 language
