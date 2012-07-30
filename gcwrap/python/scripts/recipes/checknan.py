from taskinit import *
import numpy as np

def CheckUVWForNan(ms='', fix=False):
    """
    script to check for NaN in uvw
    if fix is true it will flag the row with nan data
    and replace the uvw with 0
    you can see how to do that to other columns too
    """
    tb.open(ms, nomodify=(not fix))
    uvw=tb.getcol('UVW')
    flg=tb.getcol('FLAG_ROW')
    nanoo=np.isnan(uvw)
    for k in range(nanoo.shape[1]):
      if(nanoo[0,k] or nanoo[1,k] or nanoo[2,k]):
          print 'row=', k
          uvw[0,k]=0
          uvw[1,k]=0
          uvw[2,k]=0
          flg[k]=True
    if(fix):
        tb.putcol('UVW', uvw)
        tb.putcol('FLAG_ROW', flg)
    tb.done()
     
##########################################
def CheckColsForNan(msname='',fix=False,colnames=[],timeinterval=1000):
    """
    Check for NaNs in specified columns of the MS.
    ms : Name of MS. 
    fix : False/True : If True, 
                             For data/corrected_data, set the corresponding value to zero and
                                       flag to True in the FLAG column
                             For weight, weight_spectrum, set the corresponding value to zero
                             For all other columns, do not fix/change anything.
    colnames : ['data','weight'] :  MS Column names on which to check for NaNs
    timeinterval : 1000 : timerange in seconds, to decide chunk sizes while iterating 
                                   through the MS (remember, chunks of this size are read 
                                   into python).
    Alternate (recommended) way to flag NaNs from data columns :
        tflagdata(vis=ms, mode='clip')    
    """
    ms = casac.ms()

    # Open the MS with the intent to modify it.
    ms.open(msname,nomodify=(not fix));

    # Iterate through the dataset in chunks defined by
    # a time interval of 1000 seconds.
    ms.iterinit(interval=timeinterval);
    
    ## Initialize the ms iterator
    ms.iterorigin();
    
    ## Iterate until the end of the MS is reached.
    counter=0;
    moretodo=True;
    while moretodo: 
        counter=counter+1;
        
        print 'Chunk : ', counter 

        resdat = ms.getdata(items=colnames);
  
        for cname in colnames:
            shp = resdat[cname].shape
            nancount = np.isnan( resdat[cname] )
            if np.any( nancount ) :
                print 'Found  in chunk : ', counter, ' for col :', cname

                if fix:
                    if cname.upper() in ['DATA','CORRECTED_DATA','MODEL_DATA'] :
                        flags = ms.getdata(items=['FLAG'])
                        flags['flag'] = flags['flag'] + nancount
                        resdat[cname][nancount==True] = complex(0.0,0.0)
                        flags[cname] = resdat[cname]
                        ms.putdata(flags)
                        print 'Setting value for column : ', cname , ' to zero and corresponding flag to True'
                    elif cname.upper() in ['WEIGHT','WEIGHT_SPECTRUM'] :
                        resdat[cname][nancount==True] = complex(0.0,0.0)
                        ms.putdata({cname:resdat[cname]})
                        print 'Setting value for column : ', cname , ' to zero'
                    else:
                        print "Cannot fix. Please use 'CheckUVWForNan()' for UVW column"
          
          
        moretodo = ms.iternext();
          
    ## Close the MS
    ms.close();
          
