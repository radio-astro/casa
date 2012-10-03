########################################################################3
#  task_importgmrt.py
#
#
# Copyright (C) 2009
# Associated Universities, Inc. Washington DC, USA.
#
# This script is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be adressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# <author>
# Shannon Jaeger (University of Calgary)
# </author>
#
# <summary>
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <etymology>
# importgmrt stands for import GMRT UV FITS data.
# </etymology>
#
# <synopsis>
# task_importgmrt.py is a Python script providing an easy to use task
# for importing GMRT FITS data files into the CASA system.  A measurement
# set is produced from the GMRT FITS file.
#
# The given flag files are read and data is flagged according to
# the information found in these files. It is expected that the
# FLAG files are in the old AIPS TVFLAG file format.
#
# </synopsis> 
#
# <example>
# <srcblock>

# </srblock>
# </example>
#
# <motivation>
# To provide a user-friendly method for importing GMRT FITS files.
# </motivation>
#
# <todo>
# </todo>

import numpy
import os
from importuvfits import *
from taskinit import *
#from fg import *

def importgmrt( fitsfile, flagfile, vis ):
    retValue=False

    # First check if the output file exists. If it
    # does then we abort.  CASA policy prevents files
    # from being over-written.
    #
    # Also if the vis name given is the empty string
    # we use the default name, input fitsfile with
    # "fits" removed
    if ( len( vis ) < 1 ):
        vis = ''
        fits_list=fitsfile.split( '.' )
        last = len(fits_list)
        if ( fits_list[last-1].lower() == 'fits' or \
             fits_list[last-1].lower() == 'fit' ):
            last = last -1
        for i in range( last ):
            if ( len ( vis ) < 1 ):
                vis = fits_list[i]
            else:
                vis = vis + '.'+fits_list[i]
        vis=vis+'.MS'
        
        casalog.post( "The vis paramter is empty, consequently the" \
                      +" the default visibilty file\nname will be used, "\
                      + vis, 'WARN' )
    if ( len( vis ) > 0 and os.path.exists( vis ) ):
        casalog.post( 'Visibility file, '+vis+\
              ' exists. import GMRT can not proceed, please\n'+\
              'remove it or set vis to a different value.', 'SEVERE' )
        return retValue
    

    # Loop through the list of flag files and make sure each one
    # of them exists
    if isinstance( flagfile, str ):
        if ( len( flagfile ) > 0 ):
            flagfile=[flagfile]
        else:
            flagfile=[]
    
    ok = True
    for i in range( len( flagfile ) ):
        if ( not os.path.exists( flagfile[i] ) ):
            casalog.post( 'Unable to locate FLAG file '+ flagfile[i],
                          'SEVERE' )
            ok = False
        
    if ( not ok ):
        return retValue    


    # Ok, we've done our preliminary checks, now let's get
    # down to business and import our fitsfile.
    try:
        casalog.post( 'Starting import ...', 'NORMAL' )
        importuvfits( fitsfile, vis )
    except Exception, instance:
        casalog.post( str(instance), 'SEVERE' )
        return retValue

#    mytb, myms, myfg = gentools(['tb', 'ms', 'fg'])
    mytb, myms = gentools(['tb', 'ms'])
    aflocal = casac.agentflagger()

    # Write history
    try:
        param_names = importgmrt.func_code.co_varnames[:importgmrt.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        ok &= write_history(myms, vis, 'importgmrt', param_names,
                            param_vals, casalog)
    except Exception, instance:
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                     'WARN')
    
    # CASA's importuvfits doesn't understand the GMRT antenna
    # names very well and/or the FITS files tend to put the
    # antenna names in a place that CASA doesn't look for them.
    # Luckily the information seems to be in the "station"
    # information.  So let's fix up the Name column.
    casalog.post( 'Correcting GMRT Antenna names.', 'NORMAL1' )
    try:
        mytb.open( vis+'/ANTENNA', nomodify=False )
        stations = mytb.getcol('STATION')
        names    = []
        for idx in range( 0, len( stations ) ):
            names.append( stations[idx].split(':')[0] )
        mytb.putcol( 'NAME', numpy.array( names ) )
        mytb.done()
    except Exception, instance:
        casalog.post( 'Unable to properly name the antennas, but continuing')
        casalog.post( str(instance), 'WARN' )

    # If we don't have a flagfile then we are done!
    # Yippee awe eh!
    if ( len( flagfile ) < 1 ):
        return True

    # We've imported let's find out the observation start
    # and end times, this will be needed for flagging.
    # We use it to find the different days the observation
    # was done on.
    startObs = ''
    endObs   = ''
    try: 
        myms.open( vis )
        trange=myms.range( 'time' )['time']
        myms.done()

        # Now have the observation time range in a numpy array.
        startObs = qa.time( str( trange[0] )+'s', prec=8, form='ymd' )[0]
        endObs   = qa.time( str( trange[1] )+'s', prec=8, form='ymd' )[0]
        
    except Exception, instance:
        casalog.post( 'Unable to find obaservation start/en times', 'SEVERE' )
        casalog.post( str(instance), 'SEVERE' )
        return retValue
    
    days=[]
    startYY = startObs.split('/')[0]
    startMM = startObs.split('/')[1]
    startDD = startObs.split('/')[2]

    endYY = endObs.split('/')[0]
    endMM =endObs.split('/')[1]
    endDD =endObs.split('/')[2]    

    for year in range( int( startYY ), int( endYY ) + 1 ):
        for month in range( int( startMM ), int( endMM ) + 1 ):
            for day in range( int( startDD ), int( endDD ) + 1 ):
                days.append( str(year) + '/' + str(month) + '/' + str(day) + '/' )
	casalog.post( "DAYS: "+str(days), 'DEBUG1')


    # Read through the flag file(s) and assemble it into
    # a data structure.  The hope is to minimize the
    # number of calls to "flagdata" to make the flagging
    # fast!
    #
    # The expected file format is what is the flag file
    # format used by TVFLAG/UVFLAG in AIPS, which is as follows:
    #    1. If a line starts with '!' it's a comment
    #    2. 
    #
    casalog.post( 'Starting the flagging ...', 'NORMAL' )
					  
	# First lets save the flag information as we got it from the
	# flag file.
#    myfg.open( vis )
#    myfg.saveflagversion( 'none', 'No flagging performed yet' )
#    myfg.done()
    aflocal.open( vis )
    aflocal.saveflagversion( 'none', 'No flagging performed yet' )
    aflocal.done()

    for file in flagfile:
        casalog.post( 'Reading flag file '+file, 'NORMAL2' )
        FLAG_FILE = open( file, 'r' )
        line = FLAG_FILE.readline()
        ant_names = []		
        
        while ( len( line ) > 0 ):
            casalog.post( 'Read from flag file: '+str(line), 'DEBUG1' )

            # Default antenna list and time range
            antennas  = []
            baselines = []
            timerange = []

            # Skip comment lines, and the end of file.
            if ( len(line) < 1 or ( line[0] == '!' and line[2:6] !=  'PANT' ) ):
				line = FLAG_FILE.readline()
				continue
			
            # Store the antenna name list
            if ( line[0:6] == '! PANT' ):			
                ant_names= line.split(':')[1].lstrip(' ').split(' ')
                line = FLAG_FILE.readline()
                continue

            # First divide lines up on spaces
            casalog.post( 'LINE READ: '+line, 'DEBUG1' )
            step1=line.split(' ')   
            if ( len( step1 ) < 2 ):
                # We want a line with something on it.
                line = FLAG_FILE.readline()
                continue

            # Parse each bit of the string looking for the
            # antenna and timrange information.
            for i in range( len( step1 ) ):
                step2=step1[i].split('=')
                
                if ( len( step2 ) != 2 ):
                    # We want something with an equals sign in it!
                    continue
                casalog.post( 'keyword/value: '+str(step2), 'DEBUG1' )


                if ( step2[0].upper().startswith( 'ANT' ) ):
                    # Should be a comma separated list which
                    # is acceptable to flag data
                    #antennas=list(step2[1].split(',')
                    antennas=step2[1]
                    casalog.post( "antennas: "+antennas, 'DEBUG1')


                if ( step2[0].upper().startswith( 'BASEL' ) ):
                    # Should be a comma separated list which
                    # is acceptable to flag data
                    baselines=step2[1]
                    casalog.post( "baselines: "+baselines, 'DEBUG1')

                if ( step2[0].upper() == 'TIMERANGE' ):
                    # Time in the FLAG file is in the format
                    #   start-day,start-hour,start-min,star-sec,\
                    #   end-day,end-hour,end-min,end-sec
                    #
                    # CASA expects data to be of the form
                    #   YYYY/MM/DD/HH:MM:SS~YYYY/MM/DD/HH:MM:SS
                    # http://casa.nrao.edu/Memos/msselection/node8.html
                    #
                    # We use the day # to index into our days list and
                    # append the time on the end.  But day # starts at
                    # 1 and indexing starts at 0 so we need to adjust.
                    times     = step2[1].split(',')
                    casalog.post( "time: "+str(times), 'DEBUG1')
                    if ( int(times[0]) < 1 and int(times[4]) > len(days) ):
                        # All data for this antenna
                        timerange = ''
                    elif ( int(times[0]) < 1 ):
                        # We want all time before the end time
                        timerange = '<' + days[int(times[4])-1] \
                                    + times[5] + ':' + times[6] + ':' +times[7]
                    elif ( int(times[4]) > len(days) ):
                        # We want all times after the start time
                        timerange = '>' + days[int(times[0])-1] \
                                    + times[1]+':'+times[2]+':'+times[3]
                    else:
                        # We ahve a fully specified time range
                        timerange = days[int(times[0])-1] \
                                + times[1]+':'+times[2]+':'+times[3]\
                                +'~'\
                                +days[int(times[4])-1] \
                                +times[5]+':'+times[6]+':'+times[7]
                
            # Construct the antenna query string
            #
            # Verify the syntax of the ANTENNA & BASELINE keywords, 
            # if ANTENNA will always have only one.  I suspect we
            # with to loop through the baselines only here.
			#
			# OLD CODE FOR creating ANT SELECTION STRING
			#
            #antStr=''
            #selectAntStr=''
            #if ( len( antennas ) > 0 and len( baselines ) > 0 ):
            #    # We are selecting baselines.
            #    for ant1 in antennas:
            #        for ant2 in baselines:
            #            if ( len( antStr ) < 1 ):
            #                antStr=str(int(ant1)-1)+'&'+str(int(ant2)-1)
            #    selectAntStr=antStr
            #elif ( len( antennas ) > 0 ):
            #    antStr=str(int(antennas)-1)
            #    # A hack to make sure we always have some unflagged data.
            #    # CASA seg faults if the myfg.setdata() results in no data.
            #    selectAntStr+=antStr+','+antennas

            
            antStr=''
            #if ( len( antennas ) > 0 and len( baselines ) > 0 ):
			#	# We are selecting baselines.
			#	for ant1 in antennas:
            #        for ant2 in baselines:
            #            if ( len( antStr ) < 1 ):
            #                antStr=str(int(ant1)-1)+'&'+str(int(ant2)-1)
			#    selectAntStr=antStr
            #elif ( len( antennas ) > 0 ):
            if ( len( antennas ) > 0 ):
                antStr = ant_names[ int(antennas)-1 ].strip( ' ' )

            try:
#                casalog.post( "flagdata( "+vis+", mode='manualflag', antenna='"
#                              +antStr+"', timerange='"+timerange+"' )", 'NORMAL')
                casalog.post( "flagdata( "+vis+", mode='manual', antenna='"
                              +antStr+"', timerange='"+timerange+"' )", 'NORMAL')
                #flagdata( vis, mode='manualflag', selectdata=True, \
                #            antenna=antStr, timerange=timerange, \
                #            flagbackup=False )
#                myfg.open( vis )
#                #print "myfg.setdata( time='",timerange,"' )"
#                myfg.setdata( time=timerange )
#                #print "myfg.setmanualflags( baseline='",antStr,"', time='",timerange,"' )"
#                myfg.setmanualflags( baseline=antStr, time=timerange, unflag=False, clipexpr='' )
#                myfg.run()
#                myfg.done()
                aflocal.open(vis)
                aflocal.selectdata(timerange=timerange)
                aflocal.parsemanualparameters(antenna=antStr, timerange=timerange)
                aflocal.init()
                aflocal.run(True, True)
                aflocal.done()
            except Exception, instance:
                casalog.post( 'Unable to flag data from flag file '+file\
                              +'.\nAntennas='+antennas+' and timerange='\
                              +timerange, 'WARN' )
                casalog.post( str(instance), 'SEVERE' )
                return retValue

            line = FLAG_FILE.readline()
            
        FLAG_FILE.close()

	# Save a Flag version so we can revert back to it if we wish
#    myfg.open( vis )
#    myfg.saveflagversion( 'import', 'Flagged the data from the GMRT flag file' )
#    myfg.done()
    aflocal.open( vis )
    aflocal.saveflagversion( 'import', 'Flagged the data from the GMRT flag file' )
    aflocal.done()

    retValue = True
    return retValue


    
    

