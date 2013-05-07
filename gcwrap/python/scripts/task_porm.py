from taskinit import *
import tempfile
import shutil

def porm(
    imagename, rm, rmerr, pa0, pa0err, nturns, chisq,
    sigma, rmfg, rmmax, maxpaerr, 
):
    casalog.origin('prom')
    myia = iatool()
    mypo = potool()
    tmpim = ""
    try:
        if len(imagename) == 0:
            raise Exception, "imagename must be specified."
        if type(imagename) == type(['s']):
            # negative axis value means concatenate along spectral axis
            tmpim = tempfile.mkdtemp(suffix=".im", prefix="_porm_concat")
            print "*** aa"
            myia = myia.imageconcat(
                outfile=tmpim, infiles=imagename, relax=True,
                axis=-1, overwrite=True
            )
            print "*** ab"

            if not myia:
                raise Exception("Unable to concatenate images.")
            myia.done()
            mypo.open(tmpim)
        else:
            if (not mypo.open(imagename)):
                raise Exception, "Cannot create image analysis tool using " + imagename
        print "*** ac"

        mypo.rotationmeasure(
            rm=rm, rmerr=rmerr, pa0=pa0, pa0err=pa0err, nturns=nturns, chisq=chisq,
            sigma=sigma, rmfg=rmfg, rmmax=rmmax, maxpaerr=maxpaerr
        )
        print "*** ad"

        return True
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if (myia):
            myia.done()
        if (mypo):
            mypo.done()          
        if len(tmpim) > 0:
            try:
                shutil.rmtree(tmpim)
            except Exception, e:
                print "Could not remove " + tmpim + " because " + str(e)
            
