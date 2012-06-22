
#include <casaqt/QtPlotServer/QtRasterData.h>
#include <qwt_plot_spectrogram.h>
#include <limits>

namespace casa {

    int QtRasterData::output_limit = 0;

    QwtRasterData *QtRasterData::copy( ) const {
	// if ( output_limit < 20 ) fprintf( stderr, "in copy( )...\n" );
	QtRasterData *clone = new QtRasterData(spect);
	clone->setRangeX(m_RangeX.min, m_RangeX.max);
	clone->setRangeY(m_RangeY.min, m_RangeY.max);
	clone->setBoundingRect(bounding_box);
// 	clone->setBoundingRect(QwtDoubleRect(m_RangeX.min, m_RangeY.min, m_RangeX.max, m_RangeY.max));
	clone->setData(m_Array, m_DataSize.x, m_DataSize.y, m_minValue, m_maxValue);
	clone->length_ = length_;
	// if ( output_limit < 20 ) fprintf( stderr, "in copy( ): %f,%f\n", m_RealToArray.x, m_RealToArray.y );
	// if ( output_limit < 20 ) fprintf( stderr, "in copy( ): %f,%f\n", clone->m_RealToArray.x, clone->m_RealToArray.y );
	return clone;
    }


    void QtRasterData:: initRaster( const QwtDoubleRect &r, const QSize &raster ) {
	bounding_box = r;
	m_RealToArray.x = (bounding_box.right() - bounding_box.left()) / (m_DataSize.x - 1);
	m_RealToArray.y = (bounding_box.bottom() - bounding_box.top()) / (m_DataSize.y - 1);
	if ( output_limit < 20 ) {
	      // fprintf( stderr, "%d ---------------------> [(%f,%f), (%f,%f)] [%d,%d]\n", output_limit,
	      // 	       r.left(), r.right(), r.bottom(), r.top(), raster.width(), raster.height() );
	}
    }

    double QtRasterData::value(double x, double y) const {

	if ( x < bounding_box.left() || x > bounding_box.right() ||
	     y > bounding_box.bottom( ) || y < bounding_box.top( ) ) {
	    if ( output_limit < 20 ) {
	      // fprintf( stderr, "%d =====================> [(%f,%f), (%f,%f)]: (%f,%f)\n", output_limit,
	      // 	       bounding_box.left(), bounding_box.right(), bounding_box.bottom(), bounding_box.top(), x, y );
	      QwtDoubleRect plot_box(spect->boundingRect( ));
	      // fprintf( stderr, "%d =====================> [(%f,%f), (%f,%f)]\n", output_limit,
	      // 	       plot_box.left(), plot_box.right(), plot_box.bottom(), plot_box.top() );
	    }
	}

	++output_limit;

	// if ( output_limit < 20 ) fprintf( stderr, "in value(#1): %f, %f\n", m_RealToArray.x, m_RealToArray.y );
	// if ( output_limit < 20 ) fprintf( stderr, "in value(#1): %f, %f\n", x, y );
 	int xpos = (int)((x - m_RangeX.min) / m_RealToArray.x);
	// if ( output_limit < 20 ) fprintf( stderr, "in value(#2)...\n" );
	int ypos = (int)((y - m_RangeY.min) / m_RealToArray.y);
	// if ( output_limit < 20 ) fprintf( stderr, "in value(#3): %d,%d\n", xpos, ypos );
	int pos = ArrPos(xpos, ypos);
	// if ( output_limit < 20 ) fprintf( stderr, "in value(0x%x): %d\n",m_Array,pos );
	double dvalue = m_Array[pos];
	// if ( output_limit < 20 ) fprintf( stderr, "in value(#5)...\n" );
// 	return dvalue > 0 ? 0 : dvalue;
	return dvalue;
    }

    void QtRasterData::setBoundingRect( const QwtDoubleRect &rect ) {
	bounding_box = rect;
	// if ( output_limit < 20) fprintf( stderr, "setBoundingRect( ): (%f, %f) (%f, %f)\n", rect.left(), rect.right(), rect.top(), rect.bottom() );
	QwtRasterData::setBoundingRect( rect );
    }

    void QtRasterData::setData( const QList<double> &array, int sizex, int sizey) {

	if ( array.size( ) != sizex * sizey ) {
	    throw "internal error, data size does not match array dimensions";
	}

	m_DataSize.x = sizex;
	m_DataSize.y = sizey;

	if (m_Array != NULL)
	    delete [] m_Array;

	m_Array = new double [sizex * sizey];
	fillAndFindMinMax(array, m_minValue, m_maxValue);
// 	FILE *out = fopen("/tmp/vecdump.txt","w");
// 	fprintf( out, "%d\t%d\n", m_minValue, m_maxValue );
// 	for (int X=0; X < sizex * sizey; ++X) {
// 	  fprintf( out, "%f\n", m_Array[X] );
// 	}
// 	fclose(out);

	length_ = sizex * sizey;
	m_RealToArray.x = (bounding_box.right() - bounding_box.left()) / (m_DataSize.x - 1);
	m_RealToArray.y = (bounding_box.bottom() - bounding_box.top()) / (m_DataSize.y - 1);
    }

    void QtRasterData::setData( const double *array, int sizex, int sizey, double min, double max ) {

	// if ( output_limit < 20 ) fprintf( stderr, "in setData( )...\n" );

	m_DataSize.x = sizex;
	m_DataSize.y = sizey;
	m_minValue = min;
	m_maxValue = max;

	if (m_Array != NULL)
	    delete [] m_Array;

	int size = sizex * sizey;
	// if ( output_limit < 20 ) fprintf( stderr, "in setData( ): 0x%x\n", m_Array );
	m_Array = new double [size];
	// if ( output_limit < 20 ) fprintf( stderr, "in setData( ): 0x%x\n", m_Array );
	// if ( output_limit < 20 ) fprintf( stderr, "in setData( ): %d, %d\n", sizex, sizey );
	memcpy(m_Array, array, size * sizeof(double));
	// if ( output_limit < 20 ) fprintf( stderr, "in setData( ): %f\n", m_Array[0] );

	m_RealToArray.x = (bounding_box.right() - bounding_box.left()) / (m_DataSize.x - 1);
	m_RealToArray.y = (bounding_box.bottom() - bounding_box.top()) / (m_DataSize.y - 1);
    }

    QwtDoubleInterval QtRasterData::range() const {
	return QwtDoubleInterval(m_minValue, m_maxValue);
    }

    void QtRasterData::fillAndFindMinMax( const QList<double> &array, double &min, double &max ) {
	min = std::numeric_limits<double>::max( );
	max = std::numeric_limits<double>::min( );
	for ( int x=0; x < array.size( ); ++x ) {
	    m_Array[x] = array[x];
	    if ( array[x] > max )
		max = array[x];
	    if ( array[x] < min )
		min = array[x];
	}
    }

}
