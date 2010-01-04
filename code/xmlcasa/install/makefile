#-----------------------------------------------------------------------------
# AIPS++ code development directory makefile
#-----------------------------------------------------------------------------
#
#   Copyright (C) 2006
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
# GNU makefile used to install AIPS++ code development utilities.
#
# Original: 1994/09/05 by Mark Calabretta, ATNF.
# $Id$
#=============================================================================

# AIPS++ fundamentals.
#---------------------
AIPSROOT := $(word 1, $(CASAPATH))
AIPSARCH := $(AIPSROOT)/$(word 2, $(CASAPATH))
include $(AIPSARCH)/makedefs


# Source lists.
#--------------
SCRIPTS  := mksetupdotpy.sh setsvneditor.sh setsvneditor.csh svn_editor xmlgenpy buildmytasks

JARS := saxon8.jar

XSLTS := casa2py.xsl casa2tsum.xsl casa2tsum2.xsl casa2pycli.xsl casa2pyimp.xsl casa2pypg.xsl

# Pattern rules.
#---------------
$(ARCHBIND)/% :: $(CODEDIR)/%
	@ $(RM) $@
	  cp $< $@
	@ chmod 775 $@

$(AIPSROOT)/share/xml/%.xsl :: $(CODEDIR)/%.xsl
	amkdir -p ug=rwx,g+s,o=rx -v $(AIPSROOT)/share/xml
	@ $(RM) $@
	  cp $< $@
	@ chmod 775 $@

$(LIBOPTD)/%.jar :: $(CODEDIR)/%.jar
	@ $(RM) $@
	  cp $< $@
	@ chmod 775 $@


# Static and static pattern rules.
#---------------------------------
allsys : $(SCRIPTS) $(JARS) $(XSLTS)

# Scripts.
$(SCRIPTS) : % : $(ARCHBIND)/% ;

$(JARS) : %.jar : $(LIBOPTD)/%.jar ;

$(XSLTS) : %.xsl : $(AIPSROOT)/share/xml/%.xsl ;

show_local :
	-@ echo ""
	-@ echo "Variables defined in the install/codedevl makefile"
	-@ echo "=================================================="
	-@ echo ""
	-@ echo "SCRIPTS =$(SCRIPTS)"

help ::
	-@ echo ""
	-@ echo "Targets defined in the install/codedevl makefile"
	-@ echo "================================================"
	-@ echo ""
	-@ echo "System"
	-@ echo "------"
	-@ echo "  (SCRIPTS): install code development scripts."
