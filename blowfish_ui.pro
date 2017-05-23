#-------------------------------------------------
#
# Project created by QtCreator 2014-04-11T19:39:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = blowfish_ui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    blowfish.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    blowfish.h \
    aboutdialog.h \
    menuproxystyle.h \
    matrixpi.h

FORMS    += mainwindow.ui \
    aboutdialog.ui

RESOURCES += \
    icons.qrc
