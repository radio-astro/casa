class tee_stdout:

    def __init__(self, old_stdout, filename):
        self.old_stdout = old_stdout
        self.new_stdout = open(filename, 'w+')

    def write(self, item):
        self.old_stdout.write(item)
        self.new_stdout.write(item)

#sys.stdout = tee_stdout(sys.stdout, 'tMeasures.out')

print "\n___Testing me.dirshow()"
print me.dirshow(me.direction('venus'))
print "\n___Testing me.show()"
print me.show(me.frequency('lsrk', qa.constants('HI')))
#print qa.setformat('freq', 'keV')
#print "Expect T"
#print me.show(me.frequency('lsrk', qa.constants('HI')))
#print "Expect 5.87432838e-09 keV LSRK"
#print me.show(me.frequency('lsrk', qa.constants('HI')), refcode=F)
#print "Expect 5.87432838e-09 keV"
print "\n___Testing me.epoch()"
print ": print me.epoch('utc','today')"
print me.epoch('utc','today')
print "me.listcodes(me.epoch())"
print me.listcodes(me.epoch())
print "\n___Testing me.direction()"
print ": print me.direction('j2000','30deg','40deg')"
print me.direction('j2000','30deg','40deg')
print "me.direction('mars')"
print me.direction('mars')
print "me.listcodes(me.direction())"
print me.listcodes(me.direction())
print "\n___Testing me.getvalue()"
print ": b=me.direction('j2000','0deg','80deg')"
b=me.direction('j2000','0deg','80deg')
print ": print me.getvalue(b)"
print me.getvalue(b)
print "\n___Testing me.gettype()"
print ": print me.gettype(b)"
print me.gettype(b)
print "\n___Testing me.getref()"
print ": print me.getref(b)"
print me.getref(b)
print "\n___Testing me.getoffset()"
print ": print me.getoffset(b)"
print me.getoffset(b)
print "\n___Testing me.cometname()"
print ": print me.cometname()"
print me.cometname()
print "Expect"
print "WARNING: Method cometname fails!"
print "No Comet table present"
print "\n___Testing me.comettype()"
print ": print me.comettype()"
print me.comettype()
print "\n___Testing me.comettopo()"
print ": print me.comettopo()"
print me.comettopo()
print "Expect"
print "WARNING: Method comettopo fails!"
print "No Topocentric Comet table present"
print "\n___Testing me.framecomet()"
print ": print me.framecomet('VGEO')"
print me.framecomet('VGEO')
print ": print me.showframe()"
print me.showframe()
print "Expect"
print "position: 09:58:12.033 -030.08.43.736 6372960.26 m ITRF"
print "epoch: 2002/10/18/00:15:49 UTC"
print "direction: 00:00:00.000 +090.00.00.000 VENUS"
print "T"
print ": print me.cometname()"
print me.cometname()
print ": print me.comettype()"
print me.comettype()
print me.doframe(me.epoch('et', qa.quantity('1997/12/20/17:30:0')))
print ": print me.measure(me.direction('comet'), 'app')"
print me.measure(me.direction('comet'), 'app')
print "\n___Testing me.position()"
print ": print me.listcodes(me.position())"
print me.listcodes(me.position())
print ": print me.position('wgs84','30deg','40deg','10m')"
print me.position('wgs84','30deg','40deg','10m')
print "\n___Testing me.observatory()"
print ": print me.observatory('ATCA')"
print me.observatory('ATCA')
print "\n___Testing me.obslist()"
print ": print me.obslist()"
print me.obslist()
print "\n___Testing me.linelist()"
print ": print me.linelist()"
print me.linelist()
print "\n___Testing me.spectralline()"
print ": print me.spectralline('HI')"
print me.spectralline('HI')
print "\n___Testing me.sourcelist()"
print ": s = me.sourcelist()"
s = me.sourcelist()
print "s[0:62]"
print s[0:62]
print "\n___Testing me.source()"
print ": print me.source()"
print me.source()
print "\n___Testing me.frequency()"
print ": print me.listcodes(me.frequency())"
print me.listcodes(me.frequency())
print ": print me.frequency('lsrk','5GHz')"
print me.frequency('lsrk','5GHz')
print ": print me.frequency('lsrk','21cm')"
print me.frequency('lsrk','21cm')
print "\n___Testing me.doppler()"
print ": print me.listcodes(me.doppler())"
print me.listcodes(me.doppler())
print ": print me.doppler('radio', '0.4')"
print me.doppler('radio', '0.4')
print ": print me.doppler('radio', qa.mul(qa.quantity('0.4'),qa.constants('c')))"
print me.doppler('radio', qa.mul(qa.quantity('0.4'),qa.constants('c')))
print "\n___Testing me.radialvelocity()"
print ": print me.listcodes(me.radialvelocity())"
print me.listcodes(me.radialvelocity())
print ": print me.radialvelocity('lsrk','20km/s')"
print me.radialvelocity('lsrk','20km/s')
print "\n___Testing me.uvw()"
print ": print me.listcodes(me.uvw())"
print me.listcodes(me.uvw())
print ": print me.uvw('itrf','30deg','40deg','10m')"
print me.uvw('itrf','30deg','40deg','10m')
me.doframe(me.epoch('utc','today'))
me.doframe(me.observatory('ALMA'))
me.doframe(me.direction('mars'))
print ": print me.measure(me.uvw('itrf','30deg','40deg','10m'), 'j2000')"
print me.measure(me.uvw('itrf','30deg','40deg','10m'), 'j2000')
print "\n___Testing me.touvw()"
print ": me.doframe(me.observatory('atca'))"
me.doframe(me.observatory('atca'))
print ": me.doframe(me.source('1934-638'))"
me.doframe(me.source('1934-638'))
print ": me.doframe(me.epoch('utc',qa.unit('today')))"
me.doframe(me.epoch('utc',qa.unit('today')))
print ": b = me.baseline('itrf', '10m', '20m', '30m')"
b = me.baseline('itrf', '10m', '20m', '30m')
print ": print me.touvw(b)"
print me.touvw(b)
print ": rtn = me.touvw(b)"
rtn = me.touvw(b)
print ": print rtn['return']"
print rtn['return']
print ": print rtn['dot']"
print rtn['dot']
print ": print rtn['xyz']"
print rtn['xyz']
print ": print me.getvalue(me.touvw(b))"
print me.getvalue(me.touvw(b))
#An example with more than one value:
#
#- sb = me.baseline('itrf',qa.unit([10,50],'m'),qa.unit([20,100],'m'),
#		qa.unit([30,150],'m'))
#- me.touvw(sb,d,x); print d; print x;
#[type=uvw, refer=J2000, m2=[value=[37.4165739 187.082869] , unit=m],
#	    m1=[unit=rad, value=[-0.743811234 -0.743811234] ],
#	    m0=[unit=rad, value=[2.50094148 2.50094148] ]] 
#[value=[[1:3,]
#    0.00025643414  0.0012821707
#    0.00143537137  0.00717685684
#    0.000709009712 0.00354504856], unit=m/s]
#[value=[[1:3,]
#    -22.0746793 -110.373397
#    16.45792    82.2895998
#    -25.334668  -126.67334], unit=m]
#- me.getvalue(me.touvw(sb))          
#[*7=[unit=rad, value=[2.50094148 2.50094148] ],
#	        *8=[unit=rad, value=[-0.743811234 -0.743811234] ],
#		 *9=[value=[37.4165739 187.082869] , unit=m]] 
#- me.getvalue(me.touvw(sb))[1]
#[unit=rad, value=[2.50094148 2.50094148] ] 
#- qa.getvalue(me.getvalue(me.touvw(sb))[1])[2]
#2.50094148 
#- me.doframe(me.epoch('utc','today'))
#T 
#- me.expand(sb)
#[type=baseline, refer=ITRF, m2=[value=149.666295, unit=m],
#		 m1=[unit=rad, value=0.930274014],
#		 m0=[unit=rad, value=1.10714872]] 
#- me.expand(sb,x)
#[type=baseline, refer=ITRF, m2=[value=149.666295, unit=m], 
#		m1=[unit=rad, value=0.930274014], 
#		m0=[unit=rad, value=1.10714872]] 
#- x
#[value=[[1:3,]
#    40
#    80
#    120], unit=m] 
#- me.expand(me.touvw(sb),x); x
#[type=uvw, refer=J2000, m2=[value=149.666295, unit=m], 
#	   m1=[unit=rad, value=-0.654614537], 
#	   m0=[unit=rad, value=2.32532487]] 
#[value=[[1:3,]
#    -81.3219596
#    86.5043397
#    -91.124849], unit=m] 
#- me.touvw(me.expand(sb),xyz=x); x  
#[type=uvw, refer=J2000, m2=[value=149.666295, unit=m], 
#	   m1=[unit=rad, value=-0.654614537], 
#	   m0=[unit=rad, value=2.32532487]] 
#[value=[[1:3,]
#    -81.3219596
#    86.5043397
#    -91.124849], unit=m] 
#- a = me.touvw(sb, xyz=x); x
#[value=[[1:3,]
#    -20.3304899 -101.652449
#    21.6260849  108.130425
#    -22.7812122 -113.906061], unit=m] 
print "\n___Testing me.expand()"
print "b = me.baseline('itrf', '10m', '20m', '30m')"
b = me.baseline('itrf', '10m', '20m', '30m')
print "sb = me.baseline('itrf',qa.unit([10,50],'m'),qa.unit([20,100],'m'), qa.unit([30,150],'m'))"
sb = me.baseline('itrf',qa.unit([10,50],'m'),qa.unit([20,100],'m'), qa.unit([30,150],'m'))
print ": rtn = me.expand(b)"
rtn = me.expand(b)
print ": print rtn['return']"
print rtn['return']
print ": print rtn['xyz']"
print rtn['xyz']
print ": rtn2 = me.expand(sb)"
rtn2 = me.expand(sb)
print ": print rtn2['return']"
print rtn2['return']
print ": print rtn2['xyz']"
print rtn2['xyz']
print "[value=[[1:3,]"
print "    40"
print "    80"
print "    120], unit=m]"
print "\___Testing me.earthmagnetic()"
print ": print me.earthmagnetic('igrf')"
print me.earthmagnetic('igrf')
print ": print me.measure(me.earthmagnetic('igrf'), 'j2000')"
print me.measure(me.earthmagnetic('igrf'), 'j2000')
print "\___Testing me.baseline()"
print ": print me.listcodes(me.baseline())"
print me.listcodes(me.baseline())
print ": print me.baseline('itrf','30deg','40deg','10m')"
print me.baseline('itrf','30deg','40deg','10m')
print ": print me.measure(me.baseline('itrf','30deg','40deg','10m'), 'j2000')"
print me.measure(me.baseline('itrf','30deg','40deg','10m'), 'j2000')
print "\n___Testing me.asbaseline()"
print ": b = me.position('itrf', '10m', '20m', '30m')"
b = me.position('itrf', '10m', '20m', '30m')
print ": print b"
print b
#sb = me.position('itrf',qa.unit([10,50],'m'),qa.unit([20,100],'m'), qa.unit([30,150],'m'));
#print sb
#[type=position, refer=ITRF, m2=[value=[37.4165739 187.082869] , unit=m],
#	 m1=[unit=rad, value=[0.930274014 0.930274014] ],
#	 m0=[unit=rad, value=[1.10714872 1.10714872] ]]
print ": print b, me.asbaseline(b)"
print me.asbaseline(b)
#print sb; print me.asbaseline(sb)
#[type=position, refer=ITRF, m2=[value=[37.4165739 187.082869] , unit=m], 
#	 m1=[unit=rad, value=[0.930274014 0.930274014] ], 
#	 m0=[unit=rad, value=[1.10714872 1.10714872] ]]
#[type=baseline, refer=ITRF, m2=[value=[37.4165739 187.082869] , unit=m], 
#	 m1=[unit=rad, value=[0.930274014 0.930274014] ], 
#	 m0=[unit=rad, value=[1.10714872 1.10714872] ]]
print "\n___Testing me.listcodes()"
# Generate some direction
# Note that an empty or non-specified reference code will produce the
# measure with the default code for that measure type
print ": a=me.direction()"
a=me.direction()
print ": print me.getref(a)"
print me.getref(a)
print ": print me.ismeasure(a)"
print me.ismeasure(a)
# Get the known reference codes for direction
print ": print me.listcodes(a)"
print me.listcodes(a)
print "\n___Testing measure()"
print ": a = me.epoch('utc', 'today')" 			# a time
a = me.epoch('utc', 'today')
print ": print a"
print a
print ": print me.measure(a, 'tai')"			# convert to IAT
print me.measure(a, 'tai')
print ": print me.doframe(a)"				# set time in frame
print me.doframe(a)
print "me.doframe(me.observatory('ATCA'))"		# set position in frame
print me.doframe(me.observatory('ATCA'))
print "b=me.direction('j2000', qa.toangle('0h'), '-30deg')"  # a direction
b=me.direction('j2000', qa.toangle('0h'), '-30deg')  # a direction
print "print b"
print b
print "	m1=[value=-0.523599, unit=rad]]"
print ": print me.measure(b, 'azel')"			# convert to AZEL
print me.measure(b, 'azel')
print "	m1=[value=0.846695, unit=rad]]"
print ": print qa.angle(me.getvalue(me.measure(b, 'azel'))['value'][0]), qa.angle(me.getvalue(me.measure(b, 'azel'))['value'][1])" # show as angles
print qa.angle(me.getvalue(me.measure(b, 'azel'))['m0']), qa.angle(me.getvalue(me.measure(b, 'azel'))['m0'])
print "In the following the qv argument is used."
print "Fill the frame with necessary information"
print me.doframe(me.epoch('utc','today'))
print me.doframe(me.observatory('atca'))
print me.doframe(me.direction('mars'))
# Make a list of frequencies to be converted
a=qa.unit([1,1.1,1.2,1.3],'GHz')
print a
print "Expect [value=[1 1.1 1.2 1.3] , unit=GHz]"
print "Make a frequency measure. Although any can be used, it is advisable"
print "to use an element of the list, to make sure all units are correct"
m=me.frequency('lsrk',qa.getvalue(a), qa.getunit(a))
#print "Convert all"
#me.measure(m,'lsrd',qv=a)
#print " And check"
#print a
#print "Expect [value=[0.999995118 1.09999463 1.19999414 1.29999365] , unit=GHz]"
print "\n___Testing me.doframe()"
print ": a = me.epoch('utc','today')"
a = me.epoch('utc','today')
print ": print a"
print a
print ": print me.doframe(a)"
me.doframe(a)
print "\n___Testing me.framenow()"
print ": print me.framenow()"
print me.framenow()
print ": print me.showframe(F)"
print me.showframe(F)
print "\n___Testing me.showframe()"
print ": print me.doframe(me.epoch('utc','today'))"
print me.doframe(me.epoch('utc','today'))
print me.showframe(F)
print "\n___Testing me.toradialvelocity()"
print ": a = me.doppler('radio','0.4')"
a = me.doppler('radio','0.4')
print ": print a"
print a
print ": print me.toradialvelocity('topo',a)"
print me.toradialvelocity('topo',a)
print "\n___Testing me.tofrequency()"
print ": a = me.doppler('radio','0.4')"
a = me.doppler('radio','0.4')
print ": print a"
print a
print ": print me.tofrequency('lsrk',a,qa.constants('HI'))"
print me.tofrequency('lsrk',a,qa.constants('HI'))
print "\n___Testing me.todoppler()"
print ": f = me.frequency('lsrk','1410MHz')"     # specify a frequency
f = me.frequency('lsrk','1410MHz')
print ": print f"
print f
print ": print me.todoppler('radio', f, qa.constants('HI'))" # give doppler, using HI rest
print me.todoppler('radio', f, qa.constants('HI')) # give doppler, using HI rest
print "\n___Testing me.torestfrequency()"
print ": dp = me.doppler('radio', '2196.24984km/s')"  # a measured doppler speed 
dp = me.doppler('radio', '2196.24984km/s')  # a measured doppler speed 
print ": print dp"
print dp
print ": f = me.frequency('lsrk','1410MHz')"    # a measured frequency
f = me.frequency('lsrk','1410MHz')    # a measured frequency
print ": print f"
print f
print ": print me.torestfrequency(f, dp)"                   # the corresponding rest frequency
print me.torestfrequency(f, dp)
print "\n___ Testing me.rise()"
print "NEEDS work"
#print me.rise(me.direction('sun'))
#print "Expect [rise=[value=267.12445, unit=deg], set=[value=439.029964, unit=deg]] 
#- dq.form.long(me.rise(me.direction('sun')).rise)
#17:48:29.868
print "\n___Testing me.riseset()"
print "NEEDS work"
#- me.riseset(me.direction('sun'))                     
#[solved=T,
# rise=[last=[type=epoch, refer=LAST, m0=[value=0.0731388605, unit=d]],
#       utc=[type=epoch, refer=UTC, m0=[value=52085.8964, unit=d]]],
# set=[last=[type=epoch, refer=LAST, m0=[value=0.455732593, unit=d]],
#       utc=[type=epoch, refer=UTC, m0=[value=52086.2779, unit=d]]]] 
#- me.riseset(me.direction('sun'), dq.unit('80deg'))                     
#[solved=F,
# rise=[last=below, utc=below],
# set=[last=below, utc=below]] 
#- dq.form.long(me.riseset(me.direction('sun')).rise.utc.m0)  
#21:30:47.439
print "\n___Testing me.posangle()"
print "a=me.direction('j2000','0deg','70deg')"
a=me.direction('j2000','0deg','70deg')
print "b=me.direction('j2000','0deg','80deg')"
b=me.direction('j2000','0deg','80deg')
print ": print me.posangle(a,b)"
print me.posangle(a,b)
print ": print me.separation(a,b)"
print me.separation(a,b)
print "tim = me.epoch('utc','today')"			# set the time
tim = me.epoch('utc','today')
print ": print me.doframe(tim)"
print me.doframe(tim)
print "pos = me.observatory('ATCA')"			# set where
pos = me.observatory('ATCA')
print ": print me.doframe(pos)"
print me.doframe(pos)
print ": print me.posangle(a,b)"
print me.posangle(a,b)
print "\n___Testing me.separation()"
print "a=me.direction('j2000','0deg','70deg')"
a=me.direction('j2000','0deg','70deg')
print "b=me.direction('j2000','0deg','80deg')"
b=me.direction('j2000','0deg','80deg')
print ": print me.separation(a,b)"
print me.separation(a,b)
print "tim = me.epoch('utc','today')"			# set the time
tim = me.epoch('utc','today')
print ": print me.doframe(tim)"
print me.doframe(tim)
print "pos = me.observatory('ATCA')"			# set where
pos = me.observatory('ATCA')
print ": print me.doframe(pos)"
print me.doframe(pos)
print "c = me.measure(b, 'azel')"
c = me.measure(b, 'azel')
print ": print me.separation(a,c)"
print me.separation(a,c)
print "\n___Testing me.addxvalue()"
print "a = me.observatory('atca')"
a = me.observatory('atca')
print ": print a"
print a
print "[type=position, refer=ITRF, m2=[value=6372960.26, unit=m],"
print "		 m1=[unit=rad, value=-0.52613792],"
print "		  m0=[unit=rad, value=2.61014232]]"
print ": print me.addxvalue(a)"
print me.addxvalue(a)
print "[__*0=[value=-4750915.84, unit=m], __*1=[value=2792906.18, unit=m],"
print "			   __*2=[value=-3200483.75, unit=m]]"
#me.addxvalue(a)::
#[id=quant, shape=3] 
print ": print me.addxvalue(me.epoch('utc','today'))"
print me.addxvalue(me.epoch('utc','today'))
print "\n___Testing me.done()"
print ": print me.done()"
print me.done()
print ": print me.done()"
print me.done()
