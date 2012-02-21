import os, re
import string
from taskinit import casalog, mstool, qa, tbtool, write_history
from update_spw import join_spws, subtract_spws

def uvcontsub3(vis, fitspw, combine, fitorder, field, spw,
               scan, intent, correlation, observation):
    """Extract the line(s) of an MS."""
    retval = True
    casalog.origin('uvcontsub3')

    myms = mstool()
    mytb = tbtool()
    # This one is redundant - it is already checked at the XML level.
    if not ((type(vis) == str) and os.path.isdir(vis)):
        casalog.post('Visibility data set not found - please verify the name', 'SEVERE')
        return False

    outputvis = vis + '.contsub'
    if os.path.exists(outputvis):
        casalog.post("Output MS " + outputvis + " already exists - will not overwrite.", 'SEVERE')
        return False

    if combine and combine.lower() != 'spw':
        casalog.post("uvcontsub3 deliberately does not support combination by",
                     'SEVERE')
        casalog.post("anything except spw.", 'SEVERE')
        return False

    # MSStateGram is picky ('CALIBRATE_WVR.REFERENCE, OBSERVE_TARGET_ON_SOURCE'
    # doesn't work, but 'CALIBRATE_WVR.REFERENCE,OBSERVE_TARGET_ON_SOURCE'
    # does), and I don't want to mess with bison now.  A .upper() might be a
    # good idea too, but the MS def'n v.2 does not say whether OBS_MODE should
    # be case-insensitive.
    intent = intent.replace(', ', ',')

    if type(spw) == list:
        spw = ','.join([str(s) for s in spw])
    elif type(spw) == int:
        spw = str(spw)

    ## if ':' in spw:
    ##     casalog.post("uvcontsub3 does not yet support selection by channel for the output",
    ##                  'SEVERE')
    ##     casalog.post("Meanwhile, use split to select the desired channels", 'WARN')
    ##     return False

    if ';' in spw:
        casalog.post("uvcontsub3 does not yet support writing multiple channel groups per output spw",
                     'SEVERE')
        return False

    mytb.open(vis + '/SPECTRAL_WINDOW')
    allspw = '0~' + str(mytb.nrows() - 1)
    mytb.close()
    if 'spw' not in combine:
        spwmfitspw = subtract_spws(spw, fitspw)
        if spwmfitspw == 'UNKNOWN':
            spwmfitspw = subtract_spws(allspw, fitspw)
        if spwmfitspw:
            raise Exception, "combine must include 'spw' when the fit is being applied to spws outside fitspw."

    if type(correlation) == list:
        correlation = ', '.join(correlation)
    correlation = correlation.upper()

    mytb.open(vis, nomodify=True)
    if 'CORRECTED_DATA' in mytb.colnames():
        datacolumn = 'CORRECTED_DATA'
    else:
        # DON'T remind the user that split before uvcontsub wastes time -
        # scratch columns will eventually go away.
        datacolumn = 'DATA'
    mytb.close()

    myms.open(vis, nomodify=True)
    if not myms.contsub(outputms=outputvis,   fitspw=fitspw,
                        fitorder=fitorder,    combine=combine,
                        spw=spw,              unionspw=join_spws(fitspw, spw),
                        field=field,          scan=scan,
                        intent=intent,        correlation=correlation,
                        obs=str(observation), whichcol=datacolumn):
        myms.close()
        return False
    myms.close()

    # Write history to output MS, not the input ms.
    try:
        param_names = uvcontsub3.func_code.co_varnames[:uvcontsub3.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]   
        retval &= write_history(myms, outputvis, 'uvcontsub3', param_names, param_vals,
                                casalog)
    except Exception, instance:
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                     'WARN')

    # Update FLAG_CMD if necessary.
    if ((spw != '') and (spw != '*')):
        isopen = False
        try:
            mytb = tbtool()
            mytb.open(outputvis + '/FLAG_CMD', nomodify=False)
            isopen = True
            #print "is open"
            nflgcmds = mytb.nrows()
            #print "nflgcmds =", nflgcmds
            if nflgcmds > 0:
                mademod = False
                cmds = mytb.getcol('COMMAND')
                for rownum in xrange(nflgcmds):
                    # Matches a bare number or a string quoted any way.
                    spwmatch = re.search(r'spw\s*=\s*(\S+)', cmds[rownum])
                    if spwmatch:
                        sch1 = spwmatch.groups()[0]
                        sch1 = re.sub(r"[\'\"]", '', sch1)  # Dequote
                        # Provide a default in case the uvcontsub3 selection excludes
                        # cmds[rownum].  update_spwchan() will throw an exception
                        # in that case.
                        cmd = ''
                        try:
                            #print 'sch1 =', sch1
                            sch2 = update_spwchan(vis, spw, sch1, truncate=True)
                            #print 'sch2 =', sch2
                            ##print 'spwmatch.group() =', spwmatch.group()
                            if sch2:
                                repl = ''
                                if sch2 != '*':
                                    repl = "spw='" + sch2 + "'"
                                cmd = cmds[rownum].replace(spwmatch.group(), repl)
                        #except: # cmd[rownum] no longer applies.
                        except Exception, e:
                            casalog.post(
                                "Error %s updating row %d of FLAG_CMD" % (e,
                                                                          rownum),
                                         'WARN')
                            casalog.post('sch1 = ' + sch1, 'DEBUG1')
                            casalog.post('cmd = ' + cmd, 'DEBUG1')
                        if cmd != cmds[rownum]:
                            mademod = True
                            cmds[rownum] = cmd
                if mademod:
                    casalog.post('Updating FLAG_CMD', 'INFO')
                    mytb.putcol('COMMAND', cmds)

            
        except Exception, instance:
            casalog.post("*** Error \'%s\' updating FLAG_CMD" % (instance),
                         'SEVERE')
            retval = False
        finally:
            if isopen:
                casalog.post('Closing FLAG_CMD', 'DEBUG1')
                mytb.close()
    return retval
