QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatnoticedetails.cpp \
    chatserver.cpp \
    chatserveradmin.cpp \
    clientmanagerform.cpp \
    logthread.cpp \
    main.cpp \
    mainwindow.cpp \
    ordermanagerform.cpp \
    productmanagerform.cpp

HEADERS += \
    chatnoticedetails.h \
    chatserver.h \
    chatserveradmin.h \
    clientmanagerform.h \
    logthread.h \
    mainwindow.h \
    ordermanagerform.h \
    productmanagerform.h

FORMS += \
    chatnoticedetails.ui \
    chatserver.ui \
    chatserveradmin.ui \
    clientmanagerform.ui \
    mainwindow.ui \
    ordermanagerform.ui \
    productmanagerform.ui

TRANSLATIONS += \
    QtChatProject_ko_KR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
