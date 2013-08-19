######################################################################
# Automatically generated by qmake (2.01a) dim. 29. avr. 21:53:41 2012
######################################################################

# Define your paths here
win32 {
	BOOST_DIR = D:/vneang/dev/boost_1_54_0
	QWT_DIR = D:/vneang/dev/Qwt-6.1.0-svn
	PORTMIDI_DIR = D:/vneang/dev/portmidi
}

# Define your paths here
unix {
	BOOST_DIR = /usr
	QWT_DIR = /usr
	PORTMIDI_DIR = /usr
}

TEMPLATE = app
TARGET = eProDrums
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += debug_and_release

win32 {
	INCLUDEPATH += $${BOOST_DIR}
}

unix {
	INCLUDEPATH += $${BOOST_DIR}/include
}

INCLUDEPATH += $${QWT_DIR}/include
INCLUDEPATH += $${PORTMIDI_DIR}/include

DEFINES -= UNICODE
DEFINES += QWT_DLL

win32 {
	DEFINES += _SCL_SECURE_NO_WARNINGS
	# disable warning on too long decoration names
	QMAKE_CXXFLAGS += /wd4503
	QMAKE_CXXFLAGS_DEBUG += /Od
}

# Input
HEADERS += AddMidiNote.h \
           DialogAbout.h \
           DialogFunction.h \
           DrumKitItemDelegate.h \
           DrumKitItemModel.h \
           DrumKitItemEditor.h \
           DrumNoteItemModel.h \
           DrumNoteItemDelegate.h \
           DrumNoteItemEditor.h \
           DrumKitMidiMap.h \
           EProPlot.h \
           EProPlotCurve.h \
           EProPlotData.h \
           EProPlotIntervalData.h \
           EProPlotMarker.h \
           EProPlotZoomer.h \
		   FunctionItemDelegate.h \
		   FunctionItemEditor.h \
		   FunctionItemModel.h \
		   GraphSubWindow.h \
           HiHatPedalCurve.h \
           HiHatPedalElement.h \
           HiHatPositionCurve.h \
           MainWindow.h \
           MidiDevicesWidget.h \
           MidiEngine.h \
           MidiMessage.h \
           Pad.h \
           PadHandler.h \
           PadNotesWidget.h \
		   ParamItemDelegate.h \
		   ParamItemEditor.h \
		   ParamItemModel.h \
           Settings.h \
           SettingsDlg.h \
           Slot.h \
		   TimeBar.h \
		   TimeSlider.h \
		   TimeSpinBox.h \
		   TreeViewParameters.h \
           UserSettings.h
FORMS += AddMidiNote.ui \
         DialogAbout.ui \
         DialogFunction.ui \
         MainWindow.ui \
         PadNotesWidget.ui \
         SettingsDlg.ui
SOURCES += AddMidiNote.cpp \
           DialogAbout.cpp \
           DialogFunction.cpp \
           DrumNoteItemEditor.cpp \
           EProPlot.cpp \
           EProPlotCurve.cpp \
           EProPlotData.cpp \
           EProPlotIntervalData.cpp \
           EProPlotMarker.cpp \
           EProPlotZoomer.cpp \
		   FunctionItemEditor.cpp \
		   GraphSubWindow.cpp \
           HiHatPedalCurve.cpp \
           HiHatPedalElement.cpp \
           HiHatPositionCurve.cpp \
		   LinearFunction.cpp \
           main.cpp \
           MainWindow.cpp \
           MidiDevicesWidget.cpp \
           MidiEngine.cpp \
           MidiMessage.cpp \
           Pad.cpp \
           PadNotesWidget.cpp \
		   ParamItemEditor.cpp \
           Settings.cpp \
           SettingsDlg.cpp \
           Slot.cpp \
		   TimeBar.cpp \
		   TimeSlider.cpp \
		   TimeSpinBox.cpp \
           UserSettings.cpp
RESOURCES += EProDrums.qrc

# Documentation
manual.target = doc/collection.qhc
manual.depends = doc/*.qhp doc/*.qhcp doc/*.html
manual.commands = qcollectiongenerator doc/collection.qhcp
QMAKE_EXTRA_TARGETS += manual
PRE_TARGETDEPS += doc/collection.qhc

LIBS += -L$${PORTMIDI_DIR}/lib -lportmidi
LIBS += -L$${QWT_DIR}/lib

win32 {
	RC_FILE = EProDrums.rc
	LIBS += -L$${BOOST_DIR}/stage/lib

	debug {
		LIBS += -lqwtd
	}

	release {
		LIBS += -lqwt
	}
}

unix {
	LIBS += -lqwt
	LIBS += -L$${BOOST_DIR}/lib -lboost_system -lboost_serialization -lboost_thread -lboost_chrono -lboost_filesystem -lboost_program_options
	LIBS += -L$${PORTMIDI_DIR}/lib -lporttime
}

unix {
	target.path = /usr/bin
	INSTALLS += target
}
