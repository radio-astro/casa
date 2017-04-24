from taskinit import mstool, tbtool, casalog, write_history

def statwt2(vis):
    casalog.origin('statwt2')
    try:
        myms = mstool()
        myms.open(vis, nomodify=False)
        myms.statwt2() 
        return True
    except Exception, instance:
        casalog.post( '*** Error ***'+str(instance), 'SEVERE' )
        raise
    finally:
        myms.done()
    
