import os
import time
from taskinit import *

def plotms(vis=None, xaxis=None, xdatacolumn=None, yaxis=None,
           ydatacolumn=None, selectdata=None, field=None, spw=None,
           timerange=None, uvrange=None, antenna=None, scan=None,
           correlation=None, array=None, msselect=None, averagedata=None,
           avgchannel=None, avgtime=None, avgscan=None, avgfield=None,
           avgbaseline=None, avgantenna=None, avgspw=None, extendflag=None,
           extcorrelation=None, extchannel=None, extspw=None, extantenna=None,
           exttime=None, extscans=None, extfield=None, plotxycomp=None,
           datacolumn=None, timebin=None, crossscans=None, crossbls=None,
           width=None, extendcorr=None, extendchan=None, extendspw=None,
           extendant=None, extendtime=None):
    """
    
            Task for plotting and interacting with visibility data.  A variety
        of axes choices (including data column) along with MS selection and
        averaging options are provided for data selection.  Flag extension
        parameters are also available for flagging operations in the plotter.
        
            All of the provided parameters can also be set using the GUI once
        the application has been launched.  Additional and more specific
        operations are available through the GUI and/or through the plotms
        tool (pm).
        

    Keyword arguments:
    vis -- input visibility dataset
           default: ''
    xaxis, yaxis -- what to plot on the two axes
                    default: '' (uses PlotMS defaults/current set).
      &gt;&gt;&gt; xaxis, yaxis expandable parameters
        xdatacolumn, ydatacolumn -- which data column to use for data axes
                                    default: '' (uses PlotMS default/current
                                    set).
    
    selectdata -- data selection parameters flag
                  (see help par.selectdata for more detailed information)
                  default: False
      &gt;&gt;&gt; selectdata expandable parameters
        field -- select using field ID(s) or field name(s)
                 default: '' (all).
        spw -- select using spectral window/channels
               default: '' (all)
        timerange -- select using time range
                     default: '' (all).
        uvrange -- select using uvrange
                   default: '' (all).
        antenna -- select using antenna/baseline
                   default: '' (all).
        scan -- select using scan number
                default: '' (all).
        correlation -- select using correlations
                       default: '' (all).
        array -- select using (sub)-array range
                 default: '' (all).
        msselect -- TaQL selection expression
                    default: '' (all).
    
    averagedata -- data averaing parameters flag
                   default: False.
      &gt;&gt;&gt; averagedata expandable parameters
        avgchannel -- average over channel?  either blank for none, or a value
                      in channels.
                      default: '' (none).
        avgtime -- average over time?  either blank for none, or a value in
                   seconds.
                   default: '' (none).
        avgscan -- average over scans?  only valid if time averaging is turned
                   on.
                   default: False.
        avgfield -- average over fields?  only valid if time averaging is
                    turned on.
                    default: False.
        avgbaseline -- average over all baselines?  mutually exclusive with
                       avgantenna.
                       default: False.
        avgantenna -- average by per-antenna?  mutually exclusive with
                      avgbaseline.
                      default: False.
        avgspw -- average over all spectral windows?
                  default: False.
    
    extendflag -- have flagging extend to other data points?
                  default: False.
      &gt;&gt;&gt; extendflag expandable parameters
        extcorrelation -- extend flags based on correlation?  blank = none.
                          default: ''.
        extchannel -- extend flags based on channel?
                      default: False.
        extspw -- extend flags based on spw?
                  default: False.
        extantenna -- extend flags based on antenna?  should be either blank,
                      'all' for all baselines, or an antenna-based value.
                      default: ''.
        exttime -- extend flags based on time (within scans)?
                   default: False.
        extscans -- extend flags based on scans?  only valid if time extension
                    is turned on.
                    default: False.
        extfield -- extend flags based on field?  only valid if time extension
                    is turned on.
                    default: False.
    
    plotxycomp -- use plotxy compliance parameters?  WARNING: if used,
                  subparameters will override their corresponding plotms
                  parameter values.
                  default: False.
      &gt;&gt;&gt; plotxycomp expandable parameters
        datacolumn -- maps to xdatacolumn and ydatacolumn.
                      default: 'data'.
        timebin -- maps to avgtime.
                   default: '0'.
        crossscans -- maps to avgscan.
                      default: False.
        crossbls -- maps to avgbaseline.
                    default: False.
        width -- maps to avgchannel and/or avgspw.  'all' is ignored, 'allspw'
                 sets avgspw to true, any other value is set for avgchannel.
                 default: '1'.
        extendcorr -- maps to extcorrelation.  'half' is mapped to 'poln-dep'.
                      default: ''.
        extendchan -- maps to extchannel.  '' means False, 'all' means True.
                      default: ''.
        extendspw -- maps to extspw.  '' means False, 'all' means True.
                     default: ''.
        extendant -- maps to extantenna.  '' means False, 'all' means True.
                     default: ''.
        extendtime -- maps to exttime and/or extscans and/or extfield.  ''
                      means False for all, 'all' means True for exttime, 'scan'
                      means True for exttime and extscans, 'field' means True
                      for exttime and extfield.
                      default: ''.
    
    """

    try:
        # Check plotxy compliance parameters
        if plotxycomp:
            xdatacolumn = datacolumn
            ydatacolumn = datacolumn
            
            avgtime = timebin
            avgscan = crossscans
            avgbaseline = crossbls
            if width != 'all' and width != 'allspw': avgchannel = width
            avgspw = width == 'allspw'
            
            extcorrelation = extendcorr
            if(extcorrelation == 'half'): extcorrelation = 'poln-dep'
                
            extchannel = extendchan == 'all'
            extspw = extendspw == 'all'
            extantenna = extendant
            
            exttime = extendtime == 'all' or extendtime == 'scan' or extendtime == 'field'
            extscan = extendtime == 'scan'
            extfield = extendtime == 'field'
        
        # Check synonyms
        synonyms = {}
        synonyms['timeinterval'] = synonyms['timeint'] = 'time_interval'
        synonyms['chan'] = 'channel'
        synonyms['freq'] = 'frequency'
        synonyms['corr'] = 'correlation'
        synonyms['ant1'] = 'antenna1'
        synonyms['ant2'] = 'antenna2'
        synonyms['uvdistl'] = 'uvdist_l'
        synonyms['amplitude'] = 'amp'
        synonyms['imaginary'] = 'imag'
        synonyms['ant'] = 'antenna'
        synonyms['parallacticangle'] = 'parang'
        
        if(synonyms.has_key(xaxis)): xaxis = synonyms[xaxis]
        if(synonyms.has_key(yaxis)): yaxis = synonyms[yaxis]
        
        # Set filename and axes
        pm.setPlotMSFilename(vis, False)
        pm.setPlotAxes(xaxis, yaxis, xdatacolumn, ydatacolumn, False)
        
        # Set selection
        if not selectdata:
            field = spw = timerange = uvrange = antenna = scan = correlation = array = msselect = ''
            
        pm.setPlotMSSelection(field, spw, timerange, uvrange, antenna, scan, correlation, array, msselect, False)
        
        # Set averaging
        if not averagedata:
            avgchannel = avgtime = ''
            avgscan = avgfield = avgbaseline = avgantenna = avgspw = False
                        
        pm.setPlotMSAveraging(avgchannel, avgtime, avgscan, avgfield, avgbaseline, avgantenna, avgspw, False)
        
        # Set flag extension
        pm.setFlagExtension(extendflag, extcorrelation, extchannel, extspw, extantenna, exttime, extscans, extfield)
        
        # Update and show
        pm.update()
        pm.show()
    
    except Exception, instance:
        print "Exception during plotms task: ", instance
