QT += network widgets
requires(qtConfig(filedialog))

HEADERS += src/addurldialog.h \
           src/mainwindow.h \
           src/downloadtask.h


SOURCES += src/main.cpp \
           src/addurldialog.cpp \
           src/mainwindow.cpp \
           src/downloadtask.cpp

# Forms and resources
FORMS += forms/addurlform.ui
RESOURCES += icons.qrc

RC_FILE += tdm.rc
