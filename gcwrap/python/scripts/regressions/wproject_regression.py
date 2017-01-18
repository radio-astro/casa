import time
import os
import sys
from casa_stack_manip import stack_frame_find

gl=stack_frame_find( )
regstate = True
try:
	im=gl['casac'].imager()
	me=gl['me']
	time1=time.time()
	im.open('coma.ms')

	im.selectvis(spw=0, field=0);
	mydir=me.direction('J2000', '12h30m48', '12d24m0')
	im.defineimage(nx=200, ny=200, cellx='30arcsec', celly='30arcsec',  phasecenter=mydir); 
	im.make('outlier1re');
	im.defineimage(nx=2000, ny=2000, cellx='30arcsec',celly='30arcsec',phasecenter=0, facets=1);
	im.setoptions(ftmachine='wproject',wprojplanes=400, padding=1.2)
	im.make('bla1re')
	    
	im.clean(algorithm='wfclark',model=['bla1re', 'outlier1re'], image=['bla1re.restored', 'outlier1re.restored'], niter=10000);

	im.done()

	time2=time.time();
	print 'Time taken for wproject= ', (time2-time1)/60,'mins'
except Exception:
	print("Unexpected error:", sys.exc_info())
	regstate= False

if(regstate):
   	print ''
   	print 'Regression PASSED'
else:
   	print ''
   	print 'Regression FAILED'

