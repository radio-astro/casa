# -*- coding: utf-8 -*-
import os
from taskinit import *
import flaghelper as fh
from casac import casac


def importasdm(
    asdm=None,
    vis=None,
    singledish=None,
    antenna=None,
    corr_mode=None,
    srt=None,
    time_sampling=None,
    ocorr_mode=None,
    compression=None,
    lazy=None,
    asis=None,
    wvr_corrected_data=None,
    scans=None,
    ignore_time=None,
    process_syspower=None,
    process_caldevice=None,
    process_pointing=None,
    process_flags=None,
    tbuff=None,
    applyflags=None,
    savecmds=None,
    outfile=None,
    flagbackup=None,
    verbose=None,
    overwrite=None,
    showversion=None,
    useversion=None,
    bdfflags=None,
    with_pointing_correction=None
    ):
    """Convert an ALMA Science Data Model observation into a CASA visibility file (MS) or single-dish data format (Scantable).
           The conversion of the ALMA SDM archive format into a measurement set.  This version
           is under development and is geared to handling many spectral windows of different
           shapes.

           Keyword arguments:
           asdm -- Name of input ASDM file (directory)
               default: none; example: asdm='ExecBlock3'

       vis       -- Root ms or scantable name, note a prefix (.ms or .asap) is NOT appended to this name
           default: none
           
       singledish   -- Set True to write data as single-dish format (Scantable)
               default: False singledish expandable parameter
                 antenna -- antenna name or id.
 
       corr_mode -- correlation mode to be considered on input. Could
            be one or more of the following, ao, co, ac, or all
           default: all

       srt       -- spectral resolution type. Could be one or more of
                    the following, fr, ca, bw, or all
           default: all

       time_sampling -- specifies the time sampling, INTEGRATION and/or
                            SUBINTEGRATION. could be one or more of the following
                            i, si, or all.
           default: all

       ocorr_mode    -- output data for correlation mode AUTO_ONLY 
                            (ao) or CROSS_ONLY (co) or CROSS_AND_AUTO (ca)
           default: ca

      compression  -- produces comrpressed columns in the resulting measurement set.
                 default: False

       lazy         -- Make the MS DATA column read the ASDM Binary data directly
                       (faster import, smaller MS)
                 default: False

       asis         --  creates verbatim copies of the ASDM tables in 
                        the output measurement set. The value given to
                    this option must be a list of table names separated
                    by space characters; the wildcard character '*' is 
                            allowed in table names.

       wvr_corrected_data -- specifies wich values are considered in the 
                      ASDM binary data to fill the DATA column in 
                      the MAIN table of the MS. Expected values for 
                      this option are 'no' for the uncorrected data 
                      (this is the default), 'yes' for the corrected
                      data and 'both' for corrected and uncorrected 
                      data. In the latter case, two measurement sets
                      are created, one containing the uncorrected 
                      data and the other one, whose name is suffixed
                      by '-wvr-corrected', containing the corrected 
                      data.

       scans --  processes only the scans specified in the option's value. This value is a semicolon 
                 separated list of scan specifications. A scan specification consists in an exec bock index 
                 followed by the character ':' followed by a comma separated list of scan indexes or scan 
                 index ranges. A scan index is relative to the exec block it belongs to. Scan indexes are 
                 1-based while exec blocks's are 0-based. "0:1" or "2:2~6" or "0:1,1:2~6,8;2:,3:24~30" "1,2" 
                 are valid values for the option. "3:" alone will be interpreted as 'all the scans of the 
                 exec block#3'. An scan index or a scan index range not preceded by an exec block index will
                 be interpreted as 'all the scans with such indexes in all the exec blocks'.  By default 
                 all the scans are considered.

       ignore_time -- All the rows of the tables Feed, History, Pointing, Source, SysCal, CalDevice, SysPower,
                      and Weather are processed independently of the time range of the selected exec block / scan.

       process_syspower -- The SysPower table is processed if and only if this parameter is set to True.
              default: True

       process_caldevice -- The CalDevice table is processed if and only if this parameter is set to True.
              default: True

       process_pointing -- The Pointing table is processed if and only if this parameter is set to True.
                       If the parameter is set to False the resulting MS will have an empty POINTING table.
              default: True

      process_flags -- Process the online flags and save them to the FLAG_CMD sub-table.
              default: True

            &gt;&gt;&gt; process_flags expandable parameter
                 tbuff -- Time padding buffer (in seconds).
                    default: 0.0

                 applyflags -- Apply the online flags to the MS.
                    default: False

                 savecmds -- Save the online flags to an ASCII file.
                    default: False
                    
                 outfile -- Filename to save the online flags.
                    default: ''

       flagbackup -- Backup the FLAG column in the .flagversions.
              default: True

       verbose     -- produce log output as asdm2MS is being run.

       overwrite -- Over write an existing MS.

       showversion -- report the version of the asdm2MS being used.

       useversion -- Selects the version of asdm2MS to be used (presently only \'v3\' is available).
                     default: v3
                     
      bdfflags -- Set the MS FLAG column according to the ASDM _binary_ flags
                   default: false

      with_pointing_correction -- add (ASDM::Pointing::encoder - ASDM::Pointing::pointingDirection)
                 to the value to be written in MS::Pointing::direction 
                   default: false
           
        """

    # Python script

    # make agentflagger tool local
    aflocal = casac.agentflagger()

    # make table tool local
    tblocal = casac.table()

    try:
        casalog.origin('importasdm')
        viso = ''
        visoc = ''  # for the wvr corrected version, if needed
                # -----------------------------------------
                # beginning of importasdm_sd implementation
                # -----------------------------------------
        if singledish:
            theexecutable = 'asdm2ASAP'
                        # if useversion == 'v2':
                        #        theexecutable = 'oldasdm2ASAP'
            if compression:
                casalog.post('compression=True has no effect for single-dish format.')
                
            cmd = 'which %s > /dev/null 2>&1' % theexecutable
            ret = os.system(cmd)
            if ret == 0:
                import commands
                casalog.post('found %s' % theexecutable)
                if showversion:
                    execute_string = theexecutable + ' --help'
                else:
                    execute_string = theexecutable + ' -asdm ' + asdm
                    if len(vis) != 0:
                        execute_string += ' -asap ' + vis.rstrip('/')
                    execute_string += ' -antenna ' + str(antenna) \
                        + ' -apc ' + wvr_corrected_data \
                        + ' -time-sampling ' + time_sampling.lower() \
                        + ' -overwrite ' + str(overwrite)
                    if corr_mode == 'all':
                        execute_string += \
                            ' -corr-mode ao,ca -ocorr-mode ao'
                    else:
                        execute_string += ' -corr-mode ' \
                            + corr_mode.replace(' ', ',') \
                            + ' -ocorr-mode ao'
                    if srt == 'all':
                        execute_string += ' -srt ' + srt
                    else:
                        execute_string += ' -srt ' + srt.replace(' ',
                                ',')
                    execute_string += ' -logfile ' + casalog.logfile()
                casalog.post('execute_string is')
                casalog.post('   ' + execute_string)
                ret = os.system(execute_string)
                if ret != 0 and not showversion:
                    casalog.post(theexecutable
                                 + ' terminated with exit code '
                                 + str(ret), 'SEVERE')
                    # raise Exception, "ASDM conversion error, please check if it is a valid ASDM and/or useversion='%s' is consistent with input ASDM."%(useversion)
                    raise Exception, \
                        'ASDM conversion error, please check if it is a valid ASDM.'
            else:
                casalog.post('You have to build ASAP to be able to create single-dish data.'
                             , 'SEVERE')
                        # implementation of asis option using tb.fromASDM
            if asis != '':
                import commands
                asdmTables = commands.getoutput('ls %s/*.xml'
                        % asdm).split('\n')
                asdmTabNames = []
                for tab in asdmTables:
                    asdmTabNames.append(tab.split('/')[-1].rstrip('.xml'
                            ))
                if asis == '*':
                    targetTables = asdmTables
                    targetTabNames = asdmTabNames
                else:
                    targetTables = []
                    targetTabNames = []
                    tmpTabNames = asis.split()
                    for i in xrange(len(tmpTabNames)):
                        tab = tmpTabNames[i]
                        try:
                            targetTables.append(asdmTables[asdmTabNames.index(tab)])
                            targetTabNames.append(tab)
                        except:
                            pass
                outTabNames = []
                outTables = []
                for tab in targetTabNames:
                    out = 'ASDM_' + tab.upper()
                    outTabNames.append(out)
                    outTables.append(vis + '/' + out)
                #tbtool = casac.table()
                tb = casac.table()
                tb.open(vis, nomodify=False)
                wtb = casac.table()
                for i in xrange(len(outTables)):
                    wtb.fromASDM(outTables[i], targetTables[i])
                    tb.putkeyword(outTabNames[i], 'Table: %s'
                                  % outTables[i])
                    tb.flush()
                tb.close()
            return
                # -----------------------------------
                # end of importasdm_sd implementation
                # -----------------------------------
        if len(vis) > 0:
            viso = vis
            tmps = vis.rstrip('.ms')
            if tmps == vis:
                visoc = vis + '-wvr-corrected'
            else:
                visoc = tmps + '-wvr-corrected.ms'
            if singledish:
                viso = vis.rstrip('/') + '.importasdm.tmp.ms'
        else:
            viso = asdm.rstrip("/") + '.ms'
            visoc = asdm.rstrip("/") + '-wvr-corrected.ms'
            vis = asdm.rstrip("/")
            if singledish:
                viso = asdm.rstrip('/') + '.importasdm.tmp.ms'
                vis = asdm.rstrip('/') + '.asap'



        useversion = 'v3'
        theexecutable = 'asdm2MS'

        execute_string = theexecutable + ' --icm "' + corr_mode \
            + '" --isrt "' + srt + '" --its "' + time_sampling \
            + '" --ocm "' + ocorr_mode + '" --wvr-corrected-data "' \
            + wvr_corrected_data + '" --asis "' + asis \
            + '" --logfile "' + casalog.logfile() + '"'

        if len(scans) > 0:
            execute_string = execute_string + ' --scans ' + scans
        if ignore_time:
            execute_string = execute_string + ' --ignore-time'
        if useversion == 'v3':
            if not process_syspower:
                execute_string = execute_string + ' --no-syspower'
            if not process_caldevice:
                execute_string = execute_string + ' --no-caldevice'
            if not process_pointing:
                execute_string = execute_string + ' --no-pointing'

        if compression:
            execute_string = execute_string + ' --compression'
        elif lazy:
            execute_string = execute_string + ' --lazy'
            
        if verbose:
            execute_string = execute_string + ' --verbose'
        if not overwrite and os.path.exists(viso):
            raise Exception, \
                'You have specified an existing MS and have indicated you do not wish to overwrite it'

        # Compression
        if compression:
                   # viso = viso + '.compressed'
            viso = viso.rstrip('.ms') + '.compressed.ms'
            visoc = visoc.rstrip('.ms') + '.compressed.ms'

        vistoproc = [] # the output MSs to postprocess
        if wvr_corrected_data == 'no' or wvr_corrected_data == 'both':
            vistoproc.append(viso)
        if wvr_corrected_data == 'yes' or wvr_corrected_data == 'both':
            vistoproc.append(visoc)

        #
        # If viso+".flagversions" then process differently depending on the value of overwrite..
        #
        if flagbackup:
            for myviso in vistoproc:
                dotFlagversion = myviso + '.flagversions'
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

        if showversion:
            casalog.post("You set option \'showversion\' to True. Will just output the version information and then terminate."
                         , 'WARN')
            execute_string = theexecutable + ' --revision'

        if with_pointing_correction:
            execute_string = execute_string + ' --with-pointing-correction'

        casalog.post('Running ' + theexecutable
                     + ' standalone invoked as:')
        # print execute_string
        casalog.post(execute_string)
        exitcode = os.system(execute_string)
        if exitcode != 0:
            if not showversion:
                casalog.post(theexecutable
                             + ' terminated with exit code '
                             + str(exitcode), 'SEVERE')
                raise Exception, \
                    'ASDM conversion error. Please check if it is a valid ASDM and that data/alma/asdm is up to date.'

        if showversion:
            return

        #
        # Populate the HISTORY table of the MS with informations about the context in which it's been created
        #
        try: 
            mslocal = mstool() 
            param_names = importasdm.func_code.co_varnames[:importasdm.func_code.co_argcount] 
            param_vals = [eval(p) for p in param_names]

            for myviso in vistoproc:
                write_history(mslocal, myviso, 'importasdm', param_names, 
                              param_vals, casalog) 

        except Exception, instance: 
            casalog.post("*** Error \'%s\' updating HISTORY" % (instance), 
                         'WARN')
            return False 

        if mslocal:
            mslocal = None 
        
        # Binary Flag processing
        if bdfflags:
            
            casalog.post('Parameter bdfflags==True: flags from the ASDM binary data will be used to set the MS flags ...')
            
            bdffexecutable = 'bdflags2MS '
            bdffexecstring = bdffexecutable+' -f ALL'
            if len(scans) > 0:
                bdffexecstring = bdffexecstring + ' --scans ' + scans

            for myviso in vistoproc:
                bdffexecstring = bdffexecstring+' '+ asdm + ' ' + myviso

                casalog.post('Running '+bdffexecutable+' standalone invoked as:')
                casalog.post(bdffexecstring)

                bdffexitcode = os.system(bdffexecstring)
                if bdffexitcode != 0:
                    casalog.post(bdffexecutable
                                 + ' terminated with exit code '
                                 + str(bdffexitcode), 'SEVERE')
                    raise Exception, \
                          'ASDM binary flags conversion error. Please check if it is a valid ASDM and that data/alma/asdm is up to date.'

        for myviso in vistoproc:
            if os.path.exists(myviso) and flagbackup==True:
                aflocal.open(myviso)
                aflocal.saveflagversion('Original',
                        comment='Original flags at import into CASA',
                        merge='save')
                aflocal.done()
                
        # Importasdm Flag Parsing
        if os.access(asdm + '/Flag.xml', os.F_OK):
            # Find Flag.xml
            casalog.post('Found Flag.xml in SDM')
            
            # Find Antenna.xml
            if os.access(asdm + '/Antenna.xml', os.F_OK):
                casalog.post('Found Antenna.xml in SDM')

            else:
                raise Exception, 'Failed to find Antenna.xml in SDM'
            
            # Find SpectralWindow.xml
            if os.access(asdm + '/SpectralWindow.xml', os.F_OK):
                casalog.post('Found SpectralWindow.xml in SDM')

            else:
                raise Exception, \
                    'Failed to find SpectralWindow.xml in SDM'
                    
            #
            # Parse Flag.xml into flag dictionary
            #
            if process_flags:
                flagcmds = fh.parseXML(asdm, float(tbuff))
                onlinekeys = flagcmds.keys()

                nflags = onlinekeys.__len__()
                                
                # Apply flags to the MS
                if nflags > 0:
                    for myviso in vistoproc:
                        if applyflags:
                            # Open the MS and attach it to the tool
                            aflocal.open(myviso)
                            # Select the data
                            aflocal.selectdata()
                            # Setup the agent's parameters
                            fh.parseAgents(aflocal, flagcmds, [], True, True, '')
                            # Initialize the agents
                            aflocal.init()
                            # Run the tool
                            aflocal.run(True, True)
                            casalog.post('Applied %s flag commands to data'%str(nflags))
                            # Destroy the tool and de-attach the MS
                            aflocal.done()
                            # Save to FLAG_CMD table. APPLIED is set to True.
                            fh.writeFlagCommands(myviso, flagcmds, True, '', '', True)       
                        else:
                            casalog.post('Will not apply flags (apply_flags=False), use flagcmd to apply')

                            # Write to FLAG_CMD, APPLIED is set to False
                            fh.writeFlagCommands(myviso, flagcmds, False, '', '', True)
                    
                        # Save the flag cmds to an ASCII file
                        if savecmds:
                            # Save to standard filename
                            if outfile == '': 
                                outfile = myviso.replace('.ms','_cmd.txt')
                            
                            fh.writeFlagCommands(myviso, flagcmds, False, '', outfile, True)
                            casalog.post('Saved %s flag commands to %s'%(nflags,outfile))
                    
                else:
                    casalog.post('There are no flag commands to process')
                
        else:
            casalog.post('There is no Flag.xml in ASDM', 'WARN')
        
        
        
    except Exception, instance:

        print '*** Error ***', instance


