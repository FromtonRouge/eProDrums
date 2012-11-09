######################################################################
# Automatically generated by qmake (2.01a) dim. 29. avr. 21:53:41 2012
######################################################################

TEMPLATE = app
TARGET = eProDrums
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += $(BOOST_DIR)
INCLUDEPATH += $(QWT_DIR)/include

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
           MidiMessage.cpp \
           Pad.cpp \
           PadNotesWidget.cpp \
		   ParamItemEditor.cpp \
           Settings.cpp \
           SettingsDlg.cpp \
           Slot.cpp \
           UserSettings.cpp
RESOURCES += EProDrums.qrc

win32 {
	RC_FILE = EProDrums.rc
}

LIBS += -L$(BOOST_DIR)/stage/lib
LIBS += -lqtmain

win32 {
	LIBS += -lwinmm
}

release {
	LIBS += -L$(QWT_DIR)/lib -lqwt
}

debug {
	LIBS += -L$(QWT_DIR)/lib -lqwtd
}
