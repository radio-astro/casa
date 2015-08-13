from taskinit import mstool, tbtool, casalog, write_history

def statwt(vis, dorms, byantenna, sepacs, fitspw, fitcorr, combine,
           timebin, minsamp, field, spw, antenna, timerange, scan, intent,
           array, correlation, obs, datacolumn):
    """
    Sets WEIGHT and SIGMA using the scatter of the visibilities.
    """
    casalog.origin('statwt')
    retval = True
    try:
        myms = mstool()
        mytb = tbtool()
        
        # parameter check for those not fully implemeted
        # (should be taken out once implemented)
        if byantenna:
          raise ValueError("byantenna=True is not supported yet")
        if fitcorr !='':
          raise ValueError("fitcorr is not supported yet")
        if timebin !='0s' and timebin !='-1s':
          raise ValueError("timebin is not supported yet")
                     
        datacol = 'DATA'
        mytb.open(vis)
        colnames  = mytb.colnames()
        mytb.close()


        for datacol in ['CORRECTED_DATA', 'DATA', 'junk']:
            if datacol in colnames:
                break
        if datacol == 'junk':
            raise ValueError(vis + " does not have a data column")        

        if datacolumn == 'float_data':
           raise ValueError("float_data is not yet supported") 

        if datacolumn == 'corrected' and datacol == 'DATA': # no CORRECTED_DATA case (fall back to DATA)
           casalog.post("No %s column found, using %s column" % (datacolumn.upper()+'_DATA', datacol),'WARN')
           datacolumn = datacol
        else:
           if datacolumn=='corrected':
               datacolumn_name=datacolumn.upper()+'_DATA'
           else:
               datacolumn_name=datacolumn.upper()
           casalog.post("Using %s column to determine visibility scatter" % datacolumn_name)
       
        if ':' in spw:
            casalog.post('The channel selection part of spw will be ignored.', 'WARN')
        
        if len(correlation)>0:
            correlation=''
            casalog.post('Correlation selection in statwt has been disabled as of CASA v4.5', 'WARN')

        myms.open(vis, nomodify=False)
        retval = myms.statwt(dorms, byantenna, sepacs, fitspw, fitcorr, combine,
                             timebin, minsamp, field, spw, antenna, timerange, scan, intent,
                             array, correlation, obs, datacolumn)
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
        
    
