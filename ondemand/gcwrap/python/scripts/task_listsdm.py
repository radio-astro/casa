########################################################################
# Task to retrieve observing information from SDM XML files
#
# v1.0: 2010.12.07, M. Krauss
# v1.1: 2011.02.23, M. Krauss: added functionality for ALMA data
#
# Original code based on readscans.py, courtesy S. Meyers

import numpy as np
from taskinit import *
try:
    from xml.dom import minidom
except ImportError, e:
    print "failed to load xml.dom.minidom:\n", e
    exit(1)

def listsdm(sdm=None):

    # read Scan.xml
    xmlscans = minidom.parse(sdm+'/Scan.xml')
    scandict = {}
    startTimeShort = []
    endTimeShort = []
    rowlist = xmlscans.getElementsByTagName("row")
    for rownode in rowlist:
        rowfid = rownode.getElementsByTagName("scanNumber")
        fid = int(rowfid[0].childNodes[0].nodeValue)
        scandict[fid] = {}
        
        # number of subscans
        rowsubs = rownode.getElementsByTagName("numSubscan")
        if len(rowsubs) == 0:
            # EVLA and old ALMA data
            rowsubs = rownode.getElementsByTagName("numSubScan")
        nsubs = int(rowsubs[0].childNodes[0].nodeValue)
        
        # intents
        rownint = rownode.getElementsByTagName("numIntent")
        nint = int(rownint[0].childNodes[0].nodeValue)
        
        rowintents = rownode.getElementsByTagName("scanIntent")
        sint = str(rowintents[0].childNodes[0].nodeValue)
        sints = sint.split()
        rint = ''
        for r in range(nint):
            intent = sints[2+r]
            if rint=='':
                rint = intent
            else:
                rint += ' '+intent
        
        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName("startTime")
        start = int(rowstart[0].childNodes[0].nodeValue)
        startmjd = float(start)*1.0E-9/86400.0
        t = qa.quantity(startmjd,'d')
        starttime = qa.time(t,form="ymd",prec=8)[0]
        startTimeShort.append(qa.time(t,prec=8)[0])
        rowend = rownode.getElementsByTagName("endTime")
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjd = float(end)*1.0E-9/86400.0
        t = qa.quantity(endmjd,'d')
        endtime = qa.time(t,form="ymd",prec=8)[0]
        endTimeShort.append(qa.time(t,prec=8)[0])
        
        # source name
        rowsrc = rownode.getElementsByTagName("sourceName")
        src = str(rowsrc[0].childNodes[0].nodeValue)
        
        scandict[fid]['start'] = starttime
        scandict[fid]['end'] = endtime
        timestr = starttime+'~'+endtime
        scandict[fid]['timerange'] = timestr
        scandict[fid]['source'] = src
        scandict[fid]['intent'] = rint
        scandict[fid]['nsubs'] = nsubs

    # read Main.xml
    xmlmain = minidom.parse(sdm+'/Main.xml')
    rowlist = xmlmain.getElementsByTagName("row")
    mainScanList = []
    mainConfigList = []
    fieldIdList = []
    for rownode in rowlist:
        
        # get the scan numbers
        rowfid = rownode.getElementsByTagName("scanNumber")
        fid = int(rowfid[0].childNodes[0].nodeValue)
        mainScanList.append(fid)
        
        # get the configuration description
        rowconfig = rownode.getElementsByTagName("configDescriptionId")
        config = str(rowconfig[0].childNodes[0].nodeValue)
        mainConfigList.append(config)
        
        # get the field ID
        rowfieldid = rownode.getElementsByTagName("fieldId")
        fieldid = string.split(str(rowfieldid[0].childNodes[0].nodeValue), '_')[1]
        fieldIdList.append(fieldid)

    # read ConfigDescription.xml to relate the configuration
    # description to a (set) of data description IDs
    xmlconfig = minidom.parse(sdm+'/ConfigDescription.xml')
    rowlist = xmlconfig.getElementsByTagName("row")
    configDescList = []
    dataDescList = []
    for rownode in rowlist:
        
        # get the configuration description
        rowConfigDesc = rownode.getElementsByTagName("configDescriptionId")
        configDesc = str(rowConfigDesc[0].childNodes[0].nodeValue)
        configDescList.append(configDesc)
        
        # make a list of the data description IDs:
        rowNumDataDesc = rownode.getElementsByTagName("numDataDescription")
        numDataDesc = int(rowNumDataDesc[0].childNodes[0].nodeValue)
        
        rowDataDesc = rownode.getElementsByTagName("dataDescriptionId")
        dataDescStr = str(rowDataDesc[0].childNodes[0].nodeValue)
        dataDescSplit = dataDescStr.split()
        dataDesc = []
        for i in range(numDataDesc):
            dataDesc.append(dataDescSplit[i+2])
        dataDescList.append(dataDesc)

    # read DataDescription.xml to relate the data description IDs to
    # spectral window IDs
    xmlDataDesc = minidom.parse(sdm+'/DataDescription.xml')
    rowlist = xmlDataDesc.getElementsByTagName("row")
    dataDescElList = []
    spwIdDataDescList = []
    for rownode in rowlist:
        
        # get the data description ID, make another list:
        rowDataDescEl = rownode.getElementsByTagName("dataDescriptionId")
        dataDescEl = str(rowDataDescEl[0].childNodes[0].nodeValue)
        dataDescElList.append(dataDescEl)
        
        # get the related spectral window ID:
        rowSpwIdDataDesc = rownode.getElementsByTagName("spectralWindowId")
        spwIdDataDesc = str(rowSpwIdDataDesc[0].childNodes[0].nodeValue)
        spwIdDataDescList.append(spwIdDataDesc)

    # read SpectralWindow.xml, get information about number of
    # channels, reference frequency, baseband name, channel width.
    # Interesting that there seem to be multiple fields that give the
    # same information: chanFreqStart=reFreq,
    # chanFreqStep=chanWidth=resolution.  Why? (Note: all units are Hz)
    # Note: this is where the script breaks for ALMA data, since there
    # are different tags in SpectraWindow.xml (for varying channel widths).
    xmlSpecWin = minidom.parse(sdm+'/SpectralWindow.xml')
    rowlist = xmlSpecWin.getElementsByTagName("row")
    spwIdList = []
    nChanList = []
    refFreqList = []
    chanWidthList = []
    basebandList = []
    for rownode in rowlist:
        
        # get the various row values:
        rowSpwId = rownode.getElementsByTagName("spectralWindowId")
        rowNChan = rownode.getElementsByTagName("numChan")
        rowRefFreq = rownode.getElementsByTagName("refFreq")
        # For EVLA
        rowChanWidth = rownode.getElementsByTagName("chanWidth")
        # For ALMA
        rowChanWidthArr = rownode.getElementsByTagName("chanWidthArray")
        rowBaseband = rownode.getElementsByTagName("basebandName")
        
        # convert to values or strings and append to the relevant lists:
        spwId = str(rowSpwId[0].childNodes[0].nodeValue)
        spwIdList.append(spwId)
        nChan = int(rowNChan[0].childNodes[0].nodeValue)
        nChanList.append(nChan)
        refFreq = float(rowRefFreq[0].childNodes[0].nodeValue)
        refFreqList.append(refFreq)
        if rowChanWidth:
            chanWidth = float(rowChanWidth[0].childNodes[0].nodeValue)
            chanWidthList.append(chanWidth)
        if rowChanWidthArr:
            tmpArr = str(rowChanWidthArr[0].childNodes[0].nodeValue).split(' ')
            tmpWidth = []
            for cw in range(2, len(tmpArr)):
                thisWidth = float(tmpArr[cw])
                tmpWidth.append(thisWidth)
            chanWidthList.append(tmpWidth)
        baseband = str(rowBaseband[0].childNodes[0].nodeValue)
        basebandList.append(baseband)

    # read Field.xml
    xmlField = minidom.parse(sdm+'/Field.xml')
    rowlist = xmlField.getElementsByTagName("row")
    fieldList = []
    fieldNameList = []
    fieldCodeList = []
    fieldRAList = []
    fieldDecList = []
    fieldSrcIDList = []
    for rownode in rowlist:
        rowField = rownode.getElementsByTagName("fieldId")
        rowName = rownode.getElementsByTagName("fieldName")
        rowCode = rownode.getElementsByTagName("code")
        rowCoords = rownode.getElementsByTagName("referenceDir")
        rowSrcId = rownode.getElementsByTagName("sourceId")
        
        # convert to values or strings and append to relevent lists:
        fieldList.append(int(string.split(str(rowField[0].childNodes[0].nodeValue),'_')[1]))
        fieldNameList.append(str(rowName[0].childNodes[0].nodeValue))
        fieldCodeList.append(str(rowCode[0].childNodes[0].nodeValue))
        coordInfo = rowCoords[0].childNodes[0].nodeValue.split()
        RADeg = float(coordInfo[3])*(180.0/np.pi)
        DecDeg = float(coordInfo[4])*(180.0/np.pi)
        RAInp = {'unit': 'deg', 'value': RADeg}
        DecInp = {'unit': 'deg', 'value': DecDeg}
        RAHMS = qa.formxxx(RAInp, format='hms')
        DecDMS = qa.formxxx(DecInp, format='dms')
        fieldRAList.append(RAHMS)
        fieldDecList.append(DecDMS)
        fieldSrcIDList.append(int(rowSrcId[0].childNodes[0].nodeValue))    

    # read Antenna.xml
    xmlAnt = minidom.parse(sdm+'/Antenna.xml')
    rowlist = xmlAnt.getElementsByTagName("row")
    antList = []
    antNameList = []
    dishDiamList = []
    stationList = []
    for rownode in rowlist:
        rowAnt = rownode.getElementsByTagName("antennaId")
        rowAntName = rownode.getElementsByTagName("name")
        rowDishDiam = rownode.getElementsByTagName("dishDiameter")
        rowStation = rownode.getElementsByTagName("stationId")
        
        # convert and append
        antList.append(int(string.split(str(rowAnt[0].childNodes[0].nodeValue), '_')[1]))
        antNameList.append(str(rowAntName[0].childNodes[0].nodeValue))
        dishDiamList.append(float(rowDishDiam[0].childNodes[0].nodeValue))
        stationList.append(str(rowStation[0].childNodes[0].nodeValue))

    # read Station.xml
    xmlStation = minidom.parse(sdm+'/Station.xml')
    rowlist = xmlStation.getElementsByTagName("row")
    statIdList = []
    statNameList = []
    statLatList = []
    statLonList = []
    for rownode in rowlist:
        rowStatId = rownode.getElementsByTagName("stationId")
        rowStatName = rownode.getElementsByTagName("name")
        rowStatPos = rownode.getElementsByTagName("position")
        
        # convert and append
        statIdList.append(str(rowStatId[0].childNodes[0].nodeValue))
        statNameList.append(str(rowStatName[0].childNodes[0].nodeValue))
        posInfo = string.split(str(rowStatPos[0].childNodes[0].nodeValue))
        x = qa.quantity([float(posInfo[2])], 'm')
        y = qa.quantity([float(posInfo[3])], 'm')
        z = qa.quantity([float(posInfo[4])], 'm')
        pos = me.position('ITRF', x, y, z)
        qLon = pos['m0']
        qLat = pos['m1']
        statLatList.append(qa.formxxx(qLat, 'dms', prec=0))
        statLonList.append(qa.formxxx(qLon, 'dms', prec=0))
    
    # associate antennas with stations:
    assocStatList = []
    for station in stationList:
        i = np.where(np.array(statIdList) == station)[0]
        assocStatList.append(statNameList[i])

    # read ExecBlock.xml
    xmlExecBlock = minidom.parse(sdm+'/ExecBlock.xml')
    rowlist = xmlExecBlock.getElementsByTagName("row")
    sTime = float(rowlist[0].getElementsByTagName("startTime")[0].childNodes[0].nodeValue)*1.0E-9
    eTime = float(rowlist[0].getElementsByTagName("endTime")[0].childNodes[0].nodeValue)*1.0E-9
    # integration time in seconds, start and end times:
    intTime = eTime - sTime
    t = qa.quantity(sTime/86400.0, 'd')
    obsStart = qa.time(t, form="ymd", prec=8)[0]
    t = qa.quantity(eTime/86400.0, 'd')
    obsEnd = qa.time(t, form="ymd", prec=8)[0]
    # observer name and obs. info:
    observerName = str(rowlist[0].getElementsByTagName("observerName")[0].childNodes[0].nodeValue)
    configName = str(rowlist[0].getElementsByTagName("configName")[0].childNodes[0].nodeValue)
    telescopeName = str(rowlist[0].getElementsByTagName("telescopeName")[0].childNodes[0].nodeValue)
    numAntenna = int(rowlist[0].getElementsByTagName("numAntenna")[0].childNodes[0].nodeValue)

    # make lists like the dataDescList for spectral windows & related info:
    spwOrd = []
    nChanOrd = []
    rFreqOrd = []
    cWidthOrd = []
    bbandOrd = []
    for i in range(0, len(configDescList)):
        spwTempList = []
        nChanTempList = []
        rFreqTempList = []
        cWidthTempList = []
        bbandTempList = []
        
        for dDesc in dataDescList[i]:
            el = np.where(np.array(dataDescElList) == dDesc)[0]
            spwIdN = spwIdDataDescList[el]
            spwEl = np.where(np.array(spwIdList) == spwIdN)[0]
            spwTempList.append(int(string.split(spwIdList[spwEl], '_')[1]))
            nChanTempList.append(nChanList[spwEl])
            rFreqTempList.append(refFreqList[spwEl])
            cWidthTempList.append(chanWidthList[spwEl])
            bbandTempList.append(basebandList[spwEl])
        spwOrd.append(spwTempList)
        nChanOrd.append(nChanTempList)
        rFreqOrd.append(rFreqTempList)
        cWidthOrd.append(cWidthTempList)
        bbandOrd.append(bbandTempList)
    
    # add this info to the scan dictionary:
    for scanNum in scandict:
        spwOrdList = []
        nChanOrdList = []
        rFreqOrdList = []
        cWidthOrdList = []
        bbandOrdList = []
        # scanEl could have multiple elements if subscans are present,
        # or for ALMA data:
        scanEl = np.where(np.array(mainScanList) == scanNum)[0]
        for thisEl in scanEl:
            configEl = mainConfigList[thisEl]
            listEl = np.where(np.array(configDescList) == configEl)[0]
            spwOrdList.append(spwOrd[listEl])
            nChanOrdList.append(nChanOrd[listEl])
            rFreqOrdList.append(rFreqOrd[listEl])
            cWidthOrdList.append(cWidthOrd[listEl])
            bbandOrdList.append(bbandOrd[listEl])
        scandict[scanNum]['field'] = int(fieldIdList[scanEl[0]])
        scandict[scanNum]['spws'] = spwOrdList
        scandict[scanNum]['nchan'] = nChanOrdList
        scandict[scanNum]['reffreq'] = rFreqOrdList
        scandict[scanNum]['chanwidth'] = cWidthOrdList
        scandict[scanNum]['baseband'] = bbandOrdList
            
    # report informatio to the logger
    casalog.origin('listsdm')
    casalog.post("================================================================================")
    casalog.post("   SDM File: %s" % sdm)
    casalog.post("================================================================================")
    casalog.post("   Observer: %s" % observerName)
    casalog.post("   Facility: %s, %s-configuration" % (telescopeName, configName))
    casalog.post("      Observed from %s to %s (UTC)" % (obsStart, obsEnd))
    casalog.post("      Total integration time = %.2f seconds (%.2f hours)" % (intTime, intTime/3600))
    casalog.post(" ")
    casalog.post("Scan listing:")
    casalog.post("  Timerange (UTC)           Scan FldID  FieldName       SpwIDs         Intent(s)")

    i = 0
    #SPWs = []
    for scan in scandict:
        SPWs = []
        for spw in scandict[scan]['spws']:
            SPWs += spw
        #printSPWs = sorted(SPWs)
        printSPWs = list(set(SPWs))
        casalog.post("  %s - %s %s %s  %s %s  %s" % (startTimeShort[i], endTimeShort[i], str(scandict.keys()[i]).rjust(4), str(scandict[scan]['field']).rjust(5), scandict[scan]['source'].ljust(15), str(printSPWs), scandict[scan]['intent']))
        i = i + 1

    casalog.post(" ")
    casalog.post("Spectral window information:")
    casalog.post("  SpwID  #Chans  Ch0(MHz)  ChWidth(kHz) TotBW(MHz)  Baseband")

    for i in range(0, len(spwIdList)):
        casalog.post("  %s   %s    %s  %s     %s    %s" % (string.split(spwIdList[i], '_')[1].ljust(4), str(nChanList[i]).ljust(4), str(refFreqList[i]/1e6).ljust(8), str(np.array(chanWidthList[i])/1e3).ljust(8), str(np.array(chanWidthList[i])*nChanList[i]/1e6).ljust(8), basebandList[i].ljust(8)))
    
    casalog.post(" ")
    casalog.post("Field information:")
    casalog.post("  FldID  Code   Name             RA            Dec             SrcID")

    for i in range(0, len(fieldList)):
        casalog.post("  %s  %s %s  %s %s %s" % (str(fieldList[i]).ljust(5), fieldCodeList[i].ljust(6), fieldNameList[i].ljust(15), fieldRAList[i].ljust(13), fieldDecList[i].ljust(15), str(fieldSrcIDList[i]).ljust(5)))    
    
    casalog.post(" ")
    casalog.post("Antennas (%i):" % len(antList))
    casalog.post("  ID    Name   Station   Diam.(m)  Lat.          Long.")

    for i in range(0, len(antList)):
        casalog.post("  %s %s %s     %s     %s  %s " % (str(antList[i]).ljust(5), antNameList[i].ljust(6), assocStatList[i].ljust(5), str(dishDiamList[i]).ljust(5), statLatList[i].ljust(12), statLonList[i].ljust(12)))
    
    # return the scan dictionary
    return scandict

