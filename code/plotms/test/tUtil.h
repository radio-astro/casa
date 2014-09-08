//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#include <casa/OS/EnvVar.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <QApplication>
#include <QDebug>
class tUtil {
public:
	static String getFullPath( String fileName ){
		//Path for data
		String aipsPath = EnvironmentVariable::get("CASAPATH");
		if (aipsPath.empty()) {
			aipsPath = EnvironmentVariable::get("AIPSPATH");
		}
		String dataPath;
		qDebug() << "Aips path="<<aipsPath.c_str();
		if ( !aipsPath.empty() ){
			//Last part of path returned appears to be a space rather
			//than a slash.  "/casa/trunk linux_64b"
			int spaceIndex = aipsPath.find( " ");
			if ( spaceIndex > 0 ){
				dataPath = aipsPath.before( spaceIndex );
			}
			else {
				dataPath = aipsPath;
			}
			dataPath.append( "/data/regression/unittest/plotms/"  );
			qDebug() << "Datapath="<<dataPath.c_str();
			dataPath.append( fileName );
		}
		return dataPath;
	}

	static void updatePlot( PlotMSApp* app ){
		PlotMSPlotManager& manager = app->getPlotManager();
		int plotCount = manager.numPlots();
		if ( plotCount > 0 ){
		    PlotMSPlot* currentPlot = manager.plot( 0 );
		    PlotMSPlotParameters& params = currentPlot->parameters();
		    currentPlot->parametersHaveChanged(params,PlotMSWatchedParameters::ALL_UPDATE_FLAGS());
		}
	}

	static int clearFile( const String& fileName ){
		int result = remove( fileName.c_str());
		if ( result == 0 ){
			qDebug() << "File "<<fileName.c_str()<<" was deleted in preparation for test.";
		}
		return result;
	}

	static int exitMain( bool showGui ){
		int exitCode = 1;
		if ( showGui ){
		    QApplication::setQuitOnLastWindowClosed(false);
		    exitCode = QApplication::exec();
		}
		return exitCode;
	}

	static bool checkFile( String fileName, int minBytes, int maxBytes, int digest ){
		bool fileOK = true;
		ifstream ifile( fileName.c_str() );
		if ( ! ifile ){
			cerr<< "FAIL output file did not exist!"<<endl;
			fileOK = false;
		}
		else {
			struct stat filestatus;
			stat( fileName.c_str(), &filestatus );
			qDebug() << "Output file size is "<<filestatus.st_size;
			if ( filestatus.st_size < minBytes ){
				qDebug() << "FAIL output file size was too small min="<<minBytes;
				fileOK = false;
			}
			else if ( filestatus.st_size > maxBytes ){
				qDebug() << "FAIL output file size was too large max="<<maxBytes;
				fileOK = false;
			}
			else if ( digest > 0 ){
			/*
				        if(self.plotfile_hash):
				            self.assertEqual(
				                sha.new(open(self.plotfile_jpg, 'r').read()).hexdigest(),
				                self.plotfile_hash
				            )
				        else:
				            # store to check against following test results
				            self.plotfile_hash = sha.new(open(self.plotfile_jpg, 'r').read()).hexdigest()
			*/
			}
		}
		return fileOK;
	}
private:
	tUtil(){};
	~tUtil();
};


