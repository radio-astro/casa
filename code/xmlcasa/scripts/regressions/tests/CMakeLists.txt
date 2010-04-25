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

casa_add_python( xmlcasa xmlcasa_regressions_tests_python python/${PYTHONV}/regressions/tests
  3C129.py
  3C219D_test.py
  accum_test.py
  asdmv1-import.py
  async-segfault.py
  b1608.py
  calstat_test.py
  cleanchan_test.py
  cleanhelper_test.py
  cvel_test.py
  equinox_vis.py
  fits-import-export.py
  g192redux.py
  ggtau.py
  h121.py
  ic2233.py
  l02d.py
  leo2pt_test.py
  ngc1333.py
  ngc2403.py
  ngc4826redux.py
  ngc5921redux.py
  ngc5921uvcontsub2.py
  ngc7538.py
  orionmos4sim.py
  orionmosaic.py
  polcal_20080224_cband.py
  pylabmem.py
  secondNGC4826.py
  third4826.py
  time_then_chan_avg.py
  wproject_regression.py
)
