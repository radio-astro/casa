# This task is designed such that the user does not need to know the
# internal structure of the MS in order to change important bits of information.
# (A user with advanced knowlege of the MS can use the tb tool instead.)
# To accomplish this, I use a keyword dictionary that relates a "keyword" to
# an MS subtable and column.  The keywords should correspond to FITS keywords
# whenever possible.
#
# The name vishead is a misnomer because casa tables do not have headers in the 
# sense that FITS files do.  The name derives from the IMHEAD, GETHEAD
# and PUTHEAD verbs in AIPS.
#
# Implementation: The task open/closes the MS for each keyword
# to read or write (and an extra time to check existence before reading).
# This is not very effective (in list mode, or when using this task in a loop;
# however expert users can always use the table tool directly.

from taskinit import *
from vishead_util import *

def vishead(vis, mode=None, listitems=None, hdkey=None, hdindex=None, hdvalue=None):
    """Documentation goes here?"""

    casalog.origin('vishead')
    casalog.post("parameter vis:       " + str(vis), 'DEBUG1' )
    casalog.post("parameter mode:      " + str(mode), 'DEBUG1')
    casalog.post("parameter listitems: " + str(listitems), 'DEBUG1')
    casalog.post("parameter hdkey:     " + str(hdkey), 'DEBUG1')
    casalog.post("parameter hdindex:   " + str(hdindex), 'DEBUG1')
    casalog.post("parameter hdvalue:   " + str(hdvalue), 'DEBUG1')

    # Define vishead keywords
    keywords = {
        # Keywords from OBSERVATION TABLE
        # time_range
        'log':       ['OBSERVATION', 'LOG',      strip_r1], #OPT AoAoS
        'schedule'  :['OBSERVATION', 'SCHEDULE', strip_r1], #array
        # flag_row
        'observer' :['OBSERVATION', 'OBSERVER'],  #array
        'project'  :['OBSERVATION', 'PROJECT'],   #array
        'release_date'  :['OBSERVATION', 'RELEASE_DATE', secArray2localDate], #array
        'schedule_type'  :['OBSERVATION', 'SCHEDULE_TYPE'], #array
        'telescope':['OBSERVATION', 'TELESCOPE_NAME'],  #array

        
        # Keywords from FIELD TABLE
        'field'    :['FIELD', 'NAME'],   # also in pointing table(!)
                                        # so watch out when changing
        'ptcs':     ['FIELD', 'PHASE_DIR', dict2direction_strs],   #OPT
        'fld_code': ['FIELD', 'CODE'],        #OPT

        # Keywords from SPECTRAL_WINDOW TABLE
        #   not sure if all of these can freely edited by
        #   the user without messing up the MS' internal consistency...
        
        #don't allow change  'meas_freq_ref' :['SPECTRAL_WINDOW', 'MEAS_FREQ_REF'],
        #don't allow change  'ref_frequency' :['SPECTRAL_WINDOW', 'REF_FREQUENCY'],
        
        #per channel 'chan_freq' :['SPECTRAL_WINDOW', 'CHAN_FREQ'],

        #per channel 'chan_width' :['SPECTRAL_WINDOW', 'CHAN_WIDTH'],

        #per channel 'effective_bw' :['SPECTRAL_WINDOW', 'EFFECTIVE_BW'],
        #'resolution' :['SPECTRAL_WINDOW', 'RESOLUTION'],
        'freq_group_name' :['SPECTRAL_WINDOW', 'FREQ_GROUP_NAME'],
        # don't allow change: 'total_bandwidth' :['SPECTRAL_WINDOW', 'TOTAL_BANDWIDTH'],        
        'spw_name' :['SPECTRAL_WINDOW', 'NAME'],

        # Keywords from SOURCE TABLE
        'source_name' :['SOURCE', 'NAME'],  #OPT
        'cal_grp':     ['SOURCE', 'CALIBRATION_GROUP']  #OPT

        #MS_VERSION (probably not)
    }
    
    try:
        # In list mode, list the keywords.
        if mode == 'list' or mode == '':

            #tb.open(vis)
            #tb.summary()

            values = {}
            if not listitems:
                listitems = keywords.keys()
                listitems.sort()
            for key in listitems:
                if keywords.has_key(key):
                    kwtuple = keywords.get(key)
                    if keyword_exists(vis, kwtuple):
                        values[key] = getput_keyw('get', vis, kwtuple, '')
                        if len(kwtuple) > 2:
                            casalog.post(key + ': ' + str(kwtuple[2](values[key])),
                                         'INFO')
                        else:
                            casalog.post(key + ': ' + str(values[key]), 'INFO')  
                        casalog.post('    ' + str(kwtuple[0]) + \
                                     ' -> ' + str(kwtuple[1]), 'DEBUG1')
                    else:
                        casalog.post(key + ': <undefined>', 'INFO')
                else:
                    casalog.post("Unrecognized item: " + key, 'WARN')
                    
            return values

        # In summary mode, just list the MS basic info.
        elif mode == 'summary':
            ms.open(vis)
            ms.summary()
            ms.close()
            print "Summary information is listed in logger"

        # In GET/PUT mode, focus on 1 particular bit of MS data
        elif (mode=='get' or mode=='put'):
            if(not keywords.has_key(hdkey)): 
                raise Exception, "hdkey " + str(hdkey) +" is not a recognized keyword. Your options are " + str(keywords.keys())

            # get/put the data specified by hdkey
            if mode == 'get':
                value = getput_keyw(mode, vis, keywords[hdkey], hdindex)
                casalog.post(hdkey+': '+str(value))
                return value
            else:
                getput_keyw(mode, vis, keywords[hdkey], hdindex, hdvalue)
                casalog.post(hdkey + ' set to ' + str(hdvalue))

    except Exception, instance:
        casalog.post( str('*** Error *** ') + str(instance), 'SEVERE')

    return
