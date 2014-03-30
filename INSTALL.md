Windows
=======

Visual Studio 2010 Express
--------------------------

1.	Get the last portmidi library here (Windows section): http://sourceforge.net/p/portmedia/wiki/portmidi
	
		-	Build the portmidi.dll (read first portmidi\pm_win\README_WIN.txt)
				
			-	Download and install JDK http://www.oracle.com/technetwork/java/javase/downloads/index.html
			-	Set the environment variable JAVA_HOME to the jdk installation path
			-	Download and install CMake
			-	Run CMake and set "Where is the source code" to the portmidi directory
			-	Set "Where is to build the binaries" to a directory of your choice
			-	Click on "Configure" and select your Visual Studio version
			-	Click on "Generate", a portmidi.sln file is generated
			-	Open portmidi.sln
			-	Select "Release" and build the solution
			
		-	Copy manually pm_common/portmidi.h and porttime/porttime.h files into portmidi/include (does not exist)

2.	Download and install "Qt 5.2.1 for Windows 32-bit (VS 2010, OpenGL, 517 MB)" : http://qt-project.org/downloads
		
		-	Warning wroblem with boost and Qt (https://bugreports.qt-project.org/browse/QTBUG-29331)
		-	Open mkspecs\win32-msvc2010\qmake.conf and change

				QMAKE_COMPILER_DEFINES += _MSC_VER=1600 _WIN32
			to 
				QMAKE_COMPILER_DEFINES += _WIN32

3.	Get and build Qwt 6.1.0 library here : http://qwt.sourceforge.net
		
		-	Open qwtconfig.pri and change QWT_INSTALL_PREFIX in the win32 section to a folder of your choice
		-	Build Qwt
			
				qmake
				nmake
				nmake install

4.	Get and build Boost 1.55.0 library here : http://www.boost.org

5.	Define PORTMIDI\_DIR QWT\_DIR and BOOST\_DIR variable in the file eProDrums.pro (win32 section)

6.	Build eProDrums
	
		qmake
		nmake release

Note:	To generate .vcxproj and .vcxproj.filters

		qmake -spec win32-msvc2010 -tp vc

Linux (debian/ubuntu/raspbian)
==============================

1.	Get portmidi

		sudo apt-get install libportmidi-dev

2.	Get Qt 5.2.1

3.	Get Qwt 6.1.0 from sources (at this time 27/01/2013, there is no Qwt6 dev package)

		-	Download Qwt source code from : http://qwt.sourceforge.net
		-	Edit qwtconfig.pri and change QWT_INSTALL_PREFIX like below

    			QWT_INSTALL_PREFIX    = /usr

		-	Build Qwt :
			
				qmake
				make
				sudo make install

4.	Get Boost

		sudo apt-get install libboost1.50-all-dev

5.	Build eProDrums
	
		qmake
		make
		sudo make install

6.	Run eProDrums

		eProDrums
