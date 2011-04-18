#
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

casa_add_library( calibration
  implement/CalTables/BJonesDesc.cc
  implement/CalTables/BJonesMBuf.cc
  implement/CalTables/BJonesMCol.cc
  implement/CalTables/BJonesMRec.cc
  implement/CalTables/BJonesTable.cc
  implement/CalTables/CalBuffer.cc
  implement/CalTables/CalDescBuffer.cc
  implement/CalTables/CalDescColumns2.cc
  implement/CalTables/CalDescColumns.cc
  implement/CalTables/CalDescRecord.cc
  implement/CalTables/CalHistoryBuffer.cc
  implement/CalTables/CalHistoryColumns.cc
  implement/CalTables/CalHistRecord.cc
  implement/CalTables/CalInterp.cc
  implement/CalTables/CalInterpolation.cc
  implement/CalTables/CalInterpolator.cc
  implement/CalTables/CalIntpMatch.cc
  implement/CalTables/CalIter.cc
  implement/CalTables/CalMainBuffer.cc
  implement/CalTables/CalMainColumns.cc
  implement/CalTables/CalMainRecord.cc
  implement/CalTables/CalSet2.cc
  implement/CalTables/CalSetMetaInfo.cc
  implement/CalTables/CalTable2.cc
  implement/CalTables/CalTable.cc
  implement/CalTables/CalTableDesc2.cc
  implement/CalTables/CalTableDesc.cc
  implement/CalTables/EPointDesc.cc
  implement/CalTables/EPointMBuf.cc
  implement/CalTables/EPointMCol.cc
  implement/CalTables/EPointTable.cc
  implement/CalTables/EPointVJMRec.cc
  implement/CalTables/FVisJonesDesc.cc
  implement/CalTables/FVisJonesMRec.cc
  implement/CalTables/GJonesDesc.cc
  implement/CalTables/GJonesMBuf.cc
  implement/CalTables/GJonesMCol.cc
  implement/CalTables/GJonesTable.cc
  implement/CalTables/MJonesDesc.cc
  implement/CalTables/MJonesMRec.cc
  implement/CalTables/PlotCal.cc
  implement/CalTables/SolvableMJDesc.cc
  implement/CalTables/SolvableMJMRec.cc
  implement/CalTables/SolvableVJDesc.cc
  implement/CalTables/SolvableVJMBuf.cc
  implement/CalTables/SolvableVJMCol.cc
  implement/CalTables/SolvableVJMRec.cc
  implement/CalTables/SolvableVJTable.cc
  implement/CalTables/TimeVarMJDesc.cc
  implement/CalTables/TimeVarMJMRec.cc
  implement/CalTables/TimeVarVJDesc.cc
  implement/CalTables/TimeVarVJMBuf.cc
  implement/CalTables/TimeVarVJMCol.cc
  implement/CalTables/TimeVarVJMRec.cc
  implement/CalTables/TimeVarVJTable.cc
  implement/CalTables/TJonesDesc.cc
  implement/CalTables/TJonesMBuf.cc
  implement/CalTables/TJonesMCol.cc
  implement/CalTables/TJonesTable.cc
  implement/CalTables/VisJonesDesc.cc
  implement/CalTables/VisJonesMBuf.cc
  implement/CalTables/VisJonesMCol.cc
  implement/CalTables/VisJonesMRec.cc
  implement/CalTables/VisJonesTable.cc
   )

casa_add_assay( calibration implement/CalTables/test/tCalInterpolation.cc )
casa_add_assay( calibration implement/CalTables/test/tCalIntpMatch.cc )
