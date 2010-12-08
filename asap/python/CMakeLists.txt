###
# CMakeLists.txt for python modules
###

# root for python
set( PYTHONROOT ${ASAP_SOURCE_DIR} )
set( PYTHONDIR ${PYTHONROOT}/python )

# python modules 
set( PYTHON_MODULES
     ${PYTHONDIR}/asapfit.py
     ${PYTHONDIR}/asapfitter.py
     ${PYTHONDIR}/asaplinefind.py
     ${PYTHONDIR}/asaplotbase.py
     ${PYTHONDIR}/asaplotgui_gtk.py
     ${PYTHONDIR}/asaplotgui.py 
     ${PYTHONDIR}/asaplotgui_qt4.py
     ${PYTHONDIR}/asaplot.py
     ${PYTHONDIR}/asapmath.py
     ${PYTHONDIR}/asapplotter.py
     ${PYTHONDIR}/casatoolbar.py
     ${PYTHONDIR}/compatibility.py
     ${PYTHONDIR}/coordinate.py
     ${PYTHONDIR}/env.py
     ${PYTHONDIR}/__init__.py
     ${PYTHONDIR}/interactivemask.py
     ${PYTHONDIR}/ipysupport.py
     ${PYTHONDIR}/lagflagger.py
     ${PYTHONDIR}/linecatalog.py
     ${PYTHONDIR}/logging.py
     ${PYTHONDIR}/notationwindow.py
     ${PYTHONDIR}/opacity.py
     ${PYTHONDIR}/parameters.py
     ${PYTHONDIR}/scantable.py
     ${PYTHONDIR}/selector.py
     ${PYTHONDIR}/simplelinefinder.py
     ${PYTHONDIR}/utils.py )

install( PROGRAMS ${PYTHON_MODULES} 
         DESTINATION ${PYTHON_INSTALL_DIR} )
