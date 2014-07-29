#ifndef QTRASTERDATA_H_
#define QTRASTERDATA_H_
 
#include <qwt_raster_data.h>
#include <cstdio>

class QwtPlotSpectrogram;

namespace casa { 
/* -------------------------------------------
 * QtRasterData derived from QwtRasterData
 * Saves the values for the plot
 * -------------------------------------------
*/
 
    class QtRasterData: public QwtRasterData {
	private:
	    static int output_limit; 

	    double * m_Array;
	    double m_minValue;
	    double m_maxValue;
 
	    struct structMinMax {
		double min;
		double max;
	    };

	    struct structXY {
		double x;
	      double y;
	    };

	    structMinMax m_RangeX;
	    structMinMax m_RangeY;

	    structXY m_DataSize;
	    QwtDoubleRect bounding_box;
	    structXY m_RealToArray;

	    void setData( const double *array, int sizex, int sizey, double min, double max );
	    void fillAndFindMinMax( const QList<double> &array, double &min, double &max );
	    int length_;
 
	    QwtPlotSpectrogram *spect;

	public:
	    // Constructor giving back the QwtRasterData Constructor
	    QtRasterData( QwtPlotSpectrogram *s ) : m_Array(0), spect(s) { }
 
	    ~QtRasterData( ) { if ( m_Array ) delete [] m_Array; }

	    void initRaster( const QwtDoubleRect &r, const QSize &raster );
	    QwtRasterData *copy( ) const;
	    double value(double x, double y) const;
	    void setData(const QList<double> &array, int sizex, int sizey);
	    QwtDoubleInterval range() const;

	    void setBoundingRect( const QwtDoubleRect &rect );

	    void setRangeX(const double min, const double max) {
		m_RangeX.min = min;
		m_RangeX.max = max;
	    }
 
	    void setRangeY(const double min, const double max) {
		m_RangeY.min = min;
		m_RangeY.max = max;
	    }
 
	    int ArrPos(const int x, const int y) const {
		int result = y + m_DataSize.y * x;
		/* if ( output_limit < 20 ) fprintf( stderr, "\t\t\t\t (%f,%f) (%d, %d) => %d\n", m_DataSize.x, m_DataSize.y, x, y, result); */
		return result > length_ ? 0 : result;
	    }
    };
}

#endif
