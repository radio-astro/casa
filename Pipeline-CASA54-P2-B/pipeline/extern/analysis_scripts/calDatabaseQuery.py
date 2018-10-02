#!/usr/bin/env python
#calDatabaseQuery.py
#written by dBarkats with addition from TvK, and rKneissl 
#
#This is a class with a list of functions available to deal with
#entering and retrieving  measurements from the calibrator catalog database.
#
#
#$Id: calDatabaseQuery.py,v 1.132 2017/12/16 17:15:30 thunter Exp $


import datetime
from xmlrpclib import ServerProxy
from pylab import *
import getpass
import time
import sys, math
from . import tmUtils as tm
from . import analysisUtils as aU
import socket
import re # for matchNames
import os 
import fileinput
from types import NoneType
import distutils.spawn

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.ENDC = ''


NULL_AS_FLOAT = 1.7976931348623157e+308 
NULL_AS_FLOAT_STRING = '1.7976931348623157E308' 
NULL_AS_STRING = 'null'

def defineGridNames():
    gridNames = ['J0006-063','J0237+288','J0238+166','3c84','J0334-401',
                 'J0423-013','J0510+180','J0519-454','J0522-364',
                 'J0538-440','J0635-7516','J0750+125','J0854+201',
                 'J1037-295','J1058+015','J1107-448','J1146+399',
                 '3c273','3c279','J1337-129','J1427-421',
                 'J1517-243','J1550+054','J1613-586','3c345',
                 'J1733-130','J1751+096','J1924-292','J2025+337',
                 'J2056-472','J2148+069','J2232+117','3c454.3',
                 'J2258-279','J2357-5311']
    # old obsolete ones
    #gridNames = ['J0106-405','J0132-169','J0237+288','J0238+166','3c84',
    #             'J0334-401','J0423-013','J0510+180','J0519-454','J0522-364',
    #             'J0538-440','J0635-7516','J0750+125','J0854+201',
    #             'J0927+390', 'J1037-295','J1058+015','J1107-448','J1130-148','J1146+399',
    #             'J1147-6753','J1159+292','3c273','3c279','J1337-129','J1426+364',
    #             'J1427-421','J1517-243','J1550+054','J1613-586','3c345','J1733-130',
    #             'J1751+096','J1800+388','J1924-292','J2025+337',
    #             'J2056-472','J2148+069','J2157-694','J2202+422','J2232+117','3c454.3',
    #             'J2258-279','J2357-5311']

    return gridNames

def convertNoneToFloat(value):
    if value == None:
        return NULL_AS_FLOAT
    return float(value)

def convertNoneToString(value):
    if value == None:
        return NULL_AS_STRING
    return value

def convertPseudoNullToNone(value):
    if (value == NULL_AS_STRING)  or (value == '') or (value == NULL_AS_FLOAT) or (value == NULL_AS_FLOAT_STRING) \
           or (value == '0') or (value =='0.0') or (value == 0.0):
        return None
    return value

def checkForPseudoNullsInMeasurements(measurements): 
    for m in measurements:
        m['ra_uncertainty'] = convertPseudoNullToNone(m['ra_uncertainty'])
        m['dec_uncertainty'] = convertPseudoNullToNone(m['dec_uncertainty'])
        m['flux_uncertainty'] = convertPseudoNullToNone(m['flux_uncertainty'])
        m['degree'] = convertPseudoNullToNone(m['degree'])
        m['degree_uncertainty'] = convertPseudoNullToNone(m['degree_uncertainty'])
        m['angle'] = convertPseudoNullToNone(m['angle'])
        m['angle_uncertainty'] = convertPseudoNullToNone(m['angle_uncertainty'])
        m['origin'] = convertPseudoNullToNone(m['origin'])
        m['fluxratio'] = convertPseudoNullToNone(m['fluxratio'])
        m['uvmin'] = convertPseudoNullToNone(m['uvmin'])
        m['uvmax'] = convertPseudoNullToNone(m['uvmax'])
    return measurements

class CalibratorCatalogUpdate:
    def __init__(self, name=None,tunnel=False,server='http://sourcecat.osf.alma.cl/sourcecat/xmlrpc'):

        self.username = getpass.getuser()
        if (tunnel):
            self.s = ServerProxy('http://localhost:8080/sourcecat/xmlrpc')
        else:
            self.s = ServerProxy(server)
            #self.s = ServerProxy('http://terpsichore.sco.alma.cl:8080/sourcecat/xmlrpc')  
            
        self.connectionFailed = False  # added by T. Hunter
        try:
            self.catalogList=self.s.sourcecat.listCatalogues()
        except:
            print "Connection failed."
            if (tunnel):
                print "Before calling this function with tunnel=True, be sure to set up an ssh tunnel like so:"
                print "    ssh -N -f -L 8080:pomona.osf.alma.cl:8080 %s@tatio.aiv.alma.cl" % os.getenv("USER")
                print "(or ssh -N -f -L 8080:pomona.osf.alma.cl:8080 %s@login.alma.cl)" % os.getenv("USER")
                print "Alternatively, to access the webpage manually, open a SOCKS v5 proxy tunnel by:"
                print "(1) Use FoxyProxy to set pattern http://pomona.osf.alma.cl:8080/* to localhost port 8080"
                print "(2) ssh -D 8080 tatio.aiv.alma.cl"
                print "(3) surf to http://pomona.osf.alma.cl:8080/sourcecatweb/sourcecat"
            self.connectionFailed = True  # added by T. Hunter
            return

        self.catalogues=[]
        self.basepath = '/mnt/jaosco/data/Calsurvey'
        # for testing
        #self.basepath = '/users/dbarkats/cal_survey_tmp'

        for i in range(size(self.catalogList)):
            self.catalogues.append(self.catalogList[i]['catalogue_id'])
            
        self.typesList=self.s.sourcecat.listTypes()
        self.types=[]

        for i in range(size(self.typesList)):
            self.types.append(self.typesList[i]['type_id'])
        if (tunnel==False):
            self.hostname= socket.gethostname()
            if ('casa' not in self.hostname and 'scops' not in self.hostname and 'alma.cl' in self.hostname):
                print "### WARNING: This script is only intended to be run from scops0X/casa0X.sco.alma.cl. ### \n### Unless you are doing specific testing, only run this from scops0X/casa0X.### \n"
        #    sys.exit()
            
    def matchName(self, sourceName, Id = None, verbose = 1):
        """
        Name matching function to check that the name provided (sourceName)
        is the same as the one in the catalog (realName)
        Given a sourceName,  it returns the realName in the catalog. As long as the realName
        and the sourceName are the same (within the starting J), it does not complain.
        Also, returns the official name of this source: JXXXX-XXXX
        
        """
        realName = None
        sourceId = None
        officialName = None
        nameList = []
        
        # remove any star at the end of the sourceName
        sourceName = sourceName.strip('*')

        if Id == None:
            # get source Id from source Name
            sourceId = self.getSourceIdFromSourceName('%%%s%%'%sourceName)
        else:
            sourceId = Id
        if sourceId != None:
            Names = self.getSourceNameFromId(sourceId)
            for name in Names:
                rp = name['source_name']
                nameList.append(rp)
                if sourceName.lower() in rp.lower():
                    realName = rp

            # find official name
            #print nameList
            for name in nameList:
                
                ab = re.match("J[0-9]{4}.[0-9]{4}",name)
                #print name, ab
                if ab:
                    officialName = ab.group()
                    break
                else:
                    officialName = None
                    
            if (verbose):
                print "Name given: %s, SourceID:%d, Matched name: %s, Official Name = %s"%(sourceName, sourceId, realName, officialName)
            # if realName.startswith('J') and realName.replace('J','') == sourceName:
            return sourceId, realName, officialName
            #elif sourceName != realName:
                 #usename= raw_input("Name does not match catalog name, Use matched name (y) or exit any other key)?")
            #    print "WARNING: realName:%s in catalog does NOT match sourceName given: %s " %(realName, sourceName)
            #    return sourceId, realName
            #else:
            #    return sourceId, realName
        else:
            print "WARNING: No SourceID found for this sourceName :%s" %sourceName
            return sourceId, realName, officialName


    def addMeasurementFromALMA(self,filename, dryrun = True):
        """
        Format  of data is:
        sourceName , ra, ra_uncertainty, dec, dec_uncertainty, frequency,  flux, flux_uncertainty,
        degree, degree_uncertainty,  angle, angle_uncertainty, uvmin, uvmax, date_observed.

        Beware: if you stop in the middle of inserting a file, there is for the moment no mechanism to prevent
        you from re-entering existing measurements twice.

        """
        count = 0
        sourceNameList =[]
        sourceFluxList =[]

        if dryrun == True:
            print " ---------########--------########--------########--------########"
            print "THIS IS A DRY RUN. NOTHING WILL BE WRITTEN TO THE CALIBRATOR DATABASE"
            print " ---------########--------########--------########--------########"

        f = open(filename,'r')
        for line in f:
            
            if line.startswith('#'):
                continue
            print " \n--------------------------------------------------------------------"
            sline = line.split(',')
            sourceName = sline[0].strip()

            print line
            #check validity of sourceName and get SourceId
            sourceId,realName,officialName = self.matchName(sourceName)

            # If source doesn't exist add it to ALMA catalog
            if sourceId == None:
                createSource = raw_input("Source name %s does not exist yet. Please add it manually along with its first measurement. This is done by Sebastian Gonzalez for now (rkneissl@alma.cl/sagonzal@alma.cl)" %sourceName)
                sys.exit()

            sourceNameList.append(realName)
          
            # Get latest measurement which ALSO belong to SMA, ALMA, or ATCA, or CRATES or VLA
            measurements = self.wrapSearch(name = realName, limit = 1, sourceBandLimit = 1,sortBy = 'date_observed', asc=False,catalogues = [1,5,21,3,2,41])
            # get the latest measurement only
            if measurements != []:
                m = measurements[0]
            else:
                print "Sorry, you did not find a latest measurement for this source in the SMA,ALMA,ATCA, or VLA catalogs. Check the list of measurements for this source in the online catalog before proceeding or contact dbarkats@alma.cl if in doubt."
                sys.exit()
            
            # Put ra, dec, and their uncertainty from latest measurement into this new measurement
            self.parseMeasurement(m)
            if sline[1].strip() == 'NE': 
                ra = self.ra_decimal
            if sline[2].strip() == 'NE': 
                ra_uncertainty = convertNoneToFloat(self.ra_uncertainty)
            if sline[3].strip() == 'NE': 
                dec = self.dec_decimal
            if sline[4].strip() == 'NE': 
                dec_uncertainty = convertNoneToFloat(self.dec_uncertainty)

            # Put frequency, flux, and flux uncertainty from this new measurement
            frequency = float(sline[5].strip())
            flux  = float(sline[6].strip())
            if sline[7].strip() == 'NE':
                flux_uncertainty = NULL_AS_FLOAT
            else:
                flux_uncertainty = convertNoneToFloat(sline[7].strip())

            # For now (Aug 2011), we are not ready to put in polarization information
            #if sline[8].strip() == 'NE':
            #     degree = NULL_AS_FLOAT
            #else:
            #    # degree  = float(sline[8].strip())
            #    degree = NULL_AS_FLOAT
            #if sline[9].strip() == 'NE':
            #    degree_uncertainty = NULL_AS_FLOAT
            #else:
            #    # degree_uncertainy  = float(sline[9].strip())
            #    degree_uncertainty = NULL_AS_FLOAT
            #if sline[10].strip() == 'NE':
            #    angle = NULL_AS_FLOAT
            #else:
            #    # angle = float(sline[10].strip())
            #    angle = NULL_AS_FLOAT
            #if sline[11].strip() == 'NE':
            #    angle_uncertainty = NULL_AS_FLOAT
            #else:
            #    # angle_uncertainty  = float(sline[11].strip())
            #    angle_uncertainty = NULL_AS_FLOAT 

            degree = NULL_AS_FLOAT
            degree_uncertainty = NULL_AS_FLOAT
            angle = NULL_AS_FLOAT
            angle_uncertainty = NULL_AS_FLOAT

            #### deal with uvmin cases
            if self.uvmin is None:  # previous measurement was None
                if (sline[12].strip() == 'NE') or (sline[12].strip() == '0.0'):
                    uvmin = NULL_AS_FLOAT
                else:
                    inp = raw_input("### WARNING:  You are about to ingest a new NON-ZERO uvmin for this source. This means you have detected a different amount of large scale structure. Is this correct ? if the answer is y(es), continue and ingest it, if no, keep previous Null uvmin ###")
                    if inp.lower()[0] == 'y':
                        uvmin = float(sline[12].strip())
                        print 'Setting uvmin of sourceID:%d, sourceName :%s to %f'%(sourceId,realName, uvmin)
                    else:
                        uvmin = NULL_AS_FLOAT
                        print 'Setting uvmin of sourceID:%d, sourceName :%s to None'%(sourceId,realName)

            else: # previous measurement was non-zero value
                if abs(self.uvmin - float(sline[12].strip())/(self.uvmin)) < 0.25 :
                    uvmin = self.uvmin
                    print 'Setting uvmin to previous value for sourceID:%d, sourceName :%s, uvmin =  %f'%(sourceId,realName, uvmin)
                else:
                    inp = raw_input("##### WARNING:  You are about to ingest a MODIFIED uvmin (change greater than 25\%) for this source. This means you have detected a different amount of large scale structure. Are you sure this is correct ?\" if the answer is y(es), continue and ingest it, if no, keep previous uvmin  ####")
                    if inp.lower()[0] == 'y':
                        uvmin = float(sline[12].strip())
                        print 'Setting uvmin of sourceID:%d, sourceName :%s to %f'%(sourceId,realName, uvmin)
                    else:
                        uvmin = self.uvmin
                        print 'Setting uvmin of sourceID:%d, sourceName :%s to None'%(sourceId,realName)

            #### deal with uvmax cases
            if self.uvmax is None:  # previous measurement was None
                if (sline[13].strip() == 'NE'):
                    uvmax= NULL_AS_FLOAT
                else:
                    uvmax= float(sline[13].strip())

            elif self.uvmax < 0 and  float(sline[13].strip()) < 0:   # previous measurement is negative (not resolved)
                if self.uvmax < float(sline[13].strip()) : # new value larger
                    uvmax = self.uvmax
                else:
                    uvmax =  float(sline[13].strip())
                    
            elif self.uvmax < 0 and float(sline[13].strip()) > 0:   # prev meas negative (not resolved), new meas pos (resolved)
                inp = raw_input("##### WARNING: You are about to ingest a new POSITIVE uvmax for this source, when it used to be negative.. This means you have detected some resolution to this source.. Are you sure this is correct. Review results carefully ?\" if the answer is y(es), continue and ingest it, if no, keep previous uvmax ####")
                if inp.lower()[0] == 'y':
                    uvmax= float(sline[13].strip())
                    print 'Setting uvmax of sourceID:%d, sourceName :%s to %f'%(sourceId,realName, uvmax)
                else:
                    uvmax= self.uvmax
                    print 'Setting uvmax of sourceID:%d, sourceName :%s to %f'%(sourceId,realName, uvmax)       

            elif self.uvmax > 0 and float(sline[13].strip()) < 0:  # prev meas positive (resolved), new meas neg (not resolved)
                uvmax = self.uvmax

                
            elif self.uvmax > 0 and float(sline[13].strip()) > 0:  #previous measurement and new meas positive
                if abs(self.uvmax- float(sline[13].strip())/(self.uvmax)) < 0.25:
                    uvmax = self.uvmax
                    print 'Setting uvmax to previous value for sourceID:%d, sourceName :%d, uvmax =  %f'%(sourceId,realName, uvmax)
                else:
                    inp = raw_input("##### WARNING:  You are about to ingest a MODIFIED uvmax (change greater than 25\%) for this source. This means you have detected a different uvscale at which the source is resolved. Are you sure this is correct ?\" if the answer is y(es), continue and ingest it, if no, keep previous uvmax  ####")
                    if inp.lower()[0] == 'y':
                        uvmax = float(sline[13].strip())
                        print 'Setting uvmax of sourceID:%d, sourceName :%s to %f'%(sourceId,realName, uvmax)
                    else:
                        uvmax = self.uvmax
                        print 'Setting uvmax of sourceID:%d, sourceName :%s to %f'%(sourceId,realName, uvmax)
                    
                    
            # will eventually disappear
            fluxratio=float(1.0)
                
            date_observed = tm.get_datetime_from_isodatetime(sline[14].strip())
            origin = filename
            catalogue_id = long(5)
            names = []
            types = []

            print realName, sourceId, ra, ra_uncertainty, dec, dec_uncertainty,frequency, flux, flux_uncertainty, degree,degree_uncertainty, angle, angle_uncertainty, fluxratio, uvmin, uvmax, date_observed, origin, catalogue_id

            sourceFluxList.append([flux,flux_uncertainty,date_observed])

            #raw_input()
            if dryrun == True:
                checkAdd = 'n'
            else:
                checkAdd = 'y'

            accuracy = flux_uncertainty / flux
            accuracy_limit = {3:0.05,6:0.1,7:0.1,9:0.2}
            checkAcc = 'y'
            if ((accuracy > accuracy_limit[aU.getBand(frequency)]) and (flux > 0.1)):
                cA = raw_input('Do you really want to submit this measurement with low accuracy of %s%% ?'%(int(accuracy*100.)))
                checkAcc = cA.lower()[0]

            if ((checkAdd == 'y') and (checkAcc == 'y')):                
                measurementId = self.s.sourcecat.addMeasurement(sourceId, ra, ra_uncertainty, dec, dec_uncertainty,\
                                                                frequency, flux, flux_uncertainty, degree,\
                                                                degree_uncertainty, angle, angle_uncertainty,fluxratio,
                                                                uvmin,uvmax,date_observed, origin, catalogue_id)
                count = count + 1
                print 'measurementId',measurementId
                if measurementId != 0:
                    print "Making this new Measurement %i on sourceID %i Valid" %(measurementId, sourceId)
                    setValid = self.s.sourcecat.setMeasurementState(measurementId, True)
                    if setValid == True:
                        print "Measurement %i  on source %i is now  Valid" %(measurementId, sourceId)
                    else:
                        print "Adding measurement %i on source %i worked but making it Valid failed" %(measurementId,sourceId)
                else:
                    print "Sorry failed to add measurement to database.MeasurementId = %d"%measurementId
            else:
                continue

        #print sourceNameList
        # make plots for all these sources
        if dryrun == True:
            self.makePlots_np(sourceNameList, sourceFluxList, interactive = True)
        else:
            self.makePlots(sourceNameList, interactive = True)
            

        # rsync is now happening on a cronjob on casa02 to wwwpub.sco.alma.cl once a day.
        # check for dbarkats crontab -l
            
        print " \n You have added %i measurements to the calibrator catalog" %count
        if dryrun == False:
            upd = raw_input('Do you want to update the Grid Table with the new fluxes? (y/n) - default yes to update, unless doing several ingestions in close succession')
            checkupd = upd.lower()[0]
            if checkupd != 'n':
                print "Updating Grid Table with new fluxes"
                self.update_table()
            
        return  

    def getSourceNamesFromMeasurement(self,m,doPrint = True):
        n_alias = size(m['names'])
        names=[]
        name0 = ''
        for n in m['names']:
            names.append(n['source_name'])
            if name0 == '':
                if  n['source_name'].startswith('3c') or n['source_name'].startswith('3C'):
                    name0 = n['source_name']
                elif  n['source_name'].startswith('J'):
                    name0 = n['source_name']
            
        if name0 == '':
            name0 = n['source_name']
        if doPrint == True: print names, name0
        return names, name0

                
    def wrapSearch(self, sourceBandLimit=1, limit=10, catalogues=None, types=None, name='',
                   ra=-1.0, dec=-1.0, radius=-1.0, ranges=[], fLower=-1.0, fUpper=-1.0,
                   fluxMin=-1.0, fluxMax=-1.0, degreeMin=-1.0, degreeMax=-1.0,
                   angleMin=-361.0, angleMax=-361.0, sortBy='source_id', asc=True,
                   searchOnDate=False, dateCriteria=0, date='', onlyValid=True,
                   uvmin=-1.0, uvmax=-1.0): 
        """
        This is the basic search. It is a wrapper around the  catalog's
        searching function
        OnlyValid = True means we find only valid sources. Not invalid ones.
        date has format '2013-01-01'
        limit is a limit on the number of sources
        sourceBandLimit is the number of measurements per source
        """

        if catalogues == None:
            catalogues=self.catalogues
        elif isinstance(catalogues,list) == False :
            print "Catalogues must be a list of integers ([1,2,3]. Try again. Available catalogues are:"
            print self.catalogList
            sys.exit()
            
        if types == None:
            types=self.types
        elif isinstance(types,list) == False :
            print "Types must be a list of integers ([1,2,3]. Try again. Available types are:"
            print self.typesList
            sys.exit()     
  
        # pre 9.1.3
        #measurements = self.s.sourcecat.searchMeasurements(limit,catalogues,types,name,ra, dec,
        #                                                            radius,ranges,fLower, fUpper, fluxMin,
        #                                                            fluxMax, degreeMin,degreeMax, angleMin,
        #                                                            angleMax ,sortBy ,asc,
        #                                                            searchOnDate,dateCriteria,date)
        try:
#            print "Searching using searchMeasurements103 with a source limit %d and measurements/source limit = %d"%(limit, sourceBandLimit)
            measurements = self.s.sourcecat.searchMeasurements103(sourceBandLimit,limit,catalogues,types,name,ra, dec,
                                                                  radius,ranges,fLower, fUpper, fluxMin,
                                                                  fluxMax, degreeMin,degreeMax, angleMin,
                                                                  angleMax ,sortBy ,asc,
                                                                  searchOnDate,dateCriteria,date,
                                                                  onlyValid,uvmin,uvmax)
        except:
#            print "searchMeasurements103 failed.  Searching using the older method searchMeasurements913"
            measurements = self.s.sourcecat.searchMeasurements913(limit,catalogues,types,name,ra, dec,
                                                                  radius,ranges,fLower, fUpper, fluxMin,
                                                                  fluxMax, degreeMin,degreeMax, angleMin,
                                                                  angleMax ,sortBy ,asc,
                                                                  searchOnDate,dateCriteria,date,
                                                                  onlyValid,uvmin,uvmax)

        measurements = checkForPseudoNullsInMeasurements(measurements)
        return measurements


    def gridSourceReport(self, nsources=None, bands=[3,6,7]):
        """
        Queries the calibrator database for each grid source in succession, 
        compiling a table of the days since the most recent measurement in 
        each band.
        nsources: an optional integer specifying the maximum number of sources to report
        -Todd Hunter
        """
        gridSources = defineGridNames()
        age = {}
        lines = "Age of most recent measurement (in days)\n"
        bandHeader = '      '.join([str(i) for i in bands])
        lines += "  SourceName    %s     uvmin - uvmax\n" % (bandHeader)
        if (nsources == None):
            nsources = len(gridSources)
        for src in range(nsources):
            source = gridSources[src]
            print "Working on source %s (%d of %d)" % (source,src+1,nsources)
            age[source] = {}
            line = '%11s' % (source)
            m = self.searchFlux(source,returnMostRecent=True,verbose=False)
            if (type(m['uvmin']) == NoneType):
                uvminmax = 'None - '
            else:
                uvminmax = '%.0fkl - ' % (m['uvmin'])
            if (type(m['uvmax'])==NoneType):
                uvminmax += 'None'
            elif m['uvmax'] < 0:
                uvminmax += '(%.0fkl)' % (m['uvmax'])
            else:
                uvminmax += '%.0fkl' % (m['uvmax'])
            for band in bands:
                age[source][band] = self.searchFlux(source,band=band,verbose=False)
                line += '   %4d' % (age[source][band])
            line += '    ' + uvminmax
            lines += line + '\n'
        print lines

    def computeIntervalBetweenTwoDays(self, date1, date2):
        """
        Takes 2 strings of format 'YYYY-MM-DD' and returns the number of
        days between them.
        """
        delta = datetime.date(int(date1[0:4]),int(date1[4:6]),int(date1[6:])) - \
                datetime.date(int(date2[0:4]),int(date2[4:6]),int(date2[6:]))
        return(delta.days)

    def getCatalogName(self, catalog_id):
        for i in range(len(self.catalogList)):
            if (self.catalogList[i]['catalogue_id'] == catalog_id):
                return(self.catalogList[i]['catalogue_name'])
        return('unknown')

    def searchFlux(self, sourcename=None, date='', band = None, fLower=1e9,
                   fUpper=1e12,
                   tunnel=False, maxrows=10, limit=1000, debug=False,
                   server = '', dateCriteria=0, verbose=True,
                   measurements=None,
                   returnMostRecent=False,showDateReduced=False,
                   sourceBandLimit=1, dayWindow=-1,showPolarization=False,
                   types=None, showAllCoordinates=False, returnPosition=False):
        """
        Searches the online ALMA calibrator catalog at JAO for a specific
        source name.
    
        sourcename: required string, e.g. 'J1924-292' (% = wildcard character)
        date: optional string, YYYYMMDD, e.g. '20120101' or '2012-01-01'
              or '2012/01/01'  where delimiter can be any non-integer character
              if blank or None, then use today's date.
        fLower, fUpper: optional floating point values in GHz or Hz
        band: optional band number integer (3..9)
        tunnel: Set this to True if you are outside the JAO network and want to
                access a server only accessible from within the JAO network.
                (As of June 14, 2013, this is no longer necessary.)
        server: server to access
        limit:  the maximum number of rows to return from the server 
        sourceBandLimit: the maximum number of rows to return from the server 
                         per source (r10.x)
        maxrows:  the maximum number of rows to display
        dateCriteria: -1, 0 or 1, meaning: before, on, after (currently, only 0 works!)
        date: string of the format YYYY-MM-DD
        showDateReduced: show the date of ingestion rather than observation
        dayWindow: if non-negative, then return a list of all measurements 
                   within this many days
        measurements: a dictionary of measurements (as returned from wrapSearch)
    
        Returns:
        The age of the most recent measurement (in days)
        Unless returnMostRecent = True, in which case it returns a dictionary:
          {'date':'YYYYMMDD', 'frequency'=<float_Hz>, 'flux'=<float_Jy>, 
            'age': <int_days>}
        or unless returnPosition = True, in which case it returns RA/Dec string
        - Todd Hunter
        """
        age = -1
        if (type(date) != str):
            print "The date must be a string in format YYYYMMDD or YYYY-MM-DD, where '-' can be any delimiter."
            return(age)
        if (sourcename == None and measurements == None):
            print "You must specify either sourcename or measurements."
            return
        sourcenameString = sourcename
        sourcenameWildcard = False
        if (type(sourcenameString) == str):
            if (sourcenameString.find('%')>=0):
                sourcenameWildcard = True
        if (band == None):
            if (fLower < 1e9):
                fLower *= 1e9
            if (fUpper < 1e9):
                fUpper *= 1e9
        elif (band in aU.bandDefinitions.keys()):
            if (verbose and maxrows>0):
                print "Checking for measurements in Band %d" % (band)
            fLower = aU.bandDefinitions[band][0]
            fUpper = aU.bandDefinitions[band][1]
        else:
            print "This is not an early science band."
            print "You can use fLower and fUpper to specify a different frequency range."
            return(age)
        if (date == None or date == ''):
            date = datetime.datetime.today().strftime('%Y%m%d')
        else:
            # strip out any non-integers
            mydate = ''
            for d in date:
                if d in [str(m) for m in range(10)]:
                    mydate += d
            date = mydate
        searchOnDate = True
        #mjd = self.strDate2MJD_non_casa(date)
        mjd = aU.strDate2MJD(date)
        if (measurements == None):
#            print "Calling self.wrapSearch(name='%s',limit=%d,date='%s',fLower=%f,fUpper=%f,searchOnDate=%s,sourceBandLimit=%d,types=%s)" % (sourcename,limit,date,fLower,fUpper,searchOnDate,sourceBandLimit,str(types))
            measurements = self.wrapSearch(name=sourcename, limit=limit, date=date, fLower=fLower, 
                                           fUpper=fUpper, searchOnDate=searchOnDate,
                                           sourceBandLimit=sourceBandLimit,
                                           types=types)
        size_m = len(measurements)
        if (debug):
            print measurements
        if (size_m == 0):
            if (verbose):
                print 'No measurements found for %s at this frequency. The wildcard character for source names is %%.' % (sourcename)
            return(age)
        else:
            if size_m > maxrows :
                showrows = maxrows
            else:
                showrows = size_m
            sourcename = []
            srcdegrees = []
            degreesUncertainty = []
            origin = []
            obsdate = []
            frequencies = []
            for n in measurements:
                synonyms = []
                for nn in n['names']:
                    synonyms.append(nn['source_name'])
                synonyms = np.sort(synonyms)
                synonymString = ''
                for s in synonyms:
                    if (synonymString != ''):
                        synonymString += ' = '
                    synonymString += s
                sourcename.append(synonymString)
                obsdate.append(n['date_observed'])
                srcdegrees.append([n['ra'],n['dec']])
                if (n['ra_uncertainty'] == None):
                    n['ra_uncertainty'] = -1
                if (n['dec_uncertainty'] == None):
                    n['dec_uncertainty'] = -1
                degreesUncertainty.append([float(n['ra_uncertainty']),float(n['dec_uncertainty'])])
                frequencies.append(float(n['frequency']))
#                print "%s %s" % (n['ra'], n['ra_uncertainty'])
                origin.append(n['origin'])
            sourcename = np.unique(sourcename)

            # Sort the display by the difference in time from the requested mjd
            julian_dates = self.getJulianDates(measurements)
            julian_min = np.array([abs(x-mjd) for x in julian_dates])
            bydate = range(len(srcdegrees))
            rank = np.argsort(julian_min)  # rank might look like 3,1,4,2,0
            # This can have a tie.  argsort breaks ties by taking furthest one
            # in the list.
            # But we should break ties by choosing the higher frequency in
            # order to have predictability.
            nvalues = len(np.where(julian_min[rank] == julian_min[rank[0]])[0])
            if (nvalues > 1):
                frequencies = np.array(frequencies)
                newfirst = np.argmax(frequencies[rank][:nvalues])
                oldfirst = rank[0]
                rank[0] = rank[newfirst]
                rank[newfirst] = oldfirst
            # Make a list of all the unique coordinate pairs
            tuples = [tuple(row) for row in srcdegrees]
            uniqueDegrees = []
            for t in tuples:
                if (t not in uniqueDegrees):
                    uniqueDegrees.append(t)
#            uniqueDegrees = list(np.unique(tuples))  # this reverses the order of the tuple!
            BAD_POSITION = [1e10,1e10]
            bestPositionUncertainty = BAD_POSITION
            bestPositionMeasurement = -1
            if (not showAllCoordinates):
                # Find the uncertainties associated with each unique coordinate pair
                uniqueDegreesUncertainty = []
                uniqueOrigin = []
                uniquePositionIndices = []
                for i in range(len(uniqueDegrees)):
                    pickPositionMeasurement = -1
                    raMatches = np.where(np.transpose(srcdegrees)[0] == uniqueDegrees[i][0])[0]
                    decMatches = np.where(np.transpose(srcdegrees)[1] == uniqueDegrees[i][1])[0]
                    idx = np.intersect1d(raMatches,decMatches)
                    # idx will be a list of indices into the complete list
                    maxUncertaintyForThisPosition = [-1,-1]
                    # Find the ALMA measurement with the least uncertainty.
                    # Since the values are returned in date order, it will
                    # always be the most recent one with that uncertainty.
                    for j in idx:
                        if (np.max(degreesUncertainty[j]) > np.max(maxUncertaintyForThisPosition) and
                            origin[j].find('ALMA')>=0):
                            maxUncertaintyForThisPosition = degreesUncertainty[j]
                            pickPositionMeasurement = j
                    if (pickPositionMeasurement < 0):
                        # If no ALMA measurement available with this position, then use the best position measured elsewhere.
                        for j in idx:
                            if (np.max(degreesUncertainty[j]) > np.max(maxUncertaintyForThisPosition)):
                                maxUncertaintyForThisPosition = degreesUncertainty[j]
                                pickPositionMeasurement = j
                    if (pickPositionMeasurement < 0 and len(idx)>0):
                        # If all the non-ALMA measurements have no uncertainty, then use the first one
                        # since it does not matter which one I use.
                        pickPositionMeasurement = idx[0]
                    uniqueDegreesUncertainty.append(maxUncertaintyForThisPosition)
                    uniqueOrigin.append(origin[pickPositionMeasurement])
                    uniquePositionIndices.append(pickPositionMeasurement)
                    if (np.max(uniqueDegreesUncertainty[-1]) < np.max(bestPositionUncertainty) and 
                        np.max(uniqueDegreesUncertainty[-1]) > 0):
                        bestPositionUncertainty = maxUncertaintyForThisPosition
                        bestPositionMeasurement = pickPositionMeasurement
                # Make lists corresponding to the unique coordinate pairs
                srcdegrees = uniqueDegrees
                origin = uniqueOrigin
                degreesUncertainty = uniqueDegreesUncertainty
                if (bestPositionUncertainty == BAD_POSITION):
                    bestPositionUncertainty = [-1,-1]
                    bestPositionMeasurement = 0
            else:
                maxUncertaintyForThisPosition = BAD_POSITION
                pickPositionMeasurement = -1
                for j in bydate:
                    if (np.max(degreesUncertainty[j]) > np.max(maxUncertaintyForThisPosition) 
                        and origin[j].find('ALMA')>=0):
                        maxUncertaintyForThisPosition = degreesUncertainty[j]
                        pickPositionMeasurement = j
                if (pickPositionMeasurement < 0):
                    # If no ALMA measurement available, then use the best position measured elsewhere.
                    for j in bydate:
                        if (np.max(degreesUncertainty[j]) > np.max(maxUncertaintyForThisPosition)):
                            maxUncertaintyForThisPosition = degreesUncertainty[j]
                            pickPositionMeasurement = j
                if (pickPositionMeasurement < 0):
                    # If all the non-ALMA measurements have negative uncertainty (i.e. none), then use the first one
                    # since it dooes not matter which one I use.
                    pickPositionMeasurement = bydate[0]
                bestPositionMeasurement = pickPositionMeasurement
                uniquePositionIndices = bydate

            matches = len(sourcename)
            bestPositionString = ''
            if (verbose):
                if (matches>1):
                    print 'Multiple Matches: ', sourcename
                else:
                    print 'Source: ', sourcename[0]
            if True:
                for srcIndex in range(len(srcdegrees)):
                    deg = srcdegrees[srcIndex]
                    degUncertainty = degreesUncertainty[srcIndex]
                    # Place an asterisk next to the measurement with the preferred source position
                    if (uniquePositionIndices[srcIndex] == bestPositionMeasurement):
                        rankstring = '*%2d' % (1+list(rank).index(uniquePositionIndices[srcIndex]))
                    elif (uniquePositionIndices[srcIndex] != -1):
                        rankstring = '%3d' % (1+list(rank).index(uniquePositionIndices[srcIndex]))
                    else:
                        rankstring = '   '
                    if (degUncertainty[0] > 0):
                        uncertainty = '(%g,%gmas)' % (degUncertainty[0]*3600000, degUncertainty[1]*3600000)
                    else:
                        uncertainty = '(n/a)'
                    if verbose:
                        print '(%s) J2000: %s+-%s (%s)' % (rankstring,self.deg2radec(deg[0],deg[1],verbose=False),uncertainty, origin[srcIndex])
                    if (uniquePositionIndices[srcIndex] == bestPositionMeasurement):
                        bestPositionString = self.deg2radec(deg[0],deg[1],verbose=False)
                if (len(srcdegrees) > 1):
                    # analyze the spread of positions
                    max_sep = -1
                    for i in range(len(srcdegrees)):
                        for j in range(1,len(srcdegrees)):
                            sep = aU.angularSeparation(srcdegrees[i][0], srcdegrees[i][1],
                                                 srcdegrees[j][0], srcdegrees[j][1]
                                                 )
                            if (sep > max_sep): max_sep = sep
                    max_sep *= 3600000
                    if (max_sep > 0 and verbose):
                        print "Maximum dispersion in position = %.2f milliarcsec" % max_sep
            if (verbose or sourcenameWildcard):
                if (maxrows > 0):
                    print 'Requested Freq.: %.0f-%.0f GHz , Requested date: '%(fLower*1e-9,fUpper*1e-9), date
                    if (showPolarization):
                        print ' Rank | Flux Density (Jy) | YYYY-MM-DD | Meas. Freq | catalog | Source Name    | uvmin-uvmax | %Pol |'
                    else:
                        print ' Rank | Flux Density (Jy) | YYYY-MM-DD | Meas. Freq | catalog | Source Name    | uvmin-uvmax |'
                else:
                    print " "
            minimumAge = 1e9
            index = rank # np.argsort(julian_min)
            for i in range(showrows):
                m = measurements[index[i]]
                if (matches > 1):
                    src = []
                    for nn in m['names']:
                        src.append(nn['source_name'])
                    src = np.sort(src)[-1]
                else:
                    src = m['names'][0]['source_name']
                dateString = (('%s' % m['date_observed']).split('T'))[0]
                try:
                    dateReduced = m['origin'].split('_')[-1].split('.txt')[0] # .replace('-','')
                    if (len(dateReduced) < 8):
                        dateReduced = m['origin'].split('_')[-2]
                        
                except:
                    dateReduced = 'unknown'
                age = self.computeIntervalBetweenTwoDays(date,dateString)
                if (age < minimumAge):
                    minimumAge = age
                if (i==0):
                    mydict = []
                    firstAge = age
                angle = m['angle']
                if (m['degree'] == None):
                    degreePolarization = None
                else:
                    degreePolarization = float(m['degree'])
                if (abs(age-firstAge) <= dayWindow or dayWindow < 0):
                    if (type(m['flux_uncertainty']) == NoneType):
                        uncertaintyValue = 0.0
                    else:
                        uncertaintyValue = float(m['flux_uncertainty'])
                    mydict.append({'age': age, 'date': dateString, 'flux': m['flux'], 'frequency': m['frequency'],
                              'uncertainty': uncertaintyValue, 'dateReduced': dateReduced, 'origin': m['origin'],
                                   'degree': degreePolarization, 'angle': angle, 
                                   'uvmin': m['uvmin'], 'uvmax': m['uvmax']})
                if (showDateReduced):
                    dateString = dateReduced
                else:
                    # insert dashes
                    dateString = dateString[0:4] + '-' + dateString[4:6] + '-' + dateString[6:]
                extension = float(m['extension'])
                catalog_id = self.getCatalogName(m['catalogue_id']).split()[0] # removes the 8.4GHz from CRATES
                if (extension > 1e10):
                    extension = 0
                if (verbose or sourcenameWildcard):
                    if showPolarization:
                        if (degreePolarization == None):
                            polarization = ' n/a |'
                        else:
                            polarization = ' %4.1f%% |' % (100*degreePolarization)
                    else:
                        polarization = ''
                    if (type(m['uvmin']) == NoneType or type(m['uvmax'])==NoneType):
                        if (type(m['uvmin']) == NoneType):
                            uvminmax = 'None - '
                        else:
                            uvminmax = '%.0fkl - ' % (m['uvmin'])
                        if (type(m['uvmax'])==NoneType):
                            uvminmax += 'None'
                        elif m['uvmax'] < 0:
                            uvminmax += '(%.0fkl)' % (m['uvmax'])
                        else:
                            uvminmax += '%.0fkl' % (m['uvmax'])
                        try:
                            fluxUncertainty = float(m['flux_uncertainty'])
                            print '%5d |  %7.3f +- %.3f  | %10s | %6.2f GHz | %7s | %14s | %s |%s' % (i+1, m['flux'],
                                                                                                      fluxUncertainty, dateString,
                                                                                                      m['frequency']/1e9, catalog_id, src, uvminmax, polarization)
                        except:
                            print '%5d |  %7.3f +- %4s  | %10s | %6.2f GHz | %7s | %14s | %s |%s' % (i+1, m['flux'],
                                                                                                     m['flux_uncertainty'], dateString,
                                                                                                     m['frequency']/1e9, catalog_id, src, uvminmax, polarization)
                    else:
                        uvmin = m['uvmin']
                        uvmax = m['uvmax']
                        if (uvmin > 1e10):
                            uvmin = 0
                        if (uvmax < 0):
                            uvminmax = '%.0f-(%.0f)' % (uvmin, uvmax)
                        else:
                            uvminmax = '%.0f-%.0f' % (uvmin, uvmax)
                        try:
                            fluxUncertainty = float(m['flux_uncertainty'])
                            print '%5d |  %7.3f +- %.3f  | %10s | %6.2f GHz | %9s | %14s | %s |%s' % (i+1, m['flux'],
                                      fluxUncertainty, dateString,
                                      m['frequency']/1e9, catalog_id, src, uvminmax, polarization)
                        except:
                            print '%5d |  %7.3f +- %4s  | %10s | %6.2f GHz | %9s | %14s | %x |%s' % (i+1, m['flux'],
                                      m['flux_uncertainty'], dateString,
                                      m['frequency']/1e9, catalog_id, src, uvminmax, polarization)

            if (verbose):
                if (matches == 1):
                    sourcesString = '1 source'
                else:
                    sourcesString = '%d sources' % (matches)
                if (size_m > maxrows and maxrows > 0):
                    print "Found %d measurements of %s with %d unique positions, but truncated because maxrows was set to %d." % (size_m, sourcesString, len(uniqueDegrees), maxrows)
                    if (band=='' or band==None):
                        print "Try setting the band (e.g. band=7)."
                elif (maxrows>0):
                    print 'Only ', str(size_m),' measurements found in total of %s, with %d unique positions' % (sourcesString,len(uniqueDegrees))
        if (returnMostRecent):
            if (dayWindow < 0):
                mydict = mydict[0]
            return(mydict)
        elif (returnPosition):
            return(bestPositionString)
        else:
            return(minimumAge)
        
    def getDateOfLatestMeasurmentFromSourceName(self,sourceName):
        """
        Given a source name, it will return the date
        of the last valid measurement for that source
        """
        measurements = self.wrapSearch(name=sourceName, limit = 1, sourceBandLimit=1, sortBy = 'date_observed',asc=False)
        if measurements != []:
            m = measurements[0]
            return m['date_observed'].value
        else:
            print "No Valid measurement for this source Name %s" %sourceName 
            return None
            
    def getSourceIdFromSourceName(self,sourceName) :
        """
        Searches for all sources given the name
        Returns sourceId
        Note that this search only finds sources which have a valid measurement
        
        """
        measurements=self.wrapSearch(name=sourceName, limit = 1, sourceBandLimit = 1)
        ids=[]
        for i in range(size(measurements)):
            ids.append(measurements[i]['source_id'])

        sourceId=unique(ids)
        if size(sourceId) == 0:
            print "Sorry, could not find any sources with name %s.Try to add wildcard %% before or after source name" %sourceName
            return
        else:
            return int(sourceId[0])

  
    def getSourceNameFromId(self,sourceId):
        """
        Returns  the source names given the sourceId
        Note that this search returns even if the source has no measurements
        
        """
        source = self.s.sourcecat.getSourcePlus(sourceId,False)
        sourceNames = source['names']

        return sourceNames


    def getCatalogSourceList(self, sw='J') :
        """
        Get the list of all source names in the source catalog
        by looping through all existing source Ids.
        We currently reject all "odd" source names
        By Default, we only take source names which start with J (ie.J2350+646)
        To find all source names,  specific sw= ''

        """
        sourceList=[]
        for i in range(13000):
            sourceNames=self.getSourceNameFromId(i)
            for j in range(size(sourceNames)):
                name=sourceNames[j]['source_name']
                if name  == 'CJ2' or name == 'JVAS' or name == 'aw574' or name == 'VERA' or name == 'GVAR' or name == 'USNO' or name == 'VSOP':
                    continue
                elif name.startswith(sw) == False:
                    continue
                else:
                    sourceList.append(name)
                
        return sourceList


    def getMeasurementsFromSourceId(self,sourceId):      
        sourceDict=self.s.sourcecat.getSource(sourceId)
        measurements=checkForPseudoNullsInMeasurements(sourceDict['measurements'])
        return measurements

    
    def parseMeasurement(self, measurement):
        """
        parse a single measurement into attributes.
        
        """
        # Measurement should have already had all nulls  transformed into None.
        # this is done in wrapSearch.
        # only acts on XX_uncertainty,degree,angle,extention,origin
        m=measurement
        self.measurementId = m['measurement_id']
        self.ra_decimal = float(m['ra'])
        self.ra_hms = self.decimaldeg2dms(m['ra'], raflag = True)
        self.ra_uncertainty = m['ra_uncertainty']
        self.dec_decimal = float(m['dec'])
        self.dec_dms = self.decimaldeg2dms(m['dec'], raflag = False)
        self.dec_uncertainty = m['dec_uncertainty']
        self.frequency = float(m['frequency'])
        self.flux = float(m['flux'])
        self.flux_uncertainty = m['flux_uncertainty']
        self.degree = m['degree']
        self.degree_uncertainty = m['degree_uncertainty']
        self.angle = m['angle']
        self.angle_uncertainty = m['angle_uncertainty']
        self.uvmin = m['uvmin']
        self.uvmax = m['uvmax']
        self.fluxratio = m['fluxratio']
        self.date_observed = ('%s' % m['date_observed']).split('T')[0]
        self.origin = m['origin']
        self.catalogue_id = m['catalogue_id']
        
        return 

#####################
# Utilities not related to catalog functionality directly
####################

    def put_in_array(self, m):
        x=zeros(size(m))
        y=zeros(size(m))
        yerr=zeros(size(m))
        
        for i in range(size(m)):
            d=m[i]['date_observed'].value
            #print d
            mjd = aU.strDate2MJD(d)
            x[i]=mjd
            y[i]=m[i]['flux']
            yerr[i]=m[i]['flux_uncertainty']

        q=argsort(x)
        return sort(x),y[q],yerr[q]

    def getJulianDates(self, measurements):
        """
        Converts date strings from the ALMA calibrator database measurements  into MJD values.
        Called by searchFlux().
        """
        julian =[]
        for m in measurements:
            date = ('%s' % m['date_observed']).split('T')[0]
            #mjdate = self.strDate2MJD_non_casa(date)
            mjdate = aU.strDate2MJD(date)
            
            julian.append(mjdate)
        return julian
    
    def splitFreq(self,measurements):
        l=[]
        m3=[]
        m6=[]
        m7=[]
        m9=[]
        for m in measurements:
            if m['frequency'] < aU.bandDefinitions[3][1] and  m['frequency'] > aU.bandDefinitions[3][0]:
                m3.append(m)
            elif m['frequency'] < aU.bandDefinitions[6][1] and  m['frequency'] > aU.bandDefinitions[6][0]:
                m6.append(m)
            elif m['frequency'] < aU.bandDefinitions[7][1] and  m['frequency'] > aU.bandDefinitions[7][0]:
                m7.append(m)
            elif m['frequency'] < aU.bandDefinitions[9][1] and  m['frequency'] > aU.bandDefinitions[9][0]:
                m9.append(m)
            elif m['frequency'] < aU.bandDefinitions[3][0]:
                l.append(m)

        print "# measurments =% d ; Sum of freq measurements = %d" %(size(measurements),(size(l)+size(m3)+size(m6)+size(m7)+size(m9)))
        return     l,m3,m6,m7,m9


    def dms2decimaldeg(self, val, raflag = '') :
        """
        Converts decimal degrees to Hours:Minutes:Seconds
        If ra is True, then we are converting a Ra measurment
        and we devide by 15 to go from 0-->360deg to 0--->24 hours.
        Else, we are converting a signed Dec measurement
        
        """
        if raflag == True: ra = 15.
        elif raflag == False: ra = 1.
        elif raflag == '':
            print "You did not specifiy if the dms measurement was RA  or Dec. Set ra flag to True or False."
            return
        
        vals = val.split(':')
        if vals[0][0] == '-' :
            sgn = -1
            vals[0] = - float(vals[0])
        else : sgn = 1

        val = float64(vals[0])+float64(vals[1])/60.0+float64(vals[2])/3600.

        return ra*sgn*val

    def dms2decimaldegspecial(self, val, raflag = '') :
        """
        Only used in __addnewSourceCatalogSources.
        Obsolete NOW
        
        Converts decimal degrees to Hours:Minutes:Seconds
        If ra is True, then we are converting a Ra measurment
        and we devide by 15 to go from 0-->360deg to 0--->24 hours.
        Else, we are converting a signed Dec measurement
        Only used in __addnewSourceCatalogSources.
        Obsolete NOW
        
        """
        if raflag == True: ra = 15.
        elif raflag == False: ra = 1.
        elif raflag == '':
            print "You did not specifiy if the dms measurement was RA  or Dec. Set ra flag to True or False."
            return
        
        vals = val.split('.')
        if vals[0][0] == '-' :
            sgn = -1
            vals[0] = - float(vals[0])
        else : sgn = 1

        val = float64(vals[0])+float64(vals[1])/60.0+float64('%s.%s'%(vals[2],vals[3]))/3600.

        return ra*sgn*val

    def decimaldeg2dms(self, x , raflag = ''):
        """
        Converts decimal degrees to Hours:Minutes:Seconds
        If ra is True, then we are converting a Ra measurment
        and we devide by 15 to go from 0-->360deg to 0--->24 hours.
        Else, we are converting a signed Dec measurement
        
        """
        if raflag == True:
            x = x/15.
        elif raflag == False:
            x = x
        elif raflag == '':
            print "You did not specifiy if the dms measurement was RA  or Dec. Set ra flag to True or False."
            return
        
        if x < 0 :
            sgn = '-'
            x = -x
        else : sgn = ''
        deg = int(x)
        arcmin = int((x-deg)*60.0)
        arcsec  = ((x-deg-arcmin/60.0)*3600.)
        return ("%s%s:%s:%s" % (str(sgn),str(deg),str(arcmin),str(arcsec)))
     
    def deg2radec(self, ra=0,dec=0, verbose=True):
         """
         Convert a position in RA/Dec from degrees to sexagesimal string.
         -Todd Hunter
         """
         
         return(self.rad2radec(ra*pi/180., dec*pi/180., verbose))
     
    def rad2radec(self, ra=0,dec=0,verbose=True):
        """
        Convert a position in RA/Dec from radians to sexagesimal string which
        is comma-delimited, e.g. '20:10:49.01, +057:17:44.806'.
        The position can either be entered as scalars via the 'ra' and 'dec' 
        parameters, as a tuple via the 'ra' parameter, as an array of shape (2,1)
        via the 'ra' parameter, or
        as an imfit dictionary can be passed via the 'imfitdict' argument, and the
        position of component 0 will be displayed in RA/Dec sexagesimal.
        Todd Hunter
        """
        if (type(ra) == tuple):
            dec = ra[1]
            ra = ra[0]
        if (np.shape(ra) == (2,1)):
            dec = ra[1][0]
            ra = ra[0][0]
        if (ra<0): ra += 2*pi
        rahr = ra*12/np.pi
        decdeg = dec*180/np.pi
        hr = int(rahr)
        min = int((rahr-hr)*60)
        sec = (rahr-hr-min/60.)*3600
        if (decdeg < 0):
            mysign = '-'
        else:
            mysign = '+'
        decdeg = abs(decdeg)
        d = int(decdeg)
        dm = int((decdeg-d)*60)
        ds = (decdeg-d-dm/60.)*3600
        mystring = '%02d:%02d:%08.5f, %c%02d:%02d:%08.5f' % (hr,min,sec,mysign,d,dm,ds)
        if (verbose):
            print mystring

        return(mystring)

    def getAccuracy(self,d):
        i=-1
        result = 1
        while result != 0:
            i=i+1
            p=int(10**(i))
            result = mod(d*p,1)
            
        #print "precision is 1E-%i" %i
        return i

    def readVLBICatalog(self, catalog='/users/dbarkats/vlbi_compare/rfc_2013d_cat.txt', verbose=True):
        f=open(catalog,'r')
        lines = f.readlines()
        f.close()
        
        sname = []
        stype = []
        s_ra =[]
        s_dec = []
        s_radeg = []
        s_decdeg = []
              
        #loop over all sources in VLBI database
        for line in lines:
            if line[0] != '#':
                fields = line.split()
                if len(fields) != 25:
                    print 'bad line ', line
                    break
                stype.append(fields[0])
                sname.append(fields[2])
                ra = fields[3]+':'+fields[4]+':'+fields[5]
                s_ra.append(ra)
                s_radeg.append(self.dms2decimaldeg(ra,raflag=True))
                dec = fields[6]+':'+fields[7]+':'+fields[8]
                s_dec.append(dec)
                s_decdeg.append(self.dms2decimaldeg(dec,raflag=False))
                
        nvlbi = len(sname)
        if verbose:
            print 'number of VLBI sources in %s: %d' % (os.path.basename(catalog), nvlbi)
        return sname,stype,s_ra,s_dec,s_radeg,s_decdeg

    def readVLBACatalog(self):
        f=open('/users/dbarkats/vlbi_compare/vlbaCalib.txt','r')
        lines = f.readlines()
        f.close()
        
        vname = []
        v_ra =[]
        v_dec = []
        v_radeg = []
        v_decdeg = []
              
        #loop over all sources in VLBA database
        for line in lines:
            if line[0] != '#':
                fields = line.split()
                #print fields
                vname.append(fields[0])
                ra = fields[2][0:2]+':'+fields[2][3:5]+':'+fields[2][6:].strip('s')
                #print ra
                v_ra.append(ra)
                v_radeg.append(self.dms2decimaldeg(ra,raflag=True))
                dec = fields[3][0:3]+':'+fields[3][4:6]+':'+fields[3][7:].strip('\"')
                #print dec
                v_dec.append(dec)
                v_decdeg.append(self.dms2decimaldeg(dec,raflag=False))
                
        nvla = len(vname)
        print 'number of VLBA sources ', nvla
        return vname,v_ra,v_dec,v_radeg,v_decdeg

  
#####################
# Plotting or displaying utilities
####################
    def autoPlot(self):
        print "autoPlot"
        
        gridNames = defineGridNames()
        self.makePlots(gridNames, interactive = True)
        
    def makePlots(self, sourceNameList, interactive = False):
        
        count = 0
        plotNames = []
        for sourceName in sourceNameList:
            sourceId,realName,officialName= self.matchName(sourceName)
            all_measurements = self.wrapSearch(name=realName,catalogues=[5],limit=1, sourceBandLimit = 500, sortBy='date_observed',asc=False)
            if size(all_measurements) != 0:
                plotname = self.makeSinglePlot(realName, all_measurements,count)
                plotNames.append(plotname)
            else:
                print "No ALMA measurements found for this object yet" 
        
            count = count + 1

        if interactive:
            plotlist=''
            for plotname in plotNames: 
                plotlist = '%s %s'%(plotlist,plotname)
            raw_input(" \n ### Review all the plots with the eog that just popped up and check for sanity !! ###")
            os.system('eog %s'%plotlist)

    
    def makeSinglePlot(self,realName, all_m, fignum = 0):
        
        figure(fignum,figsize=(12,9));clf()
        l,m3,m6,m7,m9 = self.splitFreq(all_m)
        type=['s','*','o','+']
        c=['r','m','b','g']
        label=['band3','band6','band7','band9']
        i=0
        
        hold(True)
        ax=subplot(111)
        for m in [m3,m6,m7,m9]:
            x,y,yerr = self.put_in_array(m)
            if size(x) != 0:
                errorbar(x,y,yerr=yerr,fmt='o-', color=c[i],label=label[i])
            i=i+1

        xlabel('MJD');ylabel('Database Flux Density (Jy)')
        legend()
        yl = ylim()
        self.makeXgridDates()
        vlines([self.y11,self.y12, self.y13,self.y14, self.y15, self.y16, self.y17],0,1.1*yl[1],colors='k', linestyles='dashed')
        vlines([self.ya,self.yb,self.yc,self.yd,self.ye,self.yf,self.yg,self.yh,self.yi,self.yj,self.yk,self.yl],
               0,1.1*yl[1],colors='k', linestyles='-.')
        text(self.ya,1.105*yl[1],'J', size='x-small', style = 'italic')
        text(self.yb,1.105*yl[1],'F', size='x-small', style = 'italic')
        text(self.yc,1.105*yl[1],'M', size='x-small', style = 'italic')
        text(self.yd,1.105*yl[1],'A', size='x-small', style = 'italic')
        text(self.ye,1.105*yl[1],'M', size='x-small', style = 'italic')
        text(self.yf,1.105*yl[1],'J', size='x-small', style = 'italic')
        text(self.yg,1.105*yl[1],'J', size='x-small', style = 'italic')
        text(self.yh,1.105*yl[1],'A', size='x-small', style = 'italic')
        text(self.yi,1.105*yl[1],'S', size='x-small', style = 'italic')
        text(self.yj,1.105*yl[1],'O', size='x-small', style = 'italic')
        text(self.yk,1.105*yl[1],'N', size='x-small', style = 'italic')
        text(self.yl,1.105*yl[1],'D', size='x-small', style = 'italic')

        vlines([self.yn],0,1.1*yl[1],color='c')

        text(self.y11+151,0.103*yl[0],'2011', size='x-large', style = 'normal')
        text(self.y12+151,0.103*yl[0],'2012', size='x-large', style = 'normal')
        text(self.y13+151,0.103*yl[0],'2013', size='x-large', style = 'normal')
        text(self.y14+151,0.103*yl[0],'2014', size='x-large', style = 'normal')
        text(self.y15+151,0.103*yl[0],'2015', size='x-large', style = 'normal')
        text(self.y16+151,0.103*yl[0],'2016', size='x-large', style = 'normal')
        text(self.y17+151,0.103*yl[0],'2017', size='x-large', style = 'normal')
               
        #xlim([55550,57765]);ylim([0,1.1*yl[1]])
        xlim([55550,58130]);ylim([0,1.1*yl[1]])
        #ax.yaxis.grid(T)
        suptitle('ALMA Flux measurements for %s' %realName, y=0.96, size='large')
        plotname = '%s/plots/%s_current_flux.png' %(self.basepath,realName)
        savefig('%s'%plotname)
        close(fignum)
        return plotname


    def makePlots_np(self, sourceNameList, sourceFluxList, interactive = False):
        
        count = 0
        plotNames = []
        for sourceName in sourceNameList:
            sourceId,realName,officialName= self.matchName(sourceName)
            all_measurements = self.wrapSearch(name=realName,catalogues=[5],limit=1, sourceBandLimit = 500, sortBy='date_observed',asc=False)
            #print all_measurements
            if size(all_measurements) != 0:
                flux_np = sourceFluxList[count]
                plotname = self.makeSinglePlot_np(realName, flux_np, all_measurements,count)
                plotNames.append(plotname)
            else:
                print "No ALMA measurements found for this object yet" 
        
            count = count + 1

        if interactive:
            plotlist=''
            for plotname in plotNames: 
                plotlist = '%s %s'%(plotlist,plotname)
            raw_input(" \n ### Review all the plots with the eog that just popped up and check for sanity !! ###")
            os.system('eog %s'%plotlist)

    
    def makeSinglePlot_np(self, realName, flux_np, all_m, fignum = 0):
        
        figure(fignum,figsize=(12,9));clf()
        l,m3,m6,m7,m9 = self.splitFreq(all_m)
        type=['s','*','o','+']
        c=['r','m','b','g']
        label=['band3','band6','band7','band9']
        i=0
        
        hold(True)
        ax=subplot(111)
        for m in [m3,m6,m7,m9]:
            x,y,yerr = self.put_in_array(m)
            #print x,y,yerr
            if size(x) != 0:
                errorbar(x,y,yerr=yerr,fmt='o-', color=c[i],label=label[i])
            i=i+1
        #print flux_np[2]
        np_d = str(flux_np[2]).split()[0].split('-')
        np_date=aU.strDate2MJD(np_d[0]+np_d[1]+np_d[2]+'T00:00:00')
        #print np_date
        errorbar(np_date,flux_np[0],yerr=flux_np[1],fmt='o-', color='k')

        xlabel('MJD');ylabel('Database Flux Density (Jy)')
        legend()
        yl = ylim()
        self.makeXgridDates()
        vlines([self.y11,self.y12, self.y13,self.y14, self.y15, self.y16, self.y17],0,1.1*yl[1],colors='k', linestyles='dashed')
        vlines([self.ya,self.yb,self.yc,self.yd,self.ye,self.yf,self.yg,self.yh,self.yi,self.yj,self.yk,self.yl],
               0,1.1*yl[1],colors='k', linestyles='-.')
        text(self.ya,1.105*yl[1],'J', size='x-small', style = 'italic')
        text(self.yb,1.105*yl[1],'F', size='x-small', style = 'italic')
        text(self.yc,1.105*yl[1],'M', size='x-small', style = 'italic')
        text(self.yd,1.105*yl[1],'A', size='x-small', style = 'italic')
        text(self.ye,1.105*yl[1],'M', size='x-small', style = 'italic')
        text(self.yf,1.105*yl[1],'J', size='x-small', style = 'italic')
        text(self.yg,1.105*yl[1],'J', size='x-small', style = 'italic')
        text(self.yh,1.105*yl[1],'A', size='x-small', style = 'italic')
        text(self.yi,1.105*yl[1],'S', size='x-small', style = 'italic')
        text(self.yj,1.105*yl[1],'O', size='x-small', style = 'italic')
        text(self.yk,1.105*yl[1],'N', size='x-small', style = 'italic')
        text(self.yl,1.105*yl[1],'D', size='x-small', style = 'italic')

        vlines([self.yn],0,1.1*yl[1],color='c')

        text(self.y11+151,0.103*yl[0],'2011', size='x-large', style = 'normal')
        text(self.y12+151,0.103*yl[0],'2012', size='x-large', style = 'normal')
        text(self.y13+151,0.103*yl[0],'2013', size='x-large', style = 'normal')
        text(self.y14+151,0.103*yl[0],'2014', size='x-large', style = 'normal')
        text(self.y15+151,0.103*yl[0],'2015', size='x-large', style = 'normal')
        text(self.y16+151,0.103*yl[0],'2016', size='x-large', style = 'normal')
               
        xlim([55550,57765]);ylim([0,1.1*yl[1]])
        ax.yaxis.grid(T)
        suptitle('ALMA Flux measurements for %s' %realName, y=0.96, size='large')
        plotname = '%s/plots/%s_current_flux.png' %(self.basepath,realName)
        savefig('%s'%plotname)
        close(fignum)
        return plotname


    def makeXgridDates(self):
        now ='%s'%datetime.datetime.now()
        now = now.replace('-','')
        year = now[0:4]
        self.yn = aU.strDate2MJD(now)
        self.y11 = aU.strDate2MJD('20110101')
        self.y12 = aU.strDate2MJD('20120101')
        self.y13 = aU.strDate2MJD('20130101')
        self.y14 = aU.strDate2MJD('20140101')
        self.y15 = aU.strDate2MJD('20150101')
        self.y16 = aU.strDate2MJD('20160101')
        self.y17 = aU.strDate2MJD('20170101')
        self.y18 = aU.strDate2MJD('20180101')
        
        self.ya = aU.strDate2MJD('%s0101'%year)
        self.yb = aU.strDate2MJD('%s0201'%year)
        self.yc = aU.strDate2MJD('%s0301'%year)
        self.yd = aU.strDate2MJD('%s0401'%year)
        self.ye = aU.strDate2MJD('%s0501'%year)
        self.yf = aU.strDate2MJD('%s0601'%year)
        self.yg = aU.strDate2MJD('%s0701'%year)
        self.yh = aU.strDate2MJD('%s0801'%year)
        self.yi = aU.strDate2MJD('%s0901'%year)
        self.yj = aU.strDate2MJD('%s1001'%year)
        self.yk = aU.strDate2MJD('%s1101'%year)
        self.yl = aU.strDate2MJD('%s1201'%year)
                
        
    def update_table(self):
        """
        This function will update an html table of the fluxes of the grid sources
        This is then transfered once a day to  www.alma.cl/~dbarkats/cal_survey/plots
        gets called at the end of addMeasurementFromALMA

        """
        gridNames = defineGridNames()
        
        # make the plots for the gridName sources.
        self.makePlots(gridNames, interactive = False)
        now='%s'%datetime.datetime.now()
        filename = self.basepath+'/plots/grid_fluxes.html'

        f = open(filename,'w')
        f.write('<html><head><title> Grid Survey </title></head> \n')
        f.write('<body bgcolor="white" text="black"> \n')
        f.write('<h1>LATEST FLUXES OF BRIGHT SOURCES</h1>  \n')
        f.write('Measurements extracted from ALMA archive: <b> http://sourcecat.osf.alma.cl </b>  <p>\n')
        f.write('Updated on %s by  %s <p>  \n' %(now,self.username))
        f.write('<p>\n')
        f.write('Missing measurements are left blank. Measurements older than 3 months are followed by a ** <p>\n')
        f.write('Fluxes greater than 4 and 2 Jy at B3 and B7  are highlighted in red. <p>\n')
        f.write('<table border = "0" cellpadding ="3" style="font-size: 14px;"  rules = groups frame = vsides> \n')
        f.write('<p>')
        f.write('<p>')
        f.write('<COLGROUP></COLGROUP> \n')
        f.write('<COLGROUP SPAN=2></COLGROUP> \n')
        f.write('<COLGROUP></COLGROUP> \n')
        f.write('<COLGROUP SPAN=4></COLGROUP> \n')
        f.write('<COLGROUP SPAN=4></COLGROUP> \n')
        f.write('<COLGROUP SPAN=4></COLGROUP> \n')
        f.write('<COLGROUP SPAN=4></COLGROUP> \n')
        
        #f.write('<tr><th>Source Name </th><th colspan="2">Coordinates</th><th>Flux History</th><th colspan="4">Band 3</th><th colspan="4">Band 6</th><th colspan="4">Band 7</th><th colspan="4">Band 9</th></tr>  \n')
        #f.write('<tr><th></th><th>R.A. [h.m.s]</th><th>Dec [d:m:s]</th><th></th><th>Flux[Jy]</th><th>Sigma[Jy]</th><th>Date</th><th> # Days</th><th>Flux[Jy] </th><th>Sigma[Jy]</th><th>Date</th><th># days</th><th>Flux[Jy]</th><th>Sigma[Jy]</th><th>Date</th><th># days</th><th>Flux[Jy]</th><th>Sigma[Jy]</th><th>Date</th><th># days</th></tr>  \n')
        #f.write('<tr><th>Source Name </th><th colspan="2">Coordinates</th><th>Flux History</th><th colspan="4">Band 3</th><<th colspan="4">Band 7</th></tr>  \n')
        #f.write('<tr><th></th><th>R.A. [h.m.s]</th><th>Dec [d:m:s]</th><th></th><th>Flux[Jy]</th><th>Sigma[Jy]</th><th>Date</th><th> # Days</th><th>Flux[Jy]</th><th>Sigma[Jy]</th><th>Date</th><th># days</th></tr>  \n')
       
        f.write('<tr><th>Source Name </th><th colspan="2">Coordinates</th><th>Flux History</th><th colspan="4">Band 3</th><th colspan="4">Band 6</th><th colspan="4">Band 7</th></tr>  \n')
        f.write('<tr><th></th><th>R.A. [h.m.s]</th><th>Dec [d:m:s]</th><th></th><th>Flux[Jy]</th><th>Sigma[Jy]</th><th>Date</th><th> # Days</th><th>Flux[Jy] </th><th>Sigma[Jy]</th><th>Date</th><th># days</th><th>Flux[Jy]</th><th>Sigma[Jy]</th><th>Date</th><th># days</th></tr>  \n')
        
        for i in gridNames:
            sourceID,realName,officialName = self.matchName(i)
            measurements =  self.wrapSearch(name=realName,catalogues=[5], limit=1,sourceBandLimit = 1, sortBy='date_observed',asc=False)
            if measurements != []:
                m = measurements[0]
            else:
                continue
            self.parseMeasurement(m)
            f.write('<tr> \n')
            f.write('<td><strong>%10s</strong></td><td> %8s</td><td> %8s</td> \n' %(realName,self.ra_hms, self.dec_dms))
            f.write('<td><a href="http://www.alma.cl/~ahales/cal_survey/plots/%s_current_flux.png"> Plot </a></td> \n' %(realName))

            #B3
            measurements = self.wrapSearch(name=realName, limit = 1, sourceBandLimit = 1, sortBy='date_observed',asc=False,catalogues=[5], fLower=aU.bandDefinitions[3][0], fUpper=aU.bandDefinitions[3][1])
            if measurements != []:
                self.parseMeasurement(measurements[0])
                age = self.searchFlux(measurements=measurements, verbose=False)
                if age > 90:
                    sp = '**'
                else:
                    sp = ''
                    
                if self.flux > 4.0: f.write('<td bgcolor="red"><font color="yellow"> \n')
                else: f.write('<td>')
                f.write('%4.3f<sup>%s</sup></td><td>%4.3f</td><td>%s</td><td>%d</td> \n' %(self.flux,sp,convertNoneToFloat(self.flux_uncertainty),self.date_observed, age))
            else: f.write('<td></td><td></td><td></td><td></td> \n')

            #B6
            measurements = self.wrapSearch(name=realName, catalogues=[5], sortBy='date_observed',asc=False, fLower=aU.bandDefinitions[6][0], fUpper=aU.bandDefinitions[6][1])  
            if measurements != []:
                self.parseMeasurement(measurements[0])
                age = self.searchFlux(measurements=measurements, verbose=False)
                if age > 90:
                    sp = '**'
                else:
                    sp = ''
                if self.flux > 3.0: f.write('<td bgcolor="red"><font color="yellow"> \n')
                else: f.write('<td>')
                f.write('%4.3f<sup>%s</sup></td><td>%4.3f</td><td>%s</td><td>%d</td> \n' %(self.flux,sp,convertNoneToFloat(self.flux_uncertainty),self.date_observed, age))
            else: f.write('<td></td><td></td><td></td><td></td> \n')

            #B7
            measurements = self.wrapSearch(name=realName, limit = 1, sourceBandLimit = 1, catalogues=[5],asc=False, sortBy='date_observed',fLower=aU.bandDefinitions[7][0], fUpper=aU.bandDefinitions[7][1])  
            if measurements != []:
                self.parseMeasurement(measurements[0])
                age = self.searchFlux(measurements=measurements, verbose=False)
                if age > 90:
                    sp = '**'
                else:
                    sp = ''
                if self.flux > 2.0: f.write('<td bgcolor="red"><font color="yellow"> \n')
                else: f.write('<td>')
                f.write('%4.3f<sup>%s</sup></td><td>%4.3f</td><td>%s</td><td>%d</td> \n' %(self.flux,sp,convertNoneToFloat(self.flux_uncertainty),self.date_observed, age))
            else: f.write('<td></td><td></td><td></td><td></td>  \n')

            #B9
            #measurements = self.wrapSearch(name=realName, catalogues=[5],asc=False, sortBy='date_observed',fLower=aU.bandDefinitions[9][0], fUpper=aU.bandDefinitions[9][1])    
            #if measurements != []:
            #    self.parseMeasurement(measurements[0])
            #    age = self.searchFlux(measurements=measurements, verbose=False)
            #    if age > 90:
            #        sp = '**'
            #    else:
            #        sp = ''
            #    if self.flux > 1.0: f.write('<td bgcolor="red"><font color="yellow"> \n')
            #    else: f.write('<td>')
            #    f.write('%4.3f<sup>%s</sup></td><td>%4.3f</td><td>%s</td><td>%d</td> \n' %(self.flux,sp,convertNoneToFloat(self.flux_uncertainty),self.date_observed, age))
            #else: f.write('<td></td><td></td><td></td><td></td> \n')
            
        f.write('</table><br> \n')
        f.write('</body></html>  \n')

        
#####################
# Obsolete or single usage snippets of code
####################


    def udpateSourceCatalogPy(self, IP = 0):
        """
        written May 2013 by dB to take the existing measurements in ALMA Calibrator catalog
        and update sourceCatalog.py to have at least the latest measurements
        Some sources in sourceCatalog.py were NOT found in ALMA Calibrator catalog.
        Those were made into JIRA ticket (CSV-2762, CSV-2763) to make sure we observt them.
        IP = 0 by default #  if 0, write to screen, if 1, overwrite file sourceCatalog.py

        For now, this is hardwired for dbarkats
        """

        os.system('cp /users/ahales/scripts/R9.1.1_WORKING/sourceCatalog.py.orig /users/ahales/scripts/R9.1.1_WORKING/sourceCatalog.py')
        sourceCatalogPy = '/users/ahales/scripts/R9.1.1_WORKING/sourceCatalog.py'

        sCat = []
        s_not_in_ALMA_catalog=[]
        s_not_obs_in_ALMA_catalog = []

        f = open('Sources_in_sourceCatalogPy_not_in_ALMA_cal_database.txt','w')
        g = open('Sources_in_sourceCatalogPy_not_measured_in_ALMA_cal_database.txt','w')
        
        started = 0
        # read the first useless part of
        for line in fileinput.input(sourceCatalogPy,inplace = IP):
            if ('0002-478'  not in line) and (started == 0):
                if IP == 1 : print line.strip('\n')
                continue
            
            # get to the interesting catalog part
            started = 1
            sourceName = line.split()[0].strip('\"')
            if 'spole' in sourceName:
                if IP == 1: print line.strip('\n')
                started = 0
                continue
            sCat.append(sourceName)
            ra = line.split()[4].strip('\"').strip('\",')
            dec = line.split()[7].strip('\"').strip('\",')
            flux =line.split()[10]
                        
            #check validity of sourceName and get SourceId
            sourceId,realName,officialName = self.matchName(sourceName, verbose =0)

            # If source doesn't exist we must add it to ALMA catalog (LATER)
            # and set flux in sourceCatalog.py to 0.000
            if sourceId == None:
                if IP == 0:
                    print bcolors.FAIL, sourceName, ra, dec, flux, "source does not exist in ALMA Cal database \n", bcolors.ENDC
                else:
                    print line.replace("\"flux\" : %s"%flux,"\"flux\" : 0.000,").strip('\n')

                s_not_in_ALMA_catalog.append(sourceName)
                f.write('%s %s %s %s :"source does not exist in ALMA Cal database \n' %(sourceName, ra, dec, flux))
                continue
            
            measurements = self.wrapSearch(name=realName, limit = 1, sourceBandLimit = 1, sortBy = 'date_observed', asc=False, catalogues =[5])
            if measurements != []:
                m = measurements[0]
            else:
                # if source have no measurement in ALMA catalog, we must measure it
                # we must set FLUX to 0.000 in sourceCatalog.py
                if IP == 0:
                    print sourceName, ra, dec, bcolors.OKBLUE, flux,bcolors.ENDC, "No meas for this source in ALMA CAL database \n"
                else:
                    print line.replace("\"flux\" : %s"%flux,"\"flux\" : 0.000,").strip('\n')
                s_not_obs_in_ALMA_catalog.append(sourceName)
                g.write('%s %s %s %s :"source  has no measurement in ALMA Cal database \n' %(sourceName, ra, dec, flux))
                continue
            
            self.parseMeasurement(m)
            if IP == 0:
                print sourceName, ra, dec, bcolors.OKBLUE,flux, bcolors.ENDC,realName, self.ra_hms, self.dec_dms, bcolors.OKBLUE, self.flux,bcolors.ENDC, self.date_observed, '\n'
            else:
                print line.replace("\"flux\" : %s"%flux,"\"flux\" : %2.3f," %self.flux).strip('\n')

        f.close()
        g.close()
        fileinput.close()
        return  sCat, s_not_in_ALMA_catalog, s_not_obs_in_ALMA_catalog

    def __addnewSourceCatalogSources(self, dryrun = True):
        """
        Single usage function to add new sources that we found in sourceCatalog.py but are NOT in the
        ALMA CalIBRATOR CATALOG
        See CSV-2762.
        The file list of such sources  is Sources_in_sourceCatalogPy_not_in_ALMA_cal_database.txt
        is generated by the function below: updateSourceCatalogPy
        """
        sname = []
        stype = []
        s_ra =[]
        s_dec = []
        s_radeg = []
        s_decdeg = []
        
        # read the VLBI catalog
        sname,stype,s_ra,s_dec,s_radeg,s_decdeg =self.readVLBICatalog()
                
        f = open('Sources_in_sourceCatalogPy_not_in_ALMA_cal_database.txt','r')
        for line in f:
            
            print " \n---------########--------########--------########--------########"
            _line = line.split()
            sourceName = _line[0].strip()
            
            rahms = _line[1]
            dechms = _line[2]
            ras = self.dms2decimaldeg(rahms,raflag=True)
            dechms = _line[2]
            decs = self.dms2decimaldegspecial(dechms,raflag=False)

            print 'sourcCat name:%16s %s, %s '% (sourceName,rahms, dechms)
            for j in range(nvlbi):
                rav = float(s_radeg[j])
                decv = float(s_decdeg[j])
                if (absolute(decs-decv) < 0.01) and (absolute(rav-ras)) < 0.01:
                    sep = aU.angularSeparation(ras,decs,rav,decv) *3600.
                    print 'VLBI name: %10s  %s, %s type:%s  sep: %8.3f'% (sname[j],s_ra[j],s_dec[j], stype[j], sep)
                    if sep < 40.0:
                        print 'Same source'
                        break
            
            measurements = self.wrapSearch(ra = float(ras),dec = float(decs),radius = 0.01, sortBy = 'date_observed', asc=False, limit = 10)
            if measurements != []:
                m = measurements[0]
                self.parseMeasurement(m)
                name = self.getSourceNamesFromMeasurement(m, False)
                print 'ACC name:',  name,  '  %s, %s'% (self.ra_hms,self.dec_dms)
                            
    def compareVLBIpos(self):
        """
        reusable script to compare ALMA source positions to
        VLBI  Petrov Catalog in ascii format.

        The VLBI Petrov catalog comes from astrogeo.org/vlbi/solutions/
        !!!Before running this script, one should pick up the new catalog!!!

        Algorithm is:
        Loop over all sources in VLBI catalog.
        For each source in VLBI catalog, store its ra and dec.
        find the corresponding source in ALMA catalog and get its ra and dec
        If sep between alma and VLBI is less than 40 arcsec, it's considered the same source
        
        If the seperation between alma and VLBI is more than 0.01arcsec, then the ALMA pos is bad
        
        #  Source separation to be considered the same source
        SOURCE_SEP = 40.0 arcsec
        # source separation to be considered that position should be improved
        SEP_MAX = 0.01 arcsec
        """
        SOURCE_SEP = 40.0
        SEP_MAX = 0.01
        
        g=open('comparison_VLBI.txt','w')
        h=open('no_VLBI.txt','w')

        #read the VLBI catalog
        sname,stype,s_ra,s_dec,s_radeg,s_decdeg =self.readVLBICatalog()
        nvlbi = size(sname)

        #read the VLBA catalog
        vname,v_ra,v_dec,v_radeg,v_decdeg =self.readVLBACatalog()
        nvlba = size(vname)
        
        # loop over all ALMA catalog sources
        # for now exclude invalid sources (CRATES sources are invalid, see CSv-2070)
        # to include them, make them valid or replace s.sourcat.getSource with getSourcePlus(i,False)
        invalid_src = 0
        total_src = 13000
        print 'number of ALMA sources ', total_src
        tot_found_vlbi=0
        tot_found_vlba = 0
        needs_updating = 0

        g.write('VLBA/I name, ALMA name, type, sep (arcsec), ALMA pos RAdecimal, DECdecimal, RA, DEC, VLBI pos RAdecimal, Decdecimal, RA, DEC \n')
        print ('VLBA/I name, ALMA name, type, sep (arcsec), ALMA pos RAdecimal, DECdecimal, RA, DEC, VLBI pos RAdecimal, Decdecimal, RA, DEC \n')
        #for i in range(total_src):
        for i in range(10):
            if mod(i,500) == 0: print 'processing ALMA source ', i

            source = self.s.sourcecat.getSource(i)
            measurements = source['measurements']
            if size(measurements) == 0:
                invalid_src += 1
                continue
            m  = measurements[0]
            self.parseMeasurement(m)
            ras = self.ra_decimal
            decs = self.dec_decimal
            rahms =  self.ra_hms
            dechms = self.dec_dms

            oname = self.matchName('',i,verbose = 0)[2]
            
            found_vlbi = 0
            #now search for that alma source in vlbi source by ra and dec pos.
            for j in range(nvlbi): 
                rav = float(s_radeg[j])
                decv = float(s_decdeg[j])
                namev = sname[j]
                if namev == oname:
                    #sep1 = (sqrt((decs-decv)**2 + ((ras-rav)*cos(decs*3.14159/180.0))**2)) *3600.
                    sep = aU.angularSeparation(ras,decs,rav,decv) *3600.
                    comment = ''
                    if sep > SEP_MAX:
                        needs_updating +=1
                        comment = '<<<<<<'
                    
                    g.write('%10s,%14s,%s,%5.3f,%9.5f, %9.5f, %s, %s, %9.5f, %9.5f, %s, %s %s \n'% (namev, oname, stype[j], sep,ras, decs, rahms,dechms,rav, decv,s_ra[j],s_dec[j],comment))
                    print '%10s, %14s, %s, %5.3f, %9.5f, %9.5f, %s, %s, %9.5f, %9.5f, %s, %s, %s'% (namev, oname, stype[j], sep,ras, decs, rahms,dechms,rav, decv,s_ra[j],s_dec[j],comment)
                   
                            # do we want to update all the measurements in that source with the new RA/DEC
                            # if so we have to loop over all the measurements
                            #for m  in measurements: 
                            #    self.parseMeasurement(m)
                            #    print "BEFORE:",  self.ra_decimal, self.dec_decimal
                            #    print "AFTER: ", rav, decv
                            #    updated=self.s.sourcecat.updateMeasurement(self.measurementId, rav, self.ra_uncertainty, decv,dec_uncertainty, self.frequency, self.flux, self.flux_uncertainty,self.degree,self.degree_uncertainty,self.angle,self.angle_uncertainty,self.uvmin, self.uvmax,self.fluxratio,date_observed,self.origin, self.catalogue_id)
                            #    print "Updated source %s = %s" %(sourceName, updated)
                            #    setValid = self.s.sourcecat.setMeasurementState(self.measurementId, True)
                            #    if setValid == True:
                            #        print "Measurement %i  on source %s is now  Valid" %(self.measurementId, sourceName)
                
                    found_vlbi = 1
                    tot_found_vlbi +=1
                    break
        
            if found_vlbi == 0: 
                h.write('no vlbi position for %16s \n'% (oname))
                found_vlba = 0
                # now search for that alma source in vlbi source by ra and dec pos.
                for j in range(nvlba): 
                    ravv = float(v_radeg[j])
                    decvv = float(v_decdeg[j])
                    namevv = vname[j]

                    if namevv == oname:
                        sep = aU.angularSeparation(ras,decs,ravv,decvv) *3600.
                        comment = ' VLBA'
                        if sep > SEP_MAX:
                            needs_updating +=1
                            comment = 'VLBA <<<<'
                    
                        g.write('%10s,%14s, V,%5.3f,%9.5f, %9.5f, %s, %s, %9.5f, %9.5f, %s, %s %s \n'% (namevv, oname, sep,ras, decs, rahms,dechms,ravv, decvv,v_ra[j],v_dec[j],comment))
                        print '%10s, %14s, V, %5.3f, %9.5f, %9.5f, %s, %s, %9.5f, %9.5f, %s, %s, %s'% (namevv, oname,  sep,ras, decs, rahms,dechms,ravv, decvv,v_ra[j],v_dec[j],comment)
                   
                            # do we want to update all the measurements in that source with the new RA/DEC
                            # if so we have to loop over all the measurements
                            #for m  in measurements: 
                            #    self.parseMeasurement(m)
                            #    print "BEFORE:",  self.ra_decimal, self.dec_decimal
                            #    print "AFTER: ", rav, decv
                            #    updated=self.s.sourcecat.updateMeasurement(self.measurementId, rav, self.ra_uncertainty, decv,dec_uncertainty, self.frequency, self.flux, self.flux_uncertainty,self.degree,self.degree_uncertainty,self.angle,self.angle_uncertainty,self.uvmin, self.uvmax,self.fluxratio,date_observed,self.origin, self.catalogue_id)
                            #    print "Updated source %s = %s" %(sourceName, updated)
                            #    setValid = self.s.sourcecat.setMeasurementState(self.measurementId, True)
                            #    if setValid == True:
                            #        print "Measurement %i  on source %s is now  Valid" %(self.measurementId, sourceName)
                
                        found_vlba = 1
                        tot_found_vlba +=1
                        break
                
            if found_vlba == 0:
                h.write('no vlba position for %16s \n'% (oname))
                
                
        print 'Invalid sources = %d out of %d total sources' %(invalid_src,total_src)
        print  'with-VLBI-counterpart = %d out of %d total valid sources' %(tot_found_vlbi,total_src-invalid_src)
        print  'with-VLBA-counterpart = %d out of %d total valid sources' %(tot_found_vlba,total_src-invalid_src)
        print 'Sources with VLBI counterparts that need updating: %d'%needs_updating
        g.close()
        h.close()
        print '***********'
        print '***ALMA-VLBA Comparisons in file <comparison_VLBI.txt>'
        print '***ALMA sources not in VLBI list in file <no_VLBI.txt>'
        print '***********'

        
    def __makeInvalid(self):
        """
        one time script to make all crates source Invalid
        
        """
        s_crates = 0
        m_crates=0
        m_alma=0
        set_invalid = 0
        for i in range(15000):
        #for i in range(1,2):
            measurements = self.getMeasurementsFromSourceId(i)
            sourceNames = self.getSourceNameFromId(i)
            
            if len(measurements) != 0:
                for m in  measurements:
                    if m['catalogue_id'] == 3:
                        # make invalid
                        # setValid = self.s.sourcecat.setMeasurementState(m['measurement_id'], False)
                        # if setValid == True:
                            #print "Measurement %i  on source %s is now  Valid" %(m['measurement_id'], sourceNames[0])
                        #    set_invalid = set_invalid +1
                        m_crates=m_crates+1
                    
                    elif m['catalogue_id'] == 5:
                        m_alma=m_alma+1
                
            print i, sourceNames, m_crates, set_invalid, m_alma
        
    def __fixALMARADEC(self):
        files = os.popen('ls -1 ALMA*')
        count = 0
        checksources = []
        for file in files:
            file = file[:-1]
            print file
            
            f = open(file,'r')
            for line in f:
            
                if line.startswith('#'):
                    continue
                print " \n---------########--------########--------########--------########"
                _line = line.split(',')
                sourceName = _line[0].strip()

                # check validity of sourceName and get SourceId
                sourceId,realName,officialName = self.matchName(sourceName)
                
                # if source does not exist
                if sourceId == None:
                    print "sourceName does not exist in catalog"
                    sys.exit()

                measurements = []    
                measurements = self.getMeasurementsFromSourceId(sourceId)
                #print size(measurements)
                ra =[]
                dec = []
                for m in measurements:
                    self.parseMeasurement(m)
                    #print self.measurementId, self.ra_decimal, self.dec_decimal, self.origin
                    if ra == []:
                        ra.append(self.ra_decimal)
                        dec.append(self.dec_decimal)
                    else:
                        if abs(self.ra_decimal - ra[-1]) >= 2.7e-5 or abs(self.dec_decimal - dec[-1]) >=2.7e-5 :
                    
                            if sourceName in checksources:
                                continue
                            else:
                                print "coordinates do not match for this source"
                                print sourceName, self.measurementId, self.ra_decimal, ra[-1],  self.ra_decimal- ra[-1], self.dec_decimal,dec[-1], self.dec_decimal - dec[-1]
                                count = count +1
                                checksources.append(sourceName)
                                break
                        else:
                            ra.append(self.ra_decimal)
                            dec.append(self.dec_decimal)
                #raw_input()
        print count, checksources
    
    def __checkMergersByRADECnew(self):
        
        """
        Re-usable code to check all the duplicates within 10 arcseconds and print out names and coordinates
        
        """
        a = open('need_merging.txt','w')
        for id1 in range(11500):
            print "check ID", id1
            measurements = self.getMeasurementsFromSourceId(id1)
            if measurements != []:
                m = measurements[-1]
            else:
                continue
            
            self.parseMeasurement(m)
            ra1 = self.ra_decimal
            dec1 = self.dec_decimal
            sourceNames = self.getSourceNameFromId(id1)
            names1=[]
            
            for k in range(size(sourceNames)):
                if sourceNames[k]['source_name'] != 'JVAS' and sourceNames[k]['source_name'] != 'CJ2' : names1.append(sourceNames[k]['source_name'])

            # find all measurements within 10 arcsec of this ra/dec
            measurements2 = self.wrapSearch(limit = 500,ra = ra1,dec = dec1, radius = 10./3600.)
            if measurements2 != []:
                found = 0
                for j in range(size(measurements2)):
                    # check if the ra and dec are the same
                    jm = measurements2[j]
                    self.parseMeasurement(jm)
                    names2=self.getSourceNamesFromMeasurement(jm, False)
                    id2 = self.getSourceIdFromSourceName(names2[1])
                    ra2 = self.ra_decimal
                    dec2 = self.dec_decimal
                    if ra2 == ra1 or dec2 == dec1:
                        continue
                    else:
                        angle = aU.angularSeparation(ra1,dec1,ra2,dec2)*3600.
                        if angle > 0.01:
                            if found == 0 : print ' *********************'
                            found = 1
                           
                            print "Names, id1  of central QSO: ",names1, id1
                            print "Names, id2 of surrounding QSO: ",names2[0], id2
                            print "Ra/Dec1 : ",self.decimaldeg2dms(ra1,raflag=True), self.decimaldeg2dms(dec1,raflag=F)
                            print "Ra/Dec2 : ",self.decimaldeg2dms(ra2,raflag=True), self.decimaldeg2dms(dec2,raflag=F)
                            print "Angle = ", str(angle),' arcsec'
                            print " "
                            a.write('%d %d \n'%(id1,id2))
                        else:
                            continue
        a.close()

    def findNearestVLBISources(self, radecString, radius=3, catalog='rfc_2014b_cat.txt', 
                               returnDictionary=False, verbose=True):
        """
        Reads the current VLBI catalog and finds all sources within specified radius.
        Downloads the file first, if necessary, using wget.
        radius: search radius (in degrees)
        returnDictionary: if True, then return dictionary keyed by source name with value
           equal to the separation (in degrees). Otherwise, return simple list of sources.
        -Todd Hunter
        """
        if (not os.path.exists(catalog)):
            print "Downloading catalog %s from astrogeo.org" % catalog
            if '/opt/local/bin' not in os.environ['PATH']:
                os.environ['PATH'] += os.pathsep + '/opt/local/bin'
            wget = distutils.spawn.find_executable('wget',path=':'.join(sys.path)+':'+os.environ['PATH'])
            cmd = wget + ' astrogeo.org/vlbi/solutions/%s/%s' % (catalog.replace('_cat.txt',''), catalog)
            os.system(cmd)
        sname,stype,s_ra,s_dec,s_radeg,s_decdeg = self.readVLBICatalog(catalog, verbose=verbose)
        rarad, decrad = aU.radec2rad(radecString)
        sources = []
        mydict = {}
        typedict = {}
        for i in range(len(sname)):
            rav = np.radians(float(s_radeg[i]))
            decv = np.radians(float(s_decdeg[i]))
            separation = aU.angularSeparationRadians(rarad,decrad,rav,decv)
            if (separation < np.radians(radius)):
                sources.append(sname[i])
                mydict[sname[i]] = np.degrees(separation)
                typedict[sname[i]] = stype[i]
        if verbose:
            print "Found %d sources within %g degrees" % (len(sources), radius)
        if (returnDictionary):
            return mydict, typedict
        else:
            return sources

    def __checkOfficialName(self):
        """
        Go through all source ID and check that they all have the correct official name and they don't have the
        incorrect name
        
        """

        for sourceId in range(14000):
            a= s.sourcecat.getSourcePlus(sourceId,False)

            names = a['names']
            nameList= []
            for i in range(size(names)):
                nameList.append(names[i]['source_name'])
            measurements = a['measurements']
            if measurements != []:
                m = measurements[-1]
            else:
                continue
            ra = m['ra']
            dec = m['dec']
            pos = self.deg2radec(ra=ra,dec=dec, verbose = False)
            ra_string=pos.split(',')[0].strip()
            ras1 = ra_string.split(':')[0]
            ras2 = ra_string.split(':')[1]
            dec_string=pos.split(',')[1].strip()
            decs1 = dec_string.split(':')[0]
            decs2 = dec_string.split(':')[1]
            official_name = 'J%s%s%s%s'%(ras1,ras2,decs1,decs2)

            match = 0
            for i in range(size(names)):
                name = names[i]['source_name']
                nameId = names[i]['name_id']
                if len(name) == 10:
                    if name == official_name:
                        #print sourceId, name, official_name, 'Official Name match'
                        match += 1
                    else:
                        print sourceId, name, official_name, 'Bad official Name *****'
                        check = self.s.sourcecat.removeSourceName(sourceId, nameId)
                        if check == True:
                            print "bad name removed"
                        #raw_input()
            if match == 0:
                print sourceId, official_name, nameList, 'Missing official Name'
                new_name_id = self.s.sourcecat.addSourceName(sourceId, official_name)
                print "new name added to this source, nameId = %s"%new_name_id
                #raw_input()

    def __fixCratesRADEC(self):
        """
        single instance to fix all the RA/Dec of CRATES SOURCES after sources were re-ingested. see CSV-2070
        Run on May 23 2013.
        """

        for id in range(12000):
            # find the latest measurment that is NOT CRATES
            if size(measurements) > 1:
                print "check ID", id
                latest = 0
                for i in range(size(measurements)):
                    if measurements[i]['catalogue_id'] == 3:
                        continue
                    if measurements[i]['date_observed'].__ge__(measurements[latest]['date_observed']):
                        latest = i

                mlatest = measurements[latest]
                print latest, size(measurements), mlatest['date_observed']
                
                # set the latest measurement to the "GOOD" RA/DEC
                self.parseMeasurement(mlatest)
                ra1 = self.ra_decimal
                dec1 = self.dec_decimal
                ra1hms = self.ra_hms
                dec1dms = self.dec_dms
                
                for m in measurements:
                    self.parseMeasurement(m)
                    ra2 = self.ra_decimal
                    dec2 = self.dec_decimal
                    if ra2 == ra1 or dec2 == dec1:
                        continue
                    else:
                        print "BEFORE",ra2,dec2 , self.ra_hms, self.dec_dms
                        print "AFTER:", ra1,dec1, ra1hms, dec1dms
                        
                        updated=self.s.sourcecat.updateMeasurement(self.measurementId, ra1, convertNoneToFloat(self.ra_uncertainty),
                                                                   dec1, convertNoneToFloat(self.dec_uncertainty),
                                                                   self.frequency, self.flux, convertNoneToFloat(self.flux_uncertainty),
                                                                   convertNoneToFloat(self.degree),
                                                                   convertNoneToFloat(self.degree_uncertainty),
                                                                   convertNoneToFloat(self.angle),convertNoneToFloat(self.angle_uncertainty),
                                                                   convertNoneToFloat(self.uvmin), convertNoneToFloat(self.uvmax),
                                                                   convertNoneToFloat(self.fluxratio),m['date_observed'],self.origin, self.catalogue_id)
                        
                        print "Updated source %d = %s" %(id, updated)
                        setValid = self.s.sourcecat.setMeasurementState(self.measurementId, True)
                        if setValid == True:
                            print "Measurement %i  on sourceId %d is now  Valid" %(self.measurementId, id)

        
    def __addnewAT20GSource(self,filename, dryrun = True):
        """
        Single usage function to add new sources from southern_extra_with_RADEC.txt
        provided by Ed Fomalont. File with all info
        is located in osf-red:/data/cal_survey/southern_extra_with_RADEC.txt

        re-used Dec 2013 to add a  at20g catalog sources which also had a VLBI position and above 0.1 Jy.
        
        """
        count = 0
        
        f = open(filename,'r')
        for line in f:
            
            if line.startswith('#'):
                continue
            print " \n---------########--------########--------########--------########"
            _line = line.split(',')
            sourceName = _line[0].strip()
            
            #check validity of sourceName and get SourceId
            sourceId,realName, officialName = self.matchName(sourceName, verbose =True)
                       
            # If source doesn't exist add it to ALMA catalog
            if sourceId == None:
                #createSource = raw_input("Source name %s does not exist yet. Please add it manually along with its first measurement. This is done by dB for now (ahales@alma.cl)" %sourceName)
                if dryrun == True:
                    sourceId = 9999
                else:
                    sourceId = self.s.sourcecat.addSource()
                    self.s.sourcecat.addSourceType(sourceId, 1)
                    self.s.sourcecat.addSourceName(sourceId, sourceName)
                         
            # Put ra, dec, and their uncertainty from latest measurement into this new measurement
            ra_decimal = float(_line[1])   # in deg
            #ra_decimal = cc.dms2decimaldeg(ra,raflag = True)
            ra_uncertainty = float(_line[2])
            dec_decimal = float(_line[3])
            #dec_decimal = cc.dms2decimaldeg(dec,raflag = False)
            dec_uncertainty = float(_line[4])
            
            # Put frequency, flux, and flux uncertainty from this new measurement
            frequency = float(20.0e9)
            flux  = float(_line[6])
            flux_uncertainty = float(_line[7])
            degree = NULL_AS_FLOAT
            degree_uncertainty = NULL_AS_FLOAT
            angle = NULL_AS_FLOAT
            angle_uncertainty = NULL_AS_FLOAT

            if (_line[12].strip() == 'NE') or (_line[12].strip() == '0.0'):
                    uvmin = NULL_AS_FLOAT
            uvmax= float(_line[13].strip())
            
            # will eventually disappear
            fluxratio=float(1.0)
                
            date_observed = tm.get_datetime_from_isodatetime(_line[14].strip())
            origin = filename
            catalogue_id = long(21)
            names = []
            types = []
            
            print sourceName, sourceId, ra_decimal, ra_uncertainty, dec_decimal, dec_uncertainty,frequency, flux, flux_uncertainty, degree,degree_uncertainty, angle, angle_uncertainty,fluxratio, uvmin, uvmax, date_observed, origin, catalogue_id
            
            if dryrun == True:
                checkAdd = 'n'
            else:
                checkAdd = 'y'
                
            if checkAdd == 'y':
                measurementId = self.s.sourcecat.addMeasurement(sourceId, ra_decimal, ra_uncertainty, dec_decimal, dec_uncertainty,
                                                                frequency, flux, flux_uncertainty, degree,
                                                                degree_uncertainty, angle, angle_uncertainty,
                                                                fluxratio, uvmin, uvmax, date_observed, origin, catalogue_id)
                count = count + 1
                if measurementId != 0:
                    print "Making this new Measurement %i  on source %i Valid" %(measurementId, sourceId)
                    setValid = self.s.sourcecat.setMeasurementState(measurementId, True)
                    if setValid == True:
                        print "Measurement %i  on source %i is now  Valid" %(measurementId, sourceId)
                else:
                    print "Sorry adding this last measurement to source %i %s failed" %(sourceId, name)
            else:
                continue
            
        print " \n You have added %i measurements to the calibrator catalog" %count
        return

    def __fixMeasurementRADEC(self, filename) :
        """
        Another single usage function to update the RA/DEC  of all sources (not CRATES sources).
        Also used Aug 30th, to update  RA/dec of CRATES sources from osf-red:/data/cal_survey/revised_CRATES_sources_Aug302011_Ed.txt
        
        """
        f = open(filename,'r')
        for line in f:
            if line.startswith('#'):
                continue
            print " \n---------########--------########--------########--------########"
            _line = line.split(',')
            sourceName = _line[0].strip()
            measurements = self.wrapSearch(name = sourceName,catalog = 3, limit = 1, sourceBandLimit = 1000)
            for m in measurements:
                self.parseMeasurement(m)
                ra = float(_line[1].strip())
                ra_uncertainty = float(_line[2].strip())
                dec = float(_line[3].strip())
                dec_uncertainty = float(_line[4].strip())
                date_observed = datetime.datetime.strptime(self.date_observed,'%Y%m%d')
                print "BEFORE:", sourceName, self.measurementId, self.ra_decimal, self.ra_uncertainty, self.dec_decimal, self.dec_uncertainty, self.frequency, self.flux, self.flux_uncertainty,self.degree, self.degree_uncertainty,self.angle,self.angle_uncertainty,self.extension, self.fluxratio,date_observed, self.origin, self.catalogue_id
                print "AFTER: ", sourceName, self.measurementId, ra, ra_uncertainty, dec, dec_uncertainty, self.frequency, self.flux, self.flux_uncertainty,self.degree, self.degree_uncertainty,self.angle,self.angle_uncertainty,self.extension, self.fluxratio,date_observed, self.origin, self.catalogue_id
                updated=self.s.sourcecat.updateMeasurement(self.measurementId, ra, ra_uncertainty, dec, dec_uncertainty,
                                                       self.frequency, self.flux, self.flux_uncertainty,self.degree,
                                                       self.degree_uncertainty,self.angle,self.angle_uncertainty,self.extension,
                                                       self.fluxratio,date_observed,self.origin, self.catalogue_id)
                print "Updated source %s = %s" %(sourceName, updated)
                setValid = self.s.sourcecat.setMeasurementState(self.measurementId, True)
                if setValid == True:
                    print "Measurement %i  on source %s is now  Valid" %(self.measurementId, sourceName)
        return

            
    def __fixMeasurementFromSMA(self) :
        """
        Single usage snippet  of code used to modify all the SMA BAnd 6 measurements which were at 299 GHz. I put them at 230GHz.
        Used Aug 2011 only
        """
        count = 0
        measurements = self.wrapSearch(name='%%', sortBy = 'date_observed', catalogues =[1],fLower=200e9, fUpper=300e9, limit=500)
        for m in measurements:
            self.parseMeasurement(m)
            frequency = 230e9
            date_observed = datetime.datetime.strptime(self.date_observed,'%Y%m%d')
            print self.measurementId, self.ra_decimal, self.ra_uncertainty, self.dec_decimal, self.dec_uncertainty, frequency, self.flux, self.flux_uncertainty,self.degree, self.degree_uncertainty,self.angle,self.angle_uncertainty,self.extension, self.fluxratio,date_observed,  self.origin, self.catalogue_id
            updated=self.s.sourcecat.updateMeasurement(self.measurementId, self.ra_decimal, self.ra_uncertainty, self.dec_decimal,
                                                       self.dec_uncertainty,frequency, self.flux, self.flux_uncertainty,self.degree,
                                                       self.degree_uncertainty,self.angle,self.angle_uncertainty,self.extension,
                                                       self.fluxratio,date_observed,self.origin, self.catalogue_id)
            print "Updated = %s" %updated
         
            print " \n---------########--------########--------########--------########"
            count = count + 1
           
        print " \n You have added %i measurements to the calibrator catalog" %count
        return
    

    def __makeHarveyHappy(self):
        """
        Simple wrapper to get all source names in the calibrator catalog
        and output a txt file with a printout of all the latest measurements
        for those sources
        Beware, can take several minutes

        """
        #sourceList = self.getCatalogSourceList()
        self.writeMeasurementsToAscii(filename = 'Cal_catalog_All_latest_flux_only_J_names_CRATES_decimalradec.txt')

        return

  
    def __writeMeasurementsToAscii(self,  latest=True, filename=None):
        """
        Writes all the measurments for a given list of sources (or single source)
        into a txt file.
        Default is to write only the latest measurements (latest=True)
        Beware, takes several minutes
        
        """
        now='%s'%datetime.datetime.now()
        now = now[0:-7].replace(' ','T')
        
        if filename == None:
            filename='sourceList_from_ALMADB_%s.txt'%now
        
        f = open(filename,'w')
        f.write('########################### \n')
        f.write('  \n')
        f.write('Measurements extracted from ALMA archive ' + \
                'http://asaportal-dev.osf.alma.cl:8080/sourcecatweb/sourcecat/ \n')
        f.write('Date: %s \n' %now)
      
        f.write('Format [Units]: sourceName , ra [Hour:Min:Sec] , '       + \
                'ra_err [arcsec], dec [Degree:Min:Sec], dec_err [arcsec],'+ \
                'frequency [Hz], flux [Jy], flux_err [Jy], degree [%], '  + \
                'degree_err [%],  angle [deg], angle_err [deg], '         +\
                'extension[klambda], fluxratio [%], date_observed [YYYY-MM-DDT] \n')
        f.write('  \n')
        f.write('########################### \n')
        f.write('  \n')

        sourceIdList = []
        sourceList = []
        for i in range(18000):
            sourceNames=self.getSourceNameFromId(i)
            for j in range(size(sourceNames)):
                name=sourceNames[j]['source_name']
                if name  == 'CJ2' or name == 'JVAS' or name == 'aw574' or name == 'VERA' or name == 'GVAR' or name == 'USNO' or name == 'VSOP':
                    continue
                elif name.startswith('J') == False:
                    continue
                else:
                    sourceIdList.append(i)
                    sourceList.append(name)
        for k in range(size(sourceIdList)):
            sourceId = sourceIdList[k]
            sourceName = sourceList[k]
            if latest == True:
                measurements = self.s.sourcecat.getSourcePlus(sourceId,False)['measurements']
                if measurements != []:
                    measurement = measurements[-1]
                else:
                   continue 
                self.parseMeasurement(measurement)  
                f.write('%10s, %f, %.2e, %f, %.2e, %.6e, %4.3f,%4.3f,%.1f, %.1f, %.1f, %.1f, %f, %.1f, %s \n' %(sourceName,self.ra_decimal, self.ra_uncertainty, self.dec_decimal, self.dec_uncertainty, self.frequency,self.flux, self.flux_uncertainty,self.degree, self.degree_uncertainty,self.angle,self.angle_uncertainty, self.extension, self.fluxratio,self.date_observed))
            else:
                measurements = self.wrapSearch(name = sourceName, limit = 1, sourceBandLimit = 1000)
                for i in range(size(measurements)):
                    measurement = measurements[i]
                    self.parseMeasurement(measurement)    
                    f.write('%10s, %s, %.2e, %s, %.2e, %.6e, %4.3f,%4.3f, %.1f, %.1f, %.1f, %.1f, %f, %.1f, %s \n' %(sourceName, self.ra_hms, self.ra_uncertainty,self.dec_dms, self.dec_uncertainty, self.frequency,self.flux, self.flux_uncertainty,self.degree,self.degree_uncertainty,self.angle,self.angle_uncertainty,self.extension, self.fluxratio,self.date_observed))
        f.close()
        return

    def __test(self):
        for i in range(13):
            sourceNames = self.getSourceNameFromId(i)
            if size(sourceNames) != 0:
                sourceName=sourceNames[0]['source_name']
                self.matchName(sourceName)        
                raw_input()
            
# dictionnary structure
#source_id  (int)
#measurements  (dict)
#    - origin  (string)
#    - catalog_id (int)
#    - angle      (float)
#    - angle_uncertainty (float)
#    - degree     (float)
#    - extension  (float)
#    - ra (foat)
#    - ra_uncertainty (float)
#    - dec (float)
#    - dec_uncertainty (float)
#    - flux
#    - flux_uncertainty (float)
#    - flux_ratio (flt)
#    - date_observed  (datetime)
#    - names (empty)
#    - measurment_id (int)
#    - valid (Bool)
#    - frequency (flt)
#    - source_id (float)
#names (dict)
#    - name_id (int)
#    - source_name (string)
#types (dict)
#    - type_name (string)
#    - type_id (int)
    
#a=s.sourcecat.searchMeasurements(10,[1,2,3,5],[1,2,3],'3c273',-1.0,-1.0,-1.0,[],0.0,-1.0,-1.0,-1.0,-1.0,-1.0,-361.0,-361.0,'',False)
#s.sourcecat.searchMeasurements(10,[1,2],[1,2,3],'3c273',-1.0,-1.0,-1.0,[],0.0,-1.0,-1.0,-1.0,-1.0,-1.0,-361.0,-361.0,'date_observed',True)



 
