attool = casac.homefinder.find_home_by_name('atmosphereHome')
at = attool.create()
print at.initAtmProfile()
l = at.getProfile()
print l['return']
print "Output record keys: ", l.keys()
exit()
