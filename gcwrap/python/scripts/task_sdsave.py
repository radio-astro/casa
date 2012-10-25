import os
from taskinit import *

import asap as sd
from asap.scantable import is_scantable
import sdutil

def sdsave(infile, antenna, getpt, rowlist, scanlist, field, iflist, pollist, scanaverage, timeaverage, tweight, polaverage, pweight, restfreq, outfile, outform, overwrite):

        casalog.origin('sdsave')


        ###
        ### Now the actual task code
        ###

        try:
            restore = False
            rfset = (restfreq != '') and (restfreq != [])
            #load the data with or without averaging

            sdutil.assert_infile_exists(infile)

            project = sdutil.get_default_outfile_name(infile,
                                                      outfile,
                                                      '_saved')
            sdutil.assert_outfile_canoverwrite_or_nonexistent(project,
                                                              outform,
                                                              overwrite)

            s = sd.scantable(infile,average=scanaverage,antenna=antenna,getpt=getpt)

            #Select rows
            try: 
                #Apply the selection
                sel = sdutil.get_selector(in_scans=scanlist,
                                          in_ifs=iflist,
                                          in_pols=pollist,
                                          in_field=field,
                                          in_rows=rowlist)
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
            spave = sdutil.doaverage(s, scanaverage, timeaverage,
                                       tweight, polaverage, pweight)

            if spave == s and rfset and is_scantable(infile) and \
               sd.rcParams['scantable.storage'] == 'disk':
                    molids = s._getmolidcol_list()
                    restore = True
            del s

            # set rest frequency
            casalog.post('restore=%s'%(restore))
            if ( rfset ):
                spave.set_restfreqs(sdutil.normalise_restfreq(restfreq))

            # save
            sdutil.save(spave, outfile, outform, overwrite)

            # DONE
            

        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                raise Exception, instance
                return
        finally:
                try:
                        # Restore MOLECULE_ID in the table
                        if restore:
                                casalog.post( "Restoreing MOLECULE_ID column in %s " % infile )
                                spave._setmolidcol_list(molids)
                                del molids
                        # Final clean up
                        del spave
                except:
                        pass
                casalog.post('')



