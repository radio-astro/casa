import os
from taskinit import *

def importmiriad (
    mirfile=None,
    vis=None,
    tsys=None,
    spw=None,
    vel=None,
    linecal=None,
    wide=None,
    debug=None,
    ):
    """Convert a Miriad visibility file into a CASA visibility file (MS).
           The conversion of the Miriad visibility format into a measurement set.  This version
           has been tested for both ATNF and CARMA Miriad files.
................          
           Keyword arguments:
        mirfile -- Name of input Miriad visibility file (directory)
               default: none; example: mirfile='mydata.uv'

....   ms       -- Output ms name, note a postfix (.ms) is NOT appended to this name
               default: none

....   tsys   -- Set True to use the system temperature to set the visibility weights
               default: False

....   spw -- specify the spectral windows to use
........ default: all

....   vel -- Velocity system to use: LSRK, LSRD or TOPO
....       default: TOPO for ATCA, LSRK for CARMA

....   linecal       -- apply CARMA linecal on the fly?
....       default: False

....   wide    -- (CARMA only) specify the window averages to use
........ default: all
........ 
....   debug  -- specify level of debug messages (0,1,2,3)
                 default: 0 (=None)

           
        """

    # Python script

    try:
        casalog.origin('importmiriad')
        # -----------------------------------------'
        # beginning of importmiriad implementation
        # -----------------------------------------
	theexecutable = 'importmiriad'
        cmd = 'which %s > /dev/null 2>&1' % theexecutable
        ret = os.system(cmd)
	if ret == 0:
            import commands
            casalog.post('found %s' % theexecutable)
        execute_string = theexecutable
        if len(mirfile) != 0:
            execute_string += ' mirfile=' + mirfile.rstrip('/')
            execute_string += ' vis=' + vis
	    if tsys:
                execute_string += ' tsys=' + str(tsys)
	    if spw != "all":
                execute_string += ' spw=' + str(spw) # may need to convert
            if vel !="":
                execute_string += ' vel=' + str(vel)
	    if linecal:
                execute_string += ' linecal=' + str(linecal)
            if wide != "all":
		execute_string += ' wide=' + str(wide)
            if debug>0: 
		execute_string += ' debug=' + str(debug)
#            execute_string += ' -logfile ' + casalog.logfile()
            casalog.post('execute_string is')
            casalog.post('   ' + execute_string)
            ret = os.system(execute_string)
            if ret != 0 :
                casalog.post(theexecutable
                             + ' terminated with exit code '
                             + str(ret), 'SEVERE')
                raise Exception, \
                    'Miriad conversion error, please check if it is a valid Miriad file.'

        return
        # -----------------------------------
        # end of importmiriad implementation
        # -----------------------------------
       
    except Exception, e:
	casalog.post("Failed to import miriad file %s" % mirfile)
	
    return
 

