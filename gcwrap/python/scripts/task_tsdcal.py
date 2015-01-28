import sys
import os
from taskinit import *
from applycal import applycal

(cb,) = gentools(['cb'])
def tsdcal(infile=None, calmode='tsys', fraction='10%', noff=-1, width=0.5, elongated=False, applytable='',interp='', spwmap=[],
		field='', spw='', scan='', pol='', outfile='', overwrite=False): 

       """ Externally specify calibration solutions af various types
       """

       #Python script
       try:


		if ((type(infile)==str) and (os.path.exists(infile))):
                     # don't need scr col for this
                     cb.open(filename=infile,compress=False,addcorr=False,addmodel=False)
		else:
			raise Exception, 'Infile data set not found - please verify the name'

                if((type(calmode)==str) and calmode.lower() not in ['tsys', 'ps', 'apply']): 
			raise Exception, 'A calmode must be either \'ps\' or \'tsys\' or \'apply\''

		if ((type(calmode)==str) and (calmode=='apply') and (applytable =='')):
                        raise Exception, 'A applytable name must be specified'




		if ((type(calmode)==str) and (calmode=='apply') and (applytable !='') and (os.path.exists(applytable))):
                        applycal(vis=infile, docallib=False, gaintable=applytable, applymode='calonly')
                        if (outfile != ''):
                                raise UserWarning, 'An outfile is not generated but is added to MS as a new table, namely corrected data'


		calmodemap = {'tsys': 'tsys',
                              'ps': 'sdsky_ps'}
		

		if(calmode!='apply'):	     
			cb.specifycal(caltable=outfile,time="",spw=spw,pol=pol,
                            caltype=calmodemap[calmode])



              #cb.close()
       
       except UserWarning, instance:
              print '*** Warning ***',instance
              #cb.close()

       except Exception, instance:
              print '*** Error ***',instance
              #cb.close()
              raise Exception, instance

       finally:
              cb.close()
