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

casa_add_library( components
    ComponentModels/ComponentList.cc
    ComponentModels/ComponentShape.cc
    ComponentModels/ComponentType2.cc
    ComponentModels/ComponentType.cc
    ComponentModels/ConstantSpectrum.cc
    ComponentModels/DiskShape.cc
    ComponentModels/FluxCalcLogFreqPolynomial.cc
    ComponentModels/FluxCalcQS.cc
    ComponentModels/FluxCalc_SS_JPL_Butler.cc
    ComponentModels/FluxCalcVQS.cc
    ComponentModels/FluxStandard.cc
    ComponentModels/FluxStdsQS2.cc
    ComponentModels/FluxStdsQS.cc
    ComponentModels/FluxStdSrcs.cc
    ComponentModels/GaussianDeconvolver.cc
    ComponentModels/GaussianShape.cc
    ComponentModels/LimbDarkenedDiskShape.cc
    ComponentModels/PointShape.cc
    ComponentModels/SkyCompBase.cc
    ComponentModels/SkyComponent.cc
    ComponentModels/SkyComponentFactory.cc
    ComponentModels/SkyCompRep.cc
    ComponentModels/SpectralIndex.cc
    ComponentModels/SpectralModel.cc
    ComponentModels/TabularSpectrum.cc
    ComponentModels/TwoSidedShape.cc
    
    SpectralComponents/CompiledSpectralElement.cc
    SpectralComponents/GaussianMultipletSpectralElement.cc
    SpectralComponents/GaussianSpectralElement.cc
    SpectralComponents/LogTransformedPolynomialSpectralElement.cc
    SpectralComponents/LorentzianSpectralElement.cc
    SpectralComponents/PCFSpectralElement.cc
    SpectralComponents/PolynomialSpectralElement.cc
    SpectralComponents/PowerLogPolynomialSpectralElement.cc
    SpectralComponents/SpectralElement2.cc
    SpectralComponents/SpectralElement.cc
    SpectralComponents/SpectralElementFactory.cc
    SpectralComponents/SpectralEstimate.cc
    SpectralComponents/SpectralFit.cc
    SpectralComponents/SpectralList.cc
    SpectralComponents/SpectralListFactory.cc
)
install (FILES 
    ComponentModels/ComponentList.h
    ComponentModels/ComponentShape.h
    ComponentModels/ComponentType.h
    ComponentModels/ConstantSpectrum.h
    ComponentModels/DiskShape.h
    ComponentModels/FluxCalcLogFreqPolynomial.h
    ComponentModels/FluxCalcLogFreqPolynomial.tcc
    ComponentModels/FluxCalcQS.h
    ComponentModels/FluxCalc_SS_JPL_Butler.h
    ComponentModels/FluxCalcVQS.h
    ComponentModels/Flux.h
    ComponentModels/Flux.tcc
    ComponentModels/FluxStandard.h
    ComponentModels/FluxStdsQS2.h
    ComponentModels/FluxStdsQS.h
    ComponentModels/FluxStdSrcs.h
    ComponentModels/GaussianDeconvolver.h
    ComponentModels/GaussianShape.h
    ComponentModels/LimbDarkenedDiskShape.h
    ComponentModels/PointShape.h
    ComponentModels/SkyCompBase.h
    ComponentModels/SkyComponent.h
    ComponentModels/SkyComponentFactory.h
    ComponentModels/SkyCompRep.h
    ComponentModels/SpectralIndex.h
    ComponentModels/SpectralModel.h
    ComponentModels/TabularSpectrum.h
    ComponentModels/TwoSidedShape.h
	DESTINATION include/casacode/components/ComponentModels
)
install (FILES 
    SpectralComponents/CompiledSpectralElement.h
    SpectralComponents/GaussianMultipletSpectralElement.h
    SpectralComponents/GaussianSpectralElement.h
    SpectralComponents/LogTransformedPolynomialSpectralElement.h
    SpectralComponents/LorentzianSpectralElement.h
    SpectralComponents/PCFSpectralElement.h
    SpectralComponents/PolynomialSpectralElement.h
    SpectralComponents/PowerLogPolynomialSpectralElement.h
    SpectralComponents/ProfileFit1D.h
    SpectralComponents/ProfileFit1D.tcc
    SpectralComponents/Spectral2Estimate.tcc
    SpectralComponents/SpectralElementFactory.h
    SpectralComponents/SpectralElement.h
    SpectralComponents/SpectralEstimate.h
    SpectralComponents/SpectralFit2.tcc
    SpectralComponents/SpectralFit.h
    SpectralComponents/SpectralList2.tcc
    SpectralComponents/SpectralListFactory.h
    SpectralComponents/SpectralList.h
	DESTINATION include/casacode/components/SpectralComponents
)

casa_add_assay( components ComponentModels/test/dPointShape.cc )
casa_add_assay( components ComponentModels/test/dSkyCompBase.cc )
casa_add_assay( components ComponentModels/test/dSpectralModel.cc )
casa_add_assay( components ComponentModels/test/dTwoSidedShape.cc )
casa_add_assay( components ComponentModels/test/tComponentList.cc )
casa_add_assay( components ComponentModels/test/tComponentType.cc )
casa_add_assay( components ComponentModels/test/tConstantSpectrum.cc )
casa_add_assay( components ComponentModels/test/tDiskShape.cc )
casa_add_assay( components ComponentModels/test/tFluxStandard.cc )
casa_add_assay( components ComponentModels/test/tFluxStandard2.cc )
casa_add_assay( components ComponentModels/test/tFluxStandard3.cc )
casa_add_assay( components ComponentModels/test/tFluxStandard4.cc )
casa_add_assay( components ComponentModels/test/tGaussianComponent.cc )
casa_add_assay( components ComponentModels/test/tGaussianDeconvolver.cc )
casa_add_assay( components ComponentModels/test/tGaussianShape.cc )
casa_add_assay( components ComponentModels/test/tLimbDarkenedDiskShape.cc )
casa_add_assay( components ComponentModels/test/tPointComponent.cc )
casa_add_assay( components ComponentModels/test/tPointShape.cc )
casa_add_assay( components ComponentModels/test/tSkyComponentFactory.cc )
casa_add_assay( components ComponentModels/test/tSkyCompRep.cc )
casa_add_assay( components ComponentModels/test/tSpectralIndex.cc )
casa_add_assay( components ComponentModels/test/tTabularSpectrum.cc )

casa_add_assay( components SpectralComponents/test/tGaussianMultipletSpectralElement.cc )
casa_add_assay( components SpectralComponents/test/tLogTransformedPolynomialSpectralElement.cc )
casa_add_assay( components SpectralComponents/test/tLorentzianSpectralElement.cc )
casa_add_assay( components SpectralComponents/test/tPowerLogPolynomialSpectralElement.cc )
casa_add_assay( components SpectralComponents/test/tProfileFit1D.cc )
casa_add_assay( components SpectralComponents/test/tSpectralFit.cc )


                     
                              
                                    

