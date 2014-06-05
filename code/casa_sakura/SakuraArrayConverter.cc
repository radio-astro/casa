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

void SakuraArrayConverter::ConvertComplexMatrixToSakura(Int const num_pol,
		Int const num_data, Matrix<Complex> &spectrum_matrix,
		float output_data[]) {
	try {
		Vector<Float> real_vector(casa::real(spectrum_matrix.row(num_pol)));
		for (Int i = 0; i < num_data; ++i) {
			output_data[i] = real_vector[i];
		}
	} catch (AipsError x) {
		LogIO logger(LogOrigin("SakuraArrayConverter", __func__, WHERE));
		logger << _ORIGIN;
		logger << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
		RETHROW(x);
	}
}

void SakuraArrayConverter::ConvertFloatMatrixToSakura(Int const num_pol,
		Int const num_data, Matrix<Float> &spectrum_matrix,
		float output_data[]) {
	try {
		for (Int i = 0; i < num_data; ++i) {
			output_data[i] = spectrum_matrix.row(num_pol)[i];
		}
	} catch (AipsError x) {
		LogIO logger(LogOrigin("SakuraArrayConverter", __func__, WHERE));
		logger << _ORIGIN;
		logger << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
		RETHROW(x);
	}
}

void SakuraArrayConverter::ConvertSakuraToComplexMatrix(Int const num_pol,
		Int const num_data, float input_data[],
		Matrix<Complex> &output_complex_matrix) {
	try {
		Bool deleteOut;
		Complex* pout = output_complex_matrix.getStorage(deleteOut);
		for (Int i = 0; i < num_data; ++i) {
			casa::real(pout[output_complex_matrix.nrow() * i + num_pol]) = input_data[i];
			casa::imag(pout[output_complex_matrix.nrow() * i + num_pol]) = 0.0;
		}
		output_complex_matrix.putStorage(pout, deleteOut);
	} catch (AipsError x) {
		LogIO logger(LogOrigin("SakuraArrayConverter", __func__, WHERE));
		logger << _ORIGIN;
		logger << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
		RETHROW(x);
	}
}

void SakuraArrayConverter::ConvertSakuraToFloatMatrix(Int const num_pol,
		Int const num_data, float input_data[],
		Matrix<Float> &output_float_matrix) {
	try {
		Bool deleteOut;
		Float* pout = output_float_matrix.getStorage(deleteOut);
		for (Int i = 0; i < num_data; ++i) {
			pout[output_float_matrix.nrow() * i + num_pol] = input_data[i];
		}
		output_float_matrix.putStorage(pout, deleteOut);
	} catch (AipsError x) {
		LogIO logger(LogOrigin("SakuraArrayConverter", __func__, WHERE));
		logger << _ORIGIN;
		logger << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
		RETHROW(x);
	}
}

}
 // End of casa namespace.
