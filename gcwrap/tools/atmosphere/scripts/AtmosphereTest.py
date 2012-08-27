#attool = casac.homefinder.find_home_by_name('atmosphereHome')
at = casac.atmosphere()
alt = qa.quantity(2550.,'m')
tmp = qa.quantity(270.32,'K')
pre = qa.quantity(73585.,'Pa')
mxA = qa.quantity(45000,'m')
hum = 20.0
wvl = qa.quantity(-0.0056,'K/m')
dpr = qa.quantity(500.0,'Pa')
dpm = 1.25
h0  = qa.quantity(2000.0,'m')
att = 2
print "Test of constructor Atmosphere(altitude,temperature,pressure,maxAltitude,humidity,dTempdH,dP,dPm,h0,MIDLAT_SUMMER)"
myatm = at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
print myatm

print
print "Test: getGroundWH2O()"
w = at.getGroundWH2O()
print "Guessed water content: ", w['value'][0], w['unit']

print
print "Test: getProfile()"
p = at.getProfile()
print "altitude\tthickness\ttemperature\twatermassdensity\tpressure"
alt = 0
for i in range(at.getNumLayers()):
	alt += p[1]['value'][i]
	print alt, '\t', p[1]['value'][i],'\t', p[2]['value'][i],'\t', p[3]['value'][i],'\t', p[5]['value'][i]

print
print "Test of initSpectralWindow()"
nbands=1
fC=qa.quantity(88.,'GHz')
fW=qa.quantity(.5,'GHz')
fR=qa.quantity(.5,'GHz')
at.initSpectralWindow(nbands,fC,fW,fR)
n=at.getNumChan(0)
print n, "channel(s) in band 0"

print
print "Test: getOpacity()"
print " - dryOpacity ", at.getDryOpacity(), "wetOpacity ", at.getWetOpacity()['value'][0],at.getWetOpacity()['unit']

print
print "AbsCoeff getAbsCoeff() (3 first layers):", at.getAbsH2OLines(0,0)['value'][0],at.getAbsH2OLines(1,0)['value'][0],at.getAbsH2OLines(2,0)['value'][0]

print
print "Test of SkyBrightness calculations"
at.setAirMass(1.51)
at.setSkyBackgroundTemperature(qa.quantity(2.73,'K'))
at.setUserWH2O(qa.quantity(4.05,'mm'))
print "SkyBrightness =", at.getTebbSky()['value'][0], at.getTebbSky()['unit'], " TEBB"
print ""
print "=========================================================="
print "Test with spectral data"
nb=2
fC=qa.quantity([88.,90.],'GHz')
fW=qa.quantity([.5,.5],'GHz')
fR=qa.quantity([.125,.125],'GHz')
print "Test of initSpectralWindow"
at.initSpectralWindow(nb,fC,fW,fR)
n=at.getNumChan(0)
print nb, " bands ", n, "channels(s)"
print "Test: Opacity getOpacitySpec"
for s in range(at.getNumSpectralWindows()):
	print "band", s
	for i in range(n):
		print " - dryOpacity ", at.getDryOpacitySpec(s)[1][i], " wet Opacity ", at.getWetOpacitySpec(s)[1]['value'][i],at.getWetOpacitySpec(s)[1]['unit']
print ""
print "Test of SkyBrightness calculations"
at.setAirMass(1.51)
at.setSkyBackgroundTemperature(qa.quantity(2.73,'K'))
at.setUserWH2O(qa.quantity(4.05,'mm'))
for s in range(at.getNumSpectralWindows()):
	for i in range(n):
		print "Band", s, " channel ", i, "TebbSky = ", at.getTebbSky(i,s)['value'][0], at.getTebbSky()['unit']
print ""
exit()
