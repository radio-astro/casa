#attool = casac.homefinder.find_home_by_name('atmosphereHome')
#at = attool.create()
at = casac.atmosphere()
tmp = qa.quantity(270.0,'K')
pre = qa.quantity(560.0,'mbar')
hum = 20.0
alt = qa.quantity(5000,'m')
h0  = qa.quantity(2.0,'km')
wvl = qa.quantity(-5.6,'K/km')
mxA = qa.quantity(48,'km')
dpr = qa.quantity(5.0,'mbar')
dpm = 1.1
att = 1
myatm = at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
print "Atmospheric type:   ", at.getBasicAtmParms()[10]
print "Number of layers returned: ", at.getNumLayers()
print myatm
p = at.getProfile()
print p[0]
w = at.getGroundWH2O()
print "First guess precipitable water vapor content: ", w['value'][0], w['unit']
print "(This value is estimated from the relative humidity at ground level and the water vapor scale height)"
print
print
print "Example 1: Absorption profile for a single frequency: 850 GHz"
nb = 1
fC = qa.quantity([850.0],'GHz')
fW = qa.quantity([1.0],'Hz')
fR = qa.quantity([0.0],'Hz')
at.initSpectralWindow(nb,fC,fW,fR)
do=at.getDryOpacitySpec()
rchan=at.getRefChan()
print
print "Total Dry Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getDryOpacity() 
print
print "Total Dry Cont Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getDryContOpacity()
print "Total O2 lines Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getO2LinesOpacity() 
print "Total O3 lines Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getO3LinesOpacity() 
print "Total CO lines Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getCOLinesOpacity() 
print "Total N2O lines Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getN2OLinesOpacity()
print
wo=at.getWetOpacitySpec()
rchan=at.getRefChan()
print "Total Wet Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getWetOpacity()['value'][0], at.getWetOpacity()['unit']
print
print "Total H2O lines Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getH2OLinesOpacity()
print "Total H2O Cont Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getH2OContOpacity()
print
print "Total Wet Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getWetOpacity()['value'][0] / w['value'][0], " per mm of water vapor."
print
print
print "Total Dispersive Delay at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getDispersiveWetPathLength()['value'][0], at.getDispersiveWetPathLength()['unit']
print "Total Non-Dispersive Delay at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getNonDispersiveWetPathLength()['value'][0], at.getNonDispersiveWetPathLength()['unit']
print "Total Dry Delay at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getNonDispersiveDryPathLength()['value'][0], at.getNonDispersiveDryPathLength()['unit']
print "Total O2 lines Delay at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getO2LinesPathLength()['value'][0], at.getO2LinesPathLength()['unit']
print "Total O3 lines Delay at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getO3LinesPathLength()['value'][0], at.getO3LinesPathLength()['unit']
print "Total CO lines Delay at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getCOLinesPathLength()['value'][0], at.getCOLinesPathLength()['unit']
print "Total N2O lines Delay at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getN2OLinesPathLength()['value'][0], at.getN2OLinesPathLength()['unit']
print
print
print  "(your actual water vapor column is ", at.getGroundWH2O()['value'][0], at.getGroundWH2O()['unit'], ")"

#print "reference channel is ", rchan
#print "Total Dispersive Delay at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getDispersivePathLength()['value'][0] /  w['value'][0], " meters per mm of water vapor"
#print "(",100*(at.getDispersivePathLength()['value'][0] /  w['value'][0])/(at.getNonDispersivePathLength()['value'][0] / w['value'][0]), "% of the Non-dispersive one )"
print
print "change a basic parameter"
print "========================"
print "Old ground temperature: ", at.getBasicAtmParms()[2]['value'][0], at.getBasicAtmParms()[2]['unit']
new_tmp = qa.quantity(275.0,'K')
print "New ground temperature:", new_tmp['value'], new_tmp['unit']
print at.updateAtmProfile(alt, new_tmp, pre, hum, wvl, h0)
print "Absorption Profile with this new temperature.  Summary of results:"
do=at.getDryOpacitySpec()
print "Total Dry Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getDryOpacity() 
wo=at.getWetOpacitySpec()
print "Total Wet Opacity at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getWetOpacity()['value'][0] / w['value'][0], at.getWetOpacity()['unit']
print "Total Dispersive Delay at ", fC['value'][0], fC['unit'], " for 1.0 air mass: ", at.getDispersivePathLength()['value'][0] /  w['value'][0], " meters per mm of water vapor"
print "(",100*(at.getDispersivePathLength()['value'][0] /  w['value'][0])/(at.getNonDispersivePathLength()['value'][0] / w['value'][0]), "% of the Non-dispersive one )"
print
print "Add a spectral window"
print "====================="
fC2 = qa.quantity(350.0,'GHz')
fW2 = qa.quantity(0.008,'GHz')
fR2 = qa.quantity(0.002,'GHz')
nc = at.addSpectralWindow(fC2,fW2,fR2)
print "New spectral window has ", nc, " channels"
#w = at.getStartupWaterContent()
w = at.getGroundWH2O()
numSpw = at.getNumSpectralWindows()
print "There are now ", numSpw, " spectral windows"
print "Absorption profiles including this new spectral window.  Summary of results:"
print "Total Dry Opacity at ", fC['value'][0], fC['unit'], " GHz for 1.0 air mass: ", at.getDryOpacity()
for spwid in range(numSpw):
	numCh = at.getNumChan(spwid)
	print "Spectral window ", spwid, " has ", numCh, " frequency channels"
	for n in range(numCh):
		freq = at.getChanFreq(n, spwid)
		print "Total Wet Opacity at ", freq['value'][0], freq['unit'], " for 1.0 air mass: ", at.getWetOpacity(n,spwid)['value'][0] / w['value'][0], at.getWetOpacity()['unit']
		#print "Total Dispersive Delay at ", freq['value'][0], freq['unit'], " for 1.0 air mass: ", (at.getDispersivePathLength(n,spwid)['value'][0])/(w['value'][0]), " meters per mm of water vapor (", ((100*at.getDispersivePathLength(n,spwid)['value'][0])/(w['value'][0]))/(at.getNonDispersivePathLength(n,spwid)['value'][0]/w['value'][0]), "% of the Non-dispersive one )"
		#print "Total Dispersive Phase Delay at ", freq['value'][0], freq['unit'], " for 1.0 air mass: ", (at.getDispersivePhaseDelay(n,spwid)['value'][0])/(w['value'][0]), " degrees per mm of water vapor (", ((100*at.getDispersivePhaseDelay(n,spwid)['value'][0])/(w['value'][0]))/(at.getNonDispersivePhaseDelay(n,spwid)['value'][0]/w['value'][0]), "% of the Non-dispersive one )"
		print "(your actual water vapor column is ", w['value'][0], w['unit'], " of water vapor)."
		print ""
print "====================="
for spwid in range(numSpw):
	numCh = at.getNumChan(spwid)
	print "Spectral window ", spwid, " has ", numCh, " frequency channels"
	for n in range(numCh):
		freq = at.getChanFreq(n, spwid)
		#print "Total Dispersive Phase Delay at ", freq['value'][0], freq['unit'], " for 1.0 air mass: ", (at.getDispersivePhaseDelay(n,spwid)['value'][0])/(w['value'][0]), " degrees per mm of water vapor (", ((100*at.getDispersivePhaseDelay(n,spwid)['value'][0])/(w['value'][0]))/(at.getNonDispersivePhaseDelay(n,spwid)['value'][0]/w['value'][0]), "% of the Non-dispersive one )"
		print ""
exit()
