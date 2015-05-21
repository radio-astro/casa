import numpy
import os
from taskinit import *
import sdutil
ms,sdms,tb = gentools(['ms','sdms','tb'])

def tsdbaseline(infile=None, datacolumn=None, antenna=None, field=None, spw=None, timerange=None, scan=None, pol=None, maskmode=None, thresh=None, avg_limit=None, edge=None, blmode=None, dosubtract=None, blformat=None, bloutput=None, bltable=None, blfunc=None, order=None, npiece=None, applyfft=None, fftmethod=None, fftthresh=None, addwn=None, rejwn=None, clipthresh=None, clipniter=None, blparam=None, verify=None, verbose=None, showprogress=None, minnrow=None, outfile=None, overwrite=None):

    casalog.origin('tsdbaseline')

    try:
        if ((os.path.exists(outfile)) and (not overwrite)):
            raise Exception(outfile+' exists.')
        if (maskmode!='list'):
            raise ValueError, "maskmode='%s' is not supported yet" % maskmode
        if (blfunc=='variable' and not os.path.exists(blparam)):
            raise ValueError, "input file '%s' does not exists" % blparam
        if (spw == ''): spw = '*'
        
        if blmode == 'apply':
            if not os.path.exists(bltable):
                raise ValueError, "file specified in bltable '%s' does not exist." % bltable

            with sdutil.tbmanager(infile + '/DATA_DESCRIPTION') as tb:
                spw_ids = tb.getcol('SPECTRAL_WINDOW_ID')
            with sdutil.tbmanager(infile + '/ANTENNA') as tb:
                ant_ids = range(tb.nrows())
            with sdutil.tbmanager(infile + '/FEED') as tb:
                feed_ids = numpy.unique(tb.getcol('FEED_ID'))
            
            sel_cond_list = []
            for spw_idx in spw_ids:
                for ant_idx in ant_ids:
                    for feed_idx in feed_ids:
                        sel_cond_list.append({'spw': str(spw_idx),
                                              'ant': str(ant_idx),
                                              'feed': str(feed_idx)})

            sorttab_info = remove_sorted_table_keyword(infile)
            
            for sel_cond in sel_cond_list:
                sdms.open(infile)
                sdms.set_selection(spw=sel_cond['spw'], field=field, 
                                   antenna=sel_cond['ant'],
                                   timerange=timerange, scan=scan)
                #sdms.apply_baseline_table()
                sdms.close()
                
            restore_sorted_table_keyword(infile, sorttab_info)
            
        elif blmode == 'fit':
            blout_exists = False
            if (isinstance(bloutput, str) and os.path.exists(bloutput)):
                blout_exists = True
            elif isinstance(bloutput, list):
                for blout in bloutput:
                    if os.path.exists(blout):
                        blout_exists = True
                        break
            if blout_exists:
                raise ValueError, "file(s) specified in bloutput exists."

            selection = ms.msseltoindex(vis=infile, spw=spw, field=field, 
                                        baseline=str(antenna), time=timerange, 
                                        scan=scan)

            if blfunc == 'variable':
                sorttab_info = remove_sorted_table_keyword(infile)
        
            sdms.open(infile)
            sdms.set_selection(spw=sdutil.get_spwids(selection), field=field, 
                               antenna=str(antenna), timerange=timerange, 
                               scan=scan)

            params, func = prepare_for_baselining(blfunc=blfunc,
                                                  datacolumn=datacolumn,
                                                  outfile=outfile,
                                                  bltable=bloutput,
                                                  dosubtract=dosubtract,
                                                  spw=spw,
                                                  pol=pol,
                                                  order=order,
                                                  npiece=npiece,
                                                  blparam=blparam,
                                                  clip_threshold_sigma=clipthresh,
                                                  num_fitting_max=clipniter+1)
            func(**params)

            if (blfunc == 'variable'):
                restore_sorted_table_keyword(infile, sorttab_info)


    except Exception, instance:
        raise Exception, instance


def prepare_for_baselining(**keywords):
    params = {}
    funcname = 'subtract_baseline'

    blfunc = keywords['blfunc']
    keys = ['datacolumn', 'outfile', 'bltable', 'dosubtract', 'spw', 'pol']
    if blfunc in ['poly', 'chebyshev']:
        keys += ['blfunc', 'order', 'clip_threshold_sigma', 'num_fitting_max']
    elif blfunc == 'cspline':
        keys += ['npiece', 'clip_threshold_sigma', 'num_fitting_max']
        funcname += ('_' + blfunc)
    elif blfunc == 'variable':
        keys += ['blparam']
        funcname += ('_' + blfunc)
    else:
        raise ValueError, "Unsupported blfunc = %s" % blfunc
    for key in keys: params[key] = keywords[key]

    baseline_func = getattr(sdms, funcname)

    return params, baseline_func
    
    
def remove_sorted_table_keyword(infile):
    res = {'is_sorttab': False, 'sorttab_keywd': '', 'sorttab_name': ''}
    with sdutil.tbmanager(infile, nomodify=False) as tb:
        try:
            sorttab_keywd = 'SORTED_TABLE'
            if sorttab_keywd in tb.keywordnames():
                res['is_sorttab'] = True
                res['sorttab_keywd'] = sorttab_keywd
                res['sorttab_name'] = tb.getkeyword(sorttab_keywd)
                tb.removekeyword(sorttab_keywd)
        except Exception, e:
            raise Exception, e

    return res

def restore_sorted_table_keyword(infile, sorttab_info):
    if sorttab_info['is_sorttab'] and (sorttab_info['sorttab_name'] != ''):
        with sdutil.tbmanager(infile, nomodify=False) as tb:
            try:
                tb.putkeyword(sorttab_info['sorttab_keywd'],
                              sorttab_info['sorttab_name'])
            except Exception, e:
                raise Exception, e
