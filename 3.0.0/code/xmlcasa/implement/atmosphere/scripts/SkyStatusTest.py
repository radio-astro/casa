attool = casac.homefinder.find_home_by_name('atmosphereHome')
at = attool.create()
tmp = quantity(270.0,'K')
pre = quantity(560.0,'mbar')
hum = 20.0
alt = quantity(5000,'m')
h0  = quantity(2.0,'km')
wvl = quantity(-5.6,'K/km')
mxA = quantity(48,'km')
dpr = quantity(10.0,'mbar')
dpm = 1.2
att = 1
myatm = at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
print "Atmospheric type:   ", at.getBasicAtmParms()['atmType']
print "size of Temperature Profile: ", at.getNumLayers()
print myatm
p = at.getProfile()
print "Layer parameters:"
print p['return']
w = at.getStartupWaterContent()
print "First guess precipitable water vapor content: ", w.value, w.units
print "(This value is estimated from the relative humidity at ground level and the water vapor scale height)"
print
print "TEST FOR JUST 1 FREQUENCY"
print "========================="
nb = 1
fC = quantity([850.0],'GHz')
fW = quantity([0.005],'GHz')
fR = quantity([0.000],'GHz')  # force single frequency
at.initSpectralWindow(nb,fC,fW,fR)
at.setUserWH2O(quantity(1.0,'mm'))
print "getUserWH2O()=", (at.getUserWH2O()).value, (at.getUserWH2O()).units
for i in range(at.getNumSpectralWindows()):
	for j in range(at.getNumChan(i)):
		print "Frequency: ", at.getChanFreq(j,i).value, at.getChanFreq(j,i).units
		print "Wet opacity:", at.getWetOpacity(j,i).value, at.getWetOpacity(j,i).units, " for ", at.getUserWH2O().value, at.getUserWH2O().units, " H2O"
		print "Dry opacity:", at.getDryOpacity(j,i), " nepers"
		print "Sky brightness:", at.getAverageTebbSky(j).value, at.getAverageTebbSky(j).units
print ""
wh2o=quantity(0.4,'mm')
print "(INPUT CHANGE) water vapor column:", wh2o.value, wh2o.units
print "(NEW OUTPUT) T_EBB =", at.getAverageTebbSky(0,wh2o).value, at.getAverageTebbSky(0,wh2o).units
print "(Current water vapor column in Radiance Class:", at.getUserWH2O().value,at.getUserWH2O().units
print ""
at.setAirMass(2.0)
print "(INPUT CHANGE) Air mass: ", at.getAirMass()
print "(NEW OUTPUT) T_EBB = ", at.getAverageTebbSky().value, at.getAverageTebbSky().units
print ""
wh2o=quantity(0.8,"mm")
at.setUserWH2O(wh2o)
print "(INPUT CHANGE) water vapor column:", wh2o.value, wh2o.units
print "(NEW OUTPUT) T_EBB =", at.getAverageTebbSky().value, at.getAverageTebbSky(0,wh2o).units
print ""
print " TEST FOR JUST 1 SPECTRAL WINDOW WITH SEVERAL CHANNELS"
print " ====================================================="
print ""
nb2 = 1
fC2 = quantity(850.00,'GHz')
fW2 = quantity(8*1.00,'GHz')
fR2 = quantity(1.00, 'GHz')
at.initSpectralWindow(nb2,fC2,fW2,fR2)
print "water vapor column: ", at.getUserWH2O().value, at.getUserWH2O().units
print "Air mass: ", at.getAirMass()
print ""
for i in range(at.getNumChan(0)):
	print "Freq:", at.getChanFreq(i).value, at.getChanFreq(i).units, " T_EBB=", at.getTebbSky(i).value, at.getTebbSky(i).units

exit()
