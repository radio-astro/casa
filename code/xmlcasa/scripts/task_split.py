import os
import string
from taskinit import *

def split(vis, outputvis, datacolumn, field, spw, width, antenna,
          timebin, timerange, scan, array, uvrange):
    """Create a visibility subset from an existing visibility set:

    Keyword arguments:
    vis -- Name of input visibility file (MS)
            default: none; example: vis='ngc5921.ms'
    outputvis -- Name of output visibility file (MS)
                  default: none; example: outputvis='ngc5921_src.ms'
    datacolumn -- Which data column to split out
                  default='corrected'; example: datacolumn='data'
                  Options: 'data', 'corrected', 'model', 'all',
                  'float_data', 'lag_data', 'float_data,data', and
                  'lag_data,data'.
                  note: 'all' = whichever of the above that are present.
    field -- Field name
              default: field = '' means  use all sources
              field = 1 # will get field_id=1 (if you give it an
                          integer, it will retrieve the source with that index)
              field = '1328+307' specifies source '1328+307'.
                 Minimum match can be used, egs  field = '13*' will
                 retrieve '1328+307' if it is unique or exists.
                 Source names with imbedded blanks cannot be included.
    spw -- Spectral window index identifier
            default=-1 (all); example: spw=1
    antenna -- antenna names
               default '' (all),
               antenna = '3 & 7' gives one baseline with antennaid = 3,7.
    timebin -- Value for time averaging
               default='-1s' (no averaging); example: timebin='30s'
    timerange -- Time range
                 default='' means all times.  examples:
                 timerange = 'YYYY/MM/DD/hh:mm:ss~YYYY/MM/DD/hh:mm:ss'
                 timerange='< YYYY/MM/DD/HH:MM:SS.sss'
                 timerange='> YYYY/MM/DD/HH:MM:SS.sss'
                 timerange='< ddd/HH:MM:SS.sss'
                 timerange='> ddd/HH:MM:SS.sss'
    scan -- Scan numbers to select.
            default '' (all).
    array -- (Sub)array IDs to select.     
             default '' (all).
    uvrange -- uv distance range to select.
               default '' (all).
    """

    #Python script

    try:
        casalog.origin('split')

        if ((type(vis)==str) & (os.path.exists(vis))):
            ms.open(vis, nomodify=True)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
        if os.path.exists(outputvis):
            ms.close()
            raise Exception, "Output MS %s already exists - will not overwrite." % outputvis

        # No longer needed.  When did it get put in?  Note that the default
        # spw='*' in ms.split ends up as '' since the default type for a variant
        # is BOOLVEC.  (Of course!)  Therefore both split and ms.split must
        # work properly when spw=''.
        #if(spw == ''):
        #    spw = '*'
        
        if(type(antenna) == list):
            antenna = ', '.join([str(ant) for ant in antenna])

        ## Accept digits without units ...assume seconds
        timebin = qa.convert(qa.quantity(timebin), 's')['value']
        timebin = str(timebin) + 's'
        
        if timebin == '0s':
            timebin = '-1s'
            
        if type(width) == str:
            try:
                if(width.isdigit()):
                    width=[string.atoi(width)]
                elif(width.count('[') == 1 and width.count(']') == 1):
                    width = width.replace('[', '')
                    width = width.replace(']', '')
                    splitwidth = width.split(',')
                    width = []
                    for ws in splitwidth:
                        if(ws.isdigit()):
                            width.append(string.atoi(ws)) 
                else:
                    width = [1]
            except:
                raise TypeError, 'parameter width is invalid..using 1'

        do_chan_avg = spw.find('^') > -1     # '0:2~11^1' would be pointless.
        if not do_chan_avg:                  # ...look in width.
            if type(width) == int and width > 1:
                do_chan_avg = True
            elif hasattr(width, '__iter__'):
                for w in width:
                    if w > 1:
                        do_chan_avg = True
                        break

        do_both_chan_and_time_avg = (do_chan_avg and
                                     string.atof(timebin[:-1]) > 0.0)
        if do_both_chan_and_time_avg:
            # Do channel averaging first because it might be included in the spw
            # string.
            import tempfile
            cavms = tempfile.mkdtemp(suffix=outputvis)

            casalog.post('Channel averaging to ' + cavms)
            ms.split(outputms=cavms,     field=field,
                     spw=spw,            step=width,
                     baseline=antenna,   subarray=array,
                     timebin='',         time=timerange,
                     whichcol=datacolumn,
                     scan=scan,          uvrange=uvrange)
            
            # The selection was already made, so blank them before time averaging.
            field = ''
            spw = ''
            width = [1]
            antenna = ''
            array = ''
            timerange = ''
            datacolumn = 'all'
            scan = ''
            uvrange = ''

            ms.close()
            ms.open(cavms)
            casalog.post('Starting time averaging')
            
        ms.split(outputms=outputvis, field=field,
                 spw=spw,            step=width,
                 baseline=antenna,   subarray=array,
                 timebin=timebin,    time=timerange,
                 whichcol=datacolumn,
                 scan=scan,          uvrange=uvrange)
        ms.close()

        if do_both_chan_and_time_avg:
            import shutil
            shutil.rmtree(cavms)
        
        #the history should go to splitted ms, not the source ms
        # Write history to output MS
        ms.open(outputvis, nomodify=False)
        ms.writehistory(message='taskname=split', origin='split')
        ms.writehistory(message='vis         = "'+str(vis)+'"',
                        origin='split')
        ms.writehistory(message='outputvis   = "'+str(outputvis)+'"',
                        origin='split')
        ms.writehistory(message='field       = "'+str(field)+'"',
                        origin='split')
        ms.writehistory(message='spw       = '+str(spw), origin='split')
        ms.writehistory(message='width       = '+str(width), origin='split')
        ms.writehistory(message='antenna     = "'+str(antenna)+'"',
                        origin='split')
        ms.writehistory(message='timebin     = "'+str(timebin)+'"',
                        origin='split')
        ms.writehistory(message='timerange   = "'+str(timerange)+'"',
                        origin='split')
        ms.writehistory(message='datacolumn  = "'+str(datacolumn)+'"',
                        origin='split')
        ms.close()


    except Exception, instance:
        #print '*** Error ***',instance
        raise Exception, instance

