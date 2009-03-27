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
dpm = 1.25
h0  = quantity(2000.,'m')
att = 1
at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
hpt= at.getAtmTypeHPT()
n = hpt['return']
for i in range(n):
	print i, " hx: ", hpt['Hx'].value[i], " px: ", hpt['Px'].value[i], " tx: ", hpt['Tx'].value[i]
att = 2
at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
hpt= at.getAtmTypeHPT()
n = hpt['return']
for i in range(n):
	print i, " hx: ", hpt['Hx'].value[i], " px: ", hpt['Px'].value[i], " tx: ", hpt['Tx'].value[i]
att = 3
at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
hpt= at.getAtmTypeHPT()
n = hpt['return']
for i in range(n):
	print i, " hx: ", hpt['Hx'].value[i], " px: ", hpt['Px'].value[i], " tx: ", hpt['Tx'].value[i]
att = 4
at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
hpt= at.getAtmTypeHPT()
n = hpt['return']
for i in range(n):
	print i, " hx: ", hpt['Hx'].value[i], " px: ", hpt['Px'].value[i], " tx: ", hpt['Tx'].value[i]
att = 5
at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
l = at.getBasicAtmParms()
print "Test for type ", l['atmType']
hpt= at.getAtmTypeHPT()
n = hpt['return']
for i in range(n):
	print i, " hx: ", hpt['Hx'].value[i], " px: ", hpt['Px'].value[i], " tx: ", hpt['Tx'].value[i]
exit()
