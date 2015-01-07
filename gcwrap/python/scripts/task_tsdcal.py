import sys
import os
from taskinit import *
(cb,) = gentools(['cb'])
def tsdcal(infile=None, calmode='tsys', fraction='10%', noff=-1, width=0.5, elongated=False, applytable='',interp='', spwmap={},
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

		if((type(calmode)==str) and calmode!='tsys'): 
			raise Exception, 'A calmode must be tsys'

		if (outfile==''):
                     raise Exception, 'A outfile name must be specified'


                """
                if ((type(infile)==str) & (os.path.exists(infile))):
                     # don't need scr col for this
                     cb.open(filename=infile,compress=False,addcorr=False,addmodel=False)  
                else:
                     raise Exception, 'Visibility data set not found - please verify the name'

                if (caltable==''):
                     raise Exception, 'A caltable name must be specified'

                if caltype=='tecim' and not (type(infile2)==str and os.path.exists(infile2)):
                     raise Exception, 'An existing tec map must be specified in infile2'

              # call a Python function to retreive ant position offsets automatically (currently EVLA only)
              if (caltype=='antpos' and antenna==''):
                casalog.post(" Determine antenna position offests from the baseline correction database")
                import correct_ant_posns as getantposns 
                # correct_ant_posns returns a list , [return_code, antennas, offsets]
                antenna_offsets=getantposns.correct_ant_posns(infile,False)
                if ((len(antenna_offsets)==3) and
                    (int(antenna_offsets[0])==0) and
                    (len(antenna_offsets[1])>0) ) :
                       antenna = antenna_offsets[1]
                       parameter = antenna_offsets[2] 
                else:
                   #raise Exception, 'No offsets found. No caltable created.'
                   import warnings
                   warnings.simplefilter('error',UserWarning)
                   warnings.warn('No offsets found. No caltable created.')

                cb.specifycal(caltable=caltable,time="",spw=spw,antenna=antenna,pol=pol,
                            caltype=caltype,parameter=parameter,infile2=infile2)

"""
		cb.specifycal(caltable=outfile,time="",spw=spw,pol=pol,
                            caltype=calmode)



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
