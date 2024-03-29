# -------------------------------------------------
# Project created by QtCreator 2009-08-16T17:44:04
# -------------------------------------------------
QT += phonon \
    network
TARGET = qupod
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    cipodmanager.cpp \
    caudioplayer.cpp \
    csetartworkdialog.cpp \
    cmodelitem.cpp \
    cabstractitemmodel.cpp \
    csortfiltermodel.cpp \
    cartiststreemodel.cpp \
    cplaylistmodel.cpp \
    cmountpointdialog.cpp \
    ctageditdialog.cpp \
    ctageditormodel.cpp \
    caddfilesdialog.cpp \
    cmp3filemodel.cpp
HEADERS += mainwindow.h \
    cipodmanager.h \
    caudioplayer.h \
    csetartworkdialog.h \
    cmodelitem.h \
    cabstractitemmodel.h \
    csortfiltermodel.h \
    cartiststreemodel.h \
    cplaylistmodel.h \
    cmountpointdialog.h \
    ctageditdialog.h \
    ctageditormodel.h \
    caddfilesdialog.h \
    cmp3filemodel.h
FORMS += mainwindow.ui \
    csetartworkdialog.ui \
    cmountpointdialog.ui \
    ctageditdialog.ui \
    caddfilesdialog.ui
LIBS += $$system(pkg-config libgpod-1.0 --libs) $$system(pkg-config --libs taglib)
RESOURCES += qupod.qrc
unix:INCLUDEPATH += /usr/include/glib-2.0 \
    /usr/include/gtk-2.0
unix:INCLUDEPATH += /usr/lib64/glib-2.0/include
unix:INCLUDEPATH += /usr/include/taglib
