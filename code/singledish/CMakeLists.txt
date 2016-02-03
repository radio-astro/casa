#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2014-2015 by NAOJ (in the framework of the ALMA collaboration)
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

casa_add_library( singledish
    SingleDish/BaselineTable.cc
    SingleDish/BLParameterParser.cc
    SingleDish/LineFindingUtils.cc
    SingleDish/LineFinder.cc
    SingleDish/SDMSManager.cc
    SingleDish/SingleDishMS.cc
    Filler/Scantable2MSReader.cc
    Filler/SingleDishMSFiller.cc
)

install (FILES
    SingleDish/BaselineTable.h
    SingleDish/BLParameterParser.h
    SingleDish/LineFindingUtils.h
    SingleDish/LineFinder.h
    SingleDish/SDMSManager.h
    SingleDish/SingleDishMS.h
    DESTINATION include/casacode/singledish/SingleDish
)

install (FILES
	Filler/FillerUtil.h
    Filler/AntennaRecord.h
    Filler/DataRecord.h
    Filler/FieldRecord.h
    Filler/ObservationRecord.h
    Filler/ProcessorRecord.h
    Filler/SourceRecord.h
    Filler/SpectralWindowRecord.h
    Filler/SysCalRecord.h
    Filler/WeatherRecord.h
    Filler/ScantableIterator.h
    Filler/DataAccumulator.h
    Filler/ReaderInterface.h
    Filler/Scantable2MSReader.h
    Filler/SingleDishMSFiller.h
    Filler/SingleDishMSFiller.tcc
    DESTINATION include/casacode/singledish/Filler
)

### traditional c++ codes for testing ###
#casa_add_assay( singledish
#    SingleDish/test/tLineFinder.cc
#    )
### google tests ###
casa_add_google_test (MODULES singledish SOURCES SingleDish/test/LineFindingUtils_GTest.cc)
casa_add_google_test (MODULES singledish SOURCES SingleDish/test/LineFinder_GTest.cc)
casa_add_google_test (MODULES singledish SOURCES SingleDish/test/SingleDishMS_GTest.cc)
casa_add_google_test (MODULES singledish SOURCES Filler/test/SingleDishMSFiller_GTest.cc)
