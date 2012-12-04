Windows
=======

Visual Studio 2010 Express
--------------------------

1.	Build Qt

2.	Build Qwt

3.	Build Boost

4.	Define $(BOOST\_DIR) and $(QWT\_DIR)

5.	Build eProDrums
	
		qmake
		nmake

6.	To generate .vcxproj and .vcxproj.filters

		qmake -spec win32-msvc2010 -tp vc

Linux (debian/ubuntu)
=====================

1.	Get Qt

		apt-get install qt4-qmake qt4-dev-tools qt4-qtconfig

2.	Get Qwt

		-	Download Qwt source code from : http://qwt.sourceforge.net
		-	Edit qwtconfig.pri and change QWT_INSTALL_PREFIX like below

    			QWT_INSTALL_PREFIX    = /usr

		-	Build Qwt :
			
				qmake
				make
				sudo make install

3.	Get Boost

		apt-get install libboost1.50-all-dev

4.	Build eProDrums
	
		qmake
		make

5.	Run eProDrums

		./eProDrums
