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

casa_add_python( xmlcasa xmlcasa_regressions_admin_python python/${PYTHONV}/regressions/admin
  publish_summary.py
  runUnitTest.py
  testbase.py
  imageTest.py
  locatescript.py
  gprof2dot.py
  profileplot.py
  report.py
  tableMaker.py
  testwrapper.py
  tw_func.py
  tw_plotmem.py
  tw_utils.py
)

install( PROGRAMS 
         installer.pl
         process_manager.pl
         scheduler.pl
         runcasa_from_shell.sh
         tests_list.txt
         DESTINATION python/${PYTHONV}/regressions/admin )
