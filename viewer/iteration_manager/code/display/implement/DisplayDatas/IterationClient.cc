#include <display/DisplayDatas/IterationClient.qo.h>
#include <display/DisplayDatas/IterationManager.qo.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <images/Images/ImageInterface.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <casa/Containers/Record.h>

namespace casa {
    namespace viewer {

	IterationClient::IterationClient( IterationManager *m, QtDisplayData *dd ) : mgr(m), data(dd), ia(0) {
	    mgr->registerClient(this);
	}

	IterationClient::~IterationClient( ) {
	    mgr->unregisterClient(this);
	}

	std::string IterationClient::name( ) const { return data->name( ); }
	std::string IterationClient::dataType( ) const { return data->dataType( ); }
	std::string IterationClient::displayType( ) const { return data->displayType( ); }
	void IterationClient::locateFrame( unsigned int movie_axis, IPosition &start ) {
	    mgr->locateFrame( movie_axis, start, this );
	}

	ImageAnalysis *IterationClient::imageAnalysis( ) {
	    if ( ia == 0 )
		ia = data->dd()->imageanalysis( );
	    return ia;
	}

	ImageInterface<float> *IterationClient::imageInterface( ) {
	    return data->dd()->imageinterface( );
	}

	CsysAxisInfo IterationClient::movieAxisInfo( unsigned int pix_axis ) const {
	    return data ? CsysAxisInfo(data->dd( ), pix_axis) : CsysAxisInfo( );
	}

    }
}

