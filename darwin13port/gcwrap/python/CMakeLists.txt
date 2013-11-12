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

#casa_add_executable(tableplot projtable  apps/projtable/projtable.cc)
casa_add_python( python  casa_python python/${PYTHONV}
scripts/asap_init.py
scripts/assignmentFilter.py
scripts/callibrary.py
scripts/casa_in_py.py
scripts/casa.py
scripts/casapy.py
scripts/casapy_engine.py
scripts/caltab_convert2.py
scripts/check_exceptionhandling.py
scripts/check_input_saverestore.py
scripts/cleanhelper.py
scripts/correct_ant_posns.py
scripts/dbusutil.py
scripts/dict_to_table.py
scripts/filecatalog.py
scripts/filltsys.py
scripts/flaghelper.py
scripts/get_user.py
scripts/importarchive.py
scripts/listing.py
scripts/mindpipes.py
scripts/ngc5921_usecase.py
scripts/odict.py
scripts/parallel_go.py
scripts/parallelSplit.py
scripts/parameter_check.py
scripts/parameter_dictionary.py
scripts/partitionhelper.py
scripts/pCASA.py
scripts/plotcomp.py
scripts/pointcal.py
scripts/predictcomp_helper.py
scripts/regression.py
scripts/regressframe.py
scripts/regression_utility.py
scripts/refimagerhelper.py
scripts/run_sdregress_fls3hi.py
scripts/run_sdregress_irc.py
scripts/run_sdregress_orions.py
scripts/sdutil.py
scripts/setjy_helper.py
scripts/simutil.py
scripts/solar_system_setjy.py
scripts/testhelper.py
scripts/TablePlotQt4Agg.py
scripts/TablePlotTkAgg.py
scripts/task_accum.py
scripts/task_applycal.py
scripts/task_autoclean.py
scripts/task_asdmsummary.py
scripts/task_bandpass.py
scripts/task_blcal.py
scripts/task_boxit.py
scripts/task_browsetable.py
scripts/task_calstat.py
scripts/task_caltabconvert.py
scripts/task_clean.py
scripts/task_gnuclean.py
scripts/task_clearcal.py
scripts/task_clearplot.py
scripts/task_clearstat.py
scripts/task_concat.py
scripts/task_conjugatevis.py
scripts/task_csvclean.py
scripts/task_pclean.py
scripts/task_cvel.py
scripts/task_deconvolve.py
scripts/task_delmod.py
scripts/task_exportasdm.py
scripts/task_exportfits.py
scripts/task_exportuvfits.py
scripts/task_feather.py
scripts/task_find.py
scripts/task_fixplanets.py
scripts/task_fixvis.py
scripts/task_flagcmd.py
scripts/task_flagdata.py
scripts/task_flagmanager.py
scripts/task_fluxscale.py
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
scripts/task_impbcor.py
scripts/task_importasdm.py
scripts/task_importevla.py
scripts/task_importfits.py
scripts/task_importfitsidi.py
scripts/task_importgmrt.py
scripts/task_importuvfits.py
scripts/task_importvla.py
scripts/task_impv.py
scripts/task_imreframe.py
scripts/task_imregrid.py
scripts/task_imsmooth.py
scripts/task_imstat.py
scripts/task_imsubimage.py
scripts/task_imtrans.py
scripts/task_imval.py
scripts/taskinit.py
scripts/task_listcal.py
scripts/task_listhistory.py
scripts/task_listfits.py
scripts/task_listobs.py
scripts/task_listpartition.py
scripts/task_listsdm.py
scripts/task_listvis.py
scripts/taskmanager.py
scripts/task_makemask.py 
scripts/task_mosaic.py
scripts/task_msmoments.py
scripts/task_mstransform.py
scripts/task_plotants.py
scripts/task_plotbandpass.py
scripts/task_plotcal.py
scripts/task_plotms.py
scripts/task_plotuv.py
scripts/task_plotweather.py
scripts/task_plotxy.py
scripts/task_partition.py
scripts/task_polcal.py
scripts/task_predictcomp.py
scripts/task_rmfit.py
scripts/task_rmtables.py
scripts/task_sdbaseline.py
scripts/task_sdcal.py
scripts/task_sdcal2.py
scripts/task_sdcoadd.py
scripts/task_sdfit.py
scripts/task_sdflag.py
scripts/task_sdflag2.py
scripts/task_sdflagmanager.py
scripts/task_sdgrid.py
scripts/task_sdimaging.py
scripts/task_sdimprocess.py
scripts/task_sdlist.py
scripts/task_sdmath.py
scripts/task_sdplot.py
scripts/task_sdreduce.py
scripts/task_sdsave.py
scripts/task_sdscale.py
scripts/task_sdsmooth.py
scripts/task_sdstat.py
scripts/task_sdtpimaging.py
scripts/task_setjy.py
scripts/task_ssoflux.py
scripts/task_simalma.py
scripts/task_simobserve.py
scripts/task_simanalyze.py
scripts/task_slsearch.py
scripts/task_smoothcal.py
scripts/task_specfit.py
scripts/task_splattotable.py
scripts/task_split.py
scripts/task_spxfit.py
scripts/task_statwt.py
scripts/task_tclean.py
scripts/task_testconcat.py
scripts/task_uvcontsub.py
scripts/task_uvcontsub3.py
scripts/task_uvmodelfit.py
scripts/task_uvsub.py
scripts/task_viewer.py
scripts/task_virtualconcat.py
scripts/task_wvrgcal.py
scripts/task_msview.py
scripts/task_imview.py
scripts/task_vishead.py
scripts/task_visstat.py
scripts/task_widebandpbcor.py
scripts/task_widefield.py
scripts/TerminalController.py
scripts/tget.py
scripts/update_spw.py
scripts/viewertool.py
scripts/vishead_util.py
scripts/simple_cluster.py
)

add_subdirectory( scripts/tests )
add_subdirectory( scripts/regressions )
add_subdirectory( scripts/regressions/tests )
add_subdirectory( scripts/regressions/admin )
add_subdirectory( scripts/usecases )
add_subdirectory( scripts/demos )
add_subdirectory( scripts/recipes )
add_subdirectory( scripts/parallel )
