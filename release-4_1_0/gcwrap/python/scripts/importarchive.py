#! /usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import string
import inspect

from parameter_check import *


def importarchive(
    archivefiles=None,
    vis=None,
    bandname=None,
    freqtol=None,
    ):
    """Convert VLA archive file(s) to a CASA visibility data set (MS):

....Keyword arguments:
....archivefiles -- Name of input VLA archive file(s)
........default: <unset>
........example: archivefiles=['AP314_A950519.xp1','AP314_A950519.xp2']
....vis -- Name of output visibility file (MS)
........default: <unset>; example: vis='NGC7538.ms'
....bandname -- VLA Frequency band
........default: <unset> - all bands; example: bandname='K'
........<Options: '4'=48-96 MHz,'P'=298-345 MHz,'L'=1.15-1.75 GHz,
........'C'=4.2-5.1 GHz,'X'=6.8-9.6 GHz,'U'=13.5-16.3 GHz,
........'K'=20.8-25.8 GHz,'Q'=38-51 GHz>
....#projectname -- Observing project name
....#....default: <unset>; example='AP314'
....freqtol -- Tolerance in frequency shift in naming spectral windows
........default: channel width of current spectral window in Hz
........example: 150000.0
....

...."""

    a = inspect.stack()
    stacklevel = 0
    for k in range(len(a)):
        if string.find(a[k][1], 'casapy.py') > 0:
            stacklevel = k
            break
    myf = sys._getframe(len(a) - stacklevel).f_globals
        # myf=sys._getframe(1).f_globals
    vftask = myf['vftask']
    vlafiller = vftask.fill
#    fg = myf['fg']
    aflocal = casac.agentflagger()
        # ##
        # Handle globals or user over-ride of arguments
    function_signature_defaults = \
        dict(zip(importarchive.func_code.co_varnames,
             importarchive.func_defaults))
    for item in function_signature_defaults.iteritems():
        (key, val) = item
        keyVal = eval(key)
        if keyVal == None:
                        # user hasn't set it - use global/default
            pass
        else:
                        # user has set it - use over-ride
                        # sys._getframe().f_globals[key]=keyVal
            myf[key] = keyVal

    archivefiles = myf['archivefiles']
    vis = myf['vis']
    bandname = myf['bandname']
    freqtol = myf['freqtol']
        #
        # ##

    # Add type/menu/range error checking here
    if type(archivefiles) == str:
        archivefiles = [archivefiles]
    arg_names = ['archivefiles', 'vis', 'bandname', 'freqtol']
    arg_values = [archivefiles, vis, bandname, freqtol]
    arg_types = [list, str, str, float]
    try:
        parameter_checktype(arg_names, arg_values, arg_types)
        parameter_checkmenu('bandname', bandname, [
            '4',
            'P',
            'L',
            'C',
            'X',
            'U',
            'K',
            'Q',
            ])
    except TypeError, e:
        print 'importarchive -- TypeError: ', e
        return
    except ValueError, e:
        print 'importarchive -- OptionError: ', e
        return
    # ##

    # Python script
    i = 0
    overwrite = True
    try:
        for archivefile in archivefiles:
            if i > 0:
                overwrite = False
            vlafiller(msname=vis, inputfile=archivefile,
                      overwrite=overwrite, bandname=bandname,
                      freqtol=freqtol)
            i = i + 1

                # write history
        if (type(vis) == str) & os.path.exists(vis):
            ms.open(vis, nomodify=False)
        else:
            raise Exception, \
                'Visibility data set not found - please verify the name'
        ms.writehistory(message='taskname = importuvfits',
                        origin='importuvfits')
        ms.writehistory(message='fitsfile    = "' + str(fitsfile) + '"'
                        , origin='importuvfits')
        ms.writehistory(message='vis         = "' + str(vis) + '"',
                        origin='importuvfits')
        ms.close()
    except:

        print '*** Error ***'

    ok = aflocal.open(vis)
    ok = aflocal.saveflagversion('Original',
                            comment='Original flags at import into CASA'
                            , merge='save')
    ok = aflocal.done()
    saveinputs = myf['saveinputs']
    saveinputs('importarchive', 'importarchive.last')


