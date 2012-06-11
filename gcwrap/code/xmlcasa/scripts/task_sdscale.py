import os
import sys
from taskinit import *

import asap as sd

def sdscale(infile, antenna, factor, scaletsys, outfile, overwrite):

        casalog.origin('sdscale')


        try:
            #load the data with or without averaging
            if infile=='':
                raise Exception, 'infile is undefined'

            filename = os.path.expandvars(infile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception, s

            if outfile == '':
                    outfile = infile.rstrip('/')+'_scaled'+str(factor)
            if not overwrite and not outfile=='':
                outfilename = os.path.expandvars(outfile)
                outfilename = os.path.expanduser(outfilename)
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s

            s=sd.scantable(infile,average=False,antenna=antenna)

            #check the format of the infile
            if isinstance(infile, str):
              #import os.path
              #infile = os.path.expandvars(infile)
              #infile = os.path.expanduser(infile)
              #if os.path.isdir(infile) and os.path.exists(infile+'/table.info'):
              #  if os.path.exists(infile+'/table.f1'):
              if os.path.isdir(filename) and os.path.exists(filename+'/table.info'):
                if os.path.exists(filename+'/table.f1'):
                  format = 'MS2'
                else:
                  format = 'ASAP'
              else:
                  format = 'SDFITS'

            if factor == 1.0:
                  #print "scaling factor is %s. No scaling" % factor
                  casalog.post( "scaling factor is %s. No scaling" % factor )
                  return
            elif isinstance( factor, str ):
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
                  factor = lines
            #if outfile == 'none':
            #  s.scale(factor, scaletsys, True)
            #  s.save(infile, format, True)
            #  if scaletsys:
            #    print "Scaled spectra and Tsys by "+str(factor)
            #  else:
            #    print "Scaled spectra by "+str(factor)

            #  del s
            #else:
            s2 = s.scale(factor, scaletsys, False)
            if scaletsys:
                    oldtsys=s._row_callback(s._gettsys, "Original Tsys")
                    #print "Scaled spectra and Tsys by "+str(factor)
            del s
	    
            if scaletsys:
                    casalog.post( "Scaled spectra and Tsys by "+str(factor) )
                    newtsys=s2._row_callback(s2._gettsys, "Scaled Tsys")
            else:
                    #print "Scaled spectra by "+str(factor)
                    casalog.post( "Scaled spectra by "+str(factor) )
                    oldtsys=s2._row_callback(s2._gettsys, "Tsys (not scaled)")

            s2.save(outfile, format, overwrite)
            #print "Wrote scaled data to %s file, %s " % (format, outfile)
            casalog.post( "Wrote scaled data to %s file, %s " % (format, outfile) )
            del s2

            # DONE

        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return


