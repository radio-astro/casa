import os
import sys
from taskinit import *
import sdutil
import asap as sd
from asap.scantable import is_scantable, is_ms

def sdscale(infile, antenna, factor, scaletsys, outfile, overwrite):

        casalog.origin('sdscale')


        try:
            # no scaling
            if factor == 1.0:
                casalog.post( "scaling factor is %s. No scaling" % factor )
                return

            #load the data with or without averaging
            sdutil.assert_infile_exists(infile)

            #check the format of the infile
            if is_scantable(infile):
                outform = 'ASAP'
            elif is_ms(infile):
                outform = 'MS2'
            else:
                outform = 'SDFITS'

            project = sdutil.get_default_outfile_name(infile,
                                                      outfile,
                                                      '_scaled'+str(factor))
            sdutil.assert_outfile_canoverwrite_or_nonexistent(project,
                                                              outform,
                                                              overwrite)

            s = doscale(infile, antenna, factor, scaletsys)

            sdutil.save(s, outfile, outform, overwrite)
            casalog.post( "Wrote scaled data to %s file, %s " % (outform, outfile) )
            del s

            # DONE

        except Exception, instance:
                sdutil.process_exception(instance)
                raise Exception, instance
                return


def doscale(infile, antenna, factor, scaletsys):
    s=sd.scantable(infile,average=False,antenna=antenna)

    if isinstance( factor, str ):
        casalog.post( 'read factor from \'%s\'' %factor )
        f = open( factor )
        lines = f.readlines()
        f.close()
        del f
        for i in range( len(lines) ):
            lines[i] = lines[i].rstrip('\n')
            lines[i] = lines[i].split()
            for j in range( len(lines[i]) ):
                lines[i][j] = float( lines[i][j] )
        thefactor = lines
    else:
        thefactor = factor

    s2 = s.scale(thefactor, scaletsys, False)
    casalog.post( "Scaled spectra and Tsys by "+str(factor) )

    if scaletsys:
        oldtsys=s._row_callback(s._gettsys, "Original Tsys")
        newtsys=s2._row_callback(s2._gettsys, "Scaled Tsys")
    else:
        oldtsys=s2._row_callback(s2._gettsys, "Tsys (not scaled)")
    return s2
