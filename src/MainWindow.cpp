// ============================================================
// 
// This file is a part of the eProDrums rock band project
// 
// Copyright (C) 2012 by Vissale Neang <fromtonrouge at gmail dot com>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License or (at your option) version 3 or any later version
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// ============================================================ 

#include "MainWindow.h"
#include "TimeBar.h"
#include "DialogAbout.h"
#include "Settings.h"
#include "SettingsDlg.h"
#include "MidiDevicesWidget.h"
#include "TreeViewParameters.h"
#include "TreeNode.h"
#include "GroupData.h"
#include "ParameterData.h"
#include "Parameter.h"
#include "ParamItemModel.h"
#include "ParamItemProxyModel.h"

#include "Pad.h"
#include "HiHatPedalElement.h"
#include "HiHatPositionCurve.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QUndoStack>
#include <QtCore/QItemSelectionModel>
#include <QtCore/QFileInfo>

#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/trim.hpp>
namespace fs = boost::filesystem;

#include <fstream>
#include <exception>
#include <algorithm>
#include <limits>
#include <iostream>

BOOST_CLASS_EXPORT(HiHatPedalElement); 

void MainWindow::toLog(const std::string& szText)
{
	emit signalLog(boost::algorithm::trim_copy_if(szText, boost::is_any_of("\n")).c_str());
}

MainWindow::MainWindow():
	_pOldStreambuf(std::cout.rdbuf()),
	_pSettings(new Settings()),
	_currentSlot(_userSettings.configSlots.end())
{
	qRegisterMetaType<Slot::Ptr>();
	qRegisterMetaType<MidiMessage>();

	setupUi(this);
	setWindowTitle(QApplication::applicationName());

	// Proxy model for Pad Settings
	_pProxyModel = new ParamItemProxyModel(this);
	treeViewPadSettings->setModel(_pProxyModel);

	// Docks
	dockWidgetLogs->hide();
	tabifyDockWidget(dockWidgetUndo, dockWidgetSlots);
	tabifyDockWidget(dockWidgetLogs, dockWidgetPadSettings);

	// Undo stack creation
	_pUndoStack = new QUndoStack(this);
	listViewUndo->setStack(_pUndoStack);

	// Setup std::cout redirection
	_streamBuffer.open(StreamSink(boost::bind(&MainWindow::toLog, this, _1)));
	std::cout.rdbuf(&_streamBuffer);
	connect(this, SIGNAL(signalLog(const QString&)), textEditLog, SLOT(append(const QString&)));

	// Building the first toolbar
	toolBar->setWindowTitle(tr("Main toolbar"));
	toolBar->setIconSize(QSize(16, 16));
	toolBar->addAction(actionOpen);
	toolBar->addAction(actionSave);
	toolBar->addAction(actionSettings);
	toolBar->addSeparator();
	_pMidiDevicesWidget = new MidiDevicesWidget(this);
	connect(_pMidiDevicesWidget, SIGNAL(signalStart(int, int)), &_midiEngine, SLOT(start(int, int)));
	connect(_pMidiDevicesWidget, SIGNAL(signalStop()), &_midiEngine, SLOT(stop()));
	toolBar->addWidget(_pMidiDevicesWidget);
	toolBar->addSeparator();

	QWidget* pBufferWidget = new QWidget(this);
	QHBoxLayout* pLayout = new QHBoxLayout;
	pLayout->setContentsMargins(0,0,0,0);

	pLayout->addWidget(new QLabel(tr("Buffer")));
	_pSpinBoxInputBuffer = new QSpinBox(this);
	_pSpinBoxInputBuffer->setToolTip(tr("Midi input buffer in milliseconds, improve specific features (Chameleon cymbal), affects the latency"));
	_pSpinBoxInputBuffer->setMaximum(300);
	connect(_pSpinBoxInputBuffer, SIGNAL(valueChanged(int)), this, SLOT(onInputBufferChanged(int)));
	pLayout->addWidget(_pSpinBoxInputBuffer);

	pLayout->addWidget(new QLabel(tr("Latency")));
	_pAverageLatency = new QDoubleSpinBox(this);
	_pAverageLatency->setToolTip(tr("Average latency in milliseconds"));
	_pAverageLatency->setAlignment(Qt::AlignHCenter);
	_pAverageLatency->setMaximum(999);
	_pAverageLatency->setReadOnly(true);
	_pAverageLatency->setButtonSymbols(QDoubleSpinBox::NoButtons);
	pLayout->addWidget(_pAverageLatency);
	pBufferWidget->setLayout(pLayout);
	toolBar->addWidget(pBufferWidget);

	// Building the time toolbar
	TimeBar* pTimeBar = new TimeBar(this);
	connect(&_midiEngine, SIGNAL(signalMidiOut(const MidiMessage&)), pTimeBar, SLOT(onMidiOut(const MidiMessage&)), Qt::QueuedConnection);
	toolBarTime->setWindowTitle(tr("Time toolbar"));
	toolBarTime->addWidget(pTimeBar);

	// Process "assistant" for help
	_pProcessAssistant = new QProcess(this);

	// Building the graph subwindow
	_pGrapSubWindow = new GraphSubWindow(&_userSettings, this);
	connect(pTimeBar, SIGNAL(signalTimeChanged(int)), _pGrapSubWindow, SLOT(onTimeChange(int)));
	connect(_pGrapSubWindow, SIGNAL(signalTimeOffset(int)), pTimeBar, SLOT(onTimeOffset(int)));

	_pSettings->signalKitDefined.connect(boost::bind(&GraphSubWindow::onDrumKitLoaded, _pGrapSubWindow, _1, _2));
	_pSettings->signalKitDefined.connect(boost::bind(&MidiEngine::onDrumKitLoaded, &_midiEngine, _1, _2));
	connect(&_midiEngine, SIGNAL(signalHiHatStartMoving(int, int, int)), _pGrapSubWindow, SLOT(onHiHatStartMoving(int, int, int)), Qt::QueuedConnection);
	connect(&_midiEngine, SIGNAL(signalHiHatState(int)), _pGrapSubWindow, SLOT(onHiHatState(int)), Qt::QueuedConnection);
	connect(&_midiEngine, SIGNAL(signalFootCancelStarted(int, int, int)), _pGrapSubWindow, SLOT(onFootCancelStarted(int, int, int)), Qt::QueuedConnection);
	connect(&_midiEngine, SIGNAL(signalMidiOut(const MidiMessage&)), _pGrapSubWindow, SLOT(onUpdatePlot(const MidiMessage&)), Qt::QueuedConnection);
	connect(&_midiEngine, SIGNAL(signalAverageLatency(double)), _pAverageLatency, SLOT(setValue(double)), Qt::QueuedConnection);
	connect(&_midiEngine, SIGNAL(signalStarted()), this, SLOT(onMidiEngineStarted()));
	connect(&_midiEngine, SIGNAL(signalStopped()), this, SLOT(onMidiEngineStopped()));
	connect(listViewSlots, SIGNAL(signalSlotChanged(const Slot::Ptr&)), this, SLOT(onSlotChanged(const Slot::Ptr&)));
	connect(listViewSlots, SIGNAL(signalSlotChanged(const Slot::Ptr&)), &_midiEngine, SLOT(onSlotChanged(const Slot::Ptr&)));
	connect(_pSpinBoxInputBuffer, SIGNAL(valueChanged(int)), &_midiEngine, SLOT(onInputBufferChanged(int)));

	mdiArea->addSubWindow(_pGrapSubWindow);
	_pGrapSubWindow->showMaximized();

	// Create the slot model
	_pSlotItemModel = new SlotItemModel(_pSettings.get(), NULL, this);
	_pSlotItemModel->setUndoStack(_pUndoStack);

	listViewSlots->setModel(_pSlotItemModel);

	// Loading last user settings
	loadUserSettings(_pSettings->getUserSettingsFile().generic_string());

	// Building the default config if empty
	if (_userSettings.configSlots.empty())
	{
		_userSettings.configSlots.push_back(createDefaultSlot());
	}

	_pSlotItemModel->setSlots(&_userSettings.configSlots);

	_currentSlot = _userSettings.configSlots.begin();

	selectLastSlot();

	// Filling midi devices
	_pMidiDevicesWidget->setMidiInDevices(_midiEngine.getMidiInDevices());
	_pMidiDevicesWidget->setMidiOutDevices(_midiEngine.getMidiOutDevices());
	if (_pMidiDevicesWidget->setMidiIn(_pSettings->getMidiIn()) && _pMidiDevicesWidget->setMidiOut(_pSettings->getMidiOut()))
	{
		_pMidiDevicesWidget->onMidiStart();
	}

	// Settings connections
	_pSettings->signalRedrawPeriodChanged.connect(boost::bind(&GraphSubWindow::onRedrawPeriodChanged, _pGrapSubWindow, _1));
	_pSettings->signalCurveWindowLengthChanged.connect(boost::bind(&GraphSubWindow::onCurveWindowLengthChanged, _pGrapSubWindow, _1));
	_pGrapSubWindow->onRedrawPeriodChanged(_pSettings->getRedrawPeriod());
	_pGrapSubWindow->onCurveWindowLengthChanged(_pSettings->getCurveWindowLength());

	// Restore saved docks, toolbars, geometry etc.
	QSettings* pSettings = _pSettings->getQSettings();
	restoreGeometry(pSettings->value("geometry").toByteArray());
	restoreState(pSettings->value("windowState").toByteArray());

	// Create redo/undo actions and add them to the main menu
	QAction* pUndoAction = _pUndoStack->createUndoAction(this);
	pUndoAction->setIcon(QIcon(":/icons/arrow-circle-225-left.png"));
	pUndoAction->setShortcut(tr("Ctrl+z"));
	menuEdit->insertAction(actionSettings, pUndoAction);
	QAction* pRedoAction = _pUndoStack->createRedoAction(this);
	pRedoAction->setIcon(QIcon(":/icons/arrow-circle-315.png"));
	pRedoAction->setShortcut(tr("Ctrl+y"));
	menuEdit->insertAction(actionSettings, pRedoAction);
	QAction* pSeparator = new QAction(this);
	pSeparator->setSeparator(true);
	menuEdit->insertAction(actionSettings, pSeparator);

	// Add the docks and toolbars menu to the main menu
	QMenu* pDocksMenu = createPopupMenu();
	pDocksMenu->setTitle(tr("Docks and Toolbars"));
	pDocksMenu->setIcon(QIcon(":/icons/application-dock-270.png"));
	menuWindows->addMenu(pDocksMenu);
}

MainWindow::~MainWindow()
{
	// Restoring the std::cout streambuf
	std::cout.rdbuf(_pOldStreambuf);
}

void MainWindow::onMidiEngineStarted()
{
	_pSettings->setMidiIn(_pMidiDevicesWidget->getMidiInString());
	_pSettings->setMidiOut(_pMidiDevicesWidget->getMidiOutString());

	_pGrapSubWindow->clearPlots();
	_pGrapSubWindow->replot();
}

void MainWindow::onMidiEngineStopped()
{
}

void MainWindow::on_actionSave_As_triggered()
{
	fs::path pathDefaultConfig(_pSettings->getUserSettingsFile());
	QString qszDefaultDir(".\\default.epd");
	if (pathDefaultConfig.has_parent_path())
	{
		qszDefaultDir = pathDefaultConfig.parent_path().generic_string().c_str();
	}

	QFileDialog fileDlg(this, tr("Save configuration file"), qszDefaultDir, "Rock Band Pro Drums (*.epd)");
	fileDlg.setDefaultSuffix("epd");
	fileDlg.setAcceptMode(QFileDialog::AcceptSave);
	if (fileDlg.exec())
	{
		const QStringList& files = fileDlg.selectedFiles();
		saveUserSettings(files[0].toStdString());
	}
}

void MainWindow::selectLastSlot()
{
	QItemSelectionModel* pSelectionModel = listViewSlots->selectionModel();
	if (pSelectionModel)
	{
		int slotIndex = _pSettings->getLastSelectedSlotIndex();
		const QModelIndex& index = _pSlotItemModel->index(slotIndex, 0);
		if (index.isValid())
		{
			pSelectionModel->select(index, QItemSelectionModel::ClearAndSelect);
		}
		else
		{
			pSelectionModel->select(_pSlotItemModel->index(0,0), QItemSelectionModel::ClearAndSelect);
		}
	}
}

void MainWindow::on_actionOpen_triggered()
{
	fs::path pathDefaultConfig(_pSettings->getUserSettingsFile());
	QString qszDefaultDir(".\\");
	if (pathDefaultConfig.has_parent_path() && fs::exists(pathDefaultConfig.parent_path()))
	{
		qszDefaultDir = pathDefaultConfig.parent_path().generic_string().c_str();
	}

	QString filePath = QFileDialog::getOpenFileName(this, "Open configuration file", qszDefaultDir, "eProDrums Config File (*.epd)");
	if (filePath.isNull())
	{
		return;
	}

	loadUserSettings(filePath.toStdString());
	selectLastSlot();
}

void MainWindow::loadUserSettings(const std::string& szFilePath)
{
	if (!fs::exists(szFilePath))
	{
		return;
	}

	Mutex::scoped_lock lock(_mutex);

	try
	{
		fs::path pathConfig(szFilePath);
		std::ifstream ifs(pathConfig.generic_string().c_str());
		if (ifs.good())
		{
			boost::archive::xml_iarchive ia(ifs);
			ia >> BOOST_SERIALIZATION_NVP(_userSettings);
			_userSettings.filePath = pathConfig.generic_string();

			_pSlotItemModel->setSlots(&_userSettings.configSlots);

			// Global settings and all slots are connected, we can reload/load the drum kit map
			_pSettings->reloadDrumKitMidiMap();
			_pSettings->setUserSettingsFile(pathConfig.generic_string());

			setWindowTitle(QString("%1 - [%2]").arg(QApplication::applicationName()).arg(pathConfig.filename().string().c_str()));
			_pSpinBoxInputBuffer->setValue(_userSettings.bufferLength);

			// Set curve visibility
			_pGrapSubWindow->loadCurveVisibility();
		}
	}
	catch (const std::exception& e)
	{
		boost::format fmtMsg("Error while loading configuration. Reason : %s");
		fmtMsg%e.what();
		QMessageBox::critical(this,"Error while loading configuration", fmtMsg.str().c_str());
		_userSettings.configSlots.clear();
		_pSettings->setUserSettingsFile("./");
	}
}

void MainWindow::saveUserSettings(const std::string& szFilePath)
{
	try
	{
		_pGrapSubWindow->saveCurveVisibility();

		fs::path pathConfig(szFilePath);
		std::ofstream ofs(pathConfig.generic_string().c_str());
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(_userSettings);
		_userSettings.filePath = pathConfig.generic_string();
		_pSettings->setUserSettingsFile(pathConfig.generic_string());
		setWindowTitle(QString("%1 - [%2]").arg(QApplication::applicationName()).arg(pathConfig.filename().string().c_str()));
	}
	catch (const std::exception& e)
	{
		boost::format fmtMsg("Error while saving configuration. Reason : %s");
		fmtMsg%e.what();
		QMessageBox::critical(this,"Error while loading configuration", fmtMsg.str().c_str());
	}
}

void MainWindow::on_actionSave_triggered()
{
	if (fs::exists(_userSettings.filePath) && !fs::is_directory(_userSettings.filePath))
	{
		saveUserSettings(_userSettings.filePath);
	}
	else
	{
		on_actionSave_As_triggered();
	}
}

void MainWindow::on_actionQuit_triggered()
{
	QApplication::exit();
}

void MainWindow::onInputBufferChanged(int value)
{
	_userSettings.bufferLength = value;
}

Slot::Ptr MainWindow::createDefaultSlot(const QString& szSlotName)
{
	Slot::Ptr pDefaultSlot(new Slot());
	pDefaultSlot->setName(szSlotName);

	// Snare
	const Pad::Ptr& pElSnare = Pad::Ptr(new Pad(Pad::SNARE, Pad::NOTE_SNARE));
	pElSnare->typeFlam->set(Pad::TOM1);
	pDefaultSlot->getPads().push_back(pElSnare);

	// Hi Hat
	const Pad::Ptr& pElHihat = Pad::Ptr(new Pad(Pad::HIHAT, Pad::NOTE_HIHAT)); 
	pElHihat->typeFlam->set(Pad::SNARE);
	pDefaultSlot->getPads().push_back(pElHihat);

	// Hi Hat Pedal
	const HiHatPedalElement::Ptr& pElHihatPedal = HiHatPedalElement::Ptr(new HiHatPedalElement());
	pDefaultSlot->getPads().push_back(pElHihatPedal);

	const Pad::Ptr& pElTom1 = Pad::Ptr(new Pad(Pad::TOM1, Pad::NOTE_TOM1)); 
	pElTom1->typeFlam->set(Pad::TOM2);
	pDefaultSlot->getPads().push_back(pElTom1);

	const Pad::Ptr& pElTom2 = Pad::Ptr(new Pad(Pad::TOM2, Pad::NOTE_TOM2)); 
	pElTom2->typeFlam->set(Pad::TOM3);
	pDefaultSlot->getPads().push_back(pElTom2);

	const Pad::Ptr& pElTom3 = Pad::Ptr(new Pad(Pad::TOM3, Pad::NOTE_TOM3)); 
	pElTom3->typeFlam->set(Pad::TOM2);
	pDefaultSlot->getPads().push_back(pElTom3);

	const Pad::Ptr& pElCrash1 = Pad::Ptr(new Pad(Pad::CRASH1, Pad::NOTE_CRASH1)); 
	pElCrash1->typeFlam->set(Pad::RIDE);
	pDefaultSlot->getPads().push_back(pElCrash1);

	const Pad::Ptr& pElCrash2 = Pad::Ptr(new Pad(Pad::CRASH2, Pad::NOTE_CRASH2)); 
	pElCrash2->typeFlam->set(Pad::RIDE);
	pDefaultSlot->getPads().push_back(pElCrash2);

	const Pad::Ptr& pElCrash3 = Pad::Ptr(new Pad(Pad::CRASH3, Pad::NOTE_CRASH3)); 
	pDefaultSlot->getPads().push_back(pElCrash3);

	const Pad::Ptr& pElRide = Pad::Ptr(new Pad(Pad::RIDE, Pad::NOTE_RIDE)); 
	pElRide->typeFlam->set(Pad::CRASH2);
	pDefaultSlot->getPads().push_back(pElRide);

	const Pad::Ptr& pElBassDrum = Pad::Ptr(new Pad(Pad::BASS_DRUM, Pad::NOTE_BASS_DRUM)); 
	pDefaultSlot->getPads().push_back(pElBassDrum);

	return pDefaultSlot;
}

void MainWindow::on_actionAssistant_triggered()
{
	QString applicationPath(QApplication::applicationDirPath());
	std::vector<QFileInfo> collections;
	collections.push_back(QFileInfo(applicationPath + "/doc/collection.qhc"));
	collections.push_back(QFileInfo(applicationPath + "/../doc/collection.qhc"));
	QString collectionPath;
	for (size_t i=0; i<collections.size(); ++i)
	{
		if (collections[i].exists())
		{
			collectionPath = collections[i].filePath();
			break;
		}
	}

	if (!collectionPath.isEmpty())
	{
#ifdef _WIN32
		QFileInfo program(applicationPath + "/assistant.exe");
#else
		QFileInfo program(applicationPath + "/assistant");
#endif
		if (program.exists())
		{
			QStringList arguments;
			arguments << "-collectionFile" << collectionPath;
			_pProcessAssistant->start(program.filePath(), arguments);
		}
		else
		{
			QMessageBox::critical(this, tr("Error can't find assistant"), tr("Can't find the binary \"%1\"").arg(program.filePath()));
		}
	}
}

void MainWindow::on_actionAbout_triggered()
{
	boost::format fmtMsg("%s (beta) %s");
	fmtMsg%QApplication::applicationName().toStdString()%QApplication::applicationVersion().toStdString();
	DialogAbout dlgAbout(this, fmtMsg.str(), "FromtonRouge");
	dlgAbout.exec();
}

void MainWindow::on_pushButtonClearLogs_clicked(bool)
{
	textEditLog->clear();
}

void MainWindow::on_comboBoxPadSettingsType_currentIndexChanged(int index)
{
	switch(index)
	{
	case 0:
		{
			_pGrapSubWindow->showHiHatLayers(true);
			_pGrapSubWindow->showFootCancelLayers(false);
			_pGrapSubWindow->showHiHatPedalMaskLayer(false);
			break;
		}
	case 1:
		{
			_pGrapSubWindow->showHiHatLayers(false);
			_pGrapSubWindow->showFootCancelLayers(true);
			_pGrapSubWindow->showHiHatPedalMaskLayer(true);
			break;
		}
	default:
		{
			break;
		}
	}

	_pProxyModel->setPadSettingsType(index);
	_pProxyModel->invalidate();
	treeViewPadSettings->expandAll();
	treeViewPadSettings->resizeColumnToContents(0);
}

void MainWindow::on_actionSettings_triggered()
{
	SettingsDlg settings(_pSettings.get());
	connect(&_midiEngine, SIGNAL(signalMidiIn(const MidiMessage&)), &settings, SIGNAL(onMidiIn(const MidiMessage&)));
	settings.exec();
}

Slot::Ptr MainWindow::getCurrentSlot() const
{
	Mutex::scoped_lock lock(_mutex);
	if (_currentSlot!=_userSettings.configSlots.end())
	{
		return *_currentSlot;
	}
	else
	{
		return Slot::Ptr();
	}
}

void MainWindow::closeEvent(QCloseEvent* pEvent)
{
	QSettings* pSettings = _pSettings->getQSettings();
    pSettings->setValue("geometry", saveGeometry());
    pSettings->setValue("windowState", saveState());
    QMainWindow::closeEvent(pEvent);
}

void MainWindow::onSlotChanged(const Slot::Ptr& pSlot)
{
	// Save last selected slot
	QItemSelectionModel* pSelectionModel = listViewSlots->selectionModel();
	if (pSelectionModel)
	{
		const QModelIndex& index = pSelectionModel->currentIndex();
		if (index.isValid())
		{
			_pSettings->setLastSelectedSlotIndex(index.row());

			// Hi Hat Curve default states
			on_comboBoxPadSettingsType_currentIndexChanged(comboBoxPadSettingsType->currentIndex());
		}
	}

	// If the ParamItemModel does not exist we create it
	if (!pSlot->model && !pSlot->getPads().empty())
	{
		pSlot->model.reset(new ParamItemModel());
		pSlot->model->setUndoStack(_pUndoStack);

		std::vector<QColor> groupColors;
		groupColors.push_back(QColor(150, 150, 250));
		groupColors.push_back(QColor(150, 250, 150));
		groupColors.push_back(QColor(250, 150, 150));
		groupColors.push_back(QColor(250, 250, 150));
		groupColors.push_back(QColor(150, 250, 250));
		groupColors.push_back(QColor(250, 150, 250));
		groupColors.push_back(QColor(180, 180, 220));

		const Pad::List& pads = pSlot->getPads();

		const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(pads[Pad::HIHAT_PEDAL]);

		// Build the new root
		TreeNode* pRoot = pSlot->model->getRoot();

		TreeNode* pGroup1 = pRoot->addChild(new GroupData(tr("Hi-hat secured positions")));
		{
			pGroup1->addChild(new ParameterData(new Parameter(	tr("Secured close position (unit)"),
																tr("Under this position the hi-hat is in close color"),
																pElHihatPedal->securityPosition, 0, 127)));
			pGroup1->addChild(new ParameterData(new Parameter(	tr("Bow hits always in close color"),
																tr("If checked, bow hits are always in close color"),
																pElHihatPedal->isBowAlwaysYellow)));
		}

		TreeNode* pGroup2 = pRoot->addChild(new GroupData(tr("Hi-hat open color on edge accent"), tr(""), pElHihatPedal->isBlueDetectionByAccent));
		{
			LinearFunction::Description::Ptr pFuncDesc(new LinearFunction::Description());
			pFuncDesc->szLabelX = tr("Hi-hat position (unit)");
			pFuncDesc->szLabelY = tr("Velocity (unit)");
			pGroup2->addChild(new ParameterData(new Parameter(	tr("Parameters"),
																tr("List of linear functions used to determine when to convert an accented hi-hat note to open color"),
																pElHihatPedal->funcBlueAccent,
																pFuncDesc)));
			pGroup2->addChild(new ParameterData(new Parameter(	tr("Override secured position"),
																tr("If checked, edge accented hits are converted to open color even under the secured close position"),
																pElHihatPedal->isBlueAccentOverride)));
		}

		TreeNode* pGroup3 = pRoot->addChild(new GroupData(tr("Hi-hat open color detection by position"), tr(""), pElHihatPedal->isBlueDetectionByPosition));
		{
			pGroup3->addChild(new ParameterData(new Parameter(	tr("Control Position (unit)"),
																tr("Above this position the hi-hat is converted to open color"),
																pElHihatPedal->controlPosThreshold, 0, 127)));
			pGroup3->addChild(new ParameterData(new Parameter(	tr("Delay time (ms)"),
																tr("Delay time before switching to open color if the control position is above the threshold"),
																pElHihatPedal->controlPosDelayTime, 0, 5000)));
		}
		
		TreeNode* pGroup4 = pRoot->addChild(new GroupData(tr("Hi-hat open color detection by speed"), tr(""), pElHihatPedal->isBlueDetectionBySpeed));
		{
			pGroup4->addChild(new ParameterData(new Parameter(	tr("Open speed (unit/s)"),
																tr("Above this speed the hi-hat is converted to open color"),
																pElHihatPedal->openSpeed, 0, 5000, false, true)));
			pGroup4->addChild(new ParameterData(new Parameter(	tr("Close speed (unit/s)"),
																tr("Under this speed the hi-hat is converted to close color"),
																pElHihatPedal->closeSpeed, -5000, 0)));
		}

		TreeNode* pGroup5 = pRoot->addChild(new GroupData(tr("Hi-hat open color detection by acceleration"), tr(""), pElHihatPedal->isBlueDetectionByAcceleration));
		{
			pGroup5->addChild(new ParameterData(new Parameter(	tr("Open acceleration (unit/s2)"),
																tr("Above this acceleration the hi-hat is converted to open color"),
																pElHihatPedal->openAcceleration, 0, 50000, false, true)));
			pGroup5->addChild(new ParameterData(new Parameter(	tr("Open position delta (unit)"),
																tr("Minimum open delta position"),
																pElHihatPedal->openPositionDelta, 0, 127)));
			pGroup5->addChild(new ParameterData(new Parameter(	tr("Close acceleration (unit/s2)"),
																tr("Under this acceleration the hi-hat is converted to close color"),
																pElHihatPedal->closeAcceleration, -50000, 0, true, false)));
			pGroup5->addChild(new ParameterData(new Parameter(	tr("Close position delta (unit)"),
																tr("Minimum close delta position"),
																pElHihatPedal->closePositionDelta, -127, 0)));
		}

		TreeNode* pGroup6 = pRoot->addChild(new GroupData(tr("Hi-hat half-open mode"), tr(""), pElHihatPedal->isHalfOpenModeEnabled));
		{
			pGroup6->addChild(new ParameterData(new Parameter(	tr("Half open maximum position (unit)"),
																tr("Half open detection algorithm starts between [Security close position] and this position"),
																pElHihatPedal->halfOpenMaximumPosition, 0, 127)));
			pGroup6->addChild(new ParameterData(new Parameter(	tr("Half open detection time (ms)"),
																tr("Afters the specified time (ms), if the hi-hat is still in close color, it goes in half open mode. It will leave this mode if the control position go back under [Security close position]"),
																pElHihatPedal->halfOpenActivationTime, 0, 5000)));
		}

		TreeNode* pGroup7 = pRoot->addChild(new GroupData(tr("Chameleon Crash conversion")));
		{
			pGroup7->addChild(new ParameterData(new Parameter(	tr("Time window (ms)"),
																tr("Timing window used to detect simultaneous hits between cymbals"),
																pSlot->cymbalSimHitWindow, 0, 100)));

			const QString& szDescription = tr("A Chameleon Crash change its crash color to hi-hat close color if one of these pads is hit at the same time");
			pGroup7->addChild(new ParameterData(new Parameter(tr("Crash"), szDescription, pSlot->isChameleonCrashWithCrash)));
			pGroup7->addChild(new ParameterData(new Parameter(tr("Ride"), szDescription, pSlot->isChameleonCrashWithRide)));
			pGroup7->addChild(new ParameterData(new Parameter(tr("Snare"), szDescription, pSlot->isChameleonCrashWithSnare)));
			pGroup7->addChild(new ParameterData(new Parameter(tr("Tom 2"), szDescription, pSlot->isChameleonCrashWithTom2)));
			pGroup7->addChild(new ParameterData(new Parameter(tr("Tom 3"), szDescription, pSlot->isChameleonCrashWithTom3)));
		}

		TreeNode* pGroup8 = pRoot->addChild(new GroupData(tr("Foot splash cancel from control speed"), tr("A hi-hat mask window is created if conditions below are met"), pElHihatPedal->isFootCancel));
		// TODO: pElHihatPedal->connectFootCancelActivated(boost::bind(&GraphSubWindow::onFootCancelActivated, _pGrapSubWindow, _1));
		{
			pGroup8->addChild(new ParameterData(new Parameter(	tr("Control speed (unit/s)"),
																tr("Under this speed the mask window can start if other conditions are met"),
																pElHihatPedal->footCancelClosingSpeed, HiHatPedalElement::MIN_FOOT_SPEED, 0)));
			pGroup8->addChild(new ParameterData(new Parameter(	tr("Maximum hi-hat position (unit)"),
																tr("Under this position the mask window can start if other conditions are met"),
																pElHihatPedal->footCancelPos, 0, 127)));
			pGroup8->addChild(new ParameterData(new Parameter(	tr("Control Position delta (unit)"),
																tr("The delta must be greater than the entered value to activate the mask window.\nThe delta position is between the beginning of the close movement (the green point on the curve) and the current position."),
																pElHihatPedal->footCancelPosDiff, 0, 127)));
			pGroup8->addChild(new ParameterData(new Parameter(	tr("Mask Time (ms)"),
																tr("Time length of the mask window (ms)"),
																pElHihatPedal->footCancelMaskTime, 0, 200)));
			// TODO: pElHihatPedal->connectFootCancelMaskTime(boost::bind(&GraphSubWindow::onFootCancelMaskTime, _pGrapSubWindow, _1));

			pGroup8->addChild(new ParameterData(new Parameter(	tr("Velocity to ignore (unit)"),
																tr("Height of the mask window. All hi-hat hits under this velocity are ignored"),
																pElHihatPedal->footCancelVelocity, 0, 127)));
			// TODO: pElHihatPedal->connectFootCancelVelocity(boost::bind(&GraphSubWindow::onFootCancelVelocity, _pGrapSubWindow, _1));
		}

		TreeNode* pGroup9 = pRoot->addChild(new GroupData(tr("Foot splash cancel after a pedal hit"), tr("A hi-hat mask window is created when a hi-hat pedal hit is detected"), pElHihatPedal->isFootCancelAfterPedalHit));
		// TODO: pElHihatPedal->connectFootCancelAfterPedalHitActivated(boost::bind(&GraphSubWindow::onFootCancelAfterPedalHitActivated, _pGrapSubWindow, _1));
		{
			pGroup9->addChild(new ParameterData(new Parameter(	tr("Mask time (ms)"),
																tr("Time length of the mask window (ms)"),
																pElHihatPedal->footCancelAfterPedalHitMaskTime, 0, 200)));
			// TODO: pElHihatPedal->connectFootCancelAfterPedalHitMaskTime(boost::bind(&GraphSubWindow::onFootCancelAfterPedalHitMaskTime, _pGrapSubWindow, _1));

			pGroup9->addChild(new ParameterData(new Parameter(	tr("Velocity to ignore (unit)"),
																tr("Height of the mask window. All hi-hat hits under this velocity are ignored"),
																pElHihatPedal->footCancelAfterPedalHitVelocity, 0, 127)));
			// TODO: pElHihatPedal->connectFootCancelAfterPedalHitVelocity(boost::bind(&GraphSubWindow::onFootCancelAfterPedalHitVelocity, _pGrapSubWindow, _1));
		}

		TreeNode* pGroup10 = pRoot->addChild(new GroupData(tr("Cancel while open"), tr(""), pElHihatPedal->isCancelHitWhileOpen));
		{
			pGroup10->addChild(new ParameterData(new Parameter(	tr("Control Position (unit)"),
																tr("If the current control pos is >= [Position] and hi-hat hit is < [Velocity] the hi-hat hit is ignored"),
																pElHihatPedal->cancelOpenHitThreshold, 0, 127)));
			pGroup10->addChild(new ParameterData(new Parameter(	tr("Velocity (unit)"),
																tr("If the current control pos is >= [Position] and hi-hat hit is < [Velocity] the hi-hat hit is ignored"),
																pElHihatPedal->cancelOpenHitVelocity, 0, 127)));
		}

		// Flams
		Pad::List::const_iterator it = pads.begin();
		while (it!=pads.end())
		{
			const Pad::Ptr& pPad = *(it++);
			if (pPad->type->get()==Pad::HIHAT_PEDAL)
			{
				continue;
			}
			TreeNode* pGroup = pRoot->addChild(new GroupData(pPad->getName(), tr(""), pPad->isFlamActivated));
			// TODO: pPad->getColor()

			pGroup->addChild(new ParameterData(new Parameter(	tr("Pad Type of the second hit"),
																tr("Specify the Pad type of the second hit of the flam"),
																pPad->typeFlam, 0, Pad::TYPE_COUNT, Pad::DICT_NAMES)));

			LinearFunction::Description::Ptr pFuncDesc(new LinearFunction::Description());
			pFuncDesc->szLabelX = tr("Time between 2 hits (ms)");
			pFuncDesc->szLabelY = tr("Velocity factor");
			pFuncDesc->xMin = 0;
			pFuncDesc->xMax = 150;
			pFuncDesc->yMin = 0;
			pFuncDesc->yMax = 2;
			pFuncDesc->x1Default = 0;
			pFuncDesc->x2Default = 30;
			pFuncDesc->y1Default = 1.0f;
			pFuncDesc->y2Default = 1.0f;
			pFuncDesc->yStep = 0.01f;
			pFuncDesc->aStep = 0.001f;
			pFuncDesc->bStep = 0.01f;
			pFuncDesc->aDecimals = 3;

			pGroup->addChild(new ParameterData(new Parameter(	tr("Parameters"),
																tr("List of linear functions used to determine when to convert a flam in 2 different colors"),
																pPad->funcFlams, pFuncDesc)));

			pGroup->addChild(new ParameterData(new Parameter(	tr("Flam cancel detection (ms)"),
																tr("If the previous hit before the flam is under this value the flam is cancelled"),
																pPad->flamCancelDuringRoll, 0, 250)));
		}

		TreeNode* pGroup11 = pRoot->addChild(new GroupData(tr("Ghost notes")));
		it = pads.begin();
		while (it!=pads.end())
		{
			const Pad::Ptr& pPad = *(it++);

			Parameter* pParameter = new Parameter(	pPad->getName(),
													tr("Under this velocity the note is ignored"),
													pPad->ghostVelocityLimit, 0, 127);
			pParameter->setColor(QColor(pPad->color->get()));

			pGroup11->addChild(new ParameterData(pParameter));
		}
	}

	_pProxyModel->setSourceModel(pSlot->model.get());

	_pProxyModel->setPadSettingsType(comboBoxPadSettingsType->currentIndex());
	_pProxyModel->invalidate();
	treeViewPadSettings->expandAll();
	treeViewPadSettings->resizeColumnToContents(0);
}
