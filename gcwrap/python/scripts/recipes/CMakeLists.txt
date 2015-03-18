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

casa_add_python( tools tools_recipes_python python/${PYTHONV}/recipes
  almahelpers.py
  almapolhelpers.py
  automask.py
  benchmark.py
  checknan.py
  checkdataformat.py
  cl2skycat.py
  cluster.py
  cmpmses.py
  commongrid.py
  drawmaskinimage.py
  field2skycat.py
  find_autocorr.py
  gaincal_unflag_nearest.py
  getazel.py
  __init__.py
  linfeedpolhelpers.py
  lsms.py
  make_labelled_ms.py
  pixelmask2cleanmask.py
  plotazel.py
  psplit.py
  taskutil.py
  listshapes.py
  astro_stations2simdata.py
  setOrder.py
  specframeconvert.py
  tec_maps.py
  weights.py
)

add_subdirectory( ephemerides )
