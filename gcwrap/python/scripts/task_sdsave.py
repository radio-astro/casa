import os
from taskinit import *

import asap as sd

def sdsave(infile, antenna, getpt, rowlist, scanlist, field, iflist, pollist, scanaverage, timeaverage, tweight, polaverage, pweight, outfile, outform, overwrite):

        casalog.origin('sdsave')


        ###
        ### Now the actual task code
        ###

        try:
            #load the data with or without averaging
            if infile=='':
                raise Exception, 'infile is undefined'

            filename = os.path.expandvars(infile)
            file2name = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception,s 

            if outfile=='':
                project = infile.rstrip('/') + '_saved'
            else:
                project = outfile
            outfilename = os.path.expandvars(project)
            outfilename = os.path.expanduser(outfilename)
            if not overwrite and (outform!='ascii' and outform!='ASCII'):
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s

            s=sd.scantable(infile,average=scanaverage,antenna=antenna,getpt=getpt)

	    #Select rows
            sel = sd.selector()
	    if (type(rowlist) == list ):
	      rows = rowlist
	    else:
	      rows = [rowlist]
	    
	    if ( len(rows) > 0):
	      sel.set_rows(rows)

            #Select scan and field
            if (type(scanlist) == list ):
              scans = scanlist
            else:
              scans = [scanlist]

            if ( len(scans) > 0 ):
              sel.set_scans(scans)

            #Select source
            if (field != '' ):
              sel.set_name(field)

            #Select IFs
            if (type(iflist) == list):
              ifs = iflist
            else:
              ifs = [iflist]

            if (len(ifs) > 0 ):
              sel.set_ifs(ifs)

            if (type(pollist) == list):
              pols = pollist
            else:
              pols = [pollist]
            
            if(len(pols) > 0 ):
              sel.set_polarisations(pols)

            try: 
                #Apply the selection
                if not sel.is_empty():
                    s.set_selection(sel)
                del sel
            except Exception, instance:
                #print '***Error***',instance
                #print 'No output written.'
                casalog.post( str(instance), priority = 'ERROR' )
                casalog.post( 'No output written.', priority = 'ERROR' )
                return
   

            #Apply averaging
            if ( timeaverage ):
              if tweight=='none':
                errmsg = "Please specify weight type of time averaging"
                raise Exception,errmsg
              # NOTE: sd.average_time always returns new instance even if insitu=True
              stave = sd.average_time(s, weight=tweight)
              if ( polaverage ):
                if pweight=='none':
                  errmsg = "Please specify weight type of polarization averaging"
                  raise Exception,errmsg 
                if len(stave.getpolnos())<2:
                   casalog.post('Single polarization, ignore polarization averaging option','WARN')
                   #spave = stave.copy()
                   spave = stave
                else:
                   spave = stave.average_pol(weight=pweight)
              else:
                #spave = stave.copy()
                spave = stave

              del stave

            else:
              if ( polaverage ):
                if pweight=='none':
                  errmsg = "Please specify weight type of polarization averaging"
                  raise Exception,errmsg 
                if len(s.getpolnos())<2:
                  casalog.post('Single polarization, ignore polarization averaging option','WARN')
                  #spave=s.copy()
                  spave=s
                else:
                  # NOTE: average_pol always returns new instance even if insitu=True
                  spave = s.average_pol(weight=pweight)
              else:
                #spave = s.copy()
                spave = s

            del s

            # save
            if ( (outform == 'ASCII') or (outform == 'ascii') ):
                    outform = 'ASCII'
            elif ( (outform == 'ASAP') or (outform == 'asap') ):
                    outform = 'ASAP'
            elif ( (outform == 'SDFITS') or (outform == 'sdfits') ):
                    outform = 'SDFITS'
            elif ( (outform == 'MS') or (outform == 'ms') or (outform == 'MS2') or (outform == 'ms2') ):
                    outform = 'MS2'
            else:
                    outform = 'ASAP'

            if overwrite and os.path.exists(outfilename):
              os.system('rm -rf %s' % outfilename)

            spave.save(project, outform, overwrite)

            # DONE
            

        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return


