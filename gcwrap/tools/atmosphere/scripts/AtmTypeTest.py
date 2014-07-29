#attool = casac.homefinder.find_home_by_name('atmosphereHome')
#at = attool.create()
at = casac.atmosphere()
print "Test AtmType class"

alt = qa.quantity(2550.,'m')
tmp = qa.quantity(270.32,'K')
pre = qa.quantity(73585.,'Pa')
mxA = qa.quantity(45000.,'m')
hum = 20.0
wvl = qa.quantity(-0.0056,'K/m')
dpr = qa.quantity(500.,'Pa')
#dpm = 1.25
dpm = 1.5
h0  = qa.quantity(2000.,'m')
att = 1
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l[10]
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p[1]['value'][i]
	print i, '\t', height, '\t', p[1]['value'][i],'\t', p[2]['value'][i],'\t', p[3]['value'][i],'\t', p[5]['value'][i]
print

att = 2
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l[10]
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p[1]['value'][i]
	print i, '\t', height, '\t', p[1]['value'][i],'\t', p[2]['value'][i],'\t', p[3]['value'][i],'\t', p[5]['value'][i]
print

att = 3
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l[10]
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p[1]['value'][i]
	print i, '\t', height, '\t', p[1]['value'][i],'\t', p[2]['value'][i],'\t', p[3]['value'][i],'\t', p[5]['value'][i]
print 

att = 4
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l[10]
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p[1]['value'][i]
	print i, '\t', height, '\t', p[1]['value'][i],'\t', p[2]['value'][i],'\t', p[3]['value'][i],'\t', p[5]['value'][i]
print

att = 5
myatm=at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l[10]
print myatm
p = at.getProfile()
print "layer\taltitude\tthickness\ttemperature\twatermassdensity\tpressure"
height = 0
for i in range(at.getNumLayers()):
	height += p[1]['value'][i]
	print i, '\t', height, '\t', p[1]['value'][i],'\t', p[2]['value'][i],'\t', p[3]['value'][i],'\t', p[5]['value'][i]
exit()
