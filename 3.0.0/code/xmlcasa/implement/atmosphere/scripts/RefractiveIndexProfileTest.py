attool = casac.homefinder.find_home_by_name('atmosphereHome')
at = attool.create()
tmp = quantity(270.0,'K')
pre = quantity(560.0,'mbar')
hum = 20.0
alt = quantity(5000,'m')
h0  = quantity(2.0,'km')
wvl = quantity(-5.6,'K/km')
mxA = quantity(48,'km')
dpr = quantity(10.0,'mbar')
dpm = 1.2
att = 1
myatm = at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)
print "Atmospheric type:   ", at.getBasicAtmParms()['atmType']
print "Number of layers returned: ", at.getNumLayers()
print myatm
p = at.getProfile()
print p['return']
w = at.getStartupWaterContent()
print "First guess precipitable water vapor content: ", w.value, w.units
print "(This value is estimated from the relative humidity at ground level and the water vapor scale height)"
print
print
print "Example 1: Absorption profile for a single frequency: 850 GHz"
nb = 1
fC = quantity([850.0],'GHz')
fW = quantity([1.0],'Hz')
fR = quantity([0.0],'Hz')
at.initSpectralWindow(nb,fC,fW,fR)
do=at.getDryOpacitySpec()
rchan=at.getRefChan()
print
print "Total Dry Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getDryOpacity() 
print
print "Total Dry Cont Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getDryContOpacity()
print "Total O2 lines Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getO2LinesOpacity() 
print "Total O3 lines Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getO3LinesOpacity() 
print "Total CO lines Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getCOLinesOpacity() 
print "Total N2O lines Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getN2OLinesOpacity()
print
wo=at.getWetOpacitySpec()
rchan=at.getRefChan()
print "Total Wet Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getWetOpacity().value, at.getWetOpacity().units
print
print "Total H2O lines Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getH2OLinesOpacity()
print "Total H2O Cont Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getH2OContOpacity()
print
print "Total Wet Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getWetOpacity().value / w.value, " per mm of water vapor."
print
print
print "Total Dispersive Delay at ", fC.value, fC.units, " for 1.0 air mass: ", at.getDispersiveWetPathLength().value, at.getDispersiveWetPathLength().units
print "Total Non-Dispersive Delay at ", fC.value, fC.units, " for 1.0 air mass: ", at.getNonDispersiveWetPathLength().value, at.getNonDispersiveWetPathLength().units
print "Total Dry Delay at ", fC.value, fC.units, " for 1.0 air mass: ", at.getNonDispersiveDryPathLength().value, at.getNonDispersiveDryPathLength().units
print "Total O2 lines Delay at ", fC.value, fC.units, " for 1.0 air mass: ", at.getO2LinesPathLength().value, at.getO2LinesPathLength().units
print "Total O3 lines Delay at ", fC.value, fC.units, " for 1.0 air mass: ", at.getO3LinesPathLength().value, at.getO3LinesPathLength().units
print "Total CO lines Delay at ", fC.value, fC.units, " for 1.0 air mass: ", at.getCOLinesPathLength().value, at.getCOLinesPathLength().units
print "Total N2O lines Delay at ", fC.value, fC.units, " for 1.0 air mass: ", at.getN2OLinesPathLength().value, at.getN2OLinesPathLength().units
print
print
print  "(your actual water vapor column is ", at.getGroundWH2O().value, at.getGroundWH2O().units, ")"

#print "reference channel is ", rchan
#print "Total Dispersive Delay at ", fC.value, fC.units, " for 1.0 air mass: ", at.getDispersivePathLength().value /  w.value, " meters per mm of water vapor"
#print "(",100*(at.getDispersivePathLength().value /  w.value)/(at.getNonDispersivePathLength().value / w.value), "% of the Non-dispersive one )"
print
print "change a basic parameter"
print "========================"
print "Old ground temperature: ", at.getBasicAtmParms()['temperature'].value, at.getBasicAtmParms()['temperature'].units
new_tmp = quantity(275.0,'K')
print "New ground temperature:", new_tmp.value, new_tmp.units
print at.updateAtmProfile(alt, new_tmp, pre, hum, wvl, h0)
print "Absorption Profile with this new temperature.  Summary of results:"
do=at.getDryOpacitySpec()
print "Total Dry Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getDryOpacity() 
wo=at.getWetOpacitySpec()
print "Total Wet Opacity at ", fC.value, fC.units, " for 1.0 air mass: ", at.getWetOpacity().value / w.value, at.getWetOpacity().units
print "Total Dispersive Delay at ", fC.value, fC.units, " for 1.0 air mass: ", at.getDispersivePathLength().value /  w.value, " meters per mm of water vapor"
print "(",100*(at.getDispersivePathLength().value /  w.value)/(at.getNonDispersivePathLength().value / w.value), "% of the Non-dispersive one )"
print
print "Add a spectral window"
print "====================="
fC2 = quantity(350.0,'GHz')
fW2 = quantity(0.008,'GHz')
fR2 = quantity(0.002,'GHz')
nc = at.addSpectralWindow(fC2,fW2,fR2)
print "New spectral window has ", nc, " channels"
w = at.getStartupWaterContent()
numSpw = at.getNumSpectralWindows()
print "There are now ", numSpw, " spectral windows"
print "Absorption profiles including this new spectral window.  Summary of results:"
print "Total Dry Opacity at ", fC.value, fC.units, " GHz for 1.0 air mass: ", at.getDryOpacity()
for spwid in range(numSpw):
	numCh = at.getNumChan(spwid)
	print "Spectral window ", spwid, " has ", numCh, " frequency channels"
	for n in range(numCh):
		freq = at.getChanFreq(n, spwid)
		print "Total Wet Opacity at ", freq.value, freq.units, " for 1.0 air mass: ", at.getWetOpacity(n,spwid).value / w.value, at.getWetOpacity().units
		#print "Total Dispersive Delay at ", freq.value, freq.units, " for 1.0 air mass: ", (at.getDispersivePathLength(n,spwid).value)/(w.value), " meters per mm of water vapor (", ((100*at.getDispersivePathLength(n,spwid).value)/(w.value))/(at.getNonDispersivePathLength(n,spwid).value/w.value), "% of the Non-dispersive one )"
		#print "Total Dispersive Phase Delay at ", freq.value, freq.units, " for 1.0 air mass: ", (at.getDispersivePhaseDelay(n,spwid).value)/(w.value), " degrees per mm of water vapor (", ((100*at.getDispersivePhaseDelay(n,spwid).value)/(w.value))/(at.getNonDispersivePhaseDelay(n,spwid).value/w.value), "% of the Non-dispersive one )"
		print "(your actual water vapor column is ", w.value, w.units, " of water vapor)."
		print ""
print "====================="
for spwid in range(numSpw):
	numCh = at.getNumChan(spwid)
	print "Spectral window ", spwid, " has ", numCh, " frequency channels"
	for n in range(numCh):
		freq = at.getChanFreq(n, spwid)
		#print "Total Dispersive Phase Delay at ", freq.value, freq.units, " for 1.0 air mass: ", (at.getDispersivePhaseDelay(n,spwid).value)/(w.value), " degrees per mm of water vapor (", ((100*at.getDispersivePhaseDelay(n,spwid).value)/(w.value))/(at.getNonDispersivePhaseDelay(n,spwid).value/w.value), "% of the Non-dispersive one )"
		print ""
exit()
