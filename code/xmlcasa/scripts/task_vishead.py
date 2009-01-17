#
# Instructions from Gustaaf:
# The vishead task is primarily a tool for allowing the user to GET and PUT
# common data elements in an MS.  In addition, the task will also provide
# a listing of these common data elements.  vishead should resemble imhead
# when possible, to simply the user experience.
#
# The task should be designed such that the user does not need to know the
# internal structure of the MS in order to change important bits of information.
# (A user with advanced knowlege of the MS can use the tb tool instead.)
# To accomplish this, I use a keyword dictionary that relates a "keyword" to
# an MS subtable and column.  The keywords should correspond to FITS keywords
# whenever possible.
#
# The name vishead is a misnomer because casa tables do not have headers in the 
# sense that FITS files do.  The name derives (I think) from the IMHEAD, GETHEAD
# and PUTHEAD verbs in AIPS.
#

from taskinit import *

def vishead(vis,mode=None,hditem=None,hdvalue=None):
    """Documentation goes here?"""

    casalog.origin('vishead')

    # Define vishead keywords
    keyword = {
        'telescope':['OBSERVATION','TELESCOPE_NAME'],
        'observer' :['OBSERVATION','OBSERVER']
    }
    
    try:
        # In summary mode, just list the MS basic info.
        if (mode=='summary' or mode==''):
            ms.open(vis)
            ms.summary()
            ms.close
            print "Summary information is listed in logger"

        # In GET/PUT mode, focus on 1 particular bit of MS data
        elif (mode=='get' or mode=='put'):
            if(not keyword.has_key(hditem)): 
                raise Exception, "hditem must be a vishead keyword"
            table = vis + '/' + keyword[hditem][0]
            col = keyword[hditem][1]
            tb.open(table)
            if(tb.isvarcol(col)): 
                raise Exception, "vishead does not yet read variably sized columns"

            # get the data specified by hditem
            if(mode == 'get'):
                value = tb.getcol(col)
                tb.close()
                return value
            # put hdvalue into data specified by hditem
            elif(mode == 'put'):
                tb.putcol(col,hdvalue)
                tb.close()

    except Exception, instance:
        casalog.post( str('*** Error ***')+str(instance), 'SEVERE')

    return
