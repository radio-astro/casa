import math
#
# Test function
#
#tmeasures():
#	"""measures and quanta test result
#
#	Returns boolean"""
#
# final result
#
# result = true;
#
# Some aid functions
#

def qtst(a0, a1, txt='', prec=1e-12):
	"""quantity test

	Returns boolean"""
	result = true
	if abs(qa.canon(a0)['value'] - qa.canon(a1)['value']) > prec or qa.canon(a0)['unit'] != qa.canon(a1)['unit']: 
		print 'tmeasures error for',txt,'--',a0,':',a1
		result = false
	return result

def vtst(a0, a1, txt='', prec=1e-12):
	"""value test

	Returns boolean"""
	result = true
	if (abs(a0-a1) > prec):
		print 'tmeasures error for',txt,'--',a0,':',a1
		result = false;
	return result
     
def stst(a0, a1, txt=''):
	"""string test

	Returns boolean"""
	result = true
	if a0 != a1:
		print 'tmeasures error for',txt,'--',a0,':',a1
		result = false;
	return result;

def ttst(a0, txt=''):
	"""true test


	Returns boolean"""
	result = true
	if not a0:
		print 'tmeasures error for',txt,'--',a0
		result = false;
	return result

def ftst(a0, txt=''):
	"""false test

	Returns boolean"""
	result = true
	if (a0):
		print 'tmeasures error for',txt,'--',a0
		result = false;
	return result;

#
# Test routines
#
ftst(qa.isquantity(5), 'isquantity');
a = qa.quantity('5km/s');
b = qa.quantity(5,'km/s');
c = qa.quantity(5000,'m/s');
d = qa.quantity('0.5rad');
ttst(qa.isquantity(a), 'isquantity');
qtst(a,b,'quantity');
qtst(a,c,'quantity');
stst(a['unit'],b['unit'],'quantity');
qtst(a,qa.unit('0.005Mm/s'),'unit');
#
e = me.epoch('utc','1997/12/23/15:00');
ftst(me.ismeasure(a),'ismeasure');
ttst(me.ismeasure(e),'ismeasure');
qtst(e['m0'],qa.unit('50805.625d'),'epoch');
#
#  qa.setformat('prec',10);
#  vtst(qa.getformat('prec'),10,'setformat');
#  qa.setformat('long','+deg');
#  stst(qa.getformat('long'),'+deg','setformat');
##
vtst(qa.convert(a,'mm')['value'],5000000,'convert');
vtst(qa.canonical(a)['value'],5000,'canonical');
stst(qa.canon(a)['unit'],'m.s-1','canon');
ttst(qa.define('xunit','5Jy'),'define');
qtst(qa.unit('3xunit'),qa.unit('15Jy'),'define');
##
#vtst(len(split(qa.map('pre'))),65,'map');
#stst(qa.angle(d,8),"['+028.38.52.40']",'angle');
#stst(qa.time(e['m0'],8,form=["ymd", "day"]),"['Tue-1997/12/23/15:00:00.00']",'time');
ttst(qa.fits(),'fits');
##
qtst(qa.neg(a),qa.unit('-5km/s'),'neg');
qtst(qa.add(a,c),qa.unit('10km/s'),'add');
qtst(qa.mul(a,c),qa.unit('25km2.s-2'),'mul');
qtst(qa.sub(a,qa.mul('0.5',a)),qa.unit('2.5km/s'),'sub');
qtst(qa.div(a,c),qa.unit('1'),'div');
qtst(qa.norm(qa.unit('330deg')),qa.unit('-30deg'),'norm');
qtst(qa.norm(qa.unit('330deg'),0),qa.unit('330deg'),'norm');
qtst(qa.sin(d),qa.quantity(math.sin(0.5)),'sin');
qtst(qa.cos(d),qa.quantity(math.cos(0.5)),'cos');
qtst(qa.tan(d),qa.quantity(math.tan(0.5)),'tan');
qtst(qa.atan(qa.quantity(0.5)),qa.quantity(math.atan(0.5),'rad'),'atan');
qtst(qa.asin(qa.quantity(0.5)),qa.quantity(math.asin(0.5),'rad'),'asin');
qtst(qa.acos(qa.quantity(0.5)),qa.quantity(math.acos(0.5),'rad'),'acos');
qtst(qa.atan2(qa.quantity(0.5),qa.quantity(0.6)),qa.quantity(math.atan(0.5/0.6),'rad'),'atan2');
qtst(qa.abs(qa.unit('-5km/s')),a,'abs');
qtst(qa.ceil('-4.1m'),qa.unit('-4m'),'ceil');
qtst(qa.floor('-4.1m'),qa.unit('-5m'),'floor');
ttst(qa.compare(a,c),'compare');
ftst(qa.compare(a,d),'compare');
ttst(qa.check('Jy/Ms'),'check');
ftst(qa.check('xxJy/xs'),'check');
qtst(qa.pow(a,3),qa.unit('125km3.s-3'),'pow');
##
#fltst(me.observatory('atca'),'observatory');
ob = me.observatory('atca');
ttst(me.ismeasure(ob),'observatory');
ob = me.measure(ob,'itrf');
#if (me.ismeasure(ob)):
#	print qtst(ob['m0'],qa.unit('-4750915.837m'),'observatory',prec=0.5);
#	print qtst(ob['m1'],qa.unit('2792906.182m'),'observatory',prec=0.5);
#	print qtst(ob['m2'],qa.unit('-3200483.747m'),'observatory',prec=0.5);
	
#print stst(split(me.obslist())[1],'ATCA','obslist');
qtst(qa.constants('pi'),qa.quantity(math.pi),'constants');
#print stst(me.myupc('abC4d'),'ABC4D','myupc');
#print stst(me.mydownc('abC4d'),'abc4d','mydownc');
#
e0 = me.epoch('utc','50805.625d');
ttst(me.ismeasure(e0),'epoch');
qtst(me.measure(e0,'iat')['m0'],qa.add(e0['m0'],'31s'),'measure');
#
ttst(me.doframe(e0),'doframe');
ttst(me.showframe(),'showframe');
#  ftst(me.gui(),'gui');
#  ttst(qa.errorgui('Correct'),'errorgui');
#  ftst(me.epochgui(),'epochgui');
#  ftst(me.positiongui(),'positiongui');
#  ftst(me.directiongui(),'directiongui');
#  ftst(me.frequencygui(),'frequencygui');
#  ftst(me.dopplergui(),'dopplergui');
#  ftst(me.radialvelocitygui(),'radialvelocitygui');
ttst(me.ismeasure(me.direction('jup')),'direction');
##
d0 = me.direction('jup');
d0 = me.measure(d0,'j20');
ttst(me.ismeasure(d0),'direction');
if (me.ismeasure(d0)):
	qtst(d0['m0'],qa.unit('-0.6410274987rad'),'direction',prec=1e-8);
	qtst(d0['m1'],qa.unit('-0.2697987142rad'),'direction',prec=1e-8);

##
p0 = me.position('itrf','-4750915.837m','2792906.182m','-3200483.747m');
ttst(me.ismeasure(p0),'position');
p0 = me.measure(p0,'itrf');
#if (me.ismeasure(p0)):
#	print qtst(p0['m0'],qa.unit('-4750915.837m'),'position',prec=1e-8);
#	print qtst(p0['m1'],qa.unit('2792906.182m'),'position',prec=1e-8);
#	print qtst(p0['m2'],qa.unit('-3200483.747m'),'position',prec=1e-8);

#
qtst(qa.toangle(d),d,'toangle');
qtst(qa.totime(d),qa.quantity(0.5/math.pi/2,'1.d'),'totime');
#
# End test function
#
# return result;
#
# Execute test function
#
##exit !tmeasures();
#
#
