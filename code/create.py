#!/usr/bin/python

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

modules = ["graphics", "casaqt", "msvis", "plotms", "display", "casadbus", "tableplot", "flagging", "calibration", "synthesis", "alma", "oldalma", "atnf", "dish", "nrao", "xmlcasa"]
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
        "xmlcasa" : ['CASACORE', 'CCMTOOLS', 'PYTHON', 'ATM', 'READLINE', 'plotms', 'dish', 'synthesis', 'flagging', 'nrao'],
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
    cml = open("./CMakeLists.txt", "w")

    if mod in ["alma", "oldalma"]:
        cml.write("add_definitions( -DWITHOUT_ACS )\n\n")
        cml.write("if( APPLE )\n")
        cml.write("set( CMAKE_CXX_FLAGS\n")
        cml.write("     \"${CMAKE_CXX_FLAGS} -march=i586\" )\n")
        cml.write("endif()\n\n")

    if mod == "casadbus":
        cml.write("casa_add_dbus_interface( " +
                  "plotserver/PlotServerProxy.dbusproxy.h " +
                  "implement/plotserver/PlotServerProxy.dbusproxy.xml )\n")
        cml.write("casa_add_dbus_interface( " +
                  "session/DBusSession.dbusproxy.h " +
                  "implement/session/DBusSession.dbusproxy.xml )\n")
        cml.write("casa_add_dbus_interface( " +
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
        'find implement -name \\*.cc | sort | grep -w test | grep -v test.dMultichannelRaster.cc | grep -v test.dRGBTestPattern.cc | grep -v test.dRGBWCRaster.cc | grep -v test.dPanelDisplay.cc | grep -v test.dQtPlotter.cc | grep -v test.tArrayAsRaster.cc | grep -v test.dLatticeAsRaster.cc | grep -v test.dNBody.cc | grep -v test.tTblAsRaster.cc | grep -v test.dProfile2dDD.cc | grep -v test.tTblAsXY.cc | grep -v test.tTblAsContour.cc | grep -v test.tArrayAndDrawing.cc | grep -v test.dLatticeAsVector.cc | grep -v test.tApplicator.cc | grep -v test.tcal_cmpt2.cc | grep -v test.tcal_cmpt.cc | grep -v test.AtmosphereTest.cc'
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
    if mod == 'xmlcasa':
        # QT_USE_FILE also calls add_definitions, therefore do not call it at the highest level
        # necessary?? cml.write("include( ${QT_USE_FILE} )\n")
        cml.write("add_custom_command( OUTPUT version.cc COMMAND " + \
        "${CMAKE_SOURCE_DIR}/install/doover > version.cc || ${PERL_EXECUTABLE} -e 'unlink(\\\"version.cc\\\")\\; exit 1\\;' " + \
        "DEPENDS ${CMAKE_SOURCE_DIR}/VERSION )\n")

    if 'QT4' in deps[mod]:
        cml.write("include( ${QT_USE_FILE} )\n")
        #cml.write("message( the QTlibraries are ${QT_LIBRARIES})\n")
        cml.write("set( QT4_LIBRARIES ${QT_LIBRARIES} )\n")

        if qo_h_sources.find('qo') >= 0:
            cml.write("qt4_wrap_cpp( outfiles\n")
            # not here, but below   cml.write(cc_sources + '\n')
            cml.write(qo_h_sources + '\n')
            cml.write('\n)\n')
            #cml.write("message( the outfiles are ${outfiles})\n")
            
        if ui_sources.find('ui') >= 0:
            cml.write("casa_qt4_wrap_ui( outfiles_ui\n")
            cml.write(ui_sources)
            cml.write('\n)\n')
            #cml.write("message( the uioutfiles are ${outfiles_ui})\n")

        for s in qrc_sources:
            d = s[10:s.find('/', 10)]
            #print "Source is", s
            #print "Dir is", d
            cml.write("casa_qt4_add_resources( outfile %s OPTIONS -name %s )\n" % (s, d))
            cml.write("set( outfiles_qrc ${outfiles_qrc} ${outfile} )\n")


    # moc creates double namespace object files, if this
    # is already defined when moc is run

    #cml.write('include_directories( ${CMAKE_SOURCE_DIR}/include )\n')
    #cml.write('message("just included ${casadbus_INCLUDE_DIRS}")\n')

    cml.write("add_definitions( ${%s_DEFINITIONS} )\n" % mod)

    if mod == "plotms":
        cml.write('include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/implement/Gui ) ' +
                  '\n# because the generated code contain relative paths to the source tree\n\n')

    if mod == "display":
        cml.write('include_directories( ${CMAKE_CURRENT_BINARY_DIR}/QtAutoGui ) # for generated code\n')

    cml.write('include_directories( ${%s_INCLUDE_DIRS} )\n\n' % mod)

    cml.write('# Dynamic library\n')
    cml.write('add_library( %s\n' % mod)
    if 'QT4' in deps[mod]:
        cml.write(cc_sources + '\n')
        cml.write("${outfiles} ${outfiles_ui} ${outfiles_qrc}\n")
    else:
        cml.write(cc_sources)
        cml.write('\n')
        cml.write(f_sources)
    if mod == "casadbus":
        cml.write('\nplotserver/PlotServerProxy.dbusproxy.h')
        cml.write('\nsession/DBusSession.dbusproxy.h')
        cml.write('\nviewer/ViewerProxy.dbusproxy.h')
    if mod == "xmlcasa":
        cml.write('\nversion.cc')
    cml.write(')\n\n')

    cml.write('target_link_libraries( %s ${%s_LINK_TO} )\n\n' % (mod, mod) )
    cml.write('set_target_properties( %s PROPERTIES SOVERSION ${CASA_API_VERSION} )\n\n' % mod)
    cml.write('install( TARGETS %s LIBRARY DESTINATION lib )\n\n' % mod)






    cml.write('# Executables\n')
    for exe in exe_sources:
        sources = commands.getoutput(
            'find apps/%s -name \\*.cc | sort ' % exe
            )
        cml.write('add_executable( %s\n' % exe)
        cml.write(sources)
        cml.write(')\n')
        cml.write('target_link_libraries( %s %s )\n\n' % (exe, mod))

        cml.write('install( TARGETS %s RUNTIME DESTINATION bin )\n\n' % exe)
        
        cml.write('if( APPLE )\n')
        cml.write('  install( TARGETS %s RUNTIME DESTINATION apps/%s.app/Contents/MacOS )\n' % (exe, exe))
        cml.write('endif()\n')


    if mod == "casaqt":
        cml.write('install( PROGRAMS apps/qcasabrowser/casabrowser DESTINATION bin )\n')


    cml.write('# Unit tests\n')
    for t in test_sources:
        tname = os.path.basename(t).replace(".cc", "")
        if tname not in ["dDBusViewerProxy",
                         "tFlagger",
                         "tMsPlot"]:
            cml.write('add_executable( %s EXCLUDE_FROM_ALL %s )\n' % (tname, t))
            cml.write('target_link_libraries( %s %s )\n' % (tname, mod))
            cml.write('add_test( %s ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/%s )\n' % (tname, tname))
            cml.write('add_dependencies( check %s )\n' % tname)
            cml.write('\n')



    if py_sources.find('py') >= 0 and mod != "msvis":
        cml.write('#\n# Python\n#\n\n')
        cml.write('casa_add_python( %s_python python/${PYTHONV}\n' % mod )
        cml.write(py_sources)
        cml.write(' )\n\n')


    if xmltask_sources.find('xml') >= 0:
        cml.write('#\n# XML tasks\n#\n\n')
        cml.write('casa_add_tasks( %s_tasks \n' %mod )
        cml.write(xmltask_sources)
        cml.write(' )\n\n')

    if xmltool_sources.find('xml') >= 0:
        cml.write('#\n# XML tools\n#\n\n')
        cml.write('casa_tool( tool_idl tool_sources \n' )
        cml.write(xmltool_sources)
        cml.write(' )\n\n')


    if mod == "xmlcasa":
        cml.write('#\n# IDL\n#\n\n')
        cml.write('set( outputs_idl "" )\n')
        cml.write('casa_idl( outputs_idl idl/casatypes.idl Quantity)\n')
        cml.write('casa_idl( outputs_idl idl/complex.idl complex )\n')
        cml.write('casa_idl( outputs_idl idl/basicVecs.idl IntAry IntVec StringAry StringVec DoubleAry DoubleVec BoolAry BoolVec ComplexAry ComplexVec shape_type )\n')
        cml.write('\n\n')

        cml.write('casa_binding( binding_sources ${CMAKE_CURRENT_SOURCE_DIR}/idl/casatypes.idl ${CMAKE_CURRENT_SOURCE_DIR}/idl/complex.idl ${CMAKE_CURRENT_SOURCE_DIR}/idl/basicVecs.idl ${tool_idl} )\n\n')

        cml.write('include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/impl)\n' + \
                  '# for generated C++ bindings\n\n')
        cml.write('add_definitions( -pthread -fno-strict-aliasing -DNDEBUG ) # gcc specific\n')
        cml.write('add_library( casac MODULE ${tool_sources} ${outputs_idl} ${binding_sources} ')
        cml.write(' )\n\n')
        cml.write('set_target_properties( casac PROPERTIES PREFIX "" )\n\n\n')

        cml.write('target_link_libraries( casac xmlcasa ${xmlcasa_LINK_TO} )\n\n')

        cml.write('\n# This is for dependency generation to work\n')
        cml.write('add_custom_target( idl SOURCES ${tool_idl} )\n')
        cml.write('add_dependencies( casac idl )\n')

        
        cml.write('\n#\n#\n# Other modules \n#\n#\n')
        for m in [("apps/module_paramgui/paramgui_python.cc", "paramgui"), ("modules/interrupt/interrupt_python.cc", "interrupt")]:
            cml.write('add_library( ' + m[1] + ' MODULE ' + m[0] + ' )\n')
            cml.write('set_target_properties( ' + m[1] + ' PROPERTIES PREFIX "" )\n')
            cml.write('target_link_libraries( ' + m[1] + ' xmlcasa ${xmlcasa_LINK_TO} )\n')

        
        cml.write('install( TARGETS casac interrupt paramgui LIBRARY DESTINATION python/${PYTHONV} )\n\n')
        
        for d in [("scripts/regressions",       "python/${PYTHONV}/regressions", "xmlcasa_regressions"),
                  ("scripts/regressions/tests", "python/${PYTHONV}/regressions/tests", "xmlcasa_regressions_tests"),
                  ("scripts/usecases",          "python/${PYTHONV}/regressions", "xmlcasa_usecases"),
                  ("scripts/demos",             "python/${PYTHONV}/regressions", "xmlcasa_demos")]:

            cml.write('\n\nadd_subdirectory( %s )\n' % d[0])
            cmlr = open(d[0]+"/CMakeLists.txt", "w")
            cmlr.write('casa_add_python( %s_python %s\n' % (d[2], d[1]))
            
            sources = commands.getoutput(
                'cd ' + d[0] + ' && /bin/ls -1 *.py | sort | xargs -n1 echo " "'
                )
            cmlr.write(sources)
            cmlr.write('\n)\n')


    cml.close()
    os.chdir('..')
