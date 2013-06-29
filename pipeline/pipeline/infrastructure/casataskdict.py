import pipeline.hif.tasks as hif_tasks

CasaTaskDict = {
                   'hif_antpos'       : 'Antpos',
                   'hif_atmflag'      : 'Atmflag',
                   'hif_applycal'     : 'Applycal',
                   'hif_bandpass'     : 'Bandpass',
		   'hif_cleanlist'    : 'CleanList', 
                   'hif_exportdata'   : 'ExportData',
                   'hif_flagdata'     : 'FlagDeterALMA',
                   'hif_fluxcalflag'  : 'FluxcalFlag',
                   'hif_gaincal'      : 'Gaincal',
                   'hif_newgaincal'   : 'NewGaincal',
                   'hif_gfluxscale'   : 'GcorFluxscale',
                   'hif_lowgainflag'  : 'Lowgainflag',
                   'hif_importdata'   : 'ImportData',
		   'hif_makecleanlist': 'MakeCleanList', 
                   'hif_refant'       : 'RefAnt',
		   'hif_restoredata'  : 'RestoreData',
                   'hif_setjy'        : 'Setjy',
                   'hif_timegaincal'  : 'TimeGaincal',
                   'hif_tsyscal'      : 'Tsyscal',
                   'hif_tsysflag'     : 'Tsysflag',
                   'hif_tsysflagchans': 'Tsysflagchans',
                   'hif_wvrgcal'      : 'Wvrgcal',
                   'hif_wvrgcalflag'  : 'Wvrgcalflag',
                   'hsd_calsky'       : 'SDCalSky',
                   'hsd_caltsys'      : 'SDCalTsys',
                   'hsd_applycal'     : 'SDApplyCal',
                   'hsd_exportdata'   : 'SDExportData',
                   'hsd_importdata'   : 'SDImportData',
                   'hsd_inspectdata'  : 'SDInspectData',
                   'hsd_imaging'      : 'SDImaging',
                   'hsd_baseline'     : 'SDBaseline',
                   'hsd_reduce'       : 'SDReduction'
               }



ClassDict = {
    hif_tasks.Antpos : 'hif_antpos',
    hif_tasks.Applycal : 'hif_applycal',    
    hif_tasks.Atmflag : 'hif_atmflag',
    hif_tasks.Bandpass : 'hif_bandpass',
    hif_tasks.CleanList : 'hif_cleanlist',
    hif_tasks.ExportData : 'hif_exportdata',
    hif_tasks.FlagDeterALMA : 'hif_flagdata',    
    hif_tasks.Fluxcal : 'hif_fluxcal',
    hif_tasks.FluxcalFlag : 'hif_fluxcalflag',
    hif_tasks.Fluxscale : 'hif_fluxscale',
    hif_tasks.Gaincal : 'hif_gaincal',
    hif_tasks.GcorFluxscale : 'hif_gfluxscale',
    hif_tasks.ImportData : 'hif_importdata',
    hif_tasks.Lowgainflag : 'hif_lowgainflag',
    hif_tasks.MakeCleanList : 'hif_makecleanlist',
    hif_tasks.NewGaincal : 'hif_newgaincal',
    hif_tasks.RefAnt : 'hif_refant',
    hif_tasks.RestoreData : 'hif_restoredata',
    hif_tasks.Setjy : 'hif_setjy',
    hif_tasks.TimeGaincal : 'hif_timegaincal',
    hif_tasks.Tsyscal : 'hif_tsyscal',
    hif_tasks.Tsysflag : 'hif_tsysflag',
    hif_tasks.Tsysflagchans : 'hif_tsysflagchans',
    hif_tasks.Wvrgcal : 'hif_wvrgcal',
    hif_tasks.Wvrgcalflag : 'hif_wvrgcalflag',
    hif_tasks.AgentFlagger : 'hif_flagdata'
}
