
HEADERS +=	beamercontrol.h \
			configuredmx.h \
			lightbars.h \
			lightfader.h \
			lightpresets.h \
			mediacenter.h \
			preset.h
			
SOURCES +=	beamercontrol.cpp \
			configuredmx.cpp \
			lightbars.cpp \
			lightfader.cpp \
			lightpresets.cpp \
			main.cpp \
			mediacenter.cpp \
			preset.cpp
			
FORMS +=	beamercontrol.ui \
			configuredmx.ui \
			lightbars.ui \
			lightfader.ui \
			lightpresets.ui \
			mediacenter.ui \
			preset.ui
			
TARGET = bin/mediacenter
CONFIG +=	release \
			warn_on \
			qt \
			thread
			
TEMPLATE = vcapp
LIBPATH += C:\d2xx
LIBS += FTD2XX.lib
INCLUDEPATH += ../qextserialport/
QMAKE_LIBDIR += ../qextserialport/build
LIBS += -lqextserialport
win32:DEFINES  = _TTY_WIN_ QWT_DLL QT_DLL