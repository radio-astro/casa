#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2015 by NAOJ (in the framework of the ALMA collaboration)
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

casa_add_library( atmosphere
    ATM/ATMAngle.cpp
    ATM/ATMError.cpp
    ATM/ATMException.cpp
    ATM/ATMFrequency.cpp
    ATM/ATMInverseLength.cpp
    ATM/ATMLength.cpp
    ATM/ATMMassDensity.cpp
    ATM/ATMNumberDensity.cpp
    ATM/ATMOpacity.cpp
    ATM/ATMPercent.cpp
    ATM/ATMPressure.cpp
    ATM/ATMProfile.cpp
    ATM/ATMRefractiveIndex.cpp
    ATM/ATMRefractiveIndexProfile.cpp
    ATM/ATMSkyStatus.cpp
    ATM/ATMSpectralGrid.cpp
    ATM/ATMTemperature.cpp
    ATM/ATMVersion.cpp
    ATM/ATMWaterVaporRadiometer.cpp
    ATM/ATMWVRMeasurement.cpp
)

install (FILES
    ATM/ATMAngle.h
    ATM/ATMCommon.h
    ATM/ATMConstants.h
    ATM/ATMEnumerations.h
    ATM/ATMError.h
    ATM/ATMException.h
    ATM/ATMFrequency.h
    ATM/ATMHumidity.h
    ATM/ATMInverseLength.h
    ATM/ATMLength.h
    ATM/ATMMassDensity.h
    ATM/ATMNumberDensity.h
    ATM/ATMOpacity.h
    ATM/ATMPercent.h
    ATM/ATMPressure.h
    ATM/ATMProfile.h
    ATM/ATMRefractiveIndex.h
    ATM/ATMRefractiveIndexProfile.h
    ATM/ATMSkyStatus.h
    ATM/ATMSpectralGrid.h
    ATM/ATMTemperature.h
    ATM/ATMVersion.h
    ATM/ATMWaterVaporRadiometer.h
    ATM/ATMWVRMeasurement.h
    DESTINATION include/casacode/atmosphere/ATM
)

#casa_add_assay( singledish  ATM/test/tSingleDishMS.cc )
