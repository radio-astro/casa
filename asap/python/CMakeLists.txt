###
# CMakeLists.txt for python modules
###

# root for python
set( PYTHONROOT ${ASAP_SOURCE_DIR} )
set( PYTHONDIR ${PYTHONROOT}/python )

# python modules 
set( PYTHON_MODULES
     ${PYTHONDIR}/__init__.py
     ${PYTHONDIR}/asapfit.py
     ${PYTHONDIR}/asapfitter.py
     ${PYTHONDIR}/asapgrid.py
     ${PYTHONDIR}/asaplinefind.py
     ${PYTHONDIR}/asaplot.py
     ${PYTHONDIR}/asaplotbase.py
     ${PYTHONDIR}/asaplotgui.py 
     ${PYTHONDIR}/asaplotgui_gtk.py
     ${PYTHONDIR}/asaplotgui_qt4.py
     ${PYTHONDIR}/asapmath.py
     ${PYTHONDIR}/asapplotter.py
     ${PYTHONDIR}/compatibility.py
     ${PYTHONDIR}/coordinate.py
     ${PYTHONDIR}/customgui_base.py
     ${PYTHONDIR}/customgui_qt4agg.py
     ${PYTHONDIR}/customgui_tkagg.py
     ${PYTHONDIR}/env.py
     ${PYTHONDIR}/flagplotter.py
     ${PYTHONDIR}/interactivemask.py
     ${PYTHONDIR}/ipysupport.py
     ${PYTHONDIR}/lagflagger.py
     ${PYTHONDIR}/linecatalog.py
     ${PYTHONDIR}/logging.py
     ${PYTHONDIR}/opacity.py
     ${PYTHONDIR}/parameters.py
     ${PYTHONDIR}/plotter2.py
     ${PYTHONDIR}/sbseparator.py
     ${PYTHONDIR}/scantable.py
     ${PYTHONDIR}/selector.py
     ${PYTHONDIR}/simplelinefinder.py
     ${PYTHONDIR}/edgemarker.py
     ${PYTHONDIR}/svninfo.txt
     ${PYTHONDIR}/utils.py  )

install( PROGRAMS ${PYTHON_MODULES} 
         DESTINATION ${PYTHON_INSTALL_DIR} )
