#Starting: 
#Results 2/10

import timing
import time

global measures_results
global measures_comment
measures_results = {}
measures_comment = {}

#

def position():
        print 'POSITION'
	test=me.position('wgs84',qa.quantity(30.,'deg'),qa.quantity(40.,'deg'),qa.quantity(10.,'m'))
	test=me.position('itrf')
        d={'POSITION': True}
        comment={'POSITION': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def epoch():
        print 'EPOCH'
        #
        test=me.epoch('utc','today')
        d={'EPOCH': True}
        comment={'EPOCH': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def direction():
        print 'DIRECTION'
        #
	test=me.direction("j2000",qa.quantity(30.,"deg"),qa.quantity(40.,"deg"))
        test=me.direction('venus') #try URM example
	test=me.direction('azel')  # use defaults
	test=me.direction('itrf')  # doesn't notice that it's not allowed
        # But measures.g allow 'itrf' so it has been allowed in the past!
        d={'DIRECTION': True}
        comment={'DIRECTION': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def listcodes():
	print 'LISTCODES'
       #
        test=me.listcodes(me.epoch('utc'))
        d={'LISTCODES': True}
        comment={'LISTCODES': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def sourcelist():
        print 'SOURCELIST'
        #
        test=me.sourcelist()
        d={'SOURCELIST': True}
        comment={'SOURCELIST': 'Returns a string - same as Glish method'}
        measures_results.update(d)
        measures_comment.update(comment)

def linelist():
        print 'LINELIST'
        #
        test=me.linelist()
        d={'LINELIST': True}
        comment={'LINELIST': 'Returns a string - same as Glish method'}
        measures_results.update(d)
        measures_comment.update(comment)

def obslist():
        print 'OBSLIST'
        #
        test=me.obslist()
        d={'OBSLIST': True}
        comment={'OBSLIST': 'Returns a string - same as Glish method'}
        measures_results.update(d)
        measures_comment.update(comment)

def spectralline():
        print 'SPECTRALLINE'
        #
        test=me.spectralline()
        d={'SPECTRALLINE': True}
        comment={'SPECTRALLINE': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def framenow():
        print 'FRAMENOW'
        #
        test=me.framenow()
        d={'FRAMENOW': test}
        comment={'FRAMENOW': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def showframe():
        print 'SHOWFRAME'
        #
	me.doframe(me.epoch('utc',qa.quantity(53938.0504,'d')))
        test=me.showframe()
        d={'SHOWFRAME': True}
        comment={'SHOWFRAME': 'format Julian days'}
        measures_results.update(d)
        measures_comment.update(comment)

def dirshow():
        print 'DIRSHOW'
        #
        x=me.direction('azel')  # use defaults
	test=me.dirshow(x)
        d={'DIRSHOW': True}
        comment={'DIRSHOW': 'No way to set format of output'}
        measures_results.update(d)
        measures_comment.update(comment)

def doframe():
        print 'DOFRAME'
        #
	a=me.epoch('utc',qa.quantity(53937.7213,'d'))
	test=me.doframe(a)
        d={'DOFRAME': test}
        comment={'DOFRAME': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def framecomet():
        print 'FRAMECOMET'
        #
	test=me.framecomet('VGEO')
	#me.showframe()
	me.doframe(me.epoch('et',qa.quantity(50802.7292,'d')))
	me.measure(me.direction('comet'),'app')
        d={'FRAMECOMET': test}
        comment={'FRAMECOMET': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def cometname():
        print 'COMETNAME'
        #
	test=me.framecomet('VGEO')
        test=me.cometname()
        d={'COMETNAME': True}
        comment={'COMETNAME': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def comettype():
        print 'COMETTYPE'
        #
	test=me.framecomet('VGEO')
        test=me.comettype()
        d={'COMETTYPE': True}
        comment={'COMETTYPE': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def comettopo():
        print 'COMETTOPO'
        #
	test=me.framecomet('VGEO')
        test=me.comettopo()
        d={'COMETTOPO': True}
        comment={'COMETTOPO': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def rise():
        print 'RISE'
        #
        test=me.rise(me.direction('sun'))
        d={'RISE': False}
        comment={'RISE': 'unimplemented'}
        measures_results.update(d)
        measures_comment.update(comment)

def riseset():
        print 'RISESET'
        #
        test=me.riseset(me.direction('sun'))
        d={'RISESET': False}
        comment={'RISESET': 'unimplemented'}
        measures_results.update(d)
        measures_comment.update(comment)

def baseline():
        print 'BASELINE'
        #
        test=me.baseline('itrf')
	test=me.baseline('itrf',qa.quantity(30.,'deg'),qa.quantity(40.,'deg'),qa.quantity(10.,'m'))
	test=me.baseline('itrf',qa.quantity(10.,'m'),qa.quantity(20,'m'),qa.quantity(30.,'m'))
        d={'BASELINE': True}
        comment={'BASELINE': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def expand():
        print 'EXPAND'
        x=me.uvw('itrf',qa.quantity(30.,'deg'),qa.quantity(40.,'deg'),qa.quantity(10.,'m'))
        test=me.expand(x)
	me.doframe(me.observatory('atca'))
	me.doframe(me.source('1934-638'))
	me.doframe(me.epoch('utc',qa.quantity(53937.9588,'d')))
	b=me.baseline('itrf',qa.quantity(10.,'m'),qa.quantity(20,'m'),qa.quantity(30.,'m'))
	test=me.expand(b)
	#print test["return"]
	#print test["xyz"].value, test["xyz"].units
        d={'EXPAND': True}
        comment={'EXPAND': ''}
        measures_results.update(d)
        measures_comment.update(comment)


def separation():
        print 'SEPARATION'
        #
	a=me.direction('j2000',qa.quantity(0.,'deg'),qa.quantity(70.,'deg'))
	b=me.direction('j2000',qa.quantity(0.,'deg'),qa.quantity(80.,'deg'))
	test=me.separation(a,b)
	#print test.value, test.units
        d={'SEPARATION': True}
        comment={'SEPARATION': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def posangle():
        print 'POSANGLE'
        #
	a=me.direction('j2000',qa.quantity(0.,'deg'),qa.quantity(70.,'deg'))
	b=me.direction('j2000',qa.quantity(0.,'deg'),qa.quantity(80.,'deg'))
	test=me.posangle(a,b)
	#print test.value, test.units
	test=me.separation(a,b)
	#print test.value, test.units
        d={'POSANGLE': True}
        comment={'POSANGLE': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def radialvelocity():
        print 'RADIALVELOCITY'
        #
        test=me.radialvelocity('lsrk',qa.quantity(20.,'km/s')) #try URM example
        test=me.radialvelocity('lsrk')  # use defaults
        #test=me.radialvelocity('dumbo')
        d={'RADIALVELOCITY': True}
        comment={'RADIALVELOCITY': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def doppler():
        print 'DOPPLER'
        #
	test=me.doppler('radio',qa.quantity(0.4,''))
        test=me.doppler('radio',qa.quantity(1.2e8,'m/s')) #try URM example
        test=me.doppler('radio')  # use defaults
        #test=me.doppler('dumbo')
        d={'DOPPLER': True}
        comment={'DOPPLER': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def frequency():
        print 'FREQUENCY'
        #
        test=me.frequency('lsrk',qa.quantity(1410,'MHz')) #try URM example
        d={'FREQUENCY': True}
        comment={'FREQUENCY': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def uvw():
        print 'UVW'
        #
	test=me.uvw('itrf',qa.quantity(30.,'deg'),qa.quantity(40.,'deg'),qa.quantity(10.,'m'))
        test=me.uvw('itrf') #try URM example
        d={'UVW': True}
        comment={'UVW': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def earthmagnetic():
        print 'EARTHMAGNETIC'
        #
        test=me.earthmagnetic('igrf') #try URM example
	#me.measure(me.earthmagnetic('igrf'),'j2000') #missing frame info?
        d={'EARTHMAGNETIC': True}
        comment={'EARTHMAGNETIC': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def source():
        print 'SOURCE'
        #
	#me.source?
	test=me.source()
	#do a me.sourcelist() to try another
	x=me.source('J235753.2-531113')
        d={'SOURCE': True}
        comment={'SOURCE': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def observatory():
        print 'OBSERVATORY'
        #
	#me.observatory?
	me.observatory()
	#try me.obslist()
	test=me.observatory('BIMA')
        d={'OBSERVATORY': True}
        comment={'OBSERVATORY': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def done():
        print 'DONE'
        #
        test=me.done()
        d={'DONE': test}
        comment={'DONE': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def todoppler():
        print 'TODOPPLER'
	f=me.frequency('lsrk',qa.quantity(1410.,'MHz'))
	frest=me.frequency('rest',qa.quantity(1420.40575,'MHz'))
        t=me.todoppler('radio',f,frest)
	test=me.measure(t,'RADIO')
        d={'TODOPPLER': True}
        comment={'TODOPPLER': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def tofrequency():
        print 'TOFREQUENCY'
	f=me.doppler('radio',qa.quantity(0.4,''))
	frest=me.frequency('rest',qa.quantity(1420.40575,'MHz'))
	me.tofrequency('lsrk',f,frest)
        d={'TOFREQUENCY': True}
        comment={'TOFREQUENCY': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def torestfrequency():
        print 'TORESTFREQUENCY'
        #x=me.frequency('lsrk')
        #test=me.torestfrequency('topo',x)
	dp=me.doppler('radio',qa.quantity(2196.2498,'km/s'))
	f=me.frequency('lsrk',qa.quantity(1410.,'MHz'))
	me.torestfrequency(f,dp)
        d={'TORESTFREQUENCY': True}
        comment={'TORESTFREQUENCY': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def toradialvelocity():
        print 'TORADIALVELOCITY'
	a=me.doppler('radio',qa.quantity(119916983.,'m/s'))
        test=me.toradialvelocity('topo',a)
        d={'TORADIALVELOCITY': True}
        comment={'TORADIALVELOCITY': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def touvw():
        print 'TOUVW'
        #x=me.uvw('J2000')
        #test=me.touvw('B1950',x)
	me.doframe(me.observatory('atca'))
	me.doframe(me.source('1934-638'))
	me.doframe(me.epoch('utc',qa.quantity(53937.9588,'d')))
	b=me.baseline('itrf',qa.quantity(10.,'m'),qa.quantity(20.,'m'),qa.quantity(30.,'m'))
	test=me.touvw(b)
	#print test["return"]
	#print test["dot"].value, test["dot"].units
	#print test["xyz"].value, test["xyz"].units
        d={'TOUVW': True}
        comment={'TOUVW': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def measure():
        print 'MEASURE'
	x=me.epoch('utc',qa.quantity(53937.7655,'d'))
	test=me.measure(x,'tai')
        d={'MEASURE': True}
        comment={'MEASURE': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def getvalue():
        print 'GETVALUE'
	x=me.direction('j2000',qa.quantity(0.,'deg'),qa.quantity(80.,'deg'))
	test=me.getvalue(x)
	#print test.value, test.units
        d={'GETVALUE': True}
        comment={'GETVALUE': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def gettype():
        print 'GETTYPE'
	x=me.direction('j2000',qa.quantity(0.,'deg'),qa.quantity(80.,'deg'))
	me.gettype(x)
        d={'GETTYPE': True}
        comment={'GETTYPE': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def getref():
        print 'GETREF'
	x=me.direction('j2000',qa.quantity(0.,'deg'),qa.quantity(80.,'deg'))
	test=me.getref(x)
        d={'GETREF': True}
        comment={'GETREF': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def getoffset():
        print 'GETOFFSET'
	x=me.direction('j2000',qa.quantity(0.,'deg'),qa.quantity(80.,'deg'))
	test=me.getoffset(x)
        x=me.epoch('utc')
	test=me.getoffset(x)
        d={'GETOFFSET': True}
        comment={'GETOFFSET': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def show():
        print 'SHOW'
	me.show(me.frequency('lsrk',qa.quantity(1421.,'MHz')))
        x=me.epoch('utc')
        me.measure(x,'tai')
        test=me.show(x)
        d={'SHOW': True}
        comment={'SHOW': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def addxvalue():
        print 'ADDXVALUE'
        #
	a=me.observatory('atca')
	test=me.addxvalue(a)
	#print test.value, test.units
        d={'OPEN': True}
        comment={'OPEN': ''}
        measures_results.update(d)
        measures_comment.update(comment)

def asbaseline():
        print 'ASBASELINE'
        #
	a = me.epoch('utc',qa.quantity(53937.7652,'d'))
	me.measure(a,'tai')
	me.doframe(a)				# set time in frame
	me.doframe(me.observatory('ATCA'))	# set position in frame
	d = me.direction('j2000',qa.quantity(0.,'deg'),qa.quantity(-30.,'deg'))
	me.measure(d,'azel')
	me.doframe(d)				# set direction in frame
	b=me.position('itrf', qa.quantity(10.,'m'), qa.quantity(20.,'m'), qa.quantity(30.,'m'))
	me.asbaseline(b)
        d={'ASBASELINE': True}
        comment={'ASBASELINE': ''}
        measures_results.update(d)
        measures_comment.update(comment)


def measures_interface():

        timing.start()

	POSITION		= True 	# 30
	EPOCH			= True 	# 13
	LISTCODES		= True 	# 25
	SOURCELIST		= True 	# 39
	LINELIST		= True 	# 24
	OBSLIST			= True 	# 28
	SPECTRALLINE		= True 	# 40
	FRAMENOW		= True 	# 18
	SHOWFRAME		= True 	# 37
	DIRECTION		= True 	# 6
	DIRSHOW			= True 	# 7
	COMETNAME		= True 	# 3
	COMETTYPE		= True 	# 5
	COMETTOPO		= True 	# 4
	RISE			= True 	# 32
	RISESET			= True 	# 33
	BASELINE		= True 	# 2
	EXPAND			= True 	# 14
	SEPARATION		= True 	# 34
	POSANGLE		= True 	# 29
	RADIALVELOCITY		= True 	# 31
	DOPPLER			= True 	# 10
	FREQUENCY		= True 	# 19
	SOURCE			= True 	# 38
	OBSERVATORY		= True 	# 27
	EARTHMAGNETIC		= True 	# 12
	UVW			= True 	# 46
	DONE			= True 	# 9
	TODOPPLER		= True 	# 41
	TOFREQUENCY		= True 	# 42
	TORADIALVELOCITY	= True 	# 43
	TORESTFREQUENCY		= True 	# 44
	TOUVW			= True 	# 45
	MEASURE			= True 	# 26
	GETREF			= True 	# 21
	GETOFFSET		= True 	# 20
	GETTYPE			= True 	# 22
	GETVALUE		= True 	# 23
	SHOW			= True 	# 35
	SHOWAUTO		= True 	# 36

	DOFRAME			= True 	# 8
	DOSHOWAUTO		= True 	# 11
	FRAMEAUTO		= True 	# 15
	FRAMECOMET		= True 	# 16
	FRAMENOAUTO		= True 	# 17
        ADDXVALUE               = True  # 0
        ASBASELINE              = True  # 1


	if (ADDXVALUE):		addxvalue()
	if (ASBASELINE):	asbaseline()
	if (BASELINE):		baseline()
	if (COMETNAME):		cometname()
	if (COMETTOPO):		comettopo()
	if (COMETTYPE):		comettype()
	if (DIRECTION):		direction()
	if (DIRSHOW):		dirshow()
	if (DOFRAME):		doframe()
	if (DONE):		done()
	if (DOPPLER):		doppler()
	if (EARTHMAGNETIC):	earthmagnetic()
	if (EPOCH):		epoch()
	if (EXPAND):		expand()
	if (FRAMECOMET):	framecomet()
	if (FRAMENOW):		framenow()
	if (FREQUENCY):		frequency()
	if (GETOFFSET):		getoffset()
	if (GETREF):		getref()
	if (GETTYPE):		gettype()
	if (GETVALUE):		getvalue()
	if (LINELIST):		linelist()
	if (LISTCODES):		listcodes()
	if (MEASURE):		measure()
	if (OBSERVATORY):	observatory()
	if (OBSLIST):		obslist()
	if (POSANGLE):		posangle()
	if (POSITION):		position()
	if (RADIALVELOCITY):	radialvelocity()
	if (RISE):		rise()
	if (RISESET):		riseset()
	if (SEPARATION):	separation()
	if (SHOW):		show()
	if (SHOWFRAME):		showframe()
	if (SOURCE):		source()
	if (SOURCELIST):	sourcelist()
	if (SPECTRALLINE):	spectralline()
	if (TODOPPLER):		todoppler()
	if (TOFREQUENCY):	tofrequency()
	if (TORADIALVELOCITY):	toradialvelocity()
	if (TORESTFREQUENCY):	torestfrequency()
	if (TOUVW):		touvw()
	if (UVW):		uvw()
	
        timing.finish()
        print 'Measures interface time is: ',timing.milli()/1000.

        return True

measures_interface()

print '---'
print 'Measures: '
print '   Working: ',measures_results.values().count(True)
print '   Fail:    ',measures_results.values().count(False)
print '---'
format = "%15s %7s %35s"
for k,v,c in zip(measures_results.keys(),measures_results.values(),measures_comment.values()):
	print format % (k,v,c)
print '---'
print ''
