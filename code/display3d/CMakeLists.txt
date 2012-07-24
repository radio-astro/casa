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

include( ${QT_USE_FILE} )
set( QT4_LIBRARIES ${QT_LIBRARIES} )

qt4_wrap_cpp( outfiles_qoh
  3DViewer/3DController.qo.h
  3DViewer/3DDisplayData.qo.h
  3DViewer/3DWindow.qo.h
  3DViewer/Colorbar.qo.h
  3DViewer/SlideEditor.qo.h
  )

casa_add_library( display3d
  ${outfiles_qoh} ${outfiles_ui} ${outfiles_qrc}
  3DViewer/3DController.cc
  3DViewer/3DDisplayData.cc
  3DViewer/3DWindow.cc
  3DViewer/Colorbar.cc
  3DViewer/SlideEditor.cc
  3DViewer/tqlayout.cc
  )

casa_add_executable( display3d 3v apps/3v/3v.cc )
