import os
from taskinit import *
import flaghelper as fh

debug = False


# Hack to address CAS-6411, CAS-6412, and CAS-6413.
import platform
def set_library_paths_before_system_call(command_line):
    if platform.system() == "Darwin":
        if os.getenv("DYLD_FRAMEWORK_PATH"):
            command_line = "DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:" + os.getenv("DYLD_FRAMEWORK_PATH") + " " + command_line
    return os.system(command_line)
 

def importevla(
    asdm=None,
    vis=None,
    ocorr_mode=None,
    compression=None,
    asis=None,
    scans=None,
    verbose=None,
    overwrite=None,
    online=None,
    tbuff=None,
    flagzero=None,
    flagpol=None,
    shadow=None,
    tolerance=None,
    addantenna=None,
    applyflags=None,
    savecmds=None,
    outfile=None,
    flagbackup=None,
    ):
    """ Convert a Science Data Model (SDM) dataset into a CASA Measurement Set (MS)
....This version is under development and is geared to handling EVLA specific flag and
....system files, and is otherwise equivalent to importasdm.
....
....Keyword arguments:
....asdm -- Name of input SDM file (directory)
........default: none; example: asdm='TOSR0001_sb1308595_1.55294.83601028935'

...."""

    # Python script
    # Origninator: Steven T. Myers
    # Written (3.0.1) STM 2010-03-11 modify importasdm to include flagging from xml
    # Vers1.0 (3.0.1) STM 2010-03-16 add tbuff argument
    # Vers2.0 (3.0.1) STM 2010-03-29 minor improvements
    # Vers3.0 (3.0.2) STM 2010-04-13 add flagzero, doshadow
    # Vers4.0 (3.0.2) STM 2010-04-20 add flagpol
    # Vers5.0 (3.0.2) STM 2010-05-27 combine flagzero clips
    # Vers6.0 (3.1.0) STM 2010-07-01 flagbackup option
    # Vers7.0 (3.1.0) STM 2010-08-18 remove corr_mode,wvr_corrected_data,singledish,antenna
    # Vers7.1 (3.1.0) STM 2010-10-07 remove time_sampling, srt
    # Vers7.1 (3.1.0) STM 2010-10-07 use helper functions, flagger tool, fill FLAG_CMD
    # Vers7.2 (3.1.0) STM 2010-10-29 minor modifications to defaults and messages
    # Vers8.0 (3.2.0) STM 2010-11-23 tbuff not sub-par of applyflags=T
    # Vers8.1 (3.2.0) STM 2010-12-01 prec=9 on timestamps
    # Vers8.2 (3.2.0) MKH 2010-12-06 added scan selection
    # Vers8.3 (3.2.0) GAM 2011-01-18 added switchedpower option (sw power gain/tsys)
    # Vers8.4 (3.2.0) STM 2011-03-24 fix casalog.post line-length bug
    # Vers8.5 (3.4.0) STM 2011-12-08 new readflagxml for new Flag.xml format
    # Vers8.6 (3.4.0) STM 2011-02-22 full handling of new Flag.xml ant+spw+pol flags
    # Vers9.0 (3.4.0) SMC 2012-03-13 ported to use the new flagger tool (agentflagger)
    #

    # Create local versions of the flagger and ms tools
    aflocal = casac.agentflagger()
    mslocal = casac.ms()

    #
    try:
        casalog.origin('importevla')
        casalog.post('You are using importevla v9.0 SMC Updated 2012-03-13'
                     )
        viso = ''
        casalog.post('corr_mode is forcibly set to all.')
        if len(vis) > 0:
            viso = vis
        else:
            viso = asdm + '.ms'
            vis = asdm
        corr_mode = 'all'
        wvr_corrected_data = 'no'
        singledish = False
        srt = 'all'
        time_sampling = 'all'
        showversion = True
        execute_string = 'asdm2MS  --icm "' + corr_mode + '" --isrt "' \
            + srt + '" --its "' + time_sampling + '" --ocm "' \
            + ocorr_mode + '" --wvr-corrected-data "' \
            + wvr_corrected_data + '" --asis "' + asis + '" --scans "' \
            + scans + '" --logfile "' + casalog.logfile() + '"'
        if showversion:
            casalog.post('asdm2MS --revision --logfile "'
                         + casalog.logfile() + '"')
            # os.system('asdm2MS --revision --logfile "'
            #           + casalog.logfile() + '"')
            # Hack to address CAS-6411, CAS-6412, and CAS-6413.
            set_library_paths_before_system_call('asdm2MS --revision --logfile "' + casalog.logfile() + '"')
        if compression:
            execute_string = execute_string + ' --compression'
        if verbose:
            execute_string = execute_string + ' --verbose'
        if not overwrite and os.path.exists(viso):
            raise Exception, \
                'You have specified and existing ms and have indicated you do not wish to overwrite it'
        #
        # If viso+".flagversions" then process differently depending on the value of overwrite..
        #
        dotFlagversion = viso + '.flagversions'
        if os.path.exists(dotFlagversion):
            if overwrite:
                casalog.post("Found '" + dotFlagversion
                             + "' . It'll be deleted before running the filler."
                             )
                os.system('rm -rf %s' % dotFlagversion)
            else:
                casalog.post("Found '%s' but can't overwrite it."
                             % dotFlagversion)
                raise Exception, "Found '%s' but can't overwrite it." \
                    % dotFlagversion

        execute_string = execute_string + ' ' + asdm + ' ' + viso
        casalog.post('Running the asdm2MS standalone invoked as:')
        # Print execute_string
        casalog.post(execute_string)
        
        # Catch the return status and exit on failure
        # ret_status = os.system(execute_string)
        # Hack to address CAS-6411, CAS-6412, and CAS-6413.
        ret_status = set_library_paths_before_system_call(execute_string)
        if ret_status != 0:
            casalog.post('asdm2MS failed to execute with exit error '+str(ret_status), 'SEVERE')
            raise Exception, 'ASDM conversion error, please check if it is a valid ASDM.'
        
        if compression:
            visover = viso
            viso = visover.replace('.ms','.compressed.ms')
        if flagbackup:
            ok = af.open(viso)
            ok = af.saveflagversion('Original',
                    comment='Original flags on import', merge='save')
            ok = af.done()
            print 'Backed up original flag column to ' + viso \
                + '.flagversions'
            casalog.post('Backed up original flag column to ' + viso
                         + '.flagversions')
        else:
            casalog.post('Warning: will not back up original flag column'
                         , 'WARN')
        #
        # =============================
        # Begin EVLA specific code here
        # =============================
        nflags = 0
        
        # All flag cmds
        allflags = {}
                
        if os.access(asdm + '/Flag.xml', os.F_OK):
                # Find (and copy) Flag.xml
            print '  Found Flag.xml in SDM, copying to MS'
            casalog.post('Found Flag.xml in SDM, copying to MS')
            os.system('cp -rf ' + asdm + '/Flag.xml ' + viso + '/')
            # Find (and copy) Antenna.xml
            if os.access(asdm + '/Antenna.xml', os.F_OK):
                print '  Found Antenna.xml in SDM, copying to MS'
                casalog.post('Found Antenna.xml in SDM, copying to MS')
                os.system('cp -rf ' + asdm + '/Antenna.xml ' + viso
                          + '/')
            else:
                raise Exception, 'Failed to find Antenna.xml in SDM'
            # Find (and copy) SpectralWindow.xml
            if os.access(asdm + '/SpectralWindow.xml', os.F_OK):
                print '  Found SpectralWindow.xml in SDM, copying to MS'
                casalog.post('Found SpectralWindow.xml in SDM, copying to MS'
                             )
                os.system('cp -rf ' + asdm + '/SpectralWindow.xml '
                          + viso + '/')
            else:
                raise Exception, \
                    'Failed to find SpectralWindow.xml in SDM'
            #
            # Parse Flag.xml into flag dictionary
            #
            if online:
#                flago = fh.readXML(asdm, tbuff)
                flago = fh.parseXML(asdm, tbuff)
                onlinekeys = flago.keys()

                nkeys = onlinekeys.__len__()
                nflags += nkeys
                allflags = flago.copy()

                casalog.post('Created %s commands for online flags'%str(nflags))
                
        else:
            if online:
                casalog.post('ERROR: No Flag.xml in SDM', 'SEVERE')
            else:
                casalog.post('WARNING: No Flag.xml in SDM', 'WARN')

        if flagzero or shadow:
            # Get overall MS time range for later use (if needed)
            (ms_startmjds, ms_endmjds, ms_starttime, ms_endtime) = \
                getmsmjds(viso)

        # Now add zero and shadow flags
        if flagzero:
            flagz = {}
            
            # clip zero data
            # NOTE: currently hard-wired to RL basis
            # assemble into flagging commands and add to myflagd
            flagz['time'] = 0.5 * (ms_startmjds + ms_endmjds)
            flagz['interval'] = ms_endmjds - ms_startmjds
            flagz['level'] = 0
            flagz['severity'] = 0
            flagz['type'] = 'FLAG'
            flagz['applied'] = False
            flagz['antenna'] = ''
            flagz['mode'] = 'clip'

            # Flag cross-hands too
            if flagpol:
                flagz['reason'] = 'CLIP_ZERO_ALL'
#                flagz['command'] = \
#                    "mode='clip' clipzeros=True correlation='ABS_ALL'"
                command = {}
                command['mode'] = 'clip'
                command['clipzeros'] = True
                command['correlation'] = 'ABS_ALL'
                flagz['command'] = command
                flagz['id'] = 'ZERO_ALL'
                allflags[nflags] = flagz.copy()
                nflags += 1
                nflagz = 1
                
            else:
                flagz['reason'] = 'CLIP_ZERO_RR'
#                flagz['command'] = \
#                    "mode='clip' clipzeros=True correlation='ABS_RR'"
                command = {}
                command['mode'] = 'clip'
                command['clipzeros'] = True
                command['correlation'] = 'ABS_RR'
                flagz['command'] = command
                flagz['id'] = 'ZERO_RR'
                allflags[nflags] = flagz.copy()
                nflags += 1
            
                flagz['reason'] = 'CLIP_ZERO_LL'
#                flagz['command'] = \
#                    "mode='clip' clipzeros=True correlation='ABS_LL'"
                command = {}
                command['mode'] = 'clip'
                command['clipzeros'] = True
                command['correlation'] = 'ABS_LL'
                flagz['command'] = command
                flagz['id'] = 'ZERO_LL'
                allflags[nflags] = flagz.copy()
                nflags += 1
                nflagz = 2

            casalog.post('Created %s command(s) to clip zeros'%str(nflagz))

        if shadow:
            flagh = {}
            command = {}
            # flag shadowed data
            flagh['time'] = 0.5 * (ms_startmjds + ms_endmjds)
            flagh['interval'] = ms_endmjds - ms_startmjds
            flagh['level'] = 0
            flagh['severity'] = 0
            flagh['type'] = 'FLAG'
            flagh['applied'] = False
            flagh['antenna'] = ''
            flagh['mode'] = 'shadow'
            flagh['reason'] = 'SHADOW'

#            scmd = 'mode=shadow tolerance=' + str(tolerance)
#            scmd = "mode='shadow' tolerance=" + str(tolerance)
            command['mode'] = 'shadow'
            command['tolerance'] = tolerance

            if type(addantenna) == str:
                if addantenna != '':
                    # it's a filename, create a dictionary
                    antdict = fh.readAntennaList(addantenna)
#                    scmd = scmd  +' addantenna='+str(antdict) 
                    command['addantenna'] = antdict
                    
            elif type(addantenna) == dict:
                if addantenna != {}:
#                    scmd = scmd  +' addantenna='+str(addantenna)
                    command['addantenna'] = addantenna
            
#            flagh['command'] = scmd
            flagh['command'] = command
            flagh['id'] = 'SHADOW'
            allflags[nflags] = flagh.copy()
            nflags += 1

            casalog.post('Created 1 command to flag shadowed data')
        
        # List of rows to save
        allkeys = allflags.keys()
        
        # Apply the flags
        if applyflags:
            if nflags > 0:

                # Open the MS and attach the tool
                aflocal.open(viso)
                                
                # Select the data
                aflocal.selectdata()

                # Setup the agent's parameters
                fh.parseAgents(aflocal, allflags, [], True, True, '')

                # Initialize the agents
                aflocal.init()

                # Run the tool
                stats = aflocal.run(True, True)
                
                casalog.post('Applied %s flag commands to data'%str(nflags))

                # Destroy the tool
                aflocal.done()
                
                # Save the flags to FLAG_CMD and update the APPLIED column
#                fh.writeFlagCmd(viso, allflags, allkeys, True, '', '')
                fh.writeFlagCommands(viso,allflags,True,'','', True)
                
            else:

                casalog.post('There are no flags to apply')
                
        else :
            casalog.post('Will not apply flags (applyflags=False), use flagcmd to apply')
            if nflags > 0:
#                fh.writeFlagCmd(viso, allflags, allkeys, False, '', '')
                fh.writeFlagCommands(viso,allflags,False,'','', True)

               
        # Save the flag commads to an ASCII file 
        if savecmds:

            if nflags > 0:         
                # Save the cmds to a file
                if outfile == '': 
                    # Save to standard filename
                    outfile = viso.replace('.ms','_cmd.txt')
                    
#                fh.writeFlagCmd(viso, allflags, allkeys, False, '', outfile)
                fh.writeFlagCommands(viso,allflags,False,'',outfile, True)
                
                casalog.post('Saved %s flag commands to %s'%(nflags,outfile))
        
            else:
                casalog.post('There are no flag commands to save')
                
                         
    except Exception, instance:

        casalog.post('%s' % instance, 'ERROR')

        # write history
    else:
        retval = True
        try:
            param_names = importevla.func_code.co_varnames[:importevla.func_code.co_argcount]
            param_vals = [eval(p) for p in param_names]
            retval &= write_history(mslocal, vis, 'importevla', param_names,
                                    param_vals, casalog)
            
        except Exception, instance:
            casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                         'WARN')

   

# ===============================================================================


def getmsmjds(vis):
    # Get start and end times from MS, return in mjds
    # this might take too long for large MS
    # NOTE: could also use values from OBSERVATION table col TIME_RANGE
    mslocal2 = casac.ms()
    success = True
    ms_time1 = ''
    ms_time2 = ''
    ms_startmjds = 0.0
    ms_endmjds = 0.0
    try:
        mslocal2.open(vis)
        timd = mslocal2.range(['time'])
        mslocal2.close()
    except:
        success = False
        print 'Error opening MS ' + vis
    if success:
        ms_startmjds = timd['time'][0]
        ms_endmjds = timd['time'][1]
        t = qa.quantity(ms_startmjds, 's')
        t1sd = t['value']
        ms_time1 = qa.time(t, form='ymd', prec=9)[0]
        t = qa.quantity(ms_endmjds, 's')
        t2sd = t['value']
        ms_time2 = qa.time(t, form='ymd', prec=9)[0]
        casalog.post('MS spans timerange ' + ms_time1 + ' to '
                     + ms_time2)
    else:
        print 'WARNING: Could not open vis as MS to find times'
        casalog.post('WARNING: Could not open vis as MS to find times')
    return (ms_startmjds, ms_endmjds, ms_time1, ms_time2)

