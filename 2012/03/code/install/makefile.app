#-----------------------------------------------------------------------------
# makefile.app: Generic AIPS++ applications makefile
#-----------------------------------------------------------------------------
#
#   Copyright (C) 1992-2004
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
#=============================================================================

# AIPS++ fundamentals.
#---------------------
include $(word 1, $(CASAPATH))/code/install/makefile.app_c

bin    : $(TMPPCKGD) $(BINOPTD)/$(THISAPP) pyopyc $(LIBEXECS) $(LIBICONS) $(BINEXECS) $(MACAPP);

bindbg : $(TMPPCKGD) $(BINDBGD)/$(THISAPP) ;

# Programmer-oriented pattern rules.
ifeq "$(MAKEMODE)" "programmer"
   .PRECIOUS : $(PGMRBIND)/% $(PGMRBIND)/%.o $(PGMRBIND)/%.i

   $(PGMRBIND)/% : $(PGMRBIND)/%.o $(PGMRLIBR)
	-@ echo ""
	-@ $(RM) $@
	   $(C++) $(PRGAPINC) $(PGMRINCL) $(C++FLAGS) $(LDFLAGS) -o $@ $< $(PGMRLIBS)
endif

$(BINDBGD)/% : $(CODEDIR)/%.cc $(AIPSINST) \
      $(addprefix $(TMPPCKGD)/, $(addsuffix cc, $(LEXYACC))) \
      $(addprefix $(CODEDIR)/,$(AIPSIMPS)) $(DBGLIBS)
	-@ echo ""
	-@ $(TIMER)
	-@ echo "Remaking $* (dbg) because of $(?F)"
	-@ if [ -h $@ ]; then $(RM) $@; fi
	 @ cd $(TMPPCKGD) && \
	   $(C++) $(CPPDBG) -I$(TMPPCKGD) -I$(CODEDIR) $(AIPSINCL) $(C++DBG) \
	      $(LDDBG) -o $@ $< $(AIPSINST:%=%/*.cc) \
	      -L$(AIPSARCH)/libdbg \
	      $(addprefix $(CODEDIR)/,$(AIPSIMPS)) $(DBGVERSO) \
	      $(patsubst $(LIBDBGD)/lib%.$(SFXSHAR), -l%, $(DBGLIBS)) \
	      $(MODULIBS) $(XTRNLIBS) $(DUMALIB)
	-@ $(TIMER)
	-@ $(RM) $(patsubst %.cc,$(TMPPCKGD)/%.o,$(<F) $(AIPSIMPS)) \
	         $(addprefix $(TMPPCKGD)/, $(addsuffix cc, $(LEXYACC)))
	-@ chmod 775 $@

$(BINOPTD)/% : $(CODEDIR)/%.cc $(AIPSINST) \
      $(addprefix $(TMPPCKGD)/, $(addsuffix cc, $(LEXYACC))) \
      $(addprefix $(CODEDIR)/,$(AIPSIMPS)) $(OPTLIBS2)
	-@ echo ""
	-@ $(TIMER)
	-@ echo $(ECHOESC) "\e[32;1mRemaking $* (opt) because of $(?F)\e[0m"
	-@ if [ -h $@ ]; then $(RM) $@; fi
	 @ cd $(TMPPCKGD) && \
	   $(C++) $(CPPOPT) -I$(TMPPCKGD) -I$(CODEDIR) $(AIPSINCL) $(C++OPT) \
	      $(LDOPT) -o $@ $< $(AIPSINST:%=%/*.cc) \
	      -L$(AIPSARCH)/lib \
	      $(addprefix $(CODEDIR)/,$(AIPSIMPS)) $(OPTVERSO) \
	      $(patsubst $(LIBOPTD)/lib%.$(SFXSHAR), -l%, $(OPTLIBS)) \
	      $(MODULIBS) $(XTRNLIBS) $(DUMALIB)
	-@ $(TIMER)
	-@ $(RM) $(patsubst %.cc,$(TMPPCKGD)/%.o,$(<F) $(AIPSIMPS)) \
	         $(addprefix $(TMPPCKGD)/, $(addsuffix cc, $(LEXYACC)))
	-@ chmod 775 $@

#Rule to handle applications that need to be installed mac like, i.e. QT4 based executables

ifneq "$(MACAPPINSTALL)" ""
macapp :
	@ echo $(MACAPPINSTALL)
	@ amkdir $(MACAPPINSTALL)/Contents/MacOS
	@ if [ ! -f $(MACAPPINSTALL)/Contents/MacOS/$(THISAPP) ];  then \
	   ln -s $(AIPSARCH)/bin/$(THISAPP) $(MACAPPINSTALL)/Contents/MacOS; \
	fi
endif



$(LIBDBGD)/%.$(SFXSTAT) : ;
$(LIBDBGD)/%.$(SFXSHAR) : ;
$(LIBOPTD)/%.$(SFXSTAT) : ;
$(LIBOPTD)/%.$(SFXSHAR) : ;
