attool = casac.homefinder.find_home_by_name('atmosphereHome')
at = attool.create()
print "Test AtmType class"

alt = quantity(2550.,'m')
tmp = quantity(270.32,'K')
pre = quantity(73585.,'Pa')
mxA = quantity(45000.,'m')
hum = 20.0
wvl = quantity(-0.0056,'K/m')
dpr = quantity(500.,'Pa')
#dpm = 1.25
dpm = 1.5
h0  = quantity(2000.,'m')
att = 1
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p['thickness'].value[i]
	print i, '\t', height, '\t', p['thickness'].value[i],'\t', p['temperature'].value[i],'\t', p['watermassdensity'].value[i],'\t', p['pressure'].value[i]
print

att = 2
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p['thickness'].value[i]
	print i, '\t', height, '\t', p['thickness'].value[i],'\t', p['temperature'].value[i],'\t', p['watermassdensity'].value[i],'\t', p['pressure'].value[i]
print

att = 3
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p['thickness'].value[i]
	print i, '\t', height, '\t', p['thickness'].value[i],'\t', p['temperature'].value[i],'\t', p['watermassdensity'].value[i],'\t', p['pressure'].value[i]
print 

att = 4
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p['thickness'].value[i]
	print i, '\t', height, '\t', p['thickness'].value[i],'\t', p['temperature'].value[i],'\t', p['watermassdensity'].value[i],'\t', p['pressure'].value[i]
print

att = 5
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p['thickness'].value[i]
	print i, '\t', height, '\t', p['thickness'].value[i],'\t', p['temperature'].value[i],'\t', p['watermassdensity'].value[i],'\t', p['pressure'].value[i]
exit()
