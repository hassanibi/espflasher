#-------------------------------------------------
#
# Project created by QtCreator 2015-12-21T22:03:44
#
#-------------------------------------------------



QT       += core gui serialport printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = espflasher
TEMPLATE = app

DEFINES += WITH_POPPLER_QT5

SOURCES += main.cpp\
        mainwindow.cpp \
    elffile.cpp \
    espfirmwareimage.cpp \
    esprom.cpp \
    tools.cpp \
    imagelineedit.cpp \
    imagechooser.cpp \
    loglist.cpp \
    imageinfodialog.cpp \
    flashinputdialog.cpp \
    barcodeprinter.cpp \
    hexlineedit.cpp \
    makeimagedialog.cpp \
    versiondialog.cpp \
    imagefilelistview.cpp \
    preferencesdialog.cpp

HEADERS  += mainwindow.h \
    elffile.h \
    espfirmwareimage.h \
    esprom.h \
    tools.h \
    imagelineedit.h \
    imagechooser.h \
    loglist.h \
    imageinfodialog.h \
    flashinputdialog.h \
    barcodeprinter.h \
    hexlineedit.h \
    makeimagedialog.h \
    versiondialog.h \
    imagefilelistview.h \
    constants.h \
    preferencesdialog.h

FORMS    += mainwindow.ui \
    imagechooser.ui \
    imageinfodialog.ui \
    flashinputdialog.ui \
    makeimagedialog.ui \
    preferencesdialog.ui

RESOURCES += \
    resource.qrc

contains (DEFINES, WITH_POPPLER_QT5) {
    win32: LIBS += -lpoppler-qt5.dll
    else:unix: LIBS += -lpoppler-qt5
}

win32: RC_FILE = espflasher.rc
