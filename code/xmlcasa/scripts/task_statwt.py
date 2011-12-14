from taskinit import mstool, tbtool, casalog

def task_statwt(vis):
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
        
        myms.open(vis)
        myms.statwt()
        myms.close()
        
        
    except Exception, e:
        casalog.post("Error setting WEIGHT and SIGMA for %s:" % vis, 'SEVERE')
        casalog.post("%s" % e, 'SEVERE')
        retval = False
    return retval
        
    
