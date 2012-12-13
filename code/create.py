#! /usr/bin/env python

#
#  Generate CMakeLists.txt files by running
#
#       cd code/
#       ./create.py
#       mkdir build
#       cd build/
#       cmake -Wdev ..
#       make -j 2 VERBOSE=

import os
import commands

modules = ["graphics", "casaqt", "msvis", "plotms", "display", "casadbus", "tableplot", "flagging", "calibration", "synthesis", "alma", "oldalma", "atnf", "dish", "nrao", "stdcasa"]
deps = {"casaqt" :['CASACORE', 'QT4', 'QWT', 'PYTHON', 'XERCES', "graphics"],
        "plotms" :['CASACORE', 'QT4', "casaqt", "msvis"],
        "tableplot" : ['CASACORE', 'PYTHON'],
        "flagging" : ['CASACORE', 'tableplot', 'msvis'],
        "msvis" : ['CASACORE', 'graphics'],
        "graphics" : ['CASACORE', 'PGPLOT', 'X11'],
        "casadbus" : ['CASACORE', 'DBUS'],
        "calibration" : ['CASACORE', 'tableplot', 'msvis'],
        "synthesis" : ['CASACORE', 'LAPACK', 'ATM', 'casadbus', 'msvis', 'calibration'],
        "alma" : ['CASACORE', 'LIBXML2', 'Boost'],
        "oldalma" : ['CASACORE', 'LIBXML2', 'Boost'],
        "stdcasa" : ['CASACORE'],
        "display" : ['CASACORE', 'WCSLIB', 'QT4', 'casaqt', 'msvis'],
        "atnf" : ['CASACORE', 'RPFITS', 'CFITSIO'],
        "dish" : ['CASACORE'],
        "nrao" : ['CASACORE']}


os.chdir("./include")

for mod in deps.keys():
    if not os.path.exists(mod):
        os.system('ln -s ../%s/implement ./%s' % (mod, mod))

os.chdir("..")

for mod in modules:

    os.chdir(mod)
    os.system("cp /tmp/gpl.txt ./CMakeLists.txt")
    cml = open("./CMakeLists.txt", "a")

    if mod in ["alma", "oldalma"]:
        cml.write("add_definitions( -DWITHOUT_ACS )\n\n")
        cml.write("if( APPLE )\n")
        cml.write("  set( CMAKE_CXX_FLAGS\n")
        cml.write("     \"${CMAKE_CXX_FLAGS} -march=i586\" )\n")
        cml.write("endif()\n\n")

    if mod == "casadbus":
        cml.write("casa_add_dbus_proxy( " +
                  "plotserver/PlotServerProxy.dbusproxy.h " +
                  "implement/plotserver/PlotServerProxy.dbusproxy.xml )\n")
        cml.write("casa_add_dbus_proxy( " +
                  "session/DBusSession.dbusproxy.h " +
                  "implement/session/DBusSession.dbusproxy.xml )\n")
        cml.write("casa_add_dbus_proxy( " +
                  "viewer/ViewerProxy.dbusproxy.h " +
                  "implement/viewer/ViewerProxy.dbusproxy.xml )\n")
        cml.write("\n")

    py_sources = commands.getoutput(
        'find . -name \\*.py | sort | sed "s/^\\.\\///" | grep -vw modules | grep -vw implement | grep -v test | grep -vw regressions | grep -vw usecases | grep -vw demos | grep -vw recipes | xargs -n1 echo " "'
        )

    cc_sources = commands.getoutput(
        'find implement -name \\*.cc | sort | grep -vw test | grep -vw singledish | xargs -n1 echo " "'
        )

    f_sources = commands.getoutput(
        'find implement -name \\*.f | sort | grep -w fortran | xargs -n1 echo " "'
        )
    if os.path.exists('apps'):
        exe_sources = commands.getoutput(
            '\ls -1 apps/ | sort | grep -v makefile | grep -v module_paramgui'
            ).split()
    else:
        exe_sources = []
    
    test_sources = commands.getoutput(
        'find implement -name \\*.cc | sort | grep -w test | grep -v test.dMultichannelRaster.cc | grep -v test.dRGBTestPattern.cc | grep -v test.dRGBWCRaster.cc | grep -v test.dPanelDisplay.cc | grep -v test.dQtPlotter.cc | grep -v test.tArrayAsRaster.cc | grep -v test.dLatticeAsRaster.cc | grep -v test.dNBody.cc | grep -v test.tTblAsRaster.cc | grep -v test.dProfile2dDD.cc | grep -v test.tTblAsXY.cc | grep -v test.tTblAsContour.cc | grep -v test.tArrayAndDrawing.cc | grep -v test.dLatticeAsVector.cc | grep -v test.tApplicator.cc | grep -v test.tcal_cmpt2.cc | grep -v test.tcal_cmpt.cc | grep -v test.AtmosphereTest.cc | grep -v test.tFlagRow.cc | grep -v test.dRegionFile.cc | grep -v test.tSelection.cc'
        ).split()

    qo_h_sources = commands.getoutput(
        'find implement -name \\*.qo.h | sort | grep -vw test | xargs -n1 echo " "'
        )
    ui_sources = commands.getoutput(
        'find implement -name \\*.ui | sort | grep -vw test | xargs -n1 echo " "'
        )

    qrc_sources = commands.getoutput(
        'find implement -name \\*.qrc | sort | grep -vw test'
        ).split()

    xmltask_sources = commands.getoutput(
        'find . -name \\*.xml | sort | sed "s/^\\.\\///" | grep -vw makefile.xml | grep -w tasks | xargs -n1 echo " "'
        )

    xmltool_sources = commands.getoutput(
        'find . -name \\*.xml | sort | sed "s/^\\.\\///" | grep -vw makefile.xml | grep -w implement | grep -v singledish | xargs -n1 echo " "'
        )

    #print cc_sources
    if mod == 'stdcasa':
        # QT_USE_FILE also calls add_definitions, therefore do not call it at the highest level
        # necessary?? cml.write("include( ${QT_USE_FILE} )\n")
        cml.write("add_custom_command( OUTPUT version.cc\n")
        cml.write("  COMMAND \n")
        cml.write("  sed -e 's|MAJOR|${CASA_MAJOR_VERSION}|'\n")
        cml.write("      -e 's|MINOR|${CASA_MINOR_VERSION}|'\n")
        cml.write("      -e 's|PATCH|${CASA_PATCH_VERSION}|'\n")
        cml.write("      -e 's|REVISION|${SVNREVISION}|'\n")
        cml.write("      -e 's|DATE|${BUILDTIME}|'\n")
        cml.write("      -e 's|INFO||'\n")
        cml.write("      ${CMAKE_CURRENT_SOURCE_DIR}/implement/version.template > version.cc\n")
        cml.write("      || ${PERL_EXECUTABLE} -e 'unlink(\"version.cc\")\; exit 1\;'\n")
        cml.write(")\n\n")

    if 'QT4' in deps[mod]:
        cml.write("include( ${QT_USE_FILE} )\n")
        #cml.write("message( the QTlibraries are ${QT_LIBRARIES})\n")
        cml.write("set( QT4_LIBRARIES ${QT_LIBRARIES} )\n\n")

        if qo_h_sources.find('qo') >= 0:
            cml.write("qt4_wrap_cpp( outfiles_qoh\n")
            # not here, but below   cml.write(cc_sources + '\n')
            cml.write(qo_h_sources + '\n')
            cml.write('  )\n')
            #cml.write("message( the outfiles are ${outfiles})\n")
            
        if ui_sources.find('ui') >= 0:
            cml.write("\ncasa_qt4_wrap_ui( outfiles_ui\n")
            cml.write(ui_sources)
            cml.write('\n  )\n\n')
            #cml.write("message( the uioutfiles are ${outfiles_ui})\n")

        for s in qrc_sources:
            d = s[10:s.find('/', 10)]
            #print "Source is", s
            #print "Dir is", d
            cml.write("casa_qt4_add_resources( outfiles_qrc %s OPTIONS -name %s )\n" % (s, d))
        cml.write('\n')


    # moc creates double namespace object files, if this
    # is already defined when moc is run

    #cml.write('include_directories( ${CMAKE_SOURCE_DIR}/include )\n')
    #cml.write('message("just included ${casadbus_INCLUDE_DIRS}")\n')

    if mod == "plotms":
        cml.write('include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/implement/Gui ) ' +
                  '\n# because the generated code contain relative paths to the source tree\n\n')

    if mod == "display":
        cml.write('\ninclude_directories( ${CMAKE_CURRENT_BINARY_DIR}/QtAutoGui ) # for generated code\n\n')

    cml.write("casa_add_library( %s\n" % mod)
    if 'QT4' in deps[mod]:
        cml.write("  ${outfiles_qoh} ${outfiles_ui} ${outfiles_qrc}\n")
        cml.write(cc_sources + '\n')
    else:
        cml.write(cc_sources)
        cml.write('\n')
        cml.write(f_sources)
    if mod == "casadbus":
        cml.write('\nplotserver/PlotServerProxy.dbusproxy.h')
        cml.write('\nsession/DBusSession.dbusproxy.h')
        cml.write('\nviewer/ViewerProxy.dbusproxy.h')
    if mod == "stdcasa":
        cml.write(' version.cc\n')
    cml.write('  )\n\n')

    #cml.write('# Executables\n')
    for exe in exe_sources:

      if exe == "asdm2MS" or exe == "oldasdm2MS":

        sources = commands.getoutput(
            'find apps/%s -name \\*.cc | sort | xargs -n1 echo " "' % exe
            )
        cml.write('casa_add_executable( %s %s \n' % (mod, exe))
        cml.write(sources)
        cml.write('\n  )\n')
      else:
        sources = commands.getoutput(
            'find apps/%s -name \\*.cc | sort' % exe
            )
        cml.write('casa_add_executable( %s %s ' % (mod, exe))
        cml.write(sources)
        cml.write(' )\n')

    if mod == "casaqt":
        cml.write('\ninstall( PROGRAMS apps/qcasabrowser/casabrowser DESTINATION bin )\n')

    for t in test_sources:
        tname = os.path.basename(t).replace(".cc", "")
        if tname not in ["dDBusViewerProxy",
                         "tFlagger",
                         "tMsPlot"]:
            if mod == "flagging":
                cml.write('casa_add_test( %s %s )\n' % (mod, t))
            else:
                cml.write('casa_add_assay( %s %s )\n' % (mod, t))



    cml.close()
    os.chdir('..')
