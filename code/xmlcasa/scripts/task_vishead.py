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
import os

def getput_keyw(mode, vis, key, hdindex, hdvalue=''):
    table = vis + '/' + key[0]

    col = key[1]

    tb.open(table, nomodify = (mode == 'get'))

    if mode == 'get':
        if hdindex == '':
            # default: return full column
            if(tb.isvarcol(col)):
                value = tb.getvarcol(col)
            else:
                value = tb.getcol(col)
        else:
            i = int(hdindex)
            # The following seems more efficient but complains
            # that 'column XYZ is not an array column'
            #   value = tb.getcolslice(col, startrow=i-1, nrow=1)
            #
            # So read the entire column instead

            if i < 0:
                # allowed by python, but...
                raise Exception, "Illegal index " + str(i)
            
            value = tb.getcol(col)[i]  # throws exception if index too large
            
    elif mode == 'put':
        if(tb.isvarcol(col)):
            tb.close()
            raise Exception, "vishead does not yet read/write variably sized columns"
        else:
            if hdindex == '':
                # hdvalue expected to be an array
                tb.putcol(col, hdvalue)   
            else:
                # Get full column, change one element,
                # write it back. Not efficient but
                # columns used by this task are short

                i = int(hdindex)

                c = list(tb.getcol(col))
                # numpy arrays don't expand flexibly =>
                # convert to python list
                
                c[i] = hdvalue
                tb.putcol(col, c)
                                
        value = None
    else:
        tb.close()
        raise Exception, "Assertion error"

    #print "Will return", value

    tb.close()    
    return value


def keyword_exists(vis, key):
    table = vis + '/' + key[0]
    col = key[1]

    if not os.path.exists(table):
        return False

    try:
        # Throws StandardError if subtable
        # does not exist
        tb.open(table)
    except:
        return False


    return (col in tb.colnames())



def vishead(vis, mode=None, hdkey=None, hdindex=None, hdvalue=None):
    """Documentation goes here?"""

    casalog.origin('vishead')
    casalog.post( "parameter vis:     " + str(vis), 'DEBUG1' )
    casalog.post( "parameter mode:    " + str(mode), 'DEBUG1')
    casalog.post( "parameter hdkey:   " + str(hdkey), 'DEBUG1')
    casalog.post( "parameter hdindex: " + str(hdindex), 'DEBUG1')
    casalog.post( "parameter hdvalue: " + str(hdvalue), 'DEBUG1')

    # Define vishead keywords
    keywords = {
        # Keywords from OBSERVATION TABLE
        # time_range
        # log
        'schedule'  :['OBSERVATION', 'SCHEDULE'], #array
        # flag_row
        'observer' :['OBSERVATION', 'OBSERVER'],  #array
        'project'  :['OBSERVATION', 'PROJECT'],   #array
        'release_date'  :['OBSERVATION', 'RELEASE_DATE'], #array
        'schedule_type'  :['OBSERVATION', 'SCHEDULE_TYPE'], #array
        'telescope':['OBSERVATION', 'TELESCOPE_NAME'],  #array

        
        # Keywords from FIELD TABLE
        'field'    :['FIELD', 'NAME'],   # also in pointing table(!)
                                        # so watch out when changing

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
        # relevant?  'source_name' :['SOURCE', 'NAME']




        #MS_VERSION (probably not)
    }
    
    try:
        # In list mode, list the keywords.
        if mode == 'list' or mode == '':

            #tb.open(vis)
            #tb.summary()

            values = {}
            for key in keywords.keys():
                if keyword_exists(vis, keywords[key]):
                    values[key] = getput_keyw('get', vis, keywords[key], '')
                    casalog.post(key + ': ' + str(values[key]), 'INFO')
                    casalog.post('    ' + str(keywords[key][0]) + \
                                 ' -> ' + str(keywords[key][1]), 'DEBUG1')
                else:
                    casalog.post(key + ': <undefined>', 'INFO')
                    
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
