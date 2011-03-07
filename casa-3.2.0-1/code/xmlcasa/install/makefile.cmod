#-----------------------------------------------------------------------------
# makefile.imp: Generic AIPS++ class implementation makefile
#-----------------------------------------------------------------------------
#
#   Copyright (C) 1992-2002
#   Associated Universities, Inc. Washington DC, USA.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#   Correspondence concerning AIPS++ should be addressed as follows:
#          Internet email: aips2-request@nrao.edu.
#          Postal address: AIPS++ Project Office
#                          National Radio Astronomy Observatory
#                          520 Edgemont Road
#                          Charlottesville, VA 22903-2475 USA
#
#-----------------------------------------------------------------------------
# This GNU makefile is included by other AIPS++ makefiles and is not intended
# for stand-alone use.
#
# Original: 1992/06/24 by Mark Calabretta, ATNF
# $Id$
#-----------------------------------------------------------------------------

# AIPS++ fundamentals.
#---------------------
AIPSROOT := $(word 1, $(CASAPATH))
LIBJAVAD := $(AIPSROOT)/java
AIPSARCH := $(AIPSROOT)/$(word 2, $(CASAPATH))
AIPSGCODE := $(AIPSROOT)/gcode_$(word 2, $(CASAPATH))
include $(AIPSARCH)/makedefs

# Do not include template source files.
#--------------------------------------
#CPPOPT += -DAIPS_NO_TEMPLATE_SRC
#CPPDBG += -DAIPS_NO_TEMPLATE_SRC

PCKGMOD := $(subst /,-,$(subst /implement,,$(CODESUBD)))
MODULE  := $(word 2,$(subst -,$(space),$(PCKGMOD)))
PYMODULE  := $(word 3,$(subst -,$(space),$(PCKGMOD)))

ifeq "$(MAKEMODE)" "programmer"

allsys bin:
	if test -f mksetupdotpy.sh; then ./mksetupdotpy.sh; fi
	$(RM) -rf build
	$(PYTHON) setup.py build install --install-lib=$(PGMRLIBD)

else

allsys bin:
	if test -f mksetupdotpy.sh; then ./mksetupdotpy.sh; fi
	$(RM) -rf build
	$(PYTHON) setup.py build install --install-lib=$(PYLIBD)

endif

inistsys:

