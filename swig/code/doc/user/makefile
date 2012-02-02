 # $Id$

# HELPACKS ought to be a sed script that generates the package list from the
# Refman.latex source.
HELPACKS  := general utility display synthesis dish nrao nfra atnf npoi bima alma vo
DOCSUSER  := $(word 1, $(CASAPATH))/docs/user
HELPAREA  := $(word 1, $(CASAPATH))/docs/user/helpfiles
HTEXFILES := $(addsuffix .htex, $(addprefix $(HELPAREA)/, $(HELPACKS)))

EXTRA_TEXINPUTS := $(HELPAREA)

Utility.s2ps := lots_of_ps_files
General.s2ps := lots_of_ps_files
SynthesisRef.s2ps := lots_of_ps_files
ALMA.s2ps := lots_of_ps_files
ATNF.s2ps := lots_of_ps_files
Dish.s2ps := lots_of_ps_files
Display.s2ps := lots_of_ps_files
NRAO.s2ps := lots_of_ps_files
NFRA.s2ps := lots_of_ps_files
NPOI.s2ps := lots_of_ps_files
BIMA.s2ps := lots_of_ps_files
VO.s2ps := lots_of_ps_files

Aipsview       := -split +2
Synthesis      := -split +2
Refman         := -split 0
SynthesisRef   := -split +4 -link 2 -short_index
ALMA           := -split +4 -link 2 -short_index
BIMA           := -split +4 -link 2 -short_index
NPOI           := -split +4 -link 2 -short_index
NRAO           := -split +4 -link 2 -short_index
NFRA           := -split +4 -link 2 -short_index
ATNF           := -split +4 -link 2 -short_index
Display        := -split +4 -link 2 -short_index
Dish           := -split +4 -link 2 -short_index
General        := -split +4 -link 2 -short_index
Utility        := -split +4 -link 2 -short_index
VO             := -split +4 -link 2 -short_index
asciitable     := -split +2
HELPSYS        := helpsys

# Use the generic AIPS++ documentation makefile
#----------------------------------------------
include $(word 1, $(CASAPATH))/code/install/makefile.doc

#
#The reference manual requires special treatment,
#
#First we setup special dependicies for the .ps file, i.e. the helparea/helpack
#stuff,
#
#Second, we scan the source tree for all .help files and put them into a
#temporary directory (we only need to do this once, hence the if statment.
#
#Third, we translate the help files into .htex files, only copy them into the
#docs tree if they are different.  If they are different we generate the atoms
#file for use by the online help system.
#
#Fourth, the regular make system will now build the manual if text is different
#

$(DOCSAREA)/ALMA.ps.gz  : $(HELPAREA)/alma.htex ALMA.latex

$(DOCSAREA)/ATNF.ps.gz  : $(HELPAREA)/atnf.htex ATNF.latex

$(DOCSAREA)/BIMA.ps.gz  : $(HELPAREA)/bima.htex BIMA.latex

$(DOCSAREA)/Dish.ps.gz  : $(HELPAREA)/dish.htex Dish.latex

$(DOCSAREA)/Display.ps.gz  : $(HELPAREA)/display.htex Display.latex

$(DOCSAREA)/NFRA.ps.gz  : $(HELPAREA)/nfra.htex NFRA.latex

$(DOCSAREA)/NPOI.ps.gz  : $(HELPAREA)/npoi.htex NPOI.latex

$(DOCSAREA)/NRAO.ps.gz  : $(HELPAREA)/nrao.htex NRAO.latex

$(DOCSAREA)/VO.ps.gz  : $(HELPAREA)/vo.htex VO.latex

$(DOCSAREA)/Utility.ps.gz  : $(HELPAREA)/utility.htex Utility.latex

$(DOCSAREA)/SynthesisRef.ps.gz  : $(HELPAREA)/synthesis.htex SynthesisRef.latex

$(DOCSAREA)/General.ps.gz  : $(HELPAREA)/general.htex General.latex

$(DOCSAREA)/Refman.ps  : $(HTEXFILES) Refman.latex $(CODEDIR)/Refman.dir

helpsys %.htex : $(AIPSCODE)/VERSION
	@ echo $@
	@ [ -d "$(DOCSUSER)" ] || mkdir $(DOCSUSER)
	@ [ -d "$(HELPAREA)" ] || mkdir $(HELPAREA)
	@ chmod ug=rwx,g+s,o=rx $(HELPAREA)
	@ if [ "$*" = "$(HELPAREA)/aips" -o "$@" = "helpsys" ] ; then \
            trap "cd $(AIPSROOT) ; $(RM) -r $(TMPDOCD)" 0 1 2 3 15 ; \
            mkdir $(TMPDOCD) ; \
	    for pack in appsglish trialdisplay $(HELPACKS) ; do \
               find $(AIPSROOT)/code/$$pack \( -name "*.help" -o -name "*.eps" -o -name "*.ps" -o -name "*.gif" -o -name "*.epsi" \) -exec cp -p {} $(TMPDOCD) \; ; \
	    done ; \
            cd $(TMPDOCD) ; \
            cp -f *.eps* *.ps *.gif $(HELPAREA) ; \
            for fileroot in $(HELPACKS) ; do \
               echo $$fileroot.help ; \
               help2tex -noheaders -package $$fileroot $$fileroot.help > $$fileroot.htex ; \
               diff $$fileroot.htex $(HELPAREA)/$$fileroot.htex > diff.file ; \
               if [ ! -f $(HELPAREA)/$$fileroot.htex -o -s diff.file ] ; then \
                  cp $$fileroot.htex $(HELPAREA) ; \
               fi ; \
               help2tex -glish -package $$fileroot $$fileroot.help > atoms$$fileroot.g ; \
               cp atoms$$fileroot.g $(AIPSARCH)/libexec ; \
               help2tex -outline -package $$fileroot $$fileroot.help > pmt$$fileroot.tex ; \
               cp pmt$$fileroot.tex $(HELPAREA) ; \
            done \
	  fi

atoms : $(AIPSCODE)/VERSION
	@ echo $@
	@ [ -d "$(DOCSUSER)" ] || mkdir $(DOCSUSER)
	@ [ -d "$(HELPAREA)" ] || mkdir $(HELPAREA)
	@ chmod ug=rwx,g+s,o=rx $(HELPAREA)
	@ if [ "$*" = "$(HELPAREA)/aips" -o "$@" = "atoms" ] ; then \
            trap "cd $(AIPSROOT) ; $(RM) -r $(TMPDOCD)" 0 1 2 3 15 ; \
            mkdir $(TMPDOCD) ; \
	    for pack in appsglish trialdisplay $(HELPACKS) ; do \
               find $(AIPSROOT)/code/$$pack \( -name "*.help" -o -name "*.eps" -o -name "*.ps" -o -name "*.epsi" \) -exec cp -p {} $(TMPDOCD) \; ; \
	    done ; \
            cd $(TMPDOCD) ; \
            for fileroot in $(HELPACKS) ; do \
               echo $$fileroot.help ; \
               help2tex -glish -package $$fileroot $$fileroot.help > atoms$$fileroot.g ; \
               cp atoms$$fileroot.g $(AIPSARCH)/libexec ; \
            done \
	  fi





















