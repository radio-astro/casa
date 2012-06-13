print "___Testing qa.quantity()"
print qa.quantity('12:30:00')
print qa.quantity('5.4km/s')
print qa.quantity('20Jy/pc2')
q=qa.quantity('8.57132661e+09km/s')
print q
print "___Testing qa.unit()"
print qa.unit('12:30:00')
print qa.unit('5.4km/s')
print qa.quantity(qa.unit('5.4km/s'))
print qa.unit('20Jy/pc2')
q=qa.unit('8.57132661e+09km/s')
print q
print "___Testing qa.quantity()"
print qa.quantity(5.4,'km/s')
print qa.quantity(20,'Jy/pc2')
print qa.quantity(5.4, 'km/s')
q1=qa.quantity(8.57132661e+09,'km/s')
print q1
print "___Testing qa.convert()"
print qa.convert(q,'pc/h')
print qa.convert(q1,'pc/h')
print "___Testing qa.add()"
q1=qa.unit('5km')
q2=qa.unit('200m')
print qa.canonical(qa.add(q1,q2))
print "___Testing qa.compare()"
q1 =  qa.quantity('6rad')
q2 = qa.quantity('3deg');
print qa.compare(q1,q2)
print "Comparing", q1, " and ", qa.unit('3km')
print qa.compare(q1,qa.unit('3km'))
print "___Testing qa.constants()"
boltzmann = qa.constants('k')
print "Boltzmann constant is ", boltzmann
print "___Testing time quantities"
print qa.quantity('today')
print qa.quantity('5jul1998')
print qa.quantity('5jul1998/12:')
print qa.quantity('-30.12.2')
print qa.quantity('2:2:10')
print qa.unit('23h3m2.2s')  
print "___Testing angle/time interchangeably:"
print "Get the time now: b = qa.quantity('today')"
b = qa.quantity('today')
print "Time now is ", b
print "Set reference time of a=qa.quantity(51045.9972,'d')"
a = qa.quantity(51045.9972,'d')
print a
print "Expect [value=51045.9972, unit=d]"
print "Get the time as an angle: qa.toangle(a)"
print qa.toangle(a)
print "Expect [value=320731.459, unit=rad]"
print "Get the time as a normalised angle (-pi to +pi) and show as dms" 
print qa.angle(qa.norm(qa.toangle(a)))
print "Expect -001.00.56.308"
print "Get the time as a normalised angle (0 to 2pi) and show as dms"
print qa.angle(qa.norm(qa.toangle(a), 0))
print "Expect +358.59.03.692"
print "Get the time since creation of b"
print "qa.sub('today',b) = ", qa.sub('today',b)
print "___Testing isquantity()"
a=qa.quantity('5Jy')
print qa.isquantity(a)
print qa.isquantity('5JY')
print "___Testing convertfreq()"
print qa.convertfreq('5GHz','cm')
print "[value=5.99584916, unit=cm]"
print qa.convertfreq('5cm','GHz')
print "[value=5.99584916, unit=GHz]"
print "___Testing convertdop"
print qa.convertdop('1','km/s')
print "[value=299792.458, unit=km/s]"
print "<==== Do I need to check for valid units?"
print qa.convertdop('10km/s','1') 
print "[value=3.33564095e-05, unit=]"
print "___Testing quantity()"
tu = qa.quantity('1Jy')			# make quantity
print tu
print "[value=1, unit=Jy]"
print qa.quantity(tu)			# also accepts a quantity
print "[value=1, unit=Jy]"
tu = qa.unit('1Jy')			# make quantity with synonym
print tu
print "[value=1, unit=Jy]"
print qa.quantity(-1.3, 'Jy')		# make quantity with separate value
print "[value=-1.3, unit=Jy]"
print "___Testing getvalue()"
tu = qa.quantity(-1.3, 'Jy')         # make quantity
print tu
print "[value=-1.3, unit=Jy]"
print qa.getvalue(tu)
print "Expecting -1.3"
print qa.getunit(tu)
print "Expecting Jy"
print "<==== Array of quantity not working yet."
a = qa.quantity("5m/s 7A")
print a
print "Expect [id=quant, shape=2]"
print qa.getvalue(a)
print "Expect [5 7]"
print "___Testing getunit()"
tu = qa.quantity(-1.3, 'Jy')         # make quantity
print tu
print "Expect [value=-1.3, unit=Jy]"
print qa.getvalue(tu)
print "Expect -1.3"
print qa.getunit(tu)
print "Expect Jy"
print "___Testing canonical()"
print qa.canonical('1Jy')			# canonical value of a string
print "[value=1e-26, unit=kg.s-2]"
print qa.canon(qa.quantity('1Jy'))		# canonical value of a unit
print "[value=1e-26, unit=kg.s-2]"
print "___Testing canon()"
print qa.canon('1Jy')			# canonical value of a string
print "[value=1e-26, unit=kg.s-2]"
print qa.canonical(qa.quantity('1Jy'))		# canonical value of a unit
print "[value=1e-26, unit=kg.s-2]"
print "___Testing convert()"
tu = qa.quantity('5Mm/s')		# specify a quantity
print tu
print "Expect [value=5, unit=Mm/s]"
print qa.convert(tu, 'pc/a')		# convert it to parsec per year
print "Expect [value=0.00511356, unit=pc/a]"
print qa.convert(tu)			# convert to canonical units
print "Expect [value=5e+06, unit=m.s-1]"
print "___Testing define()"
print qa.define('JY','1Jy')			# your misspelling
print "Expect T"
print qa.define('VLAunit', '0.898 JY')	# a special unit using it
print "Expect T" 
print qa.quantity('5 VLAunit') 			# check its use
print "Expect [value=5, unit=VLAunit]"
print qa.convert('5 VLAunit','Jy')
print "Expect [value=4.49, unit=Jy]"
print "___Testing map()"
print qa.map('pre')
print qa.map('Constants')
print "___Testing fits()"
qa.fits()
print qa.map('user')
print "___Testing angle()"
tu = qa.quantity('5.7.12.345678')		# define an angle
print tu
print "Expect [value=5.1201, unit=deg]"
print qa.angle(tu)    				# default output
print "Expect +005.07.12"
print qa.angle(tu, prec=7)				# 7 digits
print "Expect +005.07.12.3" 
print qa.angle(tu, prec=4)				# 4 digits
print "Expect +005.07."
print qa.angle(tu, form=["tim","no_d"])		# as time, no hours shown
print "Expect :20:29"
print "__Testing time()"
print "qa.quantity('today') = ", qa.quantity('today')		# a time
tu = qa.quantity(50450.2243,'d')
print "Using reference time of [value=50450.2243, unit=d]"
print qa.time(tu)				# default format
print "Expect 05:23:01.000"
print qa.time(tu,form=["dmy"])  		# show date
print "Expect 02-Jan-1997/05:23:01.000"
print qa.time(tu,form=["ymd", "day"])		# and day
print "Expect Thu-1997/01/02/05:23:01.000"
print qa.time(tu,form=["fits"])               # FITS format    
print "Expect 1997-01-02T05:23:01.000"
print qa.time(tu,form=["fits", "local"])         # local FITS format
print "Expect 1997-01-02T15:23:01.000+10:00"
print qa.time(tu,form=["ymd", "local"])          # local time         
print "Expect 1997/01/02/15:23:01.000"
print "___Testing add()"
print qa.add('5m', '2yd')   
print "Expect [value=6.8288, unit=m]"
print "___Testing sub()"
print qa.sub('5m', '2yd')   
print "Expect [value=3.1712, unit=m]"
print "___Testing mul()"
print qa.mul('5m', '3s')   
print "Expect [value=15, unit=m.s]"
print "___Testing div()"
print qa.div('5m', '3s')   
print "Expect [value=1.66667, unit=m/(s)]"
print "___Testing neg()"
print qa.neg('5m')
print "Expect [value=-5, unit=m]"
print "___Testing norm()"
print qa.norm('713deg')
print "Expect [value=-7, unit=deg]"
print qa.norm('713deg', -2.5)
print "Expect [value=-727, unit=deg]"
print "___Testing le()"
print qa.le('5m', '2yd')
print "Expecting F"
print "___Testing lt()"
print qa.lt('5m', '2yd')
print "Expecting F"
print "___Testing eq()"
print qa.eq('5m', '2yd')
print "Expecting F"
print "___Testing ne()"
print qa.ne('5m', '2yd')
print "Expecting T"
print "___Testing gt()"
print qa.gt('5m', '2yd')
print "Expecting T"
print "___Testing ge()"
print qa.ge('5m', '2yd')
print "Expecting T"
print "___Testing sin()"
print qa.sin('7deg')
print "Expecting [value=0.121869, unit=]"
print "___Testing cos()"
print qa.cos('7deg')
print "Expecting [value=0.992546, unit=]"
print "___Testing tan()"
print qa.tan('7deg')
print "Expecting [value=0.122785, unit=]"
print "___Testing asin()"
print qa.convert(qa.asin(qa.sin('7deg')), 'deg')
print "Expecting [value=7, unit=deg]"
print "___Testing acos()"
print qa.convert(qa.acos(qa.cos('7deg')), 'deg')
print "Expecting [value=7, unit=deg]"
print "___Testing atan()"
print qa.convert(qa.atan(qa.tan('7deg')), 'deg')
print "Expecting [value=7, unit=deg]"
print "___Testing atan2()"
print qa.convert( qa.atan2( qa.sin('7deg'), qa.cos('7deg') ), 'deg' )
print "Expecting [value=7, unit=deg]"
print "___Testing abs()"
print qa.abs('-5km/s')
print "Expecting [value=5, unit=km/s]"
print "___Testing ceil()"
print qa.ceil('5.1AU')
print "Expecting [value=6, unit=AU]"
print "___Testing floor()"
print qa.floor('-5.1AU')
print "Expecting [value=-6, unit=AU]"
print "___Testing log()"
print qa.log('2')
print "Expecting [value=0.693147181, unit=]"
print "___Testing log10()"
print qa.log10('2')
print "Expecting [value=0.301029996, unit=]"
print "___Testing exp()"
print qa.exp('2')
print "Expecting [value=7.3890561, unit=]"
#print qa.exp('2m')
#print "Expecting SEVERE: Caught an exception! Event type=run exception=Quantum::exp illegal unit type 'm'"
print "___Testing sqrt()"
print qa.sqrt('2m2')
print "Expecting [value=1.41421356, unit=m]"
#print qa.sqrt('2s')
#print "Expecting SEVERE: Caught an exception! Event type=run exception=UnitVal::UnitVal Illegal unit dimensions for root"
print "___Testing compare()"
print qa.compare('5yd/a', '6m/s')  		# equal dimensions
print "Expecting T"
print qa.compare('5yd', '5s')	  		# unequal dimensions
print "Expecting F"
print "___Testing check()"
print qa.check('5AE/Jy.pc5/s')
print "Expecting T"
print qa.check('7MYs')
print "Expecting F"
print "___Testing checkfreq()"
print qa.checkfreq('5GHz')
print "Expecting T"
print qa.checkfreq('5cm')
print "Expecting T"
print qa.checkfreq('5cm/s2')
print "Expecting F"
print "___Testing pow()"
print qa.pow('7.2km/s', -3)
print "Expecting [value=0.00267918, unit=(km/s)-3]"
print "___Testing constants()"
print qa.constants()
print "Expecting [value=3.14159, unit=]"
print "___Testing isangle()"
print qa.isangle('5deg')
print "Expecting T"
print qa.isangle(qa.constants('pi'))
print "Expecting F"
print "___Testing totime()"
print qa.totime('2d5m')
print "Expecting [value=0.00578704, unit=d]"
print "___Testing toangle()"
print qa.toangle('5h30m12.6')
print "Expecting [value=82.5525, unit=deg]"
print "___Testing splitdate()"
print qa.splitdate('today')
print "Expecting something like"
print " [mjd=52075.0361, year=2001, yearday=166, month=6, monthday=15,"
print "		  week=24, weekday=5, hour=0, min=52, sec=1,"
print "		   s=1.24799991, msec=247, usec=247999] "
print qa.splitdate('183.33333333deg')
print "Expecting [mjd=0.509259259, year=1858, yearday=321, month=11, monthday=17,"
print "		   week=46, weekday=3, hour=12, min=13, sec=19,"
print "		    s=19.9999992, msec=999, usec=999999]"
print "___Testing tos()"
a = qa.quantity('2.56 yd/s')
print a
print "Expect [value=2.56, unit=yd/s]"
print qa.tos(a)
print "Expect 2.56 yd/s"
a=qa.quantity(1./7, 'km/s')
print qa.tos(a)
print "Expect 0.142857143 km/s"
print qa.tos(a,2)
print "Expect 0.14 km/s"
print qa.tos(a,20)
print "Expect 0.14285714285714284921 km/s"
print qa.tos(a)   
print "Expect 0.142857143 km/s"
print "___Testing type()"
print qa.type()
print "Expect quanta"
print "___Testing done()"
print qa.done()
print "Expect T"
print qa.done()
print "Expect T"
