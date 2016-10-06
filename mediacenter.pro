QT += widgets serialport

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

win32: LIBS += -L$$PWD/./ -lftd2xx
