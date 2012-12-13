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

casa_add_library( calanalysis
  CalAnalysis/CalAnalysis.cc
  CalAnalysis/CalAnalysisSPW_INFO.cc
  CalAnalysis/CalStats.cc
  CalAnalysis/CalStatsAXES.cc
  CalAnalysis/CalStatsDATA.cc
  CalAnalysis/CalStatsDerived.cc
  CalAnalysis/CalStatsFitter.cc
  CalAnalysis/CalStatsFIT.cc
)

install ( FILES
  CalAnalysis/CalAnalysis.h
  CalAnalysis/CalStats.h
  CalAnalysis/CalStatsDerived.h
  CalAnalysis/CalStatsFitter.h
  DESTINATION include/casacode/calanalysis/CalAnalysis )

casa_add_assay( calanalysis CalAnalysis/test/tCalStats0.cc )
casa_add_assay( calanalysis CalAnalysis/test/tCalStats1.cc )
casa_add_assay( calanalysis CalAnalysis/test/tCalStats2.cc )
casa_add_assay( calanalysis CalAnalysis/test/tCalStats3.cc )
casa_add_assay( calanalysis CalAnalysis/test/tCalStats4.cc )
casa_add_assay( calanalysis CalAnalysis/test/tCalStats5.cc )
casa_add_assay( calanalysis CalAnalysis/test/tCalStats6.cc )
casa_add_assay( calanalysis CalAnalysis/test/tCalStats7.cc )
casa_add_assay( calanalysis CalAnalysis/test/tCalStatsTab.cc )
