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

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>

int main(int argc, char** argv)
{
	int programResult = EXIT_FAILURE;

	// Program options definition
	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "This help message")
		;

	// Program options parsing
	po::variables_map vm;
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Option error : " << e.what() << std::endl;
		std::cerr << desc << std::endl;
		return programResult;
	}

	// Qt initialization
	QApplication theApp(argc, argv);
	QApplication::setOrganizationName("FromtonRouge");	// Same in .nsi file (WIN32)
	QApplication::setApplicationName("eProDrums");		// Same in .nsi file (WIN32)
	QApplication::setApplicationVersion("dev");

	// Program option help
	if (vm.count("help"))
	{
		std::stringstream str;
		str << desc << std::endl;
		QMessageBox::information(NULL, "Allowed options", str.str().c_str());
		return programResult;
	}

	MainWindow mainWindow;
	mainWindow.show();

	return theApp.exec();
}
