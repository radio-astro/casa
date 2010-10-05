import os
from taskinit import *

def ft(vis=None,field=None,spw=None,model=None,nterms=None,reffreq=None,complist=None,incremental=None):
       """ Insert a source model into the MODEL_DATA column of a visibility set:

       A source model (souce.model image) or components list is converted into a
       model visibility that is inserted into the MODEL_DATA column.  This is
       needed to use resolved source in gaincal and in fluxscale.  (Setjy will
       automatically make this ft step.)

       The sources currently available are 3C48, 3C138, 3C147, 3C286
       at 1.4, 5.0, 8.4, 15, 22, 43 GHz.  Their location is site
       dependent.  In Charlottesville and at the AOC, the models are
       in /usr/lib/casapy/data/nrao/VLA/CalModels.

       Keyword arguments:
       vis -- Name of input visibility file
               default: none; example: vis='ngc5921.ms'
       field -- Field name list
               default: '' ==> all
               NOTE: each source must be specified in a multi-source vis.
               field = '1328+307'  specifies source '1328+307'
               field = '4' specified field with index 4
       spw -- Spw selection
               default: spw = '' (all spw)
       model -- Name of input model image
               default: None;
               example: model='/usr/lib/casapy/data/nrao/VLA/CalModels/3C286_X.im'
               Note: The model visibilities are scaled from the model frequency
                     to the observed frequency of the data.
       nterms -- Number of terms used to model the sky frequency dependence
                 default: 1
                 example : nterms=3  represents a 2nd order Taylor-polynomial in frequency
                           and is to be used along with 3 model-image names. 
		           model=['xxx.image.tt0','xxx.image.tt1', 'xxx.image.tt2']
          reffreq -- Reference-frequency about which this Taylor-expansion is defined.
       complist -- Name of component list
               default: None; ; example: complist='test.cl'
               components tool not yet available
       incremental -- Add model visibility to the existing MODEL_DATA visibilties
               default: False; example: incremental=True

       """
       casalog.origin('ft')

       #Python script
       try:
               # Check if datafile exists and open it
               if ((type(vis)==str) & (os.path.exists(vis))):
                       im.open(vis, usescratch=True)
               else:
                       raise Exception, 'Visibility data set not found - please verify the name'
	
               # Select data
               im.selectvis(field=field,spw=spw, usescratch=True)
	       
               # Define image co-ordinates (all defaults)
               im.defineimage()

               # Check if number of model images matches nterms, and settaylorterms.
	       if (nterms < 1) :
		       raise Exception, 'nterms must be greater than or equal to 1';
               if (nterms > 1) :
		       if(type(model)==str or (not (type(model)==list and len(model)==nterms)) ):
			       raise Exception, 'For nterms>1, please provide a list of nterms model-image names';
	               # Check if model images exist on disk
		       for imod in model:
		            if ( not os.path.exists(imod) ):
		                 raise Exception, 'Model image '+imod+' cannot be found';
		       # parse the reference-frequency field.
                       qat=qatool.create();
                       try:
		          rff=qat.canonical(reffreq);
		       except Exception, instance:
                          print '*** Error *** In conversion of reffreq=\'',reffreq,'\' to a numerical value';
                          raise Exception, instance
                       reffreqVal=rff['value'];  # This is the frequency in Hz
		       if(reffreqVal==0.0):   # if unspecified, set the default from the model image
			       ia.open(model[0]);
			       icsys = ia.coordsys();
			       ia.close();
                               reffreqVal=icsys.referencevalue(type='spectral')['numeric'][0];
			       casalog.post('Using reference frequency from model image : '+str(reffreqVal)+' Hz');
		       # set nterms and ref-freq
		       im.settaylorterms(ntaylorterms=nterms,reffreq=reffreqVal)
	       else:
	               if(not type(model)==str and not (type(model)==list and len(model)==1) ) :
			       raise Exception, 'For nterms=1, the name of model image must contain a single string';
		       if( type(model)==list ): 
			       model = model[0];
	               # Check if model image exists on disk
		       if ( not os.path.exists(model) ):
			       raise Exception, 'Model image '+model+' cannot be found';


               # Do the forward transform and close.
               im.ft(model=model,complist=complist,incremental=incremental)
               im.close()


               #write history
               ms.open(vis,nomodify=False)
               ms.writehistory(message='taskname = ft',origin='ft')
               ms.writehistory(message='vis         = "'+str(vis)+'"',origin='ft')
               ms.writehistory(message='field       = "'+str(field)+'"',origin='ft')
               ms.writehistory(message='spw         = "'+str(spw)+'"',origin='ft')
               ms.writehistory(message='model       = "'+str(model)+'"',origin='ft')
               ms.writehistory(message='complist    = "'+str(complist)+'"',origin='ft')
               ms.writehistory(message='incremental = "'+str(incremental)+'"',origin='ft')
               ms.close()

       except Exception, instance:
               print '*** Error ***',instance
