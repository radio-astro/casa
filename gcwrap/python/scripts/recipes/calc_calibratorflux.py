# calculate calibrator flux from the the flux standard models
# (for non-SS, e.g. quasars etc) in the CASA data repository
# 
# To get flux densities of corresponding channel frequencies
# in an MS use calcfluxes_fromStandard
#
# To calculate flux density for a given frequency (in GHz)
# use calcflux_fromStandard
#
# first version: 2015.05.06  TT
#
from taskinit import *
from numpy import log10

def calcfluxes_fromStandard(sourcename, standard, vis, spw=-1):
    '''
    print flux densities for all channel frequencies in the specified spw
    for the flux calibrator in the flux standard.
    - calls calclux_fromStandard(sourcename, fGHz, standard)
    
    sourcename: calibrator source name (as defined in the standard)
    standard:   name of the standard 
                 (currently only support: 
                  - non-time variable "Perley-Butler 2013"
                 )
    vis:        MS name
    spw:        spw ids in list or -1 (all) 
    '''
    (tb,) = gentools(['tb'])
    print "vis=",vis
    if os.path.exists(vis):
        tb.open(vis+'/SPECTRAL_WINDOW')
        nspw = tb.nrows()
    else:
        raise TypeError("vis:%s does not exist" % vis)
    if spw==-1:
        selspws = range(nspw) 
    else:
        selspws = spw 
    print "spw chanFreq(GHz) flux density" 
    for ispw in selspws: 
        chanfreqs = tb.getcell('CHAN_FREQ',ispw)
        for f in chanfreqs:
            fGHz=f/1.0e9
            flx = calcflux_fromStandard(sourcename,fGHz,standard)

            print ispw, fGHz, flx

    tb.close()


 
def calcflux_fromStandard(sourcename, freq, standard, epoch=''):
    '''
    freq (in GHz)
    standard: Perley-Butler 2013, Perley-Butler 2010, ...
    epoch (optional): for a time variable source in P-B 2013
    '''
    from numpy import poly1d as p1d
    (tb,) = gentools(['tb'])
    rootdatapath = os.environ['CASAPATH'].split()[0]+"/data" 
    dbpath = rootdatapath+"/nrao/VLA/standards"
    timvarsrcs = ['3C48','3C138','3C147']
    sname = sourcename.upper()
    f0 = 1.0 # defualt ref freq = 1.0GHz
    if standard=="Perley-Butler 2013":
        tb.open(dbpath+'/PerleyButler2013Coeffs')
        if epoch=='' or (sname not in timevarsrcs):
           try:
               coeffs = tb.getcell(sname+"_coeffs",0)
           except:
               raise Execption, "cannot find "+sname+"_coeffs data"

           p = p1d(coeffs[::-1])
           
        else:
           print "time variable source is not supported yet for this test"

        tb.close()

    elif standard=="Scaife-Heald 2012":
        f0 = 0.15 # for SH2012 ref freq = 150MHz
        tb.open(dbpath+'/ScaifeHeald2012Coeffs')
        try:
           coeffs = tb.getcell(sname+"_coeffs",0)
        except:
           raise Execption, "cannot find "+sname+"_coeffs data"

        modcoeffs = coeffs[::-1]
        modcoeffs[-1] = log10(modcoeffs[-1])
        p = p1d(modcoeffs)
        tb.close()

    else:
        print "support for the standard "+standard+" has not been implemeted..."      

    return 10.0**p(log10(freq/f0))
