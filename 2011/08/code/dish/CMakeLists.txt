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

casa_add_library( dish
  implement/SDIterators/SDFITSDataIterator.cc
  implement/SDIterators/SDIterator.cc
  implement/SDIterators/SDIterAxes.cc
  implement/SDIterators/SDIterFieldCopier.cc
  implement/SDIterators/SDMSIterator.cc
  implement/SDIterators/SDRecord.cc
  implement/SDIterators/SDTableIterator.cc
  implement/SDIterators/SDTabRecCopier.cc
   )

casa_add_assay( dish implement/SDIterators/test/tSDMSIterator.cc )
casa_add_assay( dish implement/SDIterators/test/tSDRecord.cc )
casa_add_assay( dish implement/SDIterators/test/tSDTableIterator.cc )
