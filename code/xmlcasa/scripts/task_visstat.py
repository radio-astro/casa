from taskinit import *

def visstat(vis=None,
           column=None,
           useflags=None,
           spw=None,
           field=None,
           selectdata=None,
           antenna=None,
           uvrange=None,
           timerange=None,
           correlation=None,
           scan=None,
           array=None):

    casalog.origin('visstat')  
    ms.open(vis)

    s = ms.statistics(column=column,
                      useflags=useflags,
                      spw=spw,
                      field=field,
                      baseline=antenna,
                      uvrange=uvrange,
                      time=timerange,
                      correlation=correlation,
                      scan=scan,
                      array=array)

    ms.close()

    casalog.post("Values --- ", "NORMAL")

    if s['npts'] > 0:
	casalog.post("         -- number of points [npts]:           " + str(int(round(s['npts']))), "NORMAL")
	casalog.post("         -- minimum value [min]:               " + str(s['min'  ]), "NORMAL")
	casalog.post("         -- maximum value [max]:               " + str(s['max'  ]), "NORMAL")
	casalog.post("         -- Sum of values [sum]:               " + str(s['sum'  ]), "NORMAL")
	casalog.post("         -- Sum of squared values [sumsq]:     " + str(s['sumsq']), "NORMAL")

    casalog.post("Statistics --- ", "NORMAL")
    if s['npts'] > 0:
	casalog.post("        -- Mean of the values [mean]:                 " + str(s['mean']), "NORMAL")
	casalog.post("        -- Variance of the values [var]:              " + str(s['var']), "NORMAL")
	casalog.post("        -- Standard deviation of the values [stddev]: " + str(s['stddev']), "NORMAL")
	casalog.post("        -- Root mean square [rms]:                    " + str(s['rms']), "NORMAL")
	casalog.post("        -- Median of the pixel values [median]:       " + str(s['median']), "NORMAL")
	casalog.post("        -- Median of the deviations [medabsdevmed]:   " + str(s['medabsdevmed']), "NORMAL")
	casalog.post("        -- Quartile [quartile]:                       " + str(s['quartile']), "NORMAL")
    else:
	casalog.post("No valid points found", "WARN")


    return s


        

