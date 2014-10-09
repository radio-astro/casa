#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2012 by ESO (in the framework of the ALMA collaboration)
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

casa_add_library( air_casawvr
  casawvr/casawvr_main.cpp
  casawvr/mswvrdata.cpp 
  casawvr/msgaintable.cpp
  casawvr/msutils.cpp 
  casawvr/casawvr_errs.cpp
  casawvr/msspec.cpp
  casawvr/msweather.cpp 
  casawvr/msantdata.cpp
  cmdline/wvrgcal.cpp
  cmdline/wvrgcalerrors.cpp
  cmdline/wvrgcalfeedback.cpp
  src/basicphys.cpp
  src/cloudywater.cpp
  src/columns.cpp
  src/columns_data.cpp
  src/decorr.cpp
  src/dipmeasure_iface.cpp
  src/dipmodel_iface.cpp
  src/dispersion.cpp
  src/dtdltools.cpp
  src/layers.cpp
  src/libair_main.cpp
  src/lineparams.cpp
  src/lineshapes.cpp
  src/measure_iface.cpp
  src/model_iface.cpp
  src/model_make.cpp
  src/models_basic.cpp
  src/model_water.cpp
  src/partitionsum.cpp
  src/partitionsum_testdata.cpp
  src/path_measure.cpp
  src/path_transfer.cpp
  src/radiometermeasure.cpp
  src/radiometer_utils.cpp
  src/rtranfer.cpp
  src/singlelayerwater.cpp
  src/slice.cpp
  src/taumodel.cpp
  src/tbutils.cpp
  src/apps/almaabs.cpp
  src/apps/almaabs_i.cpp
  src/apps/almaopts.cpp
  src/apps/almaresults.cpp
  src/apps/antennautils.cpp
  src/apps/arraydata.cpp
  src/apps/arraygains.cpp
  src/apps/dtdlcoeffs.cpp
  src/apps/segmentation.cpp
  )

casa_add_executable( air_casawvr wvrgcal
  cmdline/wvrgcal.cpp  
  cmdline/wvrgcalerrors.cpp  
  cmdline/wvrgcalfeedback.cpp
  )
