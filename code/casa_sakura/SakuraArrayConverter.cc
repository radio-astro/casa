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
		Matrix<Complex> &output_spectrum_matrix) {
	try {
		Vector<Float> input_vector(2 * num_data);
		Int j = 0;
		for (Int i = 0; i < num_data; ++i) {
			if (i % 2 == 0) {
				input_vector[i] = input_data[j++];
			} else
				input_vector[i] = 0;
		}
		Array<Float> input_array;
		input_array.assign(input_vector);
		Array<Complex> complex_array(RealToComplex(input_array));
		Matrix<Complex> complex_matrix(2, num_data);
		complex_matrix.row(num_pol) = complex_array;
		output_spectrum_matrix.resize(2, num_data, False);
		output_spectrum_matrix.row(num_pol) = complex_matrix.row(num_pol);
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
		Matrix<Float> &output_spectrum_matrix) {
	try {
		Matrix<Float> float_matrix(2, num_data);
		for (Int i = 0; i < num_data; ++i) {
			float_matrix(num_pol, i) = input_data[i];
		}
		output_spectrum_matrix.resize(2, num_data, False);
		output_spectrum_matrix.row(num_pol) = float_matrix.row(num_pol);
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
