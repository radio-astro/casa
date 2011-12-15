from taskinit import mstool, tbtool, casalog, write_history

def statwt(vis, scattertype, byantenna, sepacs, fitspw, fitcorr, combine,
           timebin, minsamp, field, spw, antenna, timerange, scan, intent,
           array, correlation, obs):
    """
    Sets WEIGHT and SIGMA using the scatter of the visibilities.
    """
    casalog.origin('statwt')
    retval = True
    try:
        myms = mstool.create()
        mytb = tbtool.create()

        datacol = 'DATA'
        mytb.open(vis)
        colnames  = mytb.colnames()
        mytb.close()
        for datacol in ['CORRECTED_DATA', 'DATA', 'junk']:
            if datacol in colnames:
                break
        if datacol == 'junk':
            raise ValueError(vis + " does not have a data column")        
        
        myms.open(vis, nomodify=False)
        retval = myms.statwt(scattertype, byantenna, sepacs, fitspw, fitcorr, combine,
                             timebin, minsamp, field, spw, antenna, timerange, scan, intent,
                             array, correlation, obs)
        myms.close()
    except Exception, e:
        casalog.post("Error setting WEIGHT and SIGMA for %s:" % vis, 'SEVERE')
        casalog.post("%s" % e, 'SEVERE')
        if False:  # Set True for debugging.
            for p in statwt.func_code.co_varnames[:statwt.func_code.co_argcount]:
                v = eval(p)
                print p, "=", v, ", type =", type(v)
        retval = False

    if retval:
        try:
            param_names = statwt.func_code.co_varnames[:statwt.func_code.co_argcount]
            param_vals = [eval(p) for p in param_names]
            retval &= write_history(myms, vis, 'statwt', param_names, param_vals,
                                    casalog)
        except Exception, instance:
            casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                         'WARN')        
    return retval
        
    
