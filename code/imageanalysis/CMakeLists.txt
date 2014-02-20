# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
#
# This file is part of CASA.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

casa_add_library( imageanalysis

Annotations/AnnAnnulus.cc
Annotations/AnnCenterBox.cc
Annotations/AnnCircle.cc
Annotations/AnnEllipse.cc
Annotations/AnnLine.cc
Annotations/AnnotationBase.cc
Annotations/AnnPolygon.cc
Annotations/AnnPolyline.cc
Annotations/AnnRectBox.cc
Annotations/AnnRegion.cc
Annotations/AnnRotBox.cc
Annotations/AnnSymbol.cc
Annotations/AnnText.cc
Annotations/AnnVector.cc
Annotations/RegionTextList.cc

ImageAnalysis/AntennaResponses.cc
ImageAnalysis/ComponentImager.cc
ImageAnalysis/ImageAnalysis.cc
ImageAnalysis/ImageCollapserData.cc
ImageAnalysis/ImageFactory2.cc
ImageAnalysis/ImageFFT.cc
ImageAnalysis/ImageFitter.cc
ImageAnalysis/ImageInputProcessor.cc
ImageAnalysis/ImageMetaData.cc
ImageAnalysis/ImageMetaDataBase.cc
ImageAnalysis/ImageMetaDataRW.cc
ImageAnalysis/ImageMomentsProgress.cc
ImageAnalysis/ImageMomentsProgressMonitor.cc
ImageAnalysis/ImagePadder.cc
ImageAnalysis/ImagePolarimetry.cc
ImageAnalysis/ImagePolProxy.cc
ImageAnalysis/ImagePrimaryBeamCorrector.cc
ImageAnalysis/ImageProfileFitter.cc
ImageAnalysis/ImageRegridder.cc
ImageAnalysis/ImageStatsCalculator.cc
ImageAnalysis/ImageTransposer.cc
ImageAnalysis/PeakIntensityFluxDensityConverter.cc
ImageAnalysis/ProfileFitResults.cc
ImageAnalysis/PVGenerator.cc
ImageAnalysis/SpectralCollapser.cc
ImageAnalysis/SpectralFitter.cc

IO/RegionTextParser.cc
IO/AsciiAnnotationFileLine.cc
IO/FitterEstimatesFileParser.cc
IO/ImageProfileFitterResults.cc
IO/LogFile.cc
IO/OutputDestinationChecker.cc
IO/ProfileFitterEstimatesFileParser.cc

Regions/CasacRegionManager.cc
)

install (FILES
    ImageTypedefs.h
     DESTINATION include/casacode/imageanalysis
    )

install (FILES
	Annotations/AnnAnnulus.h
	Annotations/AnnCenterBox.h
	Annotations/AnnCircle.h
	Annotations/AnnEllipse.h
	Annotations/AnnLine.h
	Annotations/AnnPolygon.h
	Annotations/AnnPolyline.h
	Annotations/AnnRectBox.h
	Annotations/AnnRegion.h
	Annotations/AnnRotBox.h
	Annotations/AnnSymbol.h
	Annotations/AnnText.h
	Annotations/AnnVector.h
	Annotations/AnnotationBase.h
	Annotations/RegionTextList.h
	DESTINATION include/casacode/imageanalysis/Annotations
	)
install (FILES
	IO/AsciiAnnotationFileLine.h
	IO/FitterEstimatesFileParser.h
	IO/LogFile.h
    IO/OutputDestinationChecker.h
    IO/ImageProfileFitterResults.h
	IO/ProfileFitterEstimatesFileParser.h
	IO/RegionTextParser.h
	DESTINATION include/casacode/imageanalysis/IO
	)
install (FILES
	ImageAnalysis/AntennaResponses.h
	ImageAnalysis/ComponentImager.h
	ImageAnalysis/Image1DSmoother.h
	ImageAnalysis/Image1DSmoother.tcc
	ImageAnalysis/Image2DConvolver.h
	ImageAnalysis/Image2DConvolver.tcc
	ImageAnalysis/ImageAnalysis.h
	ImageAnalysis/ImageAnalysis2.tcc
    ImageAnalysis/ImageBoxcarSmoother.h
    ImageAnalysis/ImageBoxcarSmoother.tcc
	ImageAnalysis/ImageCollapser.h
	ImageAnalysis/ImageCollapser.tcc
	ImageAnalysis/ImageCollapserData.h
	ImageAnalysis/ImageConvolver.h
	ImageAnalysis/ImageConvolver.tcc
	ImageAnalysis/ImageCropper.h
	ImageAnalysis/ImageCropper.tcc
	ImageAnalysis/ImageDecimator.h
    ImageAnalysis/ImageDecimator.tcc
    ImageAnalysis/ImageDecimatorData.h
	ImageAnalysis/ImageDecomposer.h
	ImageAnalysis/ImageDecomposer.tcc
	ImageAnalysis/ImageFactory.h
	ImageAnalysis/ImageFactory.tcc
	ImageAnalysis/ImageFFT.h
	ImageAnalysis/ImageFFTer.h
	ImageAnalysis/ImageFFTer.tcc
	ImageAnalysis/ImageFitter.h
	ImageAnalysis/ImageFit1D.h
	ImageAnalysis/ImageFit1D.tcc
    ImageAnalysis/ImageHanningSmoother.h
    ImageAnalysis/ImageHanningSmoother.tcc
    ImageAnalysis/ImageHistograms.h
    ImageAnalysis/ImageHistograms.tcc
    ImageAnalysis/ImageHistory.h
    ImageAnalysis/ImageHistory.tcc
	ImageAnalysis/ImageInputProcessor.h
	ImageAnalysis/ImageInputProcessor2.tcc
	ImageAnalysis/ImageMaskAttacher.h
    ImageAnalysis/ImageMaskAttacher.tcc
    ImageAnalysis/ImageMaskedPixelReplacer.h
    ImageAnalysis/ImageMaskedPixelReplacer.tcc
	ImageAnalysis/ImageMetaData.h
    ImageAnalysis/ImageMetaDataBase.h
    ImageAnalysis/ImageMetaDataRW.h
	ImageAnalysis/ImageMoments.h
	ImageAnalysis/ImageMoments.tcc
    ImageAnalysis/ImageMomentsProgress.h
    ImageAnalysis/ImageMomentsProgressMonitor.h
	ImageAnalysis/ImagePadder.h
	ImageAnalysis/ImagePolarimetry.h
	ImageAnalysis/ImagePolProxy.h
	ImageAnalysis/ImagePrimaryBeamCorrector.h
	ImageAnalysis/ImageProfileFitter.h
    ImageAnalysis/ImageRegridder.h
	ImageAnalysis/ImageSourceFinder.h
	ImageAnalysis/ImageSourceFinder.tcc
	ImageAnalysis/ImageStatsCalculator.h
	ImageAnalysis/ImageTask.h
    ImageAnalysis/ImageTask.tcc
	ImageAnalysis/ImageTransposer.h
	ImageAnalysis/ImageTwoPtCorr.h
	ImageAnalysis/ImageTwoPtCorr.tcc
	ImageAnalysis/MomentsBase.h
	ImageAnalysis/MomentsBase.tcc
	ImageAnalysis/MomentCalculator.h
	ImageAnalysis/MomentCalculator.tcc
	ImageAnalysis/PeakIntensityFluxDensityConverter.h
	ImageAnalysis/ProfileFitResults.h
	ImageAnalysis/PVGenerator.h
	ImageAnalysis/SepImageConvolver.h
	ImageAnalysis/SepImageConvolver.tcc
	ImageAnalysis/SpectralCollapser.h
	ImageAnalysis/SubImageFactory.h
	ImageAnalysis/SubImageFactory.tcc
	DESTINATION include/casacode/imageanalysis/ImageAnalysis
	)
install (FILES
	Regions/CasacRegionManager.h
	DESTINATION include/casacode/imageanalysis/Regions
	)

casa_add_executable(imageanalysis imfit apps/imfit.cc )
casa_add_executable(imageanalysis imcollapse apps/imcollapse.cc )

casa_add_assay( imageanalysis Annotations/test/tAnnAnnulus.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnCenterBox.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnCircle.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnEllipse.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnLine.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnPolygon.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnRectBox.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnRotBox.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnSymbol.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnText.cc )
casa_add_assay( imageanalysis Annotations/test/tAnnVector.cc )
casa_add_assay( imageanalysis Annotations/test/tRegionTextList.cc )

casa_add_assay( imageanalysis ImageAnalysis/test/dImageHistograms.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/dImageMoments.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tAntennaResponses.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tComponentImager.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImage2DConvolver.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageAnalysis.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageAnalysis2.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageCollapser.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageConvolver.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageDecomposer.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageFit1D.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageFFT.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageFitter.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageMetaData.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageMoments.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImagePolarimetry.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageSourceFinder.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageStatsCalculator.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageInputProcessor.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImagePrimaryBeamCorrector.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageProfileFitter.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tImageTransposer.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tSepImageConvolver.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tSpectralFitter.cc )
casa_add_assay( imageanalysis ImageAnalysis/test/tSubImageFactory.cc )

casa_add_assay( imageanalysis IO/test/tFitterEstimatesFileParser.cc )
casa_add_assay( imageanalysis IO/test/tProfileFitterEstimatesFileParser.cc )
casa_add_assay( imageanalysis IO/test/tRegionTextParser.cc )

casa_add_assay( imageanalysis Regions/test/tCasacRegionManager.cc )

