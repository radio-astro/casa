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

casa_add_python( tools tools_regressions_tests_python python/${PYTHONV}/regressions/tests
  3C129.py
  3C219D_test.py
  3C391polim.py
  accum_test.py
  alma-m100-analysis.py
  alma-m100-analysis-hpc.py
  alma-ngc3256-analysis.py
  alma-titan-ephemeris.py
  asdm-imexport+analysis.py
  async-segfault.py
  calstat_test.py
  cleanchan_test.py
  cleanhelper_test.py
  coordsystest.py
  cvel+clean_test.py
  cvel_test2.py
  equinox_vis.py
  fits-import-export.py
  fls3a_hi.py
  h121.py
  ic2233.py
  imagepoltest.py
  imagetest.py
  irc_hc3n_task.py
  irc_cs_task.py
  l02d.py
  leo2pt_test.py
  listcal_test.py
  listvis_test.py
  ngc2403.py
  ngc5921redux.py
  ngc5921uvcontsub2.py
  ori_hc3n.py
  ori_sio.py
  orionmosaic.py
#  plotxy_test.py
  pointing_test.py
  polcal_20080224_cband.py
  pylabmem.py
  third4826.py
  third4826-mms.py
  time_then_chan_avg.py
  wideband.py
  widebandmosaic.py
  wproject_regression.py
  orionmos4sim.py
  sim_m51c.py
  sim_m51tp.py
  sim_ppdisk.py
  sim_testcomp.py
  sim_testcube.py
  stokesimaging_test.py
  testcube2.py
)
