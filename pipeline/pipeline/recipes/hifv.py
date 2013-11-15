# General imports

import os
import sys
import traceback
import inspect

# Make sure CASA exceptions are rethrown
try:
    if not  __rethrow_casa_exceptions:
        def_rethrow = False
    else:
        def_rethrow = __rethrow_casa_exceptions
except:
    def_rethrow = False

__rethrow_casa_exceptions = True



# Setup paths
sys.path.insert (0, os.path.expandvars("$SCIPIPE_HEURISTICS"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/h/cli/mytasks.py"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hif/cli/mytasks.py"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hifv/cli/mytasks.py"))

# Pipeline imports
import pipeline.infrastructure.casatools as casatools

#IMPORT_ONLY = 'Import only'

# Run the procedure
def hifv (vislist, importonly=False, pipelinemode='automatic', interactive=True):

    echo_to_screen = interactive
    casatools.post_to_log ("Beginning pipeline run ...")

    try:
	# Initialize the pipeline
        h_init()

        # Load the data
        hifv_importdata (vis=vislist, pipelinemode=pipelinemode)
	if importonly:
	   raise Exception(IMPORT_ONLY)
    
        # Flag known bad data
        hifv_flagdata (pipelinemode=pipelinemode)
    
        # Fill model columns for primary calibrators
        hifv_setmodel (pipelinemode=pipelinemode)
        
        # Gain curves, opacities, antenna position corrections, 
        # requantizer gains (NB: requires CASA 4.1!)
        hifv_priorcals (pipelinemode=pipelinemode)
    
        # Compute the prioritized lists of reference antennas
        hif_refant (pipelinemode=pipelinemode)
    
        # Initial test calibrations using bandpass and delay calibrators
        hifv_testBPdcals (pipelinemode=pipelinemode)
    
	# Identify and flag basebands with bad deformatters or rfi based on 
        # bp table amps and phases
        hifv_flagbaddef (pipelinemode=pipelinemode)

	# Flag spws that have no calibration at this point
        hifv_uncalspw(pipelinemode=pipelinemode, delaycaltable='testdelay.k', bpcaltable='testBPcal.b')

	# Flag possible RFI on BP calibrator using rflag
        hifv_checkflag(pipelinemode=pipelinemode)
	
	# DO SEMI-FINAL DELAY AND BANDPASS CALIBRATIONS
        # (semi-final because we have not yet determined the spectral index
        # of the bandpass calibrator)
        hifv_semiFinalBPdcals(pipelinemode=pipelinemode)

	# Use flagdata rflag mode again on calibrators
        hifv_checkflag(pipelinemode=pipelinemode, checkflagmode='semi')
    
        # Re-run semi-final delay and bandpass calibrations
        hifv_semiFinalBPdcals(pipelinemode=pipelinemode)
        
        # Flag spws that have no calibration at this point
        hifv_uncalspw(pipelinemode=pipelinemode, delaycaltable='delay.k', bpcaltable='BPcal.b')
    
        # Determine solint for scan-average equivalent
        hifv_solint(pipelinemode=pipelinemode)
    
        # Do test gain calibrations to establish short solint
        hifv_testgains (pipelinemode=pipelinemode)
    
        # Make gain table for flux density bootstrapping
        # Make a gain table that includes gain and opacity corrections for final
        # amp cal, for flux density bootstrapping
        hifv_fluxgains (pipelinemode=pipelinemode)
    
        # Do the flux density boostrapping -- fits spectral index of
        # calibrators with a power-law and puts fit in model column
        hifv_fluxboot (pipelinemode=pipelinemode)
    
        # Make the final calibration tables
        hifv_finalcals (pipelinemode=pipelinemode)
    
        # Apply all the calibrations and check the calibrated data
        hifv_applycals (pipelinemode=pipelinemode)
    
        # Flag spws that have no calibration at this point
        hifv_uncalspw(pipelinemode=pipelinemode, delaycaltable='finaldelay.k', bpcaltable='finalBPcal.b')
    
        # Now run all calibrated data, including the target, through rflag
        hifv_targetflag (pipelinemode=pipelinemode)
    
        # Calculate data weights based on standard deviation within each spw
        hifv_statwt(pipelinemode=pipelinemode)
    
    except Exception, e:
	if str(e) == IMPORT_ONLY:
	    casatools.post_to_log ("Exiting after import step ...",
	        echo_to_screen=echo_to_screen)
	else:
	    casatools.post_to_log ("Error in procedure execution ...",
	        echo_to_screen=echo_to_screen)
	    errstr = traceback.format_exc()
	    casatools.post_to_log (errstr, echo_to_screen=echo_to_screen)

    finally:

        # Save the results to the context
        h_save()

	casatools.post_to_log ("VLA CASA Pipeline finished.  Terminating procedure execution ...",
	    echo_to_screen=echo_to_screen)

	# Restore previous state
	__rethrow_casa_exceptions = def_rethrow

# Run the procedure
def hifv_withplots (vislist, importonly=False, pipelinemode='automatic', interactive=True):

    echo_to_screen = interactive
    casatools.post_to_log ("Beginning pipeline run ...")

    try:
	# Initialize the pipeline
        h_init()

        # Load the data
        hifv_importdata (vis=vislist, pipelinemode=pipelinemode)
	if importonly:
	   raise Exception(IMPORT_ONLY)
    
        # Flag known bad data
        hifv_flagdata (pipelinemode=pipelinemode)
    
        # Fill model columns for primary calibrators
        hifv_setmodel (pipelinemode=pipelinemode)
        
        # Gain curves, opacities, antenna position corrections, 
        # requantizer gains (NB: requires CASA 4.1!)
        hifv_priorcals (pipelinemode=pipelinemode)
    
        # Compute the prioritized lists of reference antennas
        hif_refant (pipelinemode=pipelinemode)
    
        # Initial test calibrations using bandpass and delay calibrators
        hifv_testBPdcals (pipelinemode=pipelinemode)
        h_save()
        execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/testBPdcals_plots.py")
    
	# Identify and flag basebands with bad deformatters or rfi based on 
        # bp table amps and phases
        hifv_flagbaddef (pipelinemode=pipelinemode)

	# Flag spws that have no calibration at this point
        hifv_uncalspw(pipelinemode=pipelinemode, delaycaltable='testdelay.k', bpcaltable='testBPcal.b')

	# Flag possible RFI on BP calibrator using rflag
        hifv_checkflag(pipelinemode=pipelinemode)
	
	# DO SEMI-FINAL DELAY AND BANDPASS CALIBRATIONS
        # (semi-final because we have not yet determined the spectral index
        # of the bandpass calibrator)
        hifv_semiFinalBPdcals(pipelinemode=pipelinemode)
        h_save()
        execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/semifinalBPdcals_plots1.py")

	# Use flagdata rflag mode again on calibrators
        hifv_checkflag(pipelinemode=pipelinemode, checkflagmode='semi')
    
        # Re-run semi-final delay and bandpass calibrations
        hifv_semiFinalBPdcals(pipelinemode=pipelinemode)
        h_save()
        execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/semifinalBPdcals_plots2.py")
        
        # Flag spws that have no calibration at this point
        hifv_uncalspw(pipelinemode=pipelinemode, delaycaltable='delay.k', bpcaltable='BPcal.b')
    
        # Determine solint for scan-average equivalent
        hifv_solint(pipelinemode=pipelinemode)
    
        # Do test gain calibrations to establish short solint
        hifv_testgains (pipelinemode=pipelinemode)
        h_save()
        execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/testgains_plots.py")
    
        # Make gain table for flux density bootstrapping
        # Make a gain table that includes gain and opacity corrections for final
        # amp cal, for flux density bootstrapping
        hifv_fluxgains (pipelinemode=pipelinemode)
    
        # Do the flux density boostrapping -- fits spectral index of
        # calibrators with a power-law and puts fit in model column
        hifv_fluxboot (pipelinemode=pipelinemode)
        h_save()
        execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/fluxboot_plots.py")
    
        # Make the final calibration tables
        hifv_finalcals (pipelinemode=pipelinemode)
        h_save()
        execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/finalcals_plots.py")
    
        # Apply all the calibrations and check the calibrated data
        hifv_applycals (pipelinemode=pipelinemode)
    
        # Flag spws that have no calibration at this point
        hifv_uncalspw(pipelinemode=pipelinemode, delaycaltable='finaldelay.k', bpcaltable='finalBPcal.b')
    
        # Now run all calibrated data, including the target, through rflag
        hifv_targetflag (pipelinemode=pipelinemode)
        h_save()
        execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/targetflag_plots.py")
    
        # Calculate data weights based on standard deviation within each spw
        hifv_statwt(pipelinemode=pipelinemode)
    
    except Exception, e:
	if str(e) == IMPORT_ONLY:
	    casatools.post_to_log ("Exiting after import step ...",
	        echo_to_screen=echo_to_screen)
	else:
	    casatools.post_to_log ("Error in procedure execution ...",
	        echo_to_screen=echo_to_screen)
	    errstr = traceback.format_exc()
	    casatools.post_to_log (errstr, echo_to_screen=echo_to_screen)

    finally:

        # Save the results to the context
        h_save()

	casatools.post_to_log ("VLA CASA Pipeline finished with plotting.  Terminating procedure execution ...",
	    echo_to_screen=echo_to_screen)

	# Restore previous state
	__rethrow_casa_exceptions = def_rethrow