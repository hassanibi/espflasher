#-------------------------------------------------
#
# Project created by QtCreator 2015-12-21T22:03:44
#
#-------------------------------------------------

QT       += core gui serialport printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = espflasher-qt
TEMPLATE = app


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
    versiondialog.cpp

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
    versiondialog.h

FORMS    += mainwindow.ui \
    imagechooser.ui \
    imageinfodialog.ui \
    flashinputdialog.ui \
    makeimagedialog.ui

RESOURCES += \
    resource.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../opt/Qt/5.5/gcc_64/lib/release/ -lpoppler-qt5
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../opt/Qt/5.5/gcc_64/lib/debug/ -lpoppler-qt5
else:unix: LIBS += -L$$PWD/../../../../opt/Qt/5.5/gcc_64/lib/ -lpoppler-qt5

INCLUDEPATH += $$PWD/../../../../opt/Qt/5.5/gcc_64/include
DEPENDPATH += $$PWD/../../../../opt/Qt/5.5/gcc_64/include
