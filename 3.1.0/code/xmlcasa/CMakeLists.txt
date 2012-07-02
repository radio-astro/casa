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


# Make sure that compile time definitions (build time, svn revision number)
# are recomputed in every build.

# This target is executed unconditionally and before any other target that
# depends on the definitions file
add_custom_target( xmlcasa_definitions COMMAND ${CMAKE_COMMAND} -E remove -f definitions )


# This is delegated to the cmake script code/install/casadef.cmake in
# order to allow setting environment variables portably.
add_custom_command( OUTPUT definitions
  COMMAND ${CMAKE_COMMAND} -Dcasadef_perl=${PERL_EXECUTABLE} -Dcasadef_source_dir=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/install/casadef.cmake > definitions_tmp
  COMMAND ${CMAKE_COMMAND} -E copy definitions_tmp definitions
)
# Using only shell redirection (>) to generate the definitions file, will cause
# the file to be empty until the command has finished executing (which is a
# feature of the shell), and thus trigger other make rules before the file is
# complete.

add_custom_target(
  xmlcasa_python_casadef
  COMMAND echo "import os" > casadef.py
  COMMAND echo "if os.environ.has_key\\\(\\'__CASAPY_TASKDIR\\'\\\):" >> casadef.py
  COMMAND echo "    task_directory = os.environ[\\'__CASAPY_TASKDIR\\']" >> casadef.py
  COMMAND echo "else:" >> casadef.py
  COMMAND echo "    task_directory = \\\"${PYTHON_TASKD_CASADEF}\\\"" >> casadef.py
  COMMAND echo "if os.environ.has_key\\\(\\'__CASAPY_PYTHONDIR\\'\\\):" >> casadef.py
  COMMAND echo "    python_library_directory = os.environ[\\'__CASAPY_PYTHONDIR\\']" >> casadef.py
  COMMAND echo "else:" >> casadef.py
  COMMAND echo "    python_library_directory = \\\"${PYTHON_LIBD_CASADEF}\\\"" >> casadef.py
  COMMAND echo "casa_version = \\\"${CASA_MAJOR_VERSION}.${CASA_MINOR_VERSION}.${CASA_PATCH_VERSION}\\\"" >> casadef.py
  COMMAND cat definitions >> casadef.py
  DEPENDS definitions
  )

add_dependencies( xmlcasa_python_casadef xmlcasa_definitions )

add_dependencies( inst xmlcasa_python_casadef )
add_dependencies( xmlcasa_fast xmlcasa_python_casadef )

install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/casadef.py DESTINATION python/${PYTHONV} )
install( PROGRAMS
       install/buildmytasks
       install/xmlgenpy
       DESTINATION bin )

install( FILES 
  install/casa2idl3.xsl
  install/casa2py.xsl
  install/casa2pyimp.xsl
  install/casa2summary.xsl
  install/casa2toolxml.xsl
  install/casa2tsum2.xsl
  install/casa2latex.xsl
  install/casa2pycli.xsl
  install/casa2pypg.xsl
  install/casa2tlatex.xsl
  install/casa2tsum.xsl
  install/casatlist.xsl
  DESTINATION ${CMAKE_INSTALL_PREFIX}/../share/xml )
install( FILES install/saxon8.jar DESTINATION lib )

# set SVNREVISION and BUILDTIME
set( casadef_quiet TRUE )
set( casadef_perl ${PERL_EXECUTABLE} )
set( casadef_source_dir ${CMAKE_SOURCE_DIR} )
include( ${CMAKE_SOURCE_DIR}/install/casadef.cmake )

add_custom_command( OUTPUT version.cc
  COMMAND egrep subversion_revision definitions > tmp1.txt
  COMMAND sed -e 's|[^0-9]*\"||g' tmp1.txt > tmp2.txt
  COMMAND echo "namespace casa { extern const int   aips_patch_version =" > tmp3.txt
  COMMAND cat tmp2.txt >> tmp3.txt
  COMMAND
  sed -e 's|MAJOR|${CASA_MAJOR_VERSION}|'
      -e 's|MINOR|${CASA_MINOR_VERSION}|'
      -e 's|PATCH|${CASA_PATCH_VERSION}|'
      -e 's|REVISION|${SVNREVISION}|'
      -e 's|DATE|${TAGGEDTIME}|'
      ${CMAKE_CURRENT_SOURCE_DIR}/implement/version.template > tmp4.txt
  COMMAND cat tmp3.txt tmp4.txt > version_tmp.cc
      || ${PERL_EXECUTABLE} -e 'unlink("version_tmp.cc")\; exit 1\;'
  COMMAND ${CMAKE_COMMAND} -E copy_if_different version_tmp.cc version.cc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/implement/version.template
  DEPENDS definitions
)

casa_add_library( xmlcasa
  implement/atmosphere/atmosphere_cmpt.cc
  implement/calibrater/calplot_cmpt.cc
  implement/casa/logsink_cmpt.cc
  implement/casa/quanta_cmpt.cc
  implement/casa/TSLogSink.cc
  implement/components/componentlist_cmpt.cc
  implement/flagging/autoflag_cmpt.cc
  implement/flagging/flagger_cmpt.cc
  implement/images/coordsys_cmpt.cc
  implement/images/image_cmpt.cc
  implement/images/imagepol_cmpt.cc
  implement/images/regionmanager_cmpt.cc
  implement/measures/measures_cmpt.cc
  implement/ms/ms_cmpt.cc
  implement/ms/msplot_cmpt.cc
  implement/nrao/vlafillertask_cmpt.cc
  implement/plotms/plotms_cmpt.cc
  implement/scimath/functional_cmpt.cc
  implement/StdCasa/CasacSupport.cc
  implement/StdCasa/conversions.cc
  implement/StdCasa/conversions_python.cc
  implement/StdCasa/interrupt.cc
  implement/StdCasa/record.cc
  implement/StdCasa/type_record.cc
  implement/StdCasa/type_variant.cc
  implement/StdCasa/value_record.cc
  implement/StdCasa/value_variant.cc
  implement/StdCasa/variant.cc
  implement/spectralline/spectralline_cmpt.cc
  implement/synthesis/calibrater_cmpt.cc
  implement/synthesis/deconvolver_cmpt.cc
  implement/synthesis/imager_cmpt.cc
  implement/synthesis/imagertask_cmpt.cc
  implement/synthesis/simulator_cmpt.cc
  implement/synthesis/vpmanager_cmpt.cc
  implement/tables/table_cmpt.cc
  implement/tables/tableindex_cmpt.cc
  implement/tables/tableiterator_cmpt.cc
  implement/tables/tableplot_cmpt.cc
  implement/tables/tablerow_cmpt.cc
  implement/utils/BaseInterface.cc
  implement/utils/CasapyWatcher.cc
  implement/utils/dbus_cmpt.cc
  implement/utils/stdBaseInterface.cc
  implement/utils/utils_cmpt.cc
  implement/version2.cc
  implement/xerces/asdmCasaSaxHandler.cc
  implement/xerces/asdmCasaXMLUtil.cc
  implement/xerces/stdcasaXMLUtil.cc
  version.cc
  )
add_dependencies( libxmlcasa_fast xmlcasa_definitions )
add_dependencies( libxmlcasa      xmlcasa_definitions )

casa_add_executable( xmlcasa casaparamgui apps/casaparamgui/casaparamgui.cc )
casa_add_executable( xmlcasa casapy apps/casapy/casapy.cc )
casa_add_assay( xmlcasa implement/xerces/test/tXMLToRecord.cc )
casa_add_test ( xmlcasa implement/flagging/test/tAutoflagger.cc )


casa_add_python( xmlcasa xmlcasa_python python/${PYTHONV}
  scripts/asap_init.py
  scripts/assignmentFilter.py
  scripts/casa_in_py.py
  scripts/casa.py
  scripts/casapy.py
  scripts/check_exceptionhandling.py
  scripts/check_input_saverestore.py
  scripts/cleanhelper.py
  scripts/dbusutil.py
  scripts/dict_to_table.py
  scripts/filecatalog.py
  scripts/fixvis_util.py
  scripts/get_user.py
  scripts/importarchive.py
  scripts/imregion.py
  scripts/listing.py
  scripts/mindpipes.py
  scripts/ngc5921_usecase.py
  scripts/odict.py
  scripts/parallel_go.py
  scripts/parameter_check.py
  scripts/parameter_dictionary.py
  scripts/pointcal.py
  scripts/qtcasapy.py
  scripts/regression.py
  scripts/regression_utility.py
  scripts/run_sdregress_fls3hi.py
  scripts/run_sdregress_irc.py
  scripts/run_sdregress_orions.py
  scripts/scopeit.py
  scripts/simutil.py
  scripts/TablePlotQt4Agg.py
  scripts/TablePlotTkAgg.py
  scripts/task_accum.py
  scripts/task_applycal.py
  scripts/task_autoclean.py
  scripts/task_bandpass.py
  scripts/task_blcal.py
  scripts/task_boxit.py
  scripts/task_browsetable.py
  scripts/task_calstat.py
  scripts/task_clean.py
  scripts/task_clearcal.py
  scripts/task_clearplot.py
  scripts/task_clearstat.py
  scripts/task_concat.py
  scripts/task_csvclean.py
  scripts/task_pclean.py
  scripts/task_cvel.py
  scripts/task_deconvolve.py
  scripts/task_exportasdm.py
  scripts/task_exportfits.py
  scripts/task_exportuvfits.py
  scripts/task_feather.py
  scripts/task_find.py
  scripts/task_fixvis.py
  scripts/task_flagautocorr.py
  scripts/task_flagcmd.py
  scripts/task_flagdata.py
  scripts/task_flagdata2.py
  scripts/task_flagmanager.py
  scripts/task_fluxscale.py
  scripts/task_fringecal.py
  scripts/task_ft.py
  scripts/task_gaincal.py
  scripts/task_gencal.py
  scripts/task_hanningsmooth.py
  scripts/task_help.py
  scripts/task_imcollapse.py
  scripts/task_imcontsub.py
  scripts/task_imfit.py
  scripts/task_imhead.py
  scripts/task_immath.py
  scripts/task_immoments.py
  scripts/task_importaipscaltable.py
  scripts/task_importasdm.py
  scripts/task_importevla.py
  scripts/task_importevla2.py
  scripts/task_importfits.py
  scripts/task_importfitsidi.py
  scripts/task_importgmrt.py
  scripts/task_importoldasdm.py
  scripts/task_importuvfits.py
  scripts/task_importvla.py
  scripts/task_imregrid.py
  scripts/task_imsmooth.py
  scripts/task_imstat.py
  scripts/task_imtrans.py
  scripts/task_imval.py
  scripts/taskinit.py
  scripts/task_listcal.py
  scripts/task_listhistory.py
  scripts/task_listobs.py
  scripts/task_listvis.py
  scripts/taskmanager.py
  scripts/task_mosaic.py
  scripts/task_newflagdata.py
  scripts/task_peel.py
  scripts/task_plotants.py
  scripts/task_plotcal.py
  scripts/task_plotms.py
  scripts/task_plotxy.py
  scripts/task_polcal.py
  scripts/task_rmtables.py
  scripts/task_sdaverage.py
  scripts/task_sdbaseline.py
  scripts/task_sdcal.py
  scripts/task_sdcoadd.py
  scripts/task_sdfit.py
  scripts/task_sdflag.py
  scripts/task_sdflagmanager.py
  scripts/task_sdimaging.py
  scripts/task_sdimprocess.py
  scripts/task_sdlist.py
  scripts/task_sdmath.py
  scripts/task_sdplot.py
  scripts/task_sdsave.py
  scripts/task_sdscale.py
  scripts/task_sdsmooth.py
  scripts/task_sdstat.py
  scripts/task_sdtpimaging.py
  scripts/task_setjy.py
  scripts/task_ssoflux.py
  scripts/task_simdata.py
  scripts/task_oldsimdata.py
  scripts/task_slsearch.py
  scripts/task_smoothcal.py
  scripts/task_specfit.py
  scripts/task_splattotable.py
  scripts/task_split.py
  scripts/task_uvcontsub2.py
  scripts/task_uvcontsub.py
  scripts/task_uvmodelfit.py
  scripts/task_uvsub.py
  scripts/task_viewer.py
  scripts/task_msview.py
  scripts/task_imview.py
  scripts/task_vishead.py
  scripts/task_visstat.py
  scripts/task_widefield.py
  scripts/TerminalController.py
  scripts/tget.py
  scripts/update_spw.py
  scripts/viewertool.py
  scripts/vishead_util.py
  scripts/task_msmoments.py )

casa_add_tasks( xmlcasa xmlcasa_tasks 
  tasks/accum.xml
  tasks/applycal.xml
  tasks/autoclean.xml
  tasks/bandpass.xml
  tasks/blcal.xml
  tasks/boxit.xml
  tasks/browsetable.xml
  tasks/calstat.xml
  tasks/clean.xml
  tasks/clearcal.xml
  tasks/clearplot.xml
  tasks/clearstat.xml
  tasks/concat.xml
  tasks/csvclean.xml
  tasks/pclean.xml
  tasks/cvel.xml
  tasks/deconvolve.xml
  tasks/exportasdm.xml
  tasks/exportfits.xml
  tasks/exportuvfits.xml
  tasks/feather.xml
  tasks/find.xml
  tasks/fixvis.xml
  tasks/flagautocorr.xml
  tasks/flagcmd.xml
  tasks/flagdata.xml
  tasks/flagdata2.xml
  tasks/flagmanager.xml
  tasks/fluxscale.xml
  tasks/fringecal.xml
  tasks/ft.xml
  tasks/gaincal.xml
  tasks/gencal.xml
  tasks/hanningsmooth.xml
  tasks/imcollapse.xml
  tasks/imcontsub.xml
  tasks/imfit.xml
  tasks/imhead.xml
  tasks/immath.xml
  tasks/immoments.xml
  tasks/importaipscaltable.xml
  tasks/importasdm.xml
  tasks/importevla.xml
  tasks/importevla2.xml
  tasks/importfits.xml
  tasks/importfitsidi.xml
  tasks/importgmrt.xml
  tasks/importoldasdm.xml
  tasks/importuvfits.xml
  tasks/importvla.xml
  tasks/imregrid.xml
  tasks/imsmooth.xml
  tasks/imstat.xml
  tasks/imtrans.xml
  tasks/imval.xml
  tasks/listcal.xml
  tasks/listhistory.xml
  tasks/listobs.xml
  tasks/listvis.xml
  tasks/mosaic.xml
  tasks/newflagdata.xml
  tasks/peel.xml
  tasks/plotants.xml
  tasks/plotcal.xml
  tasks/plotms.xml
  tasks/plotxy.xml
  tasks/polcal.xml
  tasks/rmtables.xml
  tasks/sdaverage.xml
  tasks/sdbaseline.xml
  tasks/sdcal.xml
  tasks/sdcoadd.xml
  tasks/sdfit.xml
  tasks/sdflag.xml
  tasks/sdflagmanager.xml
  tasks/sdimaging.xml
  tasks/sdimprocess.xml
  tasks/sdlist.xml
  tasks/sdmath.xml
  tasks/sdplot.xml
  tasks/sdsave.xml
  tasks/sdscale.xml
  tasks/sdsmooth.xml
  tasks/sdstat.xml
  tasks/sdtpimaging.xml
  tasks/setjy.xml
  tasks/ssoflux.xml
  tasks/simdata.xml
  tasks/oldsimdata.xml
  tasks/slsearch.xml
  tasks/smoothcal.xml
  tasks/specfit.xml
  tasks/splattotable.xml
  tasks/split.xml
  tasks/uvcontsub2.xml
  tasks/uvcontsub.xml
  tasks/uvmodelfit.xml
  tasks/uvsub.xml
  tasks/viewer.xml
  tasks/imview.xml
  tasks/msview.xml
  tasks/vishead.xml
  tasks/visstat.xml
  tasks/widefield.xml
  tasks/msmoments.xml )

casa_add_tools( tools_idl tools_sources 
  implement/atmosphere/atmosphere.xml
  implement/calibrater/calplot.xml
  implement/casa/logsink.xml
  implement/casa/quanta.xml
  implement/components/componentlist.xml
  implement/flagging/autoflag.xml
  implement/flagging/flagger.xml
  implement/images/coordsys.xml
  implement/images/imagepol.xml
  implement/images/image.xml
  implement/images/regionmanager.xml
  implement/measures/measures.xml
  implement/ms/msplot.xml
  implement/ms/ms.xml
  implement/nrao/vlafillertask.xml
  implement/plotms/plotms.xml
  implement/scimath/functional.xml
  implement/spectralline/spectralline.xml
  implement/synthesis/calibrater.xml
  implement/synthesis/deconvolver.xml
  implement/synthesis/imagertask.xml
  implement/synthesis/imager.xml
  implement/synthesis/simulator.xml
  implement/synthesis/vpmanager.xml
  implement/tables/tableindex.xml
  implement/tables/tableiterator.xml
  implement/tables/tableplot.xml
  implement/tables/tablerow.xml
  implement/tables/table.xml
  implement/utils/dbus.xml
  implement/utils/utils.xml )

# Handle source IDLs
set( outputs_idl "" )
casa_idl( outputs_idl idl/casatypes.idl Quantity)
casa_idl( outputs_idl idl/complex.idl complex )
casa_idl( outputs_idl idl/basicVecs.idl IntAry IntVec StringAry StringVec DoubleAry DoubleVec BoolAry BoolVec ComplexAry ComplexVec shape_type )

casa_pybinding( casac_sources ${CMAKE_CURRENT_SOURCE_DIR}/idl/casatypes.idl ${CMAKE_CURRENT_SOURCE_DIR}/idl/complex.idl ${CMAKE_CURRENT_SOURCE_DIR}/idl/basicVecs.idl ${tools_idl} )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/impl)
# for generated C++ bindings

add_definitions( -pthread -fno-strict-aliasing -DNDEBUG ) # may or may not be necessary for compiling python modules

casa_add_pymodule( casac ${tools_sources} ${outputs_idl} ${casac_sources} )
casa_add_pymodule( paramgui apps/module_paramgui/paramgui_python.cc )
casa_add_pymodule( interrupt modules/interrupt/interrupt_python.cc )

# Make sure that all IDLs are up to date before considering casac
# This has the desired effects that
# - Dependencies of the C++ sources are calculated after the IDLs are created
#   (IDLs might #include each other)
# - When ccmtools is invoked, all the required IDL (which may not be explicit
#   cmake dependencies for the ccmtools command) exist.
add_custom_target( idl SOURCES ${tools_idl} )
add_custom_target( generated_sources SOURCES ${tools_sources} ${outputs_idl} )
add_dependencies( generated_sources idl )
add_dependencies( libxmlcasa generated_sources )

add_subdirectory( scripts/tests )
add_subdirectory( scripts/regressions )
add_subdirectory( scripts/regressions/tests )
add_subdirectory( scripts/regressions/admin )
add_subdirectory( scripts/usecases )
add_subdirectory( scripts/demos )
add_subdirectory( scripts/recipes )
add_subdirectory( scripts/parallel )

# Documentation targets, which depend on the contents of casa_out_* being set
# Latex target
if( casa_out_latex )
  add_custom_target( doc_latex DEPENDS ${casa_out_latex} )
else()
  add_custom_target( doc_latex COMMAND echo "No LaTeX to generate!" )
endif()

# HTML target
if( casa_out_html )
  add_custom_target( doc_html DEPENDS ${casa_out_html} )
else()
  add_custom_target( doc_html COMMAND echo "LaTeX to HTML converter was not found." )
endif()

# PDF target
if( casa_out_pdf )
  add_custom_target( doc_pdf DEPENDS ${casa_out_pdf} )
else()
  add_custom_target( doc_pdf COMMAND echo "PDF to HTML converter was not found." )
endif()
