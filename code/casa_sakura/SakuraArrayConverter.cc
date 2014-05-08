#include <iostream>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>
#include <tr1/memory>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>

#include <casa_sakura/SakuraUtils.h>
#include <casa_sakura/SakuraArrayConverter.h>

// AIPS++
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Containers/RecordField.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/Block.h>
#include <casa/Quanta/MVTime.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSPointing.h>

#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableRow.h>

#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/ScalarQuantColumn.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>

#define _ORIGIN LogOrigin("Sakura", __func__, WHERE)

namespace casa {

SakuraArrayConverter::SakuraArrayConverter() :
		logger_() {
	try {
		logger_ << _ORIGIN;
	} catch (AipsError x) {
		logger_ << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}
;

SakuraArrayConverter::~SakuraArrayConverter() {
}
;

void SakuraArrayConverter::ConvertComplexMatrixToSakura(Int const num_pol,
		Int const num_data, Matrix<Complex> &spectrum_matrix,
		float output_data[]) {
	try {
		logger_ << _ORIGIN;
		Vector<Float> real_vector(casa::real(spectrum_matrix.row(num_pol)));
		for (Int i = 0; i < num_data; ++i) {
			output_data[i] = real_vector[i];
		}
	} catch (AipsError x) {
		logger_ << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

void SakuraArrayConverter::ConvertFloatMatrixToSakura(Int const num_pol,
		Int const num_data, Matrix<Float> &spectrum_matrix,
		float output_data[]) {
	try {
		logger_ << _ORIGIN;
		for (Int i = 0; i < num_data; ++i) {
			output_data[i] = spectrum_matrix.row(num_pol)[i];
		}
	} catch (AipsError x) {
		logger_ << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

}  // End of casa namespace.
