def atmtrans(freq,pwv,nchan):

    airmass = 1.5
    P = 563.0
    H = 20.0
    T = 273.0

    tropical = 1
    midLatitudeSummer = 2
    midLatitudeWinter = 3

    freqs=pl.array(range(nchan+1))*1./(nchan)*(freq[1]-freq[0])+freq[0]
    reffreq=0.5*(freqs[nchan/2-1]+freqs[nchan/2])
    chansep=(freq[1]-freq[0])/(nchan-1)

    fCenter = qa.quantity(reffreq,'GHz')
    fResolution = qa.quantity(chansep,'GHz')
    fWidth = qa.quantity(nchan*chansep,'GHz')
    at.initAtmProfile(humidity=H,temperature=qa.quantity(T,"K"),altitude=qa.quantity(5059,"m"),pressure=qa.quantity(P,'mbar'),atmType=midLatitudeWinter)
    at.initSpectralWindow(1,fCenter,fWidth,fResolution)
    at.setUserWH2O(qa.quantity(pwv,'mm'))
    rf = at.getRefFreq()['value']
    cs = at.getChanSep()['value']

    print reffreq,rf
    if (at.getRefFreq()['unit'] != 'GHz'):
        print "There is a unit mismatch for refFreq in the code."
    print chansep*1000,cs
    if (at.getChanSep()['unit'] != 'MHz'):
        print "There is a unit mismatch for chanSep in the code."

    dry = pl.array(at.getDryOpacitySpec(0)[1])
    wet = pl.array(at.getWetOpacitySpec(0)[1]['value'])
    TebbSky = at.getTebbSkySpec(spwid=0)[1]['value']

    transmission = np.exp(-airmass*(wet+dry))

    return freqs,transmission
    
