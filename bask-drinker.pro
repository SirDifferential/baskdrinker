QT       += core gui multimedia websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 force_debug_info
DEFINES += BASK_VERSION=\"\\\"1.11\\\"\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    baskclient.cpp \
    baskserver.cpp \
    keyboardhandler.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    baskclient.h \
    baskserver.h \
    keyboardhandler.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    bask-drinker_en_GB.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

win32: {
    LIBS += -luser32
}
