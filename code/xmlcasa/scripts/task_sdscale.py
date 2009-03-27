import os
import sys
from taskinit import *

import asap as sd

def sdscale(sdfile, factor, scaletsys, outfile, overwrite):

        casalog.origin('sdscale')


        try:
            #load the data with or without averaging
            if sdfile=='':
                raise Exception, 'sdfile is undefined'

            filename = os.path.expandvars(sdfile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception, s

            if outfile == '':
                    outfile = sdfile.rstrip('/')+'_scaled'+str(factor)
            if not overwrite and not outfile=='':
                outfilename = os.path.expandvars(outfile)
                outfilename = os.path.expanduser(outfilename)
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s

            s=sd.scantable(sdfile,False)

            #check the format of the sdfile
            if isinstance(sdfile, str):
              #import os.path
              #sdfile = os.path.expandvars(sdfile)
              #sdfile = os.path.expanduser(sdfile)
              #if os.path.isdir(sdfile) and os.path.exists(sdfile+'/table.info'):
              #  if os.path.exists(sdfile+'/table.f1'):
              if os.path.isdir(filename) and os.path.exists(filename+'/table.info'):
                if os.path.exists(filename+'/table.f1'):
                  format = 'MS2'
                else:
                  format = 'ASAP'
              else:
                  format = 'SDFITS'

            if factor == 1.0:
                  print "scaling factor is %s. No scaling" % factor
                  return
            #if outfile == 'none':
            #  s.scale(factor, scaletsys, True)
            #  s.save(sdfile, format, True)
            #  if scaletsys:
            #    print "Scaled spectra and Tsys by "+str(factor)
            #  else:
            #    print "Scaled spectra by "+str(factor)

            #  del s
            #else:
            s2 = s.scale(factor, scaletsys, False)
            logfile='sdscale.log'
            stdsave=sys.stdout
            tsysfile=open(logfile, 'w')
            if scaletsys:
                    sys.stdout=tsysfile
                    oldtsys=s._row_callback(s._gettsys, "Original Tsys")
                    sys.stdout=stdsave
                    print "Scaled spectra and Tsys by "+str(factor)
                    sys.stdout=tsysfile
                    newtsys=s2._row_callback(s2._gettsys, "Scaled Tsys")
                    sys.stdout=stdsave
            else:
                    print "Scaled spectra by "+str(factor)
                    sys.stdout=tsysfile
                    oldtsys=s2._row_callback(s2._gettsys, "Tsys (not scaled)")
                    sys.stdout=stdsave
            tsysfile.close()
            tsysfile=open(logfile, 'r')
            linelist=tsysfile.readlines()
            for i in range( len(linelist) ):
                    print linelist[i],
            tsysfile.close()

            s2.save(outfile, format, overwrite)
            print "Wrote scaled data to %s file, %s " % (format, outfile)
            del s,s2

            # DONE

        except Exception, instance:
                print '***Error***',instance
                return


