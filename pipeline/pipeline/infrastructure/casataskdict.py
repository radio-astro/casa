import textwrap

import pipeline.hif.tasks as hif_tasks
import pipeline.hsd.tasks as hsd_tasks
import pipeline.hifa.tasks as hifa_tasks
import pipeline.hifv.tasks as hifv_tasks

CasaTaskDict = {
                   'hif_antpos'       : 'Antpos',
                   'hif_atmflag'      : 'Atmflag',
                   'hif_applycal'     : 'Applycal',
                   'hif_bandpass'     : 'Bandpass',
                   'hif_bpflagchans'  : 'Bandpassflagchans',
		   'hif_clean'        : 'Clean', 
		   'hif_cleanlist'    : 'CleanList', 
                   'hif_exportdata'   : 'ExportData',
                   'hif_flagchans'    : 'Flagchans',
                   'hif_gaincal'      : 'Gaincal',
                   'hif_lowgainflag'  : 'Lowgainflag',
                   'hif_importdata'   : 'ImportData',
		   'hif_makecleanlist': 'MakeCleanList', 
                   'hif_refant'       : 'RefAnt',
		   'hif_restoredata'  : 'RestoreData',
                   'hif_setjy'        : 'Setjy',
                   'hsd_calsky'       : 'SDCalSky',
                   'hsd_caltsys'      : 'SDCalTsys',
                   'hsd_applycal'     : 'SDApplyCal',
                   'hsd_exportdata'   : 'SDExportData',
                   'hsd_importdata'   : 'SDImportData',
                   'hsd_importdata2'  : 'SDImportData2',
                   'hsd_convertdata'  : 'SDConvertData',
                   'hsd_inspectdata'  : 'SDInspectData',
                   'hsd_imaging'      : 'SDImaging',
                   'hsd_imaging_old'  : 'SDImagingOld',
                   'hsd_baseline_old' : 'SDBaselineOld',
                   'hsd_baseline'     : 'SDBaseline',
                   'hsd_flagdata'     : 'SDFlagData',
                   'hsd_flagbaseline' : 'SDFlagBaseline',
                   'hsd_plotflagbaseline': 'SDPlotFlagBaseline',
                   'hsd_reduce'       : 'SDReduction',
                   'hifa_importdata' : 'ALMAImportData',
                   'hifa_flagdata'    : 'FlagDeterALMA',
                   'hifa_fluxcalflag'  : 'FluxcalFlag',
                   'hifa_gfluxscale'   : 'GcorFluxscale',
                   'hifa_linpolcal'    : 'Linpolcal',
                   'hifa_timegaincal' : 'TimeGaincal',
                   'hifa_tsyscal'      : 'Tsyscal',
                   'hifa_tsysflag'     : 'Tsysflag',
                   'hifa_tsysflagchans': 'Tsysflagchans',
                   'hifa_tsysflagspectra': 'Tsysflagspectra',
                   'hifa_wvrgcal'      : 'Wvrgcal',
                   'hifa_wvrgcalflag'  : 'Wvrgcalflag',
                   'hifv_importdata'  : 'VLAImportData',
                   'hifv_flagdata'    : 'FlagDeterVLA',
                   'hifv_setmodel'    : 'SetModel',
                   'hifv_vlasetjy'    : 'VLASetjy',
                   'hifv_priorcals'   : 'Priorcals',
                   'hifv_hflag'       : 'Heuristicflag',
                   'hifv_testBPdcals' : 'testBPdcals',
                   'hifv_flagbaddef'  : 'FlagBadDeformatters',
                   'hifv_uncalspw'    : 'Uncalspw',
                   'hifv_checkflag'   : 'Checkflag',
                   'hifv_semiFinalBPdcals' : 'semiFinalBPdcals',
                   'hifv_solint'      : 'Solint',
                   'hifv_testgains'   : 'Testgains',
                   'hifv_fluxgains'   : 'Fluxgains',
                   'hifv_fluxboot'    : 'Fluxboot',
                   'hifv_finalcals'   : 'Finalcals',
                   'hifv_applycals'   : 'Applycals',
                   'hifv_targetflag'  : 'Targetflag',
                   'hifv_statwt'      : 'Statwt'
               }



classToCASATask = {
    # ALMA interferometry tasks ---------------------------------------------
    hifa_tasks.ALMAImportData : 'hifa_importdata',
    hifa_tasks.FlagDeterALMA : 'hifa_flagdata',
    hifa_tasks.FluxcalFlag : 'hifa_fluxcalflag',
    hifa_tasks.GcorFluxscale : 'hifa_gfluxscale',
    hifa_tasks.Linpolcal : 'hifa_linpolcal',
    hifa_tasks.TimeGaincal : 'hifa_timegaincal',
    hifa_tasks.Tsyscal : 'hifa_tsyscal',
    hifa_tasks.Tsysflag : 'hifa_tsysflag',
    hifa_tasks.Tsysflagchans : 'hifa_tsysflagchans',
    hifa_tasks.Tsysflagspectra : 'hifa_tsysflagspectra',
    hifa_tasks.Wvrgcal : 'hifa_wvrgcal',
    hifa_tasks.Wvrgcalflag : 'hifa_wvrgcalflag',
    # Interferometry tasks ---------------------------------------------------
    hif_tasks.Antpos : 'hif_antpos',
    hif_tasks.Applycal : 'hif_applycal',    
    hif_tasks.Atmflag : 'hif_atmflag',
    hif_tasks.Bandpass : 'hif_bandpass',
    hif_tasks.Bandpassflagchans : 'hif_bpflagchans',
    hif_tasks.Clean : 'hif_clean',
    hif_tasks.CleanList : 'hif_cleanlist',
    hif_tasks.ExportData : 'hif_exportdata',
    hif_tasks.Flagchans : 'hif_flagchans',
    hif_tasks.Fluxcal : 'hif_fluxcal',
    hif_tasks.Fluxscale : 'hif_fluxscale',
    hif_tasks.Gaincal : 'hif_gaincal',
    hif_tasks.ImportData : 'hif_importdata',
    hif_tasks.Lowgainflag : 'hif_lowgainflag',
    hif_tasks.MakeCleanList : 'hif_makecleanlist',
    hif_tasks.RefAnt : 'hif_refant',
    hif_tasks.RestoreData : 'hif_restoredata',
    hif_tasks.Setjy : 'hif_setjy',
    # Single dish tasks ------------------------------------------------------
    hsd_tasks.SDCalSky : 'hsd_calsky',
    hsd_tasks.SDCalTsys : 'hsd_caltsys',
    hsd_tasks.SDApplyCal : 'hsd_applycal',
    hsd_tasks.SDExportData : 'hsd_exportdata',
    hsd_tasks.SDImportData : 'hsd_importdata',
    hsd_tasks.SDImportData2 : 'hsd_importdata2',
    hsd_tasks.SDConvertData : 'hsd_convertdata',
    hsd_tasks.SDInspectData : 'hsd_inspectdata',
    hsd_tasks.SDImaging : 'hsd_imaging',
    hsd_tasks.SDImagingOld : 'hsd_imaging_old',
    hsd_tasks.SDBaselineOld : 'hsd_baseline_old',
    hsd_tasks.SDBaseline : 'hsd_baseline',
    hsd_tasks.SDFlagData : 'hsd_flagdata',
    hsd_tasks.SDFlagBaseline : 'hsd_flagbaseline',
    hsd_tasks.SDPlotFlagBaseline : 'hsd_plotflagbaseline',
    hsd_tasks.SDReduction : 'hsd_reduce',
    #VLA tasks
    hifv_tasks.VLAImportData       : 'hifv_importdata',
    hifv_tasks.FlagDeterVLA        : 'hifv_flagdata',
    hifv_tasks.SetModel            : 'hifv_setmodel',
    hifv_tasks.VLASetjy            : 'hifv_vlasetjy',
    hifv_tasks.Priorcals           : 'hifv_priorcals',
    hifv_tasks.Heuristicflag       : 'hifv_hflag', 
    hifv_tasks.testBPdcals         : 'hifv_testBPdcals',
    hifv_tasks.FlagBadDeformatters : 'hifv_flagbaddef',
    hifv_tasks.Uncalspw            : 'hifv_uncalspw',
    hifv_tasks.Checkflag           : 'hifv_checkflag',
    hifv_tasks.semiFinalBPdcals    : 'hifv_semiFinalBPdcals',
    hifv_tasks.Solint              : 'hifv_solint', 
    hifv_tasks.Testgains           : 'hifv_testgains',
    hifv_tasks.Fluxgains           : 'hifv_fluxgains',
    hifv_tasks.Fluxboot            : 'hifv_fluxboot', 
    hifv_tasks.Finalcals           : 'hifv_finalcals',
    hifv_tasks.Applycals           : 'hifv_applycals',
    hifv_tasks.Targetflag          : 'hifv_targetflag',
    hifv_tasks.Statwt              : 'hifv_statwt' 
}


SILENT_TASK_COMMENT = (
    'This stage performs a pipeline calculation without running any CASA '
    'commands to be put in this file.'
)

CASA_COMMANDS_PROLOGUE = (
    'This file contains CASA commands run by the pipeline. Although all '
    'commands required to calibrate the data are included here, this file '
    'cannot be executed, nor does it contain heuristic and flagging '
    'calculations performed by pipeline code. This file is useful to '
    'understand which CASA commands are being run by each pipeline task. If '
    'one wishes to re-run the pipeline, one should use the pipeline script '
    'linked on the front page of the weblog. Some stages may not have any '
    'commands listed here, e.g. hifa_importdata if conversion from ASDM to MS '
    'is not required.'
)

TASK_COMMENTS = {
    (hif_tasks.ImportData,
     hifa_tasks.ALMAImportData, 
     hifv_tasks.VLAImportData): (
        'If required, ASDMs are converted to measurement sets.'
    ),
    (hifa_tasks.ALMAAgentFlagger,
     hifa_tasks.FlagDeterALMA): (
        'Flags generated by the online telescope software, by the QA0 '
        'process, and manually set by the pipeline user.'
    ),
    (hifa_tasks.FluxcalFlag,) : (
        SILENT_TASK_COMMENT
    ),
    (hif_tasks.RefAnt,) : (
        'Antennas are prioritized and enumerated based on fraction flagged '
        'and position in the array. The best antenna is used as a reference '
        'antenna unless it gets flagged, in which case the next-best '
        'antenna is used.\n'
        '' + SILENT_TASK_COMMENT
    ),
    (hifa_tasks.Tsyscal,) : (
        'The Tsys calibration and spectral window map is computed.'
    ),
    (hifa_tasks.Tsysflag,) : (
        'The Tsys calibration table is analyzed and deviant points are flagged.'
    ),
    (hifa_tasks.Wvrgcalflag,): (
        'Water vapour radiometer corrections are calculated for each antenna. '
        'The quality of the correction is assessed by comparing a phase gain '
        'solution calculated with and without the WVR correction. This '
        'requires calculation of a temporary phase gain on the bandpass '
        'calibrator, a temporary bandpass using that temporary gain, followed '
        'by phase gains with the temporary bandpass, with and without the WVR '
        'correction. After that, some antennas are wvrflagged (so that their '
        'WVR corrections are interpolated), and then the quality of the '
        'correction recalculated.'
    ),                              
    (hif_tasks.Lowgainflag,): (
        'Sometimes antennas have significantly lower gain than nominal. Even '
        'when calibrated, it is better for ALMA data to flag these antennas. '
        'The pipeline detects this by calculating a long solint amplitude '
        'gain on the bandpass calibrator.  First, temporary phase and '
        'bandpass solutions are calculated, and then that temporary bandpass '
        'is used to calculate a short solint phase and long solint amplitude '
        'solution.'
    ),
    (hif_tasks.Setjy,) : (
        'If the amplitude calibrator is a resolved solar system source, this '
        'uses a subset of antennas with short baselines (where the resolved '
        'source model is of highest quality).'
    ),
    (hif_tasks.PhcorBandpass,) : (
        'The spectral response of each antenna is calibrated. A short-solint '
        'phase gain is calculated to remove decorrelation of the bandpass '
        'calibrator before the bandpass is calculated.'
    ),
    (hif_tasks.Bandpassflagchans,) : (
        'Very sharp features (e.g. the cores of strong atmospheric lines, '
        'and rare instrumental artifacts) in the bandpass solution are '
        'flagged.'
    ),  
    (hifa_tasks.GcorFluxscale,) : (
        'The absolute flux calibration is transferred to secondary calibrator '
        'sources.'
    ),
    (hifa_tasks.TimeGaincal,) : (
        'Time dependent gain calibrations are computed. '
    ),
    (hif_tasks.Applycal,) : (
        'Calibrations are applied  to the data. Final flagging summaries '
	'are computed'
    ),
    (hif_tasks.MakeCleanList,) : (
        'A list of target sources to be imaged is constructed. '
    ),
    (hif_tasks.CleanList,) : (
        'A list of target sources is cleaned. '
    ),
    (hif_tasks.ExportData,) : (
        'The output data products are computed. '
    ),
    # Single Dish Tasks
    (hsd_tasks.SDImportData,) : (
        'If required, ASDMs are converted to measurement sets. '
        'Furthermore, measurement sets are converted to scantable '
        'and split by antenna for single dish data reduction. '
    ),
    (hsd_tasks.SDInspectData,) : (
        'Registered data are inspected to configure calibration strategy '
        'and to group data for the following processing. \n'
        '' + SILENT_TASK_COMMENT
    ),
    (hsd_tasks.SDCalSky,) : (
        'Generates sky calibration table according to calibration '
        'strategy. '
    ),
    (hsd_tasks.SDCalTsys,) : (
        'Generates Tsys calibration table according to calibration '
        'strategy. '
    ),
    (hsd_tasks.SDApplyCal,) : (
        'Apply calibration tables. '
    ),
    (hsd_tasks.SDBaseline,) : (
        'Subtracts spectral baseline by least-square fitting with '
        'N-sigma clipping. Spectral lines are automatically detected '
        'and examined to determine the region that is masked to protect '
        'these features from the fit. \n'
        '' + SILENT_TASK_COMMENT
    ),
    (hsd_tasks.SDFlagData,) : (
        'Perform row-based flagging based on noise level and quality of '
        'spectral baseline subtraction. \n'
        '' + SILENT_TASK_COMMENT
    ),
    (hsd_tasks.SDImaging,) : (
        'Executes preparation for imaging: Exports scantables to '
        'measurement sets, apply row flags, and apply weights. '
        'Then, perform imaging.'
    ),
    (hsd_tasks.SDFlagBaseline,) : (
        'Execute baseline subtraction and flagging iteratively. '
        '' + SILENT_TASK_COMMENT
    ),
    (hsd_tasks.SDPlotFlagBaseline,) : (
        'Plot whole spectra before and after baseline subtraction. '
        '' + SILENT_TASK_COMMENT
    ),
}


def get_task_comment(task):
    """
    Get the casalog comment for the given task.
    """
    comment = ''
    for task_classes, task_comment in TASK_COMMENTS.items():
        if task.__class__ in task_classes:
            wrapped = textwrap.wrap('# ' + task_comment,
                                    subsequent_indent='# ',
                                    width=78,
                                    break_long_words=False)
            comment += '%s\n#\n' % '\n'.join(wrapped)
    
    return comment
