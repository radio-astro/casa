import os
from taskinit import *

def ft(vis=None,field=None,spw=None,model=None,complist=None,incremental=None):
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
       complist -- Name of component list
               default: None; ; example: complist='test.cl'
               components tool not yet available
       incremental -- Add model visibility to the existing MODEL_DATA visibilties
               default: False; example: incremental=True

       """
       casalog.origin('ft')

       #Python script
       try:
               if ((type(vis)==str) & (os.path.exists(vis))):
                       im.open(vis, usescratch=True)
               else:
                       raise Exception, 'Visibility data set not found - please verify the name'
               im.selectvis(field=field,spw=spw, usescratch=True)
               im.setimage()
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
