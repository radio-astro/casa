attool = casac.homefinder.find_home_by_name('atmosphereHome')
at = attool.create()
print at.initAtmProfile(dP=quantity(5.0,'mbar'),dPm=1.1)
l = at.getProfile()
print l['return']
print "Output record keys: ", l.keys()
exit()
