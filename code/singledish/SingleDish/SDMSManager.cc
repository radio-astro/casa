#include <iostream>

//#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/ArrayMath.h>

#include <ms/MeasurementSets/MSSelectionTools.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <casa_sakura/SakuraUtils.h>
#include <singledish/SingleDish/SDMSManager.h>


#define _ORIGIN LogOrigin("SDMSManager", __func__, WHERE)

namespace casa {

SDMSManager::SDMSManager()
{
}

SDMSManager::SDMSManager(SDMSManager const &other)
{
}

SDMSManager &SDMSManager::operator=(SDMSManager const &other)
{
  return *this;
}

SDMSManager::~SDMSManager()
{
  LogIO os(_ORIGIN);
}

// -----------------------------------------------------------------------
// Fill output MS with data from an input VisBuffer
// -----------------------------------------------------------------------
void SDMSManager::fillCubeToOutputMs(vi::VisBuffer2 *vb,Cube<Float> const &data_cube)
{
	setupBufferTransformations(vb);

	// make sure the shape of cube matches the number of rows to add.
	AlwaysAssert(data_cube.nplane()==nRowsToAdd_p, AipsError);

	if (not bufferMode_p)
	{
		// Create RowRef object to fill new rows
		uInt currentRows = outputMs_p->nrow();
		RefRows rowRef( currentRows, currentRows + nRowsToAdd_p/nspws_p - 1);

		// Add new rows to output MS
		outputMs_p->addRow(nRowsToAdd_p,False);

		// Fill new rows
		fillCubeToDataCols(vb,rowRef,data_cube);
		fillIdCols(vb,rowRef);
	}

    return;
}


// ----------------------------------------------------------------------------------------
// Fill main (data) columns which have to be combined together to produce bigger SPWs
// ----------------------------------------------------------------------------------------
void SDMSManager::fillCubeToDataCols(vi::VisBuffer2 *vb,RefRows &rowRef,Cube<Float> const &data_cube)
{
	if (inputWeightSpectrumAvailable_p or usewtspectrum_p)
	{
		// Don't propagate auxiliary weight spectrum and use cumSum for average
		setWeightBasedTransformations(False,MSTransformations::cumSum);

		// Write WEIGHT_SPECTRUM and SIGMA_SPECTRUM
		transformCubeOfData(vb,rowRef,vb->weightSpectrum(),outputMsCols_p->weightSpectrum(),NULL,vb->weightSpectrum());

		// Go back to normal
		setWeightBasedTransformations(channelAverage_p,weightmode_p);

		// SIGMA_SPECTRUM is not correct in the general case of spectrum transformation
		if (spectrumTransformation_p)
		{
			transformCubeOfData(vb,rowRef,vb->sigmaSpectrum(),outputMsCols_p->sigmaSpectrum(),NULL,vb->weightSpectrum());
		}
		// Within AveragingTvi2 SIGMA is already re-defined to 1/sqrt(WEIGHT)
		else if (timeAverage_p)
		{
			transformCubeOfData(vb,rowRef,vb->sigmaSpectrum(),outputMsCols_p->sigmaSpectrum(),NULL,vb->weightSpectrum());
		}
		// When CORRECTED becomes DATA, then SIGMA has to be re-defined to 1/sqrt(WEIGHT)
		else if (correctedToData_p)
		{
			// VI/VB only allocates and populates sigmaSpectrum on request
			// But its contents are not usable for this case
			// So we should just create a local storage
			Cube<Float> sigmaSpectrum;
			sigmaSpectrum = vb->weightSpectrum(); // Copy constructor does not use reference semantics, but deep copy
			// Apply transformation
			arrayTransformInPlace(sigmaSpectrum, MSTransformations::wtToSigma);
			// Write resulting sigmaSpectrum
			transformCubeOfData(vb,rowRef,sigmaSpectrum,outputMsCols_p->sigmaSpectrum(),NULL,vb->weightSpectrum());
		}
		// Pure split operation
		else
		{
			transformCubeOfData(vb,rowRef,vb->sigmaSpectrum(),outputMsCols_p->sigmaSpectrum(),NULL,vb->weightSpectrum());
		}
	}

	ArrayColumn<Bool> *outputFlagCol=NULL;
	weightSpectrumFromSigmaFilled_p = False;
	for (dataColMap::iterator iter = dataColMap_p.begin();iter != dataColMap_p.end();iter++)
	{
		// Get applicable *_SPECTRUM (copy constructor uses reference semantics)
		// If channel average or combine, otherwise no need to copy
		const Cube<Float> applicableSpectrum = getApplicableSpectrum(vb,iter->first);

		// Apply transformations
		switch (iter->first)
		{
			case MS::DATA:
			{
				if (mainColumn_p == MS::DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}
				Cube<Complex> cdata_cube(data_cube.shape());
				convertArray(cdata_cube,data_cube);
				transformCubeOfData(vb,rowRef,cdata_cube,outputMsCols_p->data(), outputFlagCol,applicableSpectrum);

				break;
			}
			case MS::CORRECTED_DATA:
			{
				if (mainColumn_p == MS::CORRECTED_DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				Cube<Complex> cdata_cube(data_cube.shape());
				convertArray(cdata_cube,data_cube);
				if (iter->second == MS::DATA)
				{
					transformCubeOfData(vb,rowRef,cdata_cube,outputMsCols_p->data(), outputFlagCol,applicableSpectrum);
				}
				else
				{
					transformCubeOfData(vb,rowRef,cdata_cube,outputMsCols_p->correctedData(), outputFlagCol,applicableSpectrum);
				}

				break;
			}
			case MS::MODEL_DATA:
			{
				if (mainColumn_p == MS::MODEL_DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				if (iter->second == MS::DATA)
				{
					transformCubeOfData(vb,rowRef,vb->visCubeModel(),outputMsCols_p->data(), outputFlagCol,applicableSpectrum);
				}
				else
				{
					transformCubeOfData(vb,rowRef,vb->visCubeModel(),outputMsCols_p->modelData(), outputFlagCol,applicableSpectrum);
				}
				break;
			}
			case MS::FLOAT_DATA:
			{
				if (mainColumn_p == MS::FLOAT_DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				transformCubeOfData(vb,rowRef,data_cube,outputMsCols_p->floatData(), outputFlagCol,applicableSpectrum);

				break;
			}
			case MS::LAG_DATA:
			{
				// jagonzal: TODO
				break;
			}
			default:
			{
				// jagonzal: TODO
				break;
			}
		}
	}

    // Special case for flag category
    if (inputFlagCategoryAvailable_p)
    {
    	if (spectrumReshape_p)
    	{
    		IPosition transformedCubeShape = getShape(); //[nC,nF,nR]
    		IPosition inputFlagCategoryShape = vb->flagCategory().shape(); // [nC,nF,nCategories,nR]
    		IPosition flagCategoryShape(4,	inputFlagCategoryShape(1),
    										transformedCubeShape(2),
    										inputFlagCategoryShape(2),
    										transformedCubeShape(2));
    		Array<Bool> flagCategory(flagCategoryShape,False);

        	outputMsCols_p->flagCategory().putColumnCells(rowRef, flagCategory);
    	}
    	else
    	{
        	outputMsCols_p->flagCategory().putColumnCells(rowRef, vb->flagCategory());
    	}
    }

	return;
}

}  // End of casa namespace.
