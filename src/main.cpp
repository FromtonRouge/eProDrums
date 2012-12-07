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

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include <iostream>

int main(int argc, char** argv)
{
	int programResult = EXIT_FAILURE;
	QApplication theApp(argc, argv);

	try
    {
        MainWindow mainWindow;
        mainWindow.show();
        programResult = theApp.exec();
    }
	catch (const std::exception& e)
	{
		QMessageBox::critical(NULL, "Error while starting application", e.what());
	}

	return programResult;
}
