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

casa_add_python( xmlcasa xmlcasa_tests_python python/${PYTHONV}/tests
  test_boxit.py
  test_clean.py
  test_clearstat.py
  test_csvclean.py
  test_cvel.py
  test_exportasdm.py
  test_fixvis.py
  test_flagdata.py
  test_hanningsmooth.py
  test_imcontsub.py
  test_imfit.py
  test_imhead.py
  test_immath.py
  test_immoments.py
  test_importasdm.py
  test_importevla.py
  test_importfitsidi.py
  test_importoldasdm.py
  test_imregrid.py
  test_imsmooth.py
  test_imstat.py
  test_imval.py
  test_listhistory.py
  test_listvis.py
  test_plotants.py
  test_plotms.py
  test_report.py
  test_sdplot.py
  test_smoothcal.py
  test_vishead.py
  test_visstat.py
  unittests_list.txt
)
