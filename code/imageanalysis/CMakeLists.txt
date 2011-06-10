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

casa_add_library( imageanalysis
implement/Annotations/AnnAnnulus.cc
implement/Annotations/AnnCenterBox.cc
implement/Annotations/AnnCircle.cc
implement/Annotations/AnnEllipse.cc
implement/Annotations/AnnLine.cc
implement/Annotations/AnnotationBase.cc
implement/Annotations/AnnPolygon.cc
implement/Annotations/AnnRectBox.cc
implement/Annotations/AnnRegion.cc
implement/Annotations/AnnRotBox.cc
implement/Annotations/AnnSymbol.cc
implement/Annotations/AnnText.cc
implement/Annotations/AnnVector.cc
implement/Annotations/AsciiAnnotationList.cc
implement/IO/AsciiAnnotationFileParser.cc
implement/IO/AsciiAnnotationFileLine.cc
)

casa_add_assay( imageanalysis implement/Annotations/test/tAnnAnnulus.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnCenterBox.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnCircle.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnEllipse.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnLine.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnPolygon.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnRectBox.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnRotBox.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnSymbol.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnText.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAnnVector.cc )
casa_add_assay( imageanalysis implement/Annotations/test/tAsciiAnnotationList.cc )

casa_add_assay( imageanalysis implement/IO/test/tAsciiAnnotationFileParser.cc )
