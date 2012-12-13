#attool = casac.homefinder.find_home_by_name('atmosphereHome')
at = casac.atmosphere()
print at.initAtmProfile(dP=qa.quantity(5.0,'mbar'),dPm=1.1)
l = at.getProfile()
print l[0]
#print "Output record keys: ", l.keys()
exit()
