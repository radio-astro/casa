import os
import shutil
import string
from taskinit import *
import pdb

def imreframe(imagename=None, output=None, outframe=None, epoch=None, restfreq=None):
    try:
        casalog.origin('imreframe')
        if(output==imagename):
            output=''
        needregrid=False
        outframe=string.lower(outframe)
        if(((outframe == 'topo') or (outframe=='geo')) and (epoch != '')):
            needregrid=True
        myia,me=gentools(['ia', 'me'])
        myia.open(imagename)
        c=myia.coordsys()
        me.doframe(me.observatory(c.telescope()))
        me.doframe(c.referencevalue('m', 'direction')['measure']['direction'])
        me.doframe(c.epoch())
        reffreq=c.referencevalue('m', 'spectral')['measure']['spectral']['frequency']
        hasspec,pixax,worldax=c.findcoordinate('spectral')
        if(not hasspec):
            raise Exception, 'Could not find spectral axis'
        if(outframe != ''):
            c.setconversiontype(spectral=outframe)
        if(restfreq != ''):
            c.setrestfrequency(value=qa.quantity(restfreq, 'Hz'))
        if(epoch != ''):
            c.setepoch(me.epoch('utc', epoch))
            me.doframe(me.epoch('utc', epoch))
        if(not needregrid):
            if(output != ''):
                myia.fromimage(outfile=output, infile=imagename, overwrite=True)
                myia.close()
                myia.open(output)
            myia.setcoordsys(c.torecord())
            myia.done()
        else:
            c.setreferencecode(outframe, 'spectral', True)
            newreffreq=me.measure(reffreq, outframe)
            c.setreferencevalue(qa.tos(newreffreq['m0']), 'spectral')
            outname='_temp_regrid_image' if(output=='') else output
            shp=myia.shape()             
            ib=myia.regrid(outfile=outname, shape=shp, csys=c.torecord(), 
                           axes=pixax, overwrite=True, asvelocity=False)
            ib.setcoordsys(c.torecord())
            if(output==''):
                myia.done()
                ib.rename(name=imagename, overwrite=True)
            myia.done()
            ib.done()
        return True
        
    except Exception, instance:
        if('myia' in locals()):
            myia.close()
        if('ib' in locals()):
            ib.close()
        raise instance
 
