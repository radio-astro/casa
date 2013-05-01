#attool = casac.homefinder.find_home_by_name('atmosphereHome')
#at = attool.create()
at = casac.atmosphere()
tmp = qa.quantity(270.0,'K')
pre = qa.quantity(560.0,'mbar')
hum = 20.0
alt = qa.quantity(5000,'m')
h0  = qa.quantity(2.0,'km')
wvl = qa.quantity(-5.6,'K/km')
mxA = qa.quantity(48,'km')
#dpr = qa.quantity(10.0,'mbar')
dpr = qa.quantity(5.0,'mbar')
#dpm = 1.2
dpm = 1.1
att = 1

myatm = at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
print "Atmospheric type:   ", at.getBasicAtmParms()[10]
print "Number of layers returned: ", at.getNumLayers()
print myatm
p = at.getProfile()
print "Layer parameters:"
print p[0]
w = at.getGroundWH2O()
print "First guess precipitable water vapor content: ", w['value'][0], w['unit']
print "(This value is estimated from the relative humidity at ground level and the water vapor scale height)"
print
print "TEST FOR JUST 1 FREQUENCY"
print "========================="
nb = 1
fC = qa.quantity([850.0],'GHz')
fW = qa.quantity([0.005],'GHz')
fR = qa.quantity([0.000],'GHz')  # force single frequency
at.initSpectralWindow(nb,fC,fW,fR)
at.setUserWH2O(qa.quantity(1.0,'mm'))
print "getUserWH2O()=", (at.getUserWH2O())['value'][0], (at.getUserWH2O())['unit']
for i in range(at.getNumSpectralWindows()):
	for j in range(at.getNumChan(i)):
		print "Frequency: ", at.getChanFreq(j,i)['value'][0], at.getChanFreq(j,i)['unit']
		print "Wet opacity:", at.getWetOpacity(j,i)['value'][0], at.getWetOpacity(j,i)['unit'], " for ", at.getUserWH2O()['value'][0], at.getUserWH2O()['unit'], " H2O"
		print "Dry opacity:", at.getDryOpacity(j,i), " nepers"
		print "Sky brightness:", at.getAverageTebbSky(j)['value'][0], at.getAverageTebbSky(j)['unit']
print ""
wh2o=qa.quantity(0.45,'mm')
print "(INPUT CHANGE) water vapor column:", wh2o['value'], wh2o['unit']
print "(NEW OUTPUT) T_EBB =", at.getAverageTebbSky(0,wh2o)['value'][0], at.getAverageTebbSky(0,wh2o)['unit']
print "(Current water vapor column in Radiance Class:", at.getUserWH2O()['value'][0],at.getUserWH2O()['unit']
print ""
at.setAirMass(2.0)
print "(INPUT CHANGE) Air mass: ", at.getAirMass()
print "(NEW OUTPUT) T_EBB = ", at.getAverageTebbSky()['value'][0], at.getAverageTebbSky()['unit']
print ""
wh2o=qa.quantity(0.8,"mm")
at.setUserWH2O(wh2o)
print "(INPUT CHANGE) water vapor column:", wh2o['value'], wh2o['unit']
print "(NEW OUTPUT) T_EBB =", at.getAverageTebbSky()['value'][0], at.getAverageTebbSky(0,wh2o)['unit']
print ""
print " TEST FOR 1 SPECTRAL WINDOW WITH SEVERAL CHANNELS"
print " ====================================================="
print ""
numchan=4
chansep = qa.quantity(0.5,'GHz')
nb2 = 1
fC2 = qa.quantity(899.55,'GHz')
fW2 = qa.quantity(numchan*chansep['value'],chansep['unit'])
fR2 = chansep
at.initSpectralWindow(nb2,fC2,fW2,fR2)
unit_wh2o = qa.quantity(1.0,'mm')
print "Setting user H2O", unit_wh2o['value'], unit_wh2o['unit']
at.setUserWH2O(unit_wh2o)
print "water vapor column: ", at.getUserWH2O()['value'][0], at.getUserWH2O()['unit']
print "Air mass: ", at.getAirMass()
print ""
for i in range(at.getNumChan(0)):
	print "Freq:", at.getChanFreq(i)['value'][0], at.getChanFreq(i)['unit'], "/ T_EBB=", at.getTebbSky(i)['value'][0], at.getTebbSky(i)['unit']

at.setAirMass(1.0)
at.setUserWH2O(qa.quantity(0.45,"mm"))
print "water vapor column:", at.getUserWH2O()['value'][0], at.getUserWH2O()['unit']
print "Air mass          :", at.getAirMass()
for i in range(at.getNumChan(0)):
	print "Freq: ", at.getChanFreq(i)['value'][0], at.getChanFreq(i)['unit'], "/ T_EBB=", at.getTebbSky(i)['value'][0], at.getTebbSky(i)['unit']

exit()
