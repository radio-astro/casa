#!/usr/bin/env python
import os, sys, traceback, email, errno, mimetypes, xml.dom.minidom, StringIO, getopt, re, exceptions, operator, string
from string import *
from tokenize import *
from token import *

from xml.dom.minidom import parse, parseString

class SDMDataObjectStreamReader:

    def position(self):
        return self.f.tell()

    def nextLine(self):
        line = self.f.readline()
        if len(line) > 0 and line[len(line)-1] == "\n":
            line = line[:-1]
        self.currentLine = line
        return line

    def HeaderField2Pair(self, hf):
        name = None
        value = None
        colonIndex = string.find(hf, ":")
        if colonIndex == -1:
            return (name, value)

        if colonIndex > 0:
            name = hf[:colonIndex]

        if colonIndex < len(hf) - 1:
            value = hf[colonIndex + 1:]

        return (name, value)

    def requireHeaderField(self, hf):
        name, value = self.HeaderField2Pair(self.nextLine())
        #print name, value
        if string.upper(name) != hf:
            print 'Missing "', hf, '" field.'
            sys.exit(1)
        return name, value

    def unquote(self, s):
        if len(s) < 2:
            return s
        if s[0] == '"' and s[len(s)-1] == '"':
            return s[1:-1]
        if s[0] == "'" and s[len(s)-1] == "'":
            return s[1:-1]
        return s

    def requireBoundaryInCT(self, ctValue):
        cvValueItems = [item.lstrip().rstrip() for item in ctValue.split(";")] 
        cvValueItemsNameValue = [item.partition("=") for item in cvValueItems]
        boundaryValues = [item[2] for item in cvValueItemsNameValue if string.upper(item[0]) == 'BOUNDARY' and item[2] != ''] 
        if boundaryValues == []:
            return None
        else:
            return self.unquote(boundaryValues[0])

    def skipUntilEmptyLine(self, maxSkips):
        numSkip = 0
        line = self.nextLine()
        while line != '' and numSkip <= maxSkips :
            line = self.nextLine()
            numSkip+=1
        if numSkip > maxSkips:
            print "could not find an expected empty line"
            sys.exit(1)

    def requireMIMEHeader(self):
        # MIME-Version
        line = self.nextLine()
        name, value = self.HeaderField2Pair(line)
        #print name, value
        #if line != "MIME-Version: 1.0":
        if not line.endswith("IME-Version: 1.0"):
            print 'Missing "MIME-Version: 1.0"'
            Sys.exit(1)

        # Content-Type
        name, value = self.requireHeaderField("CONTENT-TYPE")
        # boundary in value
        boundary_1 = self.requireBoundaryInCT(value)
        self.boundary_1 = boundary_1
        #print boundary_1

        # Content-Description
        #print self.requireHeaderField("CONTENT-DESCRIPTION")

        # Content-Location
        #print self.requireHeaderField("CONTENT-LOCATION")

        # Look for an empty line during 10 lines maximum.
        self.skipUntilEmptyLine(10)

        return boundary_1

    def accumulateUntilBoundary(self, boundary, maxLines):
        numLines = 0
        line = self.nextLine()
        result = ""
        while  numLines <= maxLines and string.find(line, "--"+boundary) != 0:
            result += line
            numLines += 1
            line = self.nextLine()

        if numLines > maxLines:
            print "Could not find '--" + boundary + "' in the next " + maxLines + "."
            sys.exit(1)

        return result

    def requireBoundary(self, boundary, maxLines):
        numLines = 0
        line = self.nextLine()
        while numLines <= maxLines and line != "--"+boundary:
            numLines += 1
            line = self.nextLine()

        if numLines > maxLines:
            print "Could not read the expected boundary '"+boundary+"'"
            sys.exit(1)

    def lookForBinaryPartSize(self, dom, partName):
        result = None
        elements = dom.getElementsByTagName(partName)
        if elements != []:
            size = elements.item(0).getAttribute("size")
            if size == "":
                print "Missing 'size' attribute in element '", element.nodeName, "."
                sys.exit(1)
            result = int(size)
        return result

    def requireCrossDataType(self, dom):
        result = None
        elements = dom.getElementsByTagName("crossData")
        if elements == []:
            print "Missing  'crossData' element in '" , dom.toprettyxml() , "'"
            sys.exit(1)

        result = elements.item(0).getAttribute("type")
        if result == "":
            print "Missing 'type' attribute in element '", element.nodeName, "."
            sys.exit(1)

        return result

    def requireSDMDataHeaderMIMEPart(self):
        #
        # Require the presence of boundary
        self.requireBoundary(self.boundary_1, 0)

        #
        # Ignore header fields
        #
        #requireHeaderField(f, "CONTENT-TYPE")
        #requireHeaderField(f, "CONTENT-TRANSFER-ENCODING")
        #requireHeaderField(f, "CONTENT-LOCATION")

        self.skipUntilEmptyLine(10)
        sdmDataHeader = self.accumulateUntilBoundary(self.boundary_1, 100)
        dom = parseString(sdmDataHeader)
        self.binaryPartSize = {}

        for partName in ["actualDurations", "actualTimes", "autoData", "crossData", "zeroLags", "flags"]:
            size = self.lookForBinaryPartSize(dom, partName)
            if size != None:
                self.binaryPartSize[partName] = size

        #print dom.toprettyxml()
        #print self.binaryPartSize
        self.sdmDataHeaderDOM = dom

    def requireSDMDataSubsetMIMEPart(self):
        name, value = self.requireHeaderField("CONTENT-TYPE")
        self.boundary_2  = self.requireBoundaryInCT(value)
        
        self.requireHeaderField("CONTENT-DESCRIPTION")
        self.requireBoundary(self.boundary_2, 10)
        self.skipUntilEmptyLine(10)

        sdmDataSubsetHeader = self.accumulateUntilBoundary(self.boundary_2, 100)
        sdmDataSubsetHeaderDOM = parseString(sdmDataSubsetHeader)

        done = False
        crossDataDesc = {}
        autoDataDesc = {}
        autoDataDesc["type"] = "FLOAT32_TYPE"
        while not done :
            self.requireHeaderField("CONTENT-TYPE")
            name, value = self.requireHeaderField("CONTENT-LOCATION")
            result = re.match("([0-9]+/)+(actualDurations|actualTimes|autoData|crossData|zeroLags|flags)\.bin", value.lstrip().rstrip())
            if result == None:
                print "Could not identify the part name in '" , value , "'"
                sys.exit(1)

            binaryPartName = result.group(2)

            if not self.binaryPartSize.has_key(binaryPartName):
                print "The size of '" + binaryPartName + "' has not been announced in the  data header"
                sys.exit(1)

            if binaryPartName == "crossData":
                crossDataType = self.requireCrossDataType(sdmDataSubsetHeaderDOM)
                crossDataDesc["type"] = str(crossDataType)
                            
            self.skipUntilEmptyLine(10)
            if binaryPartName == "actualDurations" or binaryPartName == "actualTimes":
                numberOfBytesPerValue = 8 
            elif binaryPartName == "autoData" :
                numberOfBytesPerValue = 4
            elif binaryPartName == "crossData" :
                if crossDataType == "INT16_TYPE":
                    numberOfBytesPerValue = 2
                elif crossDataType == "INT32_TYPE" or crossDataType == "FLOAT32_TYPE" :
                    numberOfBytesPerValue = 4
            elif binaryPartName == "flags":
                numberOfBytesPerValue = 4
            elif binaryPartName == "zeroLags":
                numberOfBytesPerValue = 4

            numberOfBytesToRead = numberOfBytesPerValue * self.binaryPartSize[binaryPartName]
            if binaryPartName == "crossData":
                crossDataDesc["base"] = self.position()
            elif binaryPartName == "autoData":
                autoDataDesc["base"] = self.position() 
            bytes = self.f.read(numberOfBytesToRead) 
            if len(bytes) < numberOfBytesToRead:
                print "EOF reached while reading a binary attachment ('" + binaryPartName + "')"
                sys.exit(1)

            #print "I have read " , self.binaryPartSize[binaryPartName] , " values for " , binaryPartName

            line = self.nextLine() # Absorb the nl right after the last byte of the binary attachment
            line = self.nextLine() # This should boundary_2

            if string.find(line, "--"+self.boundary_2) != 0:
                print "Unexpected '", line, "' after the binary part '", binaryPartName, "'" 
                sys.exit(1)

            done = line == "--"+self.boundary_2+"--"
        return (crossDataDesc, autoDataDesc)

    def open(self, f):
        self.f = f
        self.boundary_1 = self.requireMIMEHeader()
        self.requireSDMDataHeaderMIMEPart()

    def getSDMDataHeader(self):
        return self.sdmDataHeaderDOM

    def hasData(self):
        return self.currentLine != "--"+self.boundary_1+"--"

    def getData(self):
        cdd, add = self.requireSDMDataSubsetMIMEPart()
        line = self.nextLine()
        return (cdd, add)

    def processBDF(self, f):
        print "in processBDF"
        boundary_1 = self.requireMIMEHeader()

        self.requireSDMDataHeaderMIMEPart(boundary_1)


MSMainRowNum = 0

def traverseALMARadiometricData(numAntenna, dataDesc, basebands):
    global MSMainRowNum

    numberOfBytesPerValue = 0
    if dataDesc["type"] == "FLOAT32_TYPE" :
        numberOfBytesPerValue = 4
    else:
        print "Unknown data type '%s' for radiometric data" % dataDesc["type"]
        exit

    # Number of spectral windows
    numberOfSpectralWindows = len(basebands)
    
    # Number of frequency channels supposed to be constant all over the basebands.
    numChan =  basebands[0]["numSpectralPoint"]

    # Number of polarizations supposed to be constant all over the basebands.
    numPol = len(basebands[0]["sdPolProducts"].split())

    # Initialize offset to the beginning of the attachment.
    offset = dataDesc["base"]

    for time in range(int(dataDesc["numTime"])):
        osio = StringIO.StringIO()

        # General information.
        osio.write("%d" % MSMainRowNum)                                    # MS Main row number of the 1st row of this time slot
        osio.write("|%s" % dataDesc["bdfName"])                       # The name of the BDF (to be replaced with a integer index)

        #### The BAL first (actually no baselines here)
        ###osio.write("|%d" % 0)                                              # 0 baselines.
        ###osio.write("|%d" % numberOfSpectralWindows)                        # The number of spectral windows.
        ###osio.write("|%d" % numChan)                                        # The number of frequency channels.
        ###osio.write("|%d" % numPol)                                         # The number of polarizations.

        # Then the ANT.
        osio.write("|%d" % numAntenna)                                     # The number of antennas.
        osio.write("|%d" % numberOfSpectralWindows)                        # The number of spectral windows.
        osio.write("|%d" % numChan)                                        # The number of frequency channels.
        osio.write("|%d" % numPol)                                         # The number of polarizations.

        # Distances between axes.
        stepBL = numberOfSpectralWindows * numChan * numPol                # The distance between two consecutive baselines
        stepSpw = numPol * numChan                                         # ????
        osio.write("|%d" % stepBL)                                         # The crossdata step per baseline.
        osio.write("|%d" % stepSpw)                                        # The crossdata step per spectral window.
        ###osio.write("|%d" % stepBL)                                         # The autodata step per baseline.
        ###osio.write("|%d" % stepSpw)                                        # The autodata step per spectral window.

        # Scale factors.
        scaleFactors = [baseband["scaleFactor"] for baseband in basebands]
        osio.write("|"); osio.write(scaleFactors)
        
        # Offset of cross data from the beginning of the file (has no real sense here !)
        ##osio.write("|%d" % -1)
    
        # Offset of the auto data from the beginning of the file.
        osio.write("|%d" % offset)

        # Type of data (10 = auto data)
        osio.write("|%d" % 10)

        # Prepare the offset from the beginning of the file for the next time slot.
        offset = offset + numberOfSpectralWindows * numChan * numPol * numberOfBytesPerValue

        # Prepare the MS Main row number for the next time slot.
        MSMainRowNum = MSMainRowNum + numAntenna * numberOfSpectralWindows

        print osio.getvalue()
        osio.close()
        
def traverseALMAAutoData(numAntenna, crossDataDesc, autoDataDesc, basebands):
    global MSMainRowNum

    if numAntenna==0:
        return
 
    # Number of spectral windows.
    numberOfSpectralWindows = len(basebands)
    
    # Number of baselines.
    numberOfBaselines = numAntenna * (numAntenna - 1) / 2
 
    # Number of frequency channels supposed to be constant all over the basebands.
    numChan =  basebands[0]["numSpectralPoint"]

    # Number of polarizations supposed to be constant all over the basebands.
    numPol = len(basebands[0]["crossPolProducts"].split())


    osio = StringIO.StringIO()

    # General information.
    osio.write("%d" % MSMainRowNum)                                    # MS Main row number of the 1st row of this time slot
    osio.write("|%s" % crossDataDesc["bdfName"])                       # The name of the BDF (to be replaced with a integer index).

    # Then the ANT.
    osio.write("|%d" % numAntenna)                                     # The number of antennas.
    osio.write("|%d" % numberOfSpectralWindows)                        # The number of spectral windows.
    osio.write("|%d" % numChan)                                        # The number of frequency channels.
    osio.write("|%d" % numPol)                                         # The number of polarizations.

    # Distances between axes.
    stepBL = numberOfSpectralWindows * numChan * numPol                # The distance between two consecutive baselines
    stepSpw = numPol * numChan                                         # ????
    osio.write("|%d" % stepBL)                                         # The autodata step per baseline.
    osio.write("|%d" % stepSpw)                                        # The autodata step per spectral window.

    # Scale factors.
    scaleFactors = [ 1.0 ]
    osio.write("|"); osio.write(scaleFactors)

    # Offset of the auto data from the beginning of the file.
    osio.write("|%d" % autoDataDesc["base"])

    # Type of auto data.
    dataType = 10
    osio.write("|%d" % dataType)

    # Output the result
    print osio.getvalue()
    osio.close()

    # Increment the global value MSMainRowNum
    MSMainRowNum = MSMainRowNum + numAntenna * numberOfSpectralWindows

def traverseALMACrossData(numAntenna, crossDataDesc, autoDataDesc, basebands):
    global MSMainRowNum

    if numAntenna==0:
        return
 
    # Number of spectral windows.
    numberOfSpectralWindows = len(basebands)
    
    # Number of baselines.
    numberOfBaselines = numAntenna * (numAntenna - 1) / 2
 
    # Number of frequency channels supposed to be constant all over the basebands.
    numChan =  basebands[0]["numSpectralPoint"]

    # Number of polarizations supposed to be constant all over the basebands.
    numPol = len(basebands[0]["crossPolProducts"].split())

    osio = StringIO.StringIO()

    # now the cross data
    # General information.
    osio.write("%d" % MSMainRowNum)                                    # MS Main row number of the 1st row of this time slot
    osio.write("|%s" % crossDataDesc["bdfName"])                       # The name of the BDF (to be replaced with a integer index).

    # The BAL first
    osio.write("|%d" % numberOfBaselines)                              # The number of baselines.
    osio.write("|%d" % numberOfSpectralWindows)                        # The number of spectral windows.
    osio.write("|%d" % numChan)                                        # The number of frequency channels.
    osio.write("|%d" % numPol)                                         # The number of polarizations.

    # Distances between axes.
    stepBL = numberOfSpectralWindows * numChan * numPol                # The distance between two consecutive baselines
    stepSpw = numPol * numChan                                         # ????
    osio.write("|%d" % stepBL)                                         # The crossdata step per baseline.
    osio.write("|%d" % stepSpw)                                        # The crossdata step per spectral window.

    # Scale factors.
    scaleFactors = [baseband["scaleFactor"] for baseband in basebands]
    osio.write("|"); osio.write(scaleFactors)

    # Offset of cross data from the beginning of the file.
    osio.write("|%d" % crossDataDesc["base"])
    
    # Type of cross data.
    crossDataType = crossDataDesc["type"]
    if crossDataType == "INT16_TYPE":
        dataType = 0
    elif crossDataType == "INT32_TYPE":
        dataType = 1
    elif crossDataType == "INT64_TYPE":
        dataType = 2
    elif crossDataType == "FLOAT32_TYPE":
        dataType = 3
    else :
        print "Unrecognized cross data type '%s'" % crossDataType
    osio.write("|%d" % dataType)

    # Output the result
    print osio.getvalue()
    osio.close()

    # Increment the global value MSMainRowNum
    MSMainRowNum = MSMainRowNum + numberOfBaselines * numberOfSpectralWindows

def main() :
    #
    # The path to the ASDM is expected to be argv[1]
    #
    try:
        options, arg = getopt.getopt(sys.argv[1:], ":h", ["help"])
    except getopt.GetoptError, err:
            usage();
            sys.exit(2)

    #
    # Build the ordered list of the BDF files from the content of the ASDM Main table
    #
    asdmMainFile = arg[0] + "/Main.xml"
    
    asdmMainDOM = xml.dom.minidom.parse(asdmMainFile)
    bdfNames = [ string.replace(string.replace(entityRef.getAttribute("entityId"), "/", "_", ), ":", "_" ) for entityRef in asdmMainDOM.getElementsByTagName("EntityRef") ]

    crossDataAxesRE = re.compile("BAL( +BAB)?( +APC)?( +SPP)?( +POL)?") 
    autoDataAxesRE = re.compile("ANT( +BAB)?( +APC)?( +SPP)?( +POL)?") 
    #
    # now process sequentially the bdf files
    #
    for bdfName in bdfNames :
        for product in ['auto','cross']:
            #print "> Processing %s" %  bdfName
            try :
                f  = open(arg[0]+"/ASDMBinary/"+bdfName)
            except IOError, e:
                print e;
                continue

            sdosr = SDMDataObjectStreamReader()
            sdosr.open(f)
            globalHeaderDOM = sdosr.getSDMDataHeader()
            #
            # What's the processor type. 
            #
            processorType = (globalHeaderDOM.getElementsByTagName("processorType"))[0].childNodes[0].nodeValue
            #print processorType

            #
            # What's the correlation mode
            #
            correlationMode = (globalHeaderDOM.getElementsByTagName("correlationMode"))[0].childNodes[0].nodeValue
            #print correlationMode

            #
            # The process depends on the processor type.
            #
            if processorType == "CORRELATOR":
                crossDataDesc = {}
                crossDataDesc["bdfName"] = bdfName
                crossDataDesc["crossOrAuto"] = "cross"
                crossDataDesc["correlationMode"] = correlationMode

                elems = globalHeaderDOM.getElementsByTagName("crossData")
                if len(elems) > 0:
                    crossDataDesc["size"] = int(elems[0].getAttribute("size"))
                    crossDataDesc["axes"] = str(elems[0].getAttribute("axes"))

                if crossDataAxesRE.match(crossDataDesc["axes"]) == None:
                    print "These axes '%s' are not recognized for cross data." % crossDataDesc["axes"] 
                    continue

                autoDataDesc = {}
                autoDataDesc["bdfName"] = bdfName
                autoDataDesc["crossOrAuto"] = "auto"
                autoDataDesc["correlationMode"] = correlationMode

                elems = globalHeaderDOM.getElementsByTagName("autoData")
                if len(elems) > 0:
                    autoDataDesc["size"] = int(elems[0].getAttribute("size"))
                    autoDataDesc["axes"] = str(elems[0].getAttribute("axes"))
                    normalized = str(elems[0].getAttribute("normalized"))
                    if normalized == None:
                        autoDataDesc["normalized"] = False
                    else:
                        autoDataDesc["normalized"] = normalized == "true"

                if autoDataAxesRE.match(autoDataDesc["axes"]) == None:
                    print "These axes '%s' are not recognized for auto data." % autoDataDesc["axes"] 
                    continue

                numAntenna = int(globalHeaderDOM.getElementsByTagName("numAntenna")[0].childNodes[0].nodeValue)

                #
                # listify the baseband elements
                #
                basebands          = []
                for spw in globalHeaderDOM.getElementsByTagName("spectralWindow"):
                    attributes = {"sdPolProducts": str(spw.getAttribute("sdPolProducts")),
                                  "crossPolProducts": str(spw.getAttribute("crossPolProducts")),
                                  "scaleFactor": float(spw.getAttribute("scaleFactor")),
                                  "numSpectralPoint": int(spw.getAttribute("numSpectralPoint")),
                                  "numBin" : int(spw.getAttribute("numBin"))}
                    basebands.append(attributes)

                #
                # Then let's traverse the BDF.
                #
                # first auto
                while sdosr.hasData() :
                    cdd, add = sdosr.getData()
                    for k, v in cdd.iteritems(): 
                        crossDataDesc[k]=v
                    #print crossDataDesc

                    for k, v in add.iteritems():
                        autoDataDesc[k]=v
                    #print autoDataDesc
                    #print ">>> file %s, next subset, cross data starts at %d and auto data starts at %d " % (crossDataDesc["bdfName"], crossDataDesc["base"], autoDataDesc["base"])
                    if(product=='auto'):
                        traverseALMAAutoData(numAntenna, crossDataDesc, autoDataDesc, basebands)
                    else:
                        traverseALMACrossData(numAntenna, crossDataDesc, autoDataDesc, basebands)

            elif (processorType == "RADIOMETER" and product == 'auto'):
                radiometricDataDesc = {}
                radiometricDataDesc["bdfName"] = bdfName
                radiometricDataDesc["crossOrAuto"] = "auto"
                radiometricDataDesc["correlationMode"] = correlationMode
                radiometricDataDesc["numTime"] = (globalHeaderDOM.getElementsByTagName("numTime"))[0].childNodes[0].nodeValue

                elems = globalHeaderDOM.getElementsByTagName("autoData")
                if len(elems) > 0:
                    radiometricDataDesc["size"] = int(elems[0].getAttribute("size"))
                    radiometricDataDesc["axes"] = str(elems[0].getAttribute("axes"))
                    normalized = str(elems[0].getAttribute("normalized"))
                    if normalized == None:
                        radiometricDataDesc["normalized"] = False
                    else:
                        radiometricDataDesc["normalized"] = normalized == "true"        

                #
                # listify the baseband elements
                #
                basebands          = []
                for spw in globalHeaderDOM.getElementsByTagName("spectralWindow"):
                    attributes = {"sdPolProducts": str(spw.getAttribute("sdPolProducts")),
                                  "crossPolProducts": str(spw.getAttribute("crossPolProducts")),
                                  "scaleFactor": 1.0,
                                  "numSpectralPoint": int(spw.getAttribute("numSpectralPoint")),
                                  "numBin" : int(spw.getAttribute("numBin"))}
                    basebands.append(attributes)        

                cdd, add = sdosr.getData()
                for k, v in add.iteritems():
                    radiometricDataDesc[k]=v

                traverseALMARadiometricData(numAntenna, radiometricDataDesc, basebands) 

#
# the main entry point
#
if __name__ == "__main__":
    main()
