#-------------------------------------------------
#
# Project created by QtCreator 2017-06-12T01:37:19
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = eve_api
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += eve_api.h

FORMS    += widget.ui

RESOURCES += \
    icons.qrc

win32:RC_ICONS += icons/6_64_3.ico
