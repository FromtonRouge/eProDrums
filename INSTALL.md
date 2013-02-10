Windows
=======

Visual Studio 2010 Express
--------------------------

1.	Get the last portmidi library here (Windows section): http://sourceforge.net/apps/trac/portmedia/wiki/portmidi
	
		-	Build the portmidi.dll
		-	Move manually portmidi.h and porttime.h files into an include directory of your choice

2.	Get and build Qt 4.8 opensource library here : http://qt-project.org/downloads

		-	Due to a compatibility issue with boost you have to modify the file qt-everywhere-opensource-src-4.8.x\mkspecs\win32-msvc2010\qmake.config and change the following line

		QMAKE_CFLAGS            = -nologo -Zm200 -Zc:wchar_t-

		to 

		QMAKE_CFLAGS            = -nologo -Zm200 -Zc:wchar_t

		-	Build Qt

3.	Get and build the last Qwt library here : http://qwt.sourceforge.net

4.	Get and build the last Boost library here : http://www.boost.org

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

2.	Get Qt

		sudo apt-get install qt4-qmake qt4-dev-tools qt4-qtconfig

3.	Get Qwt from sources (at this time 27/01/2013, there is no Qwt6 dev package)

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
