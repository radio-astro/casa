attool = casac.homefinder.find_home_by_name('atmosphereHome')
at = attool.create()
alt = quantity(2550.,'m')
tmp = quantity(270.32,'K')
pre = quantity(73585.,'Pa')
mxA = quantity(45000,'m')
hum = 20.0
wvl = quantity(-0.0056,'K/m')
dpr = quantity(500.0,'Pa')
dpm = 1.25
h0  = quantity(2000.0,'m')
att = 2
print "Test of constructor Atmosphere(altitude,temperature,pressure,maxAltitude,humidity,dTempdH,dP,dPm,h0,MIDLAT_SUMMER)"
myatm = at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
print myatm

print
print "Test: getGroundWH2O()"
w = at.getGroundWH2O()
print "Guessed water content: ", w.value, w.units

print
print "Test: getProfile()"
p = at.getProfile()
print "altitude\tthickness\ttemperature\twatermassdensity\tpressure"
alt = 0
for i in range(at.getNumLayers()):
	alt += p['thickness'].value[i]
	print alt, '\t', p['thickness'].value[i],'\t', p['temperature'].value[i],'\t', p['watermassdensity'].value[i],'\t', p['pressure'].value[i]

print
print "Test of initSpectralWindow()"
nbands=1
fC=quantity(88.,'GHz')
fW=quantity(.5,'GHz')
fR=quantity(.5,'GHz')
at.initSpectralWindow(nbands,fC,fW,fR)
n=at.getNumChan(0)
print n, "channel(s) in band 0"

print
print "Test: getOpacity()"
print " - dryOpacity ", at.getDryOpacity(), "wetOpacity ", at.getWetOpacity().value,at.getWetOpacity().units

print
print "AbsCoeff getAbsCoeff() (3 first layers):", at.getAbsH2OLines(0,0).value,at.getAbsH2OLines(1,0).value,at.getAbsH2OLines(2,0).value

print
print "Test of SkyBrightness calculations"
at.setAirMass(1.51)
at.setSkyBackgroundTemperature(quantity(2.73,'K'))
at.setUserWH2O(quantity(4.05,'mm'))
print "SkyBrightness =", at.getTebbSky().value, at.getTebbSky().units, " TEBB"
print ""
print "=========================================================="
print "Test with spectral data"
nb=2
fC=quantity([88.,90.],'GHz')
fW=quantity([.5,.5],'GHz')
fR=quantity([.125,.125],'GHz')
print "Test of initSpectralWindow"
at.initSpectralWindow(nb,fC,fW,fR)
n=at.getNumChan(0)
print nb, " bands ", n, "channels(s)"
print "Test: Opacity getOpacitySpec"
for s in range(at.getNumSpectralWindows()):
	print "band", s
	for i in range(n):
		print " - dryOpacity ", at.getDryOpacitySpec(s)['dryOpacity'][i], " wet Opacity ", at.getWetOpacitySpec(s)['wetOpacity'].value[i],at.getWetOpacitySpec(s)['wetOpacity'].units
print ""
print "Test of SkyBrightness calculations"
at.setAirMass(1.51)
at.setSkyBackgroundTemperature(quantity(2.73,'K'))
at.setUserWH2O(quantity(4.05,'mm'))
for s in range(at.getNumSpectralWindows()):
	for i in range(n):
		print "Band", s, " channel ", i, "TebbSky = ", at.getTebbSky(i,s).value, at.getTebbSky().units
print ""
exit()
