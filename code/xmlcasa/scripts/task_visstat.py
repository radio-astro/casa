from taskinit import *

def visstat(vis=None,
            axis=None,
            datacolumn=None,
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

    if axis in ['amp', 'amplitude', 'phase', 'imag', 'imaginary', 'real']:
        complex_type = axis
        col = datacolumn
    else:
        complex_type = ''
        col = axis

    if (not selectdata):
        antenna=''
        uvrange=''
        timerange=''
        correlation=''
        scan=''
        array=''
        
    s = ms.statistics(column=col.upper(),
                      complex_value=complex_type,
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

    for stats in s.keys():
        casalog.post(stats + " values --- ", "NORMAL")
        
        if s[stats]['npts'] > 0:
            casalog.post("         -- number of points [npts]:           " + str(int(round(s[stats]['npts']))), "NORMAL")
            casalog.post("         -- minimum value [min]:               " + str(s[stats]['min'  ]), "NORMAL")
            casalog.post("         -- maximum value [max]:               " + str(s[stats]['max'  ]), "NORMAL")
            casalog.post("         -- Sum of values [sum]:               " + str(s[stats]['sum'  ]), "NORMAL")
            casalog.post("         -- Sum of squared values [sumsq]:     " + str(s[stats]['sumsq']), "NORMAL")

        casalog.post(stats + " statistics --- ", "NORMAL")
        if s[stats]['npts'] > 0:
                casalog.post("        -- Mean of the values [mean]:                 " + str(s[stats]['mean']), "NORMAL")
                casalog.post("        -- Variance of the values [var]:              " + str(s[stats]['var']), "NORMAL")
                casalog.post("        -- Standard deviation of the values [stddev]: " + str(s[stats]['stddev']), "NORMAL")
                casalog.post("        -- Root mean square [rms]:                    " + str(s[stats]['rms']), "NORMAL")
                casalog.post("        -- Median of the pixel values [median]:       " + str(s[stats]['median']), "NORMAL")
                casalog.post("        -- Median of the deviations [medabsdevmed]:   " + str(s[stats]['medabsdevmed']), "NORMAL")
                casalog.post("        -- Quartile [quartile]:                       " + str(s[stats]['quartile']), "NORMAL")
        else:
            casalog.post(stats + " -- No valid points found", "WARN")

    return s


        

