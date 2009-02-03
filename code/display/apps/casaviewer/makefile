XLIBLIST:=QT4 PGPLOT WCSLIB LAPACK F77 CFITSIO MIRLIB PYTHON
AIPSROOT := $(word 1, $(CASAPATH))
AIPSARCH := $(AIPSROOT)/$(word 2, $(CASAPATH))
include $(AIPSARCH)/makedefs
SKIP_MAKEDEFS := YES

ifneq "$(wildcard $(QT4ROOT)/*)" ""
# Use the generic AIPS++ app makefile.
#------------------------------------------------------
include $(word 1, $(CASAPATH))/code/install/makefile.app
endif



# qtviewer build notes  6/22/06  dk
#
# The make system is now set up for automatic SYSTEM builds of qtviewer.
#
# To enable it, the site makedefs file should set $(QT4ROOT) to a location
# where Qt-4.1.3 is installed; qtviewer cannot be built using earlier
# Qt versions.
#
# After aipsinit.[c]sh, run qtviewer with:
#
#   qtviewer    
#
#
# For DEVELOPERS building qtviewer, i.e., making changes and building in the
# following directories:
# 
#   display/apps/qtviewer/
#   display/implement/QtViewer/
#   display/implement/QtAutoGui/
# 
# After checking out all files in those directories (including the icons/
# subdirectories), you should do the following in display/implement/QtViewer/
# and display/implement/QtAutoGui/:
# 
#   gmake qt
# 
# Then proceed with g++ compilations as usual, using:
#
#   gmake OPT=1
#
# in those directories, and:
#
#   gmake OPT=1 qtviewer
#
# in apps/qtviewer/.
#
# Check out demo applications in implement/QtViewer/test/ as needed, and
# build them using a similar command, e.g.:
#
#   gmake OPT=1 dQtClean
# 
#
# If you only change regular *.h or *.cc files (with no other suffixes), you
# do not need to repeat the 'gmake qt' pre-compiling step.  However, if you
# change a *.qo.h, *.ui or *.qrc file, you should repeat 'gmake qt' in the
# relevant implement/ directory before 'gmake OPT=1'.  If your change was to
# a .ui file (normally done through designer), do the regular re-compilation
# ('gmake OPT=1') in _both_ implement/ directories.
#
# Files of the form *.moc.cc, *.ui.h, or *.qrc.cc are intermediate, generated
# files; they should not be altered directly by the programmer, or checked in. 
# To remove them completely from an implement directory use  gmake qtclean. 
#
# Hopefully, that's all you'll need to know.  (If not...)
#
#
# ...more details:
#
# 'gmake qt' performs the three types of pre-compilation which qt code
# requires, as detailed in the table below.
# 
# Primary, checked-in file     Secondary, generated file   Qt pre-compiler
# --------------------------   -------------------------   ------------------
# *.qo.h                       *.moc.cc                    $(QT4ROOT)/bin/moc
# *.ui                         *.ui.h                      $(QT4ROOT)/bin/uic
# *.qrc                        *.qrc.cc                    $(QT4ROOT)/bin/rcc
# *.h  without other suffixes  -none-
# *.cc without other suffixes  -none-
# 
# All files above ending in .h and .cc contain standard C++ header and
# implementation code, respectively (as usual), and are [also] compiled by
# g++.
#
# The header for MyClass.cc is  MyClass.qo.h  rather than  MyClass.h  if that
# header file contains Qt's Q_OBJECT macro.  Primarily, this allows MyClass
# to derive from QObject and communicate via 'signals and slots'; however
# it requires MyClass.moc.cc to be generated from the header and compiled in
# as well.
#
# The files ending in .ui and .qrc are Qt-specific xml files: *.ui is
# wysiwig Gui designer output, *.qrc specifies 'resources' like icons.
#
# The makefile rules cause MyResources.qrc to be translated using the
# '-name MyResources' flag, which allows more than one resource file to
# be merged into the app.  This means they are to be initialzed in the
# code using:
#
#   Q_INIT_RESOURCE(MyResources);   (or  'qInitResources_MyResources();' )
# 
# rather than simply:
# 
#   Q_INIT_RESOURCE();
#
#
# Qt docs:
#
#   http://doc.trolltech.com/4.1/index.html
#   http://doc.trolltech.com/4.1/overviews.html
#   http://doc.trolltech.com/4.1/classes.html
#   http://doc.trolltech.com/4.1/functions.html
