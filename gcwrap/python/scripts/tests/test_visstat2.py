from tasks import *
from taskinit import *
from __main__ import inp
from __main__ import default
import os
import shutil
import unittest
from itertools import izip
import copy
from sdstat import sdstat

#     Functional tests of visstat 

epsilon = 0.0001

#fits='ngc5921.fits'
#msfile = 'ngc5921.ms'

# Path for data
#datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/visstat/"
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/visstat2/"

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    #DATADIR = str(os.environ.get('TEST_DATADIR'))+'/visstat/'
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/visstat2/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR

print 'visstat2 tests will use data from '+datapath         

class visstat2_test(unittest.TestCase):
    def setUp(self):
        # MS in repository was flagged with the following command:
        # flagdata(vis=self.msfile, antenna='17', flagbackup=False)
        
        #self.msfile = "ngc5921.ms"
        self.msfile = "ngc5921_add_corect_model.ms"

        
        #self.msfile2 ="OrionS_rawACSmod_calTPave.ms"
        #self.msfile2 ="OrionS_rawACSmod_if2_cal.ms"
        
        #self.msfile2_asap = "OrionS_rawACSmod_calTPave.asap"
        #self.msfile2_asap = "OrionS_rawACSmod_if2_cal.asap"


        self.msfile2 ="OrionS_rawACSmod_calave.ms"
        self.msfile2_asap="OrionS_rawACSmod_calave.asap"

 
        self.msfile3="OrionS_rawACSmod_calave_intent_on1_off3.ms"
        self.msfile4="OrionS_rawACSmod_calave_intent_on1.ms"

        self.msfile5="OrionS_rawACSmod_calave_intent_on3_off1.ms"
        self.msfile6="OrionS_rawACSmod_calave_intent_off1.ms"

        #if(not os.path.exists(self.msfile)):
        #if(os.path.exists(self.msfile)):
            #shutil.copytree(datapath+self.msfile, self.msfile)
            
            #adding corrected and model datacolumns
            #clearcal(self.msfile, addmodel=True)

        
        shutil.copytree(datapath+self.msfile, self.msfile)
        shutil.copytree(datapath+self.msfile2, self.msfile2)
        shutil.copytree(datapath+self.msfile2_asap, self.msfile2_asap)
        shutil.copytree(datapath+self.msfile3, self.msfile3)
        shutil.copytree(datapath+self.msfile4, self.msfile4)
        shutil.copytree(datapath+self.msfile5, self.msfile5)
        shutil.copytree(datapath+self.msfile6, self.msfile6)
        #clearcal(self.msfile, addmodel=True)

        #default('visstat')
        default('visstat2')

    def tearDown(self):
        shutil.rmtree(self.msfile)
        shutil.rmtree(self.msfile2)
        shutil.rmtree(self.msfile2_asap)
        shutil.rmtree(self.msfile3)
        shutil.rmtree(self.msfile4) 
        shutil.rmtree(self.msfile5)
        shutil.rmtree(self.msfile6)






    def test1(self):
        '''Visstat2 1: Default values'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        
        #visstat
        expected = {self.msfile:
                    {'DATA': {'max': 73.75,
                              'mean': 4.8371031336376049,
                              'medabsdevmed': 0.045013353228569031,
                              'median': 0.053559452295303345,
                              'min': 2.2130521756480448e-05,
                              'npts': 2660994.0,
                              #'quartile': 0.30496686697006226,
                              'rms': 17.081207275390625,
                              'stddev': 16.382008275102407,
                              'sum': 12871502.415990865,
                              'sumsq': 776391995.30866611,
                              'var': 268.37019512552371}}}
        
        
        #visstat2
        expected2 = {self.msfile:
                    {'DATA_DESC_ID=0': {'isMasked': True,
                                        'isWeighted': False,
                                        'max': 73.75,
                                        'maxDatasetIndex': 12,
                                        'maxIndex': 2408,
                                        'mean': 4.837103133618731,
                                        'medabsdevmed': 0.04501341888681054,
                                        'median': 0.05355948396027088,
                                        'min': 2.2130521756480448e-05,
                                        'minDatasetIndex': 54,
                                        'minIndex': 8692,
                                        'npts': 2660994.0,
                                        #'quartile': 0.3291134536266327,
                                        'rms': 17.081207832906546,
                                        'stddev': 16.382008276126726,
                                        'sum': 12871502.415939873,
                                        'sumsq': 776391995.3973862,
                                        'variance': 268.3701951590845}}}

        
        #visstat
        s = visstat(vis=self.msfile, axis='amp', datacolumn='data')
        
        #visstat2
        s2 = visstat2(vis=self.msfile, axis='amp', datacolumn='data', reportingaxes='ddid')

        #visstat
        if s.keys() != expected[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], s)

        #visstat2
        if s2.keys() != expected2[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], s2)
            raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], s))
                            
        #visstat
        if not s.has_key('DATA'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat does not have key DATA"

        #visstat2
        if not s2.has_key('DATA_DESC_ID=0'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat2 does not have key DATA"
            raise Exception("Dictionary returned from visstat does not have key DATA")


        #compared results between visstat and visstat2
        for e in expected[self.msfile]['DATA'].keys():
            e2=e
            if e == 'var':
                e2='variance'

            print "Checking %s: %s vs %s" % \
                   (e, expected[self.msfile]['DATA'][e], s2['DATA_DESC_ID=0'][e2])
            failed = False
            if expected[self.msfile]['DATA'][e] == 0:
                if s['DATA'][e] != 0:
                    failed = True
            else:
                if abs((expected[self.msfile]['DATA'][e] - s2['DATA_DESC_ID=0'][e2])/expected[self.msfile]['DATA'][e]) > 0.0001:
                    failed = True

            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected %s, got %s" % \
                      (str(expected[self.msfile]['DATA'][e]), str(s2['DATA_DESC_ID=0'][e2]))
                raise Exception("Numbers differ, expected %s, got %s" % \
                      (str(expected[self.msfile]['DATA'][e]), str(s2['DATA_DESC_ID=0'][e2])))

        #visstat2
        for e in expected2[self.msfile]['DATA_DESC_ID=0'].keys():
            print ''
            print "Checking %s: %s vs %s" % \
                   (e, expected2[self.msfile]['DATA_DESC_ID=0'][e], s2['DATA_DESC_ID=0'][e])
            failed = False
            if expected2[self.msfile]['DATA_DESC_ID=0'][e] == 0:
                if s2['DATA_DESC_ID=0'][e] != 0:
                    failed = True
            else:
                if abs((expected2[self.msfile]['DATA_DESC_ID=0'][e] - s2['DATA_DESC_ID=0'][e])/expected2[self.msfile]['DATA_DESC_ID=0'][e]) > 0.0001:
                    failed = True
            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected2 %s, got %s" % \
                      (str(expected2[self.msfile]['DATA_DESC_ID=0'][e]), str(s2['DATA_DESC_ID=0'][e]))


        self.assertTrue(retValue['success'],retValue['error_msgs'])









    def test12(self):
        '''Visstat2 12: Default values with datacolum=model, reporingaxis=ddid'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        #clearcal(self.msfile, addmodel=True)
        
        #visstat
        expected = {self.msfile:
                    {'MODEL': {'max': 1.0,
                              'mean': 1.0,
                              'medabsdevmed': 0.0,
                              'median': 1.0,
                              'min': 1.0,
                              'npts': 2660994.0,
                              'quartile': 0.0,
                              'rms': 1.0,
                              'stddev': 0.0,
                              'sum': 2660994.0,
                              'sumsq': 2660994.0,
                              'var': 0.0}}}
        
        
        #visstat2
        expected2 = {self.msfile:
                    {'DATA_DESC_ID=0': {'isMasked': True,
                                        'isWeighted': False,
                                        'max': 1.0,
                                        'maxDatasetIndex': 0,
                                        'maxIndex': 0,
                                        'mean': 1.0,
                                        'medabsdevmed': 0.0,
                                        'median': 1.0,
                                        'min': 1.0,
                                        'minDatasetIndex': 0,
                                        'minIndex': 0,
                                        'npts': 2660994.0,
                                        'quartile': 1.0,
                                        'rms': 1.0,
                                        'stddev': 0.0,
                                        'sum': 2660994.0,
                                        'sumsq': 2660994.0,
                                        'variance': 0.0}}}

        



        #print 'expected[self.msfile]', expected[self.msfile]
        #print ''
        #print 'expected2[self.msfile][\'DATA\']', expected2[self.msfile]['DATA_DESC_ID=0']
        #print ''


        #visstat
        s = visstat(vis=self.msfile, axis='amp', datacolumn='model')
        
        #visstat2
        s2 = visstat2(vis=self.msfile, axis='amp', datacolumn='model', reportingaxes='ddid')

        
        #print 's', s
        #print ''
        #print 's2', s2



        #visstat
        if s.keys() != expected[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], s)


        #visstat2
        if s2.keys() != expected2[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], s2)



#            raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
#                            (expected[self.msfile], s))
                            
        # Detailed check of values, column=DATA
        #print ''
        #print "Expected =", expected[self.msfile]
        #print ''
        #print "Got =", s
        #print ''
        #print "Got2 =", s2

        #visstat
        if not s.has_key('MODEL'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat does not have key DATA"

        #visstat2
        if not s2.has_key('DATA_DESC_ID=0'):
            #print ''
            #print "NO DATA key"
            #print ''
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat2 does not have key DATA"
            #print "retValue ", retValue


#            raise Exception("Dictionary returned from visstat does not have key DATA")


        #compared results between visstat and visstat2
        for e in expected[self.msfile]['MODEL'].keys():
            e2=e
            if e == 'var':
                e2='variance'
                
            print ''
            print "Checking %s: %s vs %s" % \
                   (e, expected[self.msfile]['MODEL'][e], s2['DATA_DESC_ID=0'][e2])
            failed = False
            if expected[self.msfile]['MODEL'][e] == 0:
                if s['MODEL'][e] != 0:
                    failed = True
            else:
                if abs((expected[self.msfile]['MODEL'][e] - s2['DATA_DESC_ID=0'][e2])/expected[self.msfile]['MODEL'][e]) > 0.0001:
                    failed = True
            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected %s, got %s" % \
                      (str(expected[self.msfile]['MODEL'][e]), str(s['MODEL'][e]))


#                raise Exception("Numbers differ, expected %s, got %s" % \
#                      (str(expected[self.msfile]['DATA'][e]), str(s['DATA'][e])))


        print ''

        #visstat2
        for e in expected2[self.msfile]['DATA_DESC_ID=0'].keys():
            print ''
            print "Checking %s: %s vs %s" % \
                   (e, expected2[self.msfile]['DATA_DESC_ID=0'][e], s2['DATA_DESC_ID=0'][e])
            failed = False
            if expected2[self.msfile]['DATA_DESC_ID=0'][e] == 0:
                if s2['DATA_DESC_ID=0'][e] != 0:
                    failed = True
            else:
                if abs((expected2[self.msfile]['DATA_DESC_ID=0'][e] - s2['DATA_DESC_ID=0'][e])/expected2[self.msfile]['DATA_DESC_ID=0'][e]) > 0.0001:
                    failed = True
            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected2 %s, got %s" % \
                      (str(expected2[self.msfile]['DATA_DESC_ID=0'][e]), str(s2['DATA_DESC_ID=0'][e]))


        self.assertTrue(retValue['success'],retValue['error_msgs'])








    def test9(self):
        '''Visstat2 9: Default values with datacolum=corrected, reporingaxis=ddid'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        #clearcal(self.msfile)
        
        #visstat
        expected = {self.msfile:
                    {'CORRECTED': {'max': 73.75,
                              'mean': 4.837103133618521,
                              'medabsdevmed': 0.04501341888681054,
                              'median': 0.05355948396027088,
                              'min': 2.2130521756480448e-05,
                              'npts': 2660994.0,
                              #'quartile': 0.3049671892076731,
                              'rms': 17.081207275390625,
                              'stddev': 16.38200827612749,
                              'sum': 12871502.415940082,
                              'sumsq': 776391995.3974303,
                              'var': 268.3701951591095}}}
        
        
        #visstat2
        expected2 = {self.msfile:
                    {'DATA_DESC_ID=0': {'isMasked': True,
                                        'isWeighted': False,
                                        'max': 73.75,
                                        'maxDatasetIndex': 12,
                                        'maxIndex': 2408,
                                        'mean': 4.837103133618731,
                                        'medabsdevmed': 0.04501341888681054,
                                        'median': 0.05355948396027088,
                                        'min': 2.2130521756480448e-05,
                                        'minDatasetIndex': 54,
                                        'minIndex': 8692,
                                        'npts': 2660994.0,
                                        #'quartile': 0.3291134536266327,
                                        'rms': 17.081207832906546,
                                        'stddev': 16.382008276126726,
                                        'sum': 12871502.415939873,
                                        'sumsq': 776391995.3973862,
                                        'variance': 268.3701951590845}}}

        


        #visstat
        s = visstat(vis=self.msfile, axis='amp', datacolumn='corrected')
        
        #visstat2
        s2 = visstat2(vis=self.msfile, axis='amp', datacolumn='corrected', reportingaxes='ddid')

 
        #visstat
        if s.keys() != expected[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], s)

        #visstat2
        if s2.keys() != expected2[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], s2)
            raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], s))
                            
        #visstat
        if not s.has_key('CORRECTED'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat does not have key DATA"

        #visstat2
        if not s2.has_key('DATA_DESC_ID=0'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat2 does not have key DATA"
            raise Exception("Dictionary returned from visstat does not have key DATA")


        #compared results between visstat and visstat2
        for e in expected[self.msfile]['CORRECTED'].keys():
            e2=e
            if e == 'var':
                e2='variance'
                
            print ''
            print "Checking %s: %s vs %s" % \
                   (e, expected[self.msfile]['CORRECTED'][e], s2['DATA_DESC_ID=0'][e2])
            failed = False
            if expected[self.msfile]['CORRECTED'][e] == 0:
                if s['DATA'][e] != 0:
                    failed = True
            else:
                if abs((expected[self.msfile]['CORRECTED'][e] - s2['DATA_DESC_ID=0'][e2])/expected[self.msfile]['CORRECTED'][e]) > 0.0001:
                    failed = True
            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected %s, got %s" % \
                      (str(expected[self.msfile]['CORRECTED'][e]), str(s2['CORRECTED'][e]))
                raise Exception("Numbers differ, expected %s, got %s" % \
                      (str(expected[self.msfile]['CORRECTED'][e]), str(s2['CORRECTED'][e])))

        #visstat2
        for e in expected2[self.msfile]['DATA_DESC_ID=0'].keys():
            print ''
            print "Checking %s: %s vs %s" % \
                   (e, expected2[self.msfile]['DATA_DESC_ID=0'][e], s2['DATA_DESC_ID=0'][e])
            failed = False
            if expected2[self.msfile]['DATA_DESC_ID=0'][e] == 0:
                if s2['DATA_DESC_ID=0'][e] != 0:
                    failed = True
            else:
                if abs((expected2[self.msfile]['DATA_DESC_ID=0'][e] - s2['DATA_DESC_ID=0'][e])/expected2[self.msfile]['DATA_DESC_ID=0'][e]) > 0.0001:
                    failed = True
            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected2 %s, got %s" % \
                      (str(expected2[self.msfile]['DATA_DESC_ID=0'][e]), str(s2['DATA_DESC_ID=0'][e]))


        self.assertTrue(retValue['success'],retValue['error_msgs'])







    def test2(self):     
        '''Visstat2 2: Check channel selections, useflags=True, repotingaxes='ddid',correlation=corr, datacolumn=data, axis=amp'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        for ch in [1, 2, 4, 7, 13, 62]:
          for corr in ['ll', 'rr', 'll,rr']:
            print "Call with spw='0:1~"+str(ch)+"', correlation="+corr
            s = visstat(vis=self.msfile, axis='amp', datacolumn='data', spw='0:1~'+str(ch), correlation=corr,useflags=True)
            s2 = visstat2(vis=self.msfile, axis='amp', datacolumn='data', spw='0:1~'+str(ch), correlation=corr, reportingaxes='ddid', useflags=True)
            print ''
            print 's', s
            print ''
            print 's2', s2
            print ''
            n_expected = 2660994/63 * ch   
            if corr in ['ll', 'rr']:
                n_expected /= 2
            
            #n = int(s['DATA']['npts'])
            n = int(s2['DATA_DESC_ID=0']['npts'])
            
            print "Checking npts: %s vs %s" % (n, n_expected)
            if n != n_expected:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError:"+str(n_expected) + " points expected, but npts = " + str(n)
#                raise Exception(str(n_expected) + " points expected, but npts = " + str(n))

        self.assertTrue(retValue['success'],retValue['error_msgs'])        








    """
    def test3(self):
        '''Visstat2 3: Test on different columns and axis'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        print "Create scratch columns."
        if testmms:
            clearcal(self.msfile)
        else:
            cblocal = cbtool()
            cblocal.open(self.msfile)
            cblocal.close()

        tblocal = tbtool()
        tblocal.open(self.msfile)
        cols = tblocal.colnames()
        tblocal.close()
        print 'initial cols=',cols

        cplx = ['amp', 'amplitude', 'phase', 'imag', 'imaginary', 'real']
        for x in cplx:
            cols.append(x)
        print ''
        print 'cols = ', cols
        print ''
        cols.remove('DATA')
        have_corr = False
	if('CORRECTED_DATA' in cols):
            cols.remove('CORRECTED_DATA')
            have_corr = True
            print 'CORRECTED_DATA exist!!!!!!!!!!!!!!!!!!!!!'
        have_model = False
        if('MODEL_DATA' in cols):
            cols.remove('MODEL_DATA')
            have_model = True
        cols.append('UVRANGE')

        cols = [x.lower() for x in cols]

        cols.remove('uvw')
        cols.remove('weight')
        cols.remove('sigma')
        cols.remove('weight_spectrum')
         

        print "Trying these column names", cols


        for col in cols:
            data_cols = ['']
	    print 'col ',col
	    print 'cplx ',cplx
            if col in cplx:
                data_cols = ['data'] # not supported: 'residual'
                if have_corr:
                    data_cols.append('corrected')
                #if have_model:
                #    data_cols.append('model')
                
            #data_cols.remove('corrected')
            #data_cols.remove('model')

            print 'data_cols= ', data_cols
            for dc in data_cols:
                print "Call with axis =", col, "; datacolumn =", dc
                if dc != '':
                    #s = visstat(vis=self.msfile, axis=col, datacolumn=dc)
                    s = visstat2(vis=self.msfile, axis=col, datacolumn=dc)
                else:
                    #s = visstat(vis=self.msfile, axis=col)
                    s = visstat2(vis=self.msfile, axis=col)
                print "Result was", s
                if col.upper() in ["FLAG_CATEGORY", "EXPOSURE", "OBSERVATION_ID", "PROCESSOR_ID", "STATE_ID", "TIME_CENTROID"]:
                    # no support for FLAG_CATEGORY, EXPOSURE, OBSERVATION_ID, ...
                    # so expect failure
                    if s != None:
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: "+str(s)
#                        raise Exception("Error! " + str(s))
                elif not type(s) is dict:
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Return value " + str(s) + " is not a dictionary"
#                    raise Exception("Error! Return value " + str(s) + " is not a dictionary")
                else:
                    #if dc == '' and \
                    #        not col.upper() in s.keys() and \
                    #        not col.upper()+'_0' in s.keys():
                    # 
                    #    print col.upper()
                    #    print col.upper()+'_0'
                    #    print s.keys()
                    #
                    #  
                    #    retValue['success']=False
                    #    retValue['error_msgs']=retValue['error_msgs']\
                    #    +"\nError: Missing key " + col.upper() + " in result"
#                        raise Exception("Missing key " + col.upper() + " in result")



                    
                    #if dc != '' and not dc.upper() in s.keys():
                    #    retValue['success']=False
                    #    retValue['error_msgs']=retValue['error_msgs']\
                    #    +"\nError: Missing key " + dc.upper() + " in result"
#                        raise Exception("Missing key " + dc.upper() + " in result")
                    
                    if dc !='' and not 'DATA_DESC_ID=0' in s.keys():
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Missing key " + dc.upper() + " in result"


        print 'retValue[\'success\'] ', retValue['success']
        print 'retValue[\'error_msgs\'] ', retValue['error_msgs']
        self.assertTrue(retValue['success'],retValue['error_msgs'])
        """





    def test4(self):
        '''Visstat2 4: Test of special cases'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        for a in range(1, 5):
            s = visstat(vis=self.msfile, axis='ANTENNA1', antenna=str(a)+'&26')
            s2 = visstat2(vis=self.msfile, axis='ANTENNA1', antenna=str(a)+'&26')
          
          
            print ''
            print s
            print ''
            
            print ''
            print s2
            print ''
            
            print ''
            print "antenna =", a, "; mean = ", s['ANTENNA1']['mean']
            print ''
            print "antenna =", a, "; mean = ", s2['DATA_DESC_ID=0']['mean']

            # Note there's a counting from 0 or 1 issue here
            # with the antenna numbering
            if self.msfile == self.msfile:
                offset = 1
            else:
                offset = 0
            if abs((s['ANTENNA1']['mean']+offset) - a) > epsilon:
            #if abs((s['DATA_DESC_ID=0']['mean']+offset) - a) > epsilon:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Failed when antenna= "+str(a)+'&26'
#                raise Exception("Error!")



            #if abs((s['ANTENNA1']['mean']+offset) - a) > epsilon:
            if abs((s2['DATA_DESC_ID=0']['mean']+offset) - a) > epsilon:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Failed when antenna= "+str(a)+'&26'
#                raise Exception("Error!")


    
        #visstat
        for scan in range(1, 8):
            s = visstat(vis=self.msfile, axis='SCAN_NUMBER', scan=str(scan))
            #s = visstat(vis=self.msfile, axis='scan_number', scan=str(scan))
            print ''
            print s
            print ''

            print "scan =", scan, "; mean = ", s['SCAN_NUMBER']['mean']
            if abs(s['SCAN_NUMBER']['mean'] - scan) > epsilon:        
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed for scan = "+str(scan)
#                raise Exception("Error!")

        self.assertTrue(retValue['success'],retValue['error_msgs'])


        #visstat2 
        for scan in range(1, 8):
            #s = visstat(vis=self.msfile, axis='SCAN_NUMBER', scan=str(scan))
            s2 = visstat2(vis=self.msfile, axis='scan_number', scan=str(scan))
            print ''
            print s2
            print ''

            print "scan =", scan, "; mean = ", s['SCAN_NUMBER']['mean']
            if abs(s2['DATA_DESC_ID=0']['mean'] - scan) > epsilon:        
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed for scan = "+str(scan)
#                raise Exception("Error!")

        self.assertTrue(retValue['success'],retValue['error_msgs'])






#    def test5(self):
#        '''Visstat2 5: Test that flagging impact statistics'''
#
#        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
# 
#        flagdata(vis=self.msfile, scan="1", flagbackup=False)        
#        s1 = visstat(vis=self.msfile, axis='scan_number', scan='>=1',useflags=True)
#        s2 = visstat2(vis=self.msfile, axis='scan', scan='>=1',useflags=True)
#        print ''
#        print 's1', s1
#        print ''
#        print 's2',s2
#        print ''
#
#        print "scan_number min = ", s1['SCAN_NUMBER']['min']
#        print "data_desc_id=0 min = ", s2['DATA_DESC_ID=0']['min']
#        print 'epsilon ', epsilon
#        if abs(s2['DATA_DESC_ID=0']['min'] - 1) > epsilon:
#            retValue['success']=False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=self.msfile, axis='scan_number', scan='>=1') "
# #            raise Exception("Error")
#
#        #flagdata(vis=self.msfile, scan="1", flagbackup=False)
#        
#        s1 = visstat(vis=self.msfile, axis='scan_number', scan='>=1')
#        s2 = visstat2(vis=self.msfile, axis='scan_number', scan='>=1')
#        print "scan_number min = ", s1['SCAN_NUMBER']['min']
#        print "data_desc_id=0 min = ", s2['DATA_DESC_ID=0']['min']
#
#        if abs(s2['DATA_DESC_ID=0']['min'] - 2) > epsilon:
#            retValue['success']=False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='scan_number', scan='>=1') "\
#                +"when data is flagged as flagdata(vis=msfile, scan='1')"
# #            raise Exception("Error")
#
#        flagdata(vis=self.msfile, scan="2", flagbackup=False)
#        #s = visstat(vis=self.msfile, axis='scan_number', scan='>=1')
#        s = visstat2(vis=self.msfile, axis='scan_number', scan='>=1')
#        print "min = ", s['DATA_DESC_ID=0']['min']
#        if abs(s['DATA_DESC_ID=0']['min'] - 3) > epsilon:
#            retValue['success']=False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='scan_number', scan='>=1') "\
#                +"when data is flagged as flagdata(vis=msfile, scan='2')"
# #            raise Exception("Error")
#
#        #s = visstat(vis=self.msfile, axis='scan_number', useflags=False, scan='>=1')
#        s = visstat2(vis=self.msfile, axis='scan_number', useflags=False, scan='>=1')
#        print "min = ", s['DATA_DESC_ID=0']['min']
#        if abs(s['DATA_DESC_ID=0']['min'] - 1) > epsilon:
#            retValue['success']=False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='scan_number', useflags=False, scan='>=1')"
# #            raise Exception("Error")
#
#        self.assertTrue(retValue['success'],retValue['error_msgs'])








#    def test6(self):
#        '''Visstat2 6: Test when all selected rows are flagged'''
#        flagdata(vis=self.msfile,mode='manual',antenna='1;1&&1', flagbackup=False)
#
#        #res = visstat(vis=self.msfile,antenna='1',useflags=True)
#        res = visstat2(vis=self.msfile,antenna='1',useflags=True)
#        self.assertFalse(res, 'All data are flagged. An exception should have been raised')




    def test8(self):
        '''Visstat2 7: Test when using reportingaxes='integration, datacolumn=data,corrected,model'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   
        clearcal(self.msfile, addmodel=True)
       
        correlation_type=['','LL','RR']
        datacolumn_list=['data', 'corrected', 'model']

        tb.open(self.msfile)
        ref=tb.getcolkeyword('TIME','MEASINFO')['Ref']
        tt=tb.getcol('TIME')
        #print tt[0]
        tb.close()

        print qa.time(me.epoch(ref,'%fs' % tt[0])['m0'], prec=8, form='ymd')[0]
        
        trange = qa.time(me.epoch('ref','%fs' % tt[0])['m0'], prec=8, form='ymd')[0]
        s2 = visstat2(vis=self.msfile, axis='amp', timerange=str(trange),reportingaxes='integration')
        s2_keys=s2.keys()

        #remove 'quartile'
        check_list1=['rms', 'medabsdevmed', 'min', 'max', 'sum', 'median', 'sumsq', 'stddev', 'var', 'npts', 'mean']
        #remove 'quartile'
        index_var =check_list1.index('var')
        check_list2=copy.deepcopy(check_list1)
        check_list2[index_var]='variance'


        for dt in datacolumn_list:
            for col in correlation_type:
                num_tt=0
                for time in tt:
                    trange = qa.time(me.epoch('ref','%fs' % time)['m0'], prec=8, form='ymd')[0]
                    s1 = visstat(vis=self.msfile, axis='amp', timerange=str(trange), correlation=col, datacolumn=dt)
                    s2 = visstat2(vis=self.msfile, axis='amp', timerange=str(trange),reportingaxes='integration', correlation=col, datacolumn=dt)

                    for list1, list2 in izip(check_list1, check_list2):
                        if(abs(s1[dt.upper()][list1]-s2[str(s2_keys[0])][list2]) > 0.000001):
                            retValue['success']= False
                            retValue['error_msgs']=retValue['error_msgs'] + list1 +list2
                            # +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='integragtion')"

                    num_tt +=1
                    if num_tt==10:
                        break

        self.assertTrue(retValue['success'],retValue['error_msgs'])
           



#    def test10(self):
#        '''Visstat2 10: Test using reportingaxes=field and datacolumn=corrected'''
#        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   

        #clearcal(self.msfile)


        #check_datacolumn=['data', 'corrected', 'model']
        #check_axis=['amp']
        #check_reporitngaxis=['ddid', 'field', 'integration']
        #check_list=['rms','medabsdevmed','min','max','sum', 'median', 'sumsq', 'stddev','var','npts','sumsq','stddev', 'mean']
        #check_fieldnum=['0','1','2']


        #for key in check_datacolumn:
        #    print key
        #    print key.upper()
        #    visstat(vis==self.msfile, axis='amp', field='0')






 #       s1_f0=visstat(vis=self.msfile, axis='amp', field='0', datacolumn='corrected')
 #       s1_f1=visstat(vis=self.msfile, axis='amp', field='1', datacolumn='corrected')
 #       s1_f2=visstat(vis=self.msfile, axis='amp', field='2', datacolumn='corrected')
        
 #       s2=visstat2(vis=self.msfile, axis='amp', reportingaxes='field', datacolumn='corrected')
        
 #       print ''
 #       print s1_f0
 #       print ''
 #       print s1_f1
 #       print ''
 #       print s1_f2
 #       print ''
 #       print s2


        




        #rms#
  #      if abs(s2['FIELD_ID=0']['rms'] - s1_f0['CORRECTED']['rms']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
  #      if abs(s2['FIELD_ID=1']['rms'] - s1_f1['CORRECTED']['rms']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['rms'] - s1_f2['CORRECTED']['rms']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


        #medabsdevmed#
  #      if abs(s2['FIELD_ID=0']['medabsdevmed'] - s1_f0['CORRECTED']['medabsdevmed']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
  #      if abs(s2['FIELD_ID=1']['medabsdevmed'] - s1_f1['CORRECTED']['medabsdevmed']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['medabsdevmed'] - s1_f2['CORRECTED']['medabsdevmed']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"
         

        #min#
  #      if abs(s2['FIELD_ID=0']['min'] - s1_f0['CORRECTED']['min']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
  #      if abs(s2['FIELD_ID=1']['min'] - s1_f1['CORRECTED']['min']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['min'] - s1_f2['CORRECTED']['min']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #max#
  #      if abs(s2['FIELD_ID=0']['max'] - s1_f0['CORRECTED']['max']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
  #      if abs(s2['FIELD_ID=1']['max'] - s1_f1['CORRECTED']['max']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['max'] - s1_f2['CORRECTED']['max']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['max'] - s1_f2['CORRECTED']['max']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #sum#
  #      if abs(s2['FIELD_ID=0']['sum'] - s1_f0['CORRECTED']['sum']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
  #      if abs(s2['FIELD_ID=1']['sum'] - s1_f1['CORRECTED']['sum']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['sum'] - s1_f2['CORRECTED']['sum']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['sum'] - s1_f2['CORRECTED']['sum']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


   #     #quartile#
   #     #if abs(s2['FIELD_ID=0']['quartile'] - s1_f0['DATA']['quartile']) > 0.000001:
   #     #    retValue['success']= False
   #     #    retValue['error_msgs']=retValue['error_msgs']\
   #     #        +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
        #if abs(s2['FIELD_ID=1']['quartile'] - s1_f1['DATA']['quartile']) > 0.000001:
        #    retValue['success']= False
        #    retValue['error_msgs']=retValue['error_msgs']\
        #        +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #if abs(s2['FIELD_ID=2']['quartile'] - s1_f2['DATA']['quartile']) > 0.000001:
        #    retValue['success']= False
        #    retValue['error_msgs']=retValue['error_msgs']\
        #        +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #if abs(s2['FIELD_ID=2']['quartile'] - s1_f2['DATA']['quartile']) > 0.000001:
        #    retValue['success']= False
        #    retValue['error_msgs']=retValue['error_msgs']\
        #        +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


  #      #median#
  #      if abs(s2['FIELD_ID=0']['median'] - s1_f0['CORRECTED']['median']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
  #      if abs(s2['FIELD_ID=1']['median'] - s1_f1['CORRECTED']['median']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

   #     if abs(s2['FIELD_ID=2']['median'] - s1_f2['CORRECTED']['median']) > 0.000001:
   #         retValue['success']= False
   #         retValue['error_msgs']=retValue['error_msgs']\
   #             +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['median'] - s1_f2['CORRECTED']['median']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


        #sumsq#
   #     if abs(s2['FIELD_ID=0']['sumsq'] - s1_f0['CORRECTED']['sumsq']) > 0.000001:
   #         retValue['success']= False
   #         retValue['error_msgs']=retValue['error_msgs']\
   #             +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
   #     if abs(s2['FIELD_ID=1']['sumsq'] - s1_f1['CORRECTED']['sumsq']) > 0.000001:
   #         retValue['success']= False
   #         retValue['error_msgs']=retValue['error_msgs']\
   #             +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['sumsq'] - s1_f2['CORRECTED']['sumsq']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['sumsq'] - s1_f2['CORRECTED']['sumsq']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #stddev#
#        if abs(s2['FIELD_ID=0']['stddev'] - s1_f0['CORRECTED']['stddev']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
 #       if abs(s2['FIELD_ID=1']['stddev'] - s1_f1['CORRECTED']['stddev']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['stddev'] - s1_f2['CORRECTED']['stddev']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['stddev'] - s1_f2['CORRECTED']['stddev']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"
        #var variance#
  #      if abs(s2['FIELD_ID=0']['variance'] - s1_f0['CORRECTED']['var']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
#        if abs(s2['FIELD_ID=1']['variance'] - s1_f1['CORRECTED']['var']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['variance'] - s1_f2['CORRECTED']['var']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['variance'] - s1_f2['CORRECTED']['var']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #npts#
#        if abs(s2['FIELD_ID=0']['npts'] - s1_f0['CORRECTED']['npts']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
#        if abs(s2['FIELD_ID=1']['npts'] - s1_f1['CORRECTED']['npts']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['npts'] - s1_f2['CORRECTED']['npts']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['npts'] - s1_f2['CORRECTED']['npts']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


        #mean#
#        if abs(s2['FIELD_ID=0']['mean'] - s1_f0['CORRECTED']['mean']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
#        if abs(s2['FIELD_ID=1']['mean'] - s1_f1['CORRECTED']['mean']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['mean'] - s1_f2['CORRECTED']['mean']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['mean'] - s1_f2['CORRECTED']['mean']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"



#        self.assertTrue(retValue['success'],retValue['error_msgs'])



#    def test11(self):
#        '''Visstat2 10: Test when using reportingaxes=field and datacolumn=model'''
#        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   

        #clearcal(self.msfile, addmodel=True)


        #check_datacolumn=['data', 'corrected', 'model']
        #check_axis=['amp']
        #check_reporitngaxis=['ddid', 'field', 'integration']
        #check_list=['rms','medabsdevmed','min','max','sum', 'median', 'sumsq', 'stddev','var','npts','sumsq','stddev', 'mean']
        #check_fieldnum=['0','1','2']


        #for key in check_datacolumn:
        #    print key
        #    print key.upper()
        #    visstat(vis==self.msfile, axis='amp', field='0')






 #       s1_f0=visstat(vis=self.msfile, axis='amp', field='0', datacolumn='model')
 #       s1_f1=visstat(vis=self.msfile, axis='amp', field='1', datacolumn='model')
 #       s1_f2=visstat(vis=self.msfile, axis='amp', field='2', datacolumn='model')
        
 #       s2=visstat2(vis=self.msfile, axis='amp', reportingaxes='field', datacolumn='model')
        
 #       print ''
 #       print s1_f0
 #       print ''
 #       print s1_f1
 #       print ''
 #       print s1_f2
 #       print ''
 #       print s2


        




        #rms#
 #       if abs(s2['FIELD_ID=0']['rms'] - s1_f0['MODEL']['rms']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
 #       if abs(s2['FIELD_ID=1']['rms'] - s1_f1['MODEL']['rms']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

 #       if abs(s2['FIELD_ID=2']['rms'] - s1_f2['MODEL']['rms']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


        #medabsdevmed#
  #      if abs(s2['FIELD_ID=0']['medabsdevmed'] - s1_f0['MODEL']['medabsdevmed']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
  #      if abs(s2['FIELD_ID=1']['medabsdevmed'] - s1_f1['MODEL']['medabsdevmed']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['medabsdevmed'] - s1_f2['MODEL']['medabsdevmed']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"
         

        #min#
  #      if abs(s2['FIELD_ID=0']['min'] - s1_f0['MODEL']['min']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
  #      if abs(s2['FIELD_ID=1']['min'] - s1_f1['MODEL']['min']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

  #      if abs(s2['FIELD_ID=2']['min'] - s1_f2['MODEL']['min']) > 0.000001:
  #          retValue['success']= False
  #          retValue['error_msgs']=retValue['error_msgs']\
  #              +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #max#
 #       if abs(s2['FIELD_ID=0']['max'] - s1_f0['MODEL']['max']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
 #       if abs(s2['FIELD_ID=1']['max'] - s1_f1['MODEL']['max']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

 #       if abs(s2['FIELD_ID=2']['max'] - s1_f2['MODEL']['max']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

 #       if abs(s2['FIELD_ID=2']['max'] - s1_f2['MODEL']['max']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #sum#
 #       if abs(s2['FIELD_ID=0']['sum'] - s1_f0['MODEL']['sum']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
 #       if abs(s2['FIELD_ID=1']['sum'] - s1_f1['MODEL']['sum']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

 #       if abs(s2['FIELD_ID=2']['sum'] - s1_f2['MODEL']['sum']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

 #       if abs(s2['FIELD_ID=2']['sum'] - s1_f2['MODEL']['sum']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


        #quartile#
        #if abs(s2['FIELD_ID=0']['quartile'] - s1_f0['DATA']['quartile']) > 0.000001:
        #    retValue['success']= False
        #    retValue['error_msgs']=retValue['error_msgs']\
        #        +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
        #if abs(s2['FIELD_ID=1']['quartile'] - s1_f1['DATA']['quartile']) > 0.000001:
        #    retValue['success']= False
        #    retValue['error_msgs']=retValue['error_msgs']\
        #        +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #if abs(s2['FIELD_ID=2']['quartile'] - s1_f2['DATA']['quartile']) > 0.000001:
        #    retValue['success']= False
        #    retValue['error_msgs']=retValue['error_msgs']\
        #        +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #if abs(s2['FIELD_ID=2']['quartile'] - s1_f2['DATA']['quartile']) > 0.000001:
        #    retValue['success']= False
        #    retValue['error_msgs']=retValue['error_msgs']\
        #        +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


        #median#
#        if abs(s2['FIELD_ID=0']['median'] - s1_f0['MODEL']['median']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
#        if abs(s2['FIELD_ID=1']['median'] - s1_f1['MODEL']['median']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

 #       if abs(s2['FIELD_ID=2']['median'] - s1_f2['MODEL']['median']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

 #       if abs(s2['FIELD_ID=2']['median'] - s1_f2['MODEL']['median']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


        #sumsq#
#        if abs(s2['FIELD_ID=0']['sumsq'] - s1_f0['MODEL']['sumsq']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
#        if abs(s2['FIELD_ID=1']['sumsq'] - s1_f1['MODEL']['sumsq']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['sumsq'] - s1_f2['MODEL']['sumsq']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['sumsq'] - s1_f2['MODEL']['sumsq']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #stddev#
#        if abs(s2['FIELD_ID=0']['stddev'] - s1_f0['MODEL']['stddev']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
#        if abs(s2['FIELD_ID=1']['stddev'] - s1_f1['MODEL']['stddev']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['stddev'] - s1_f2['MODEL']['stddev']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['stddev'] - s1_f2['MODEL']['stddev']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"
        #var variance#
#        if abs(s2['FIELD_ID=0']['variance'] - s1_f0['MODEL']['var']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
#        if abs(s2['FIELD_ID=1']['variance'] - s1_f1['MODEL']['var']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['variance'] - s1_f2['MODEL']['var']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['variance'] - s1_f2['MODEL']['var']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        #npts#
 #       if abs(s2['FIELD_ID=0']['npts'] - s1_f0['MODEL']['npts']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
 #       if abs(s2['FIELD_ID=1']['npts'] - s1_f1['MODEL']['npts']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

 #       if abs(s2['FIELD_ID=2']['npts'] - s1_f2['MODEL']['npts']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

 #       if abs(s2['FIELD_ID=2']['npts'] - s1_f2['MODEL']['npts']) > 0.000001:
 #           retValue['success']= False
 #           retValue['error_msgs']=retValue['error_msgs']\
 #               +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"


        #mean#
#        if abs(s2['FIELD_ID=0']['mean'] - s1_f0['MODEL']['mean']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

        
#        if abs(s2['FIELD_ID=1']['mean'] - s1_f1['MODEL']['mean']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['mean'] - s1_f2['MODEL']['mean']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"

#        if abs(s2['FIELD_ID=2']['mean'] - s1_f2['MODEL']['mean']) > 0.000001:
#            retValue['success']= False
#            retValue['error_msgs']=retValue['error_msgs']\
#                +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='field')"



#        self.assertTrue(retValue['success'],retValue['error_msgs'])






#    def test12(self):
#        '''Visstat2 12: Test when using reportingaxes='integration, correlation=[all,LL,RR], datacolumn=[data,corrected,model] spw=[0,1,2,3]'''
#        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   
#        #clearcal(self.msfile, addmodel=True)
#       
#        correlation_type=['RR', 'LL']
#        datacolumn_list=['float_data']
#
#        sd_correlation_type=['0', '1']
#
#        spw_list=['0', '1', '2', '3']
#
#        reporting_axes=['integration']
# 
#
#
#        tb.open(self.msfile2)
#        #tb.open('/work/mkuniyoshi/test_7449/visstat2/OrionS_rawACSmod_calave.ms')
#        ref=tb.getcolkeyword('TIME','MEASINFO')['Ref']
#        tt=tb.getcol('TIME')
#        #print tt[0]
#        tb.close()
#
#        print ''
#        print 'self.msfile2', self.msfile2
#        print ''
#
#        #print qa.time(me.epoch(ref,'%fs' % tt[0])['m0'], prec=8, form='ymd')[0]
#        
#        trange = qa.time(me.epoch('ref','%fs' % tt[0])['m0'], prec=8, form='ymd')[0]
#        s2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reporting_axes[0], datacolumn='float_data')
#
#        #s2=0
#        #if reporting_axes=='integration':
#        #    s2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reporting_axes[0], datacolumn='float_data')
#        #elif reporting_axes=='ddid':
#        #    s2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reporting_axes[1], datacolumn='float_data')
#        #elif reporting_axes=='field':
#        #    s2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reporting_axes[3], datacolumn='float_data')
#
#
# #s2 = visstat2(vis='/work/mkuniyoshi/test_7449/visstat2/OrionS_rawACSmod_calave.ms', axis='amp',reportingaxes='integration', datacolumn='float_data')
#
# 
#
#
#        print 's2', s2
#        s2_keys=s2.keys()
#
#        print 's2_keys', s2_keys
#        
#
#
#        #sdstat
#        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']
#        
#       
#        #test
#        
#        print ''
#        print 'trange', trange
#        print '' 
#
#        result_sdstat=sdstat(infile=self.msfile2_asap, pol='',timerange=str(trange))
#        #result_sdstat=sdstat(infile='/work/mkuniyoshi/test_7449/visstat2/OrionS_rawACSmod_calave.ms', pol='')
#        print ''
#        print 'result_sdstat=sdstat', result_sdstat
#        print ''
#
#
#
#        #for reptaxis in reporting_axes:
#        #    if reptaxis=='integration':
#        #        s2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reptaxis, datacolumn='float_data')
#        #    elif reptaxis ='ddid':
#        #        s2 = visstat2(vis=self.msfile2, axis='amp',reportingaxes=reptaxis, datacolumn='float_data')
#
#
#        for dt in datacolumn_list:
#            for col, sd_pol in izip(correlation_type, sd_correlation_type):
#                num_tt=0
#                for time in tt:
#                    for spwin in spw_list:
#                        #print 'spw ', spwin
#                        trange = qa.time(me.epoch('ref','%fs' % time)['m0'], prec=8, form='ymd')[0]
#                        sd = sdstat(infile=self.msfile2_asap, timerange=str(trange), pol=sd_pol, spw=spwin)
#                        v2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reporting_axes[0], correlation=col, datacolumn=dt, spw=spwin)
# 
#                        for check in check_list:
#                            #print v2
#                            if(abs((sd[check]-v2[str(s2_keys[0])][check])/sd[check]) > 0.0001):
#                                retValue['success']= False
#                                retValue['error_msgs']=retValue['error_msgs']
#                            # +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='integragtion')"
#
#                        num_tt +=1
#                        if num_tt==1:
#                            break
#
#        self.assertTrue(retValue['success'],retValue['error_msgs'])
#



    def test13(self):
        '''Visstat2 13: Test using reportingaxes=ddid, correlation=[all,LL,RR], datacolumn=float_data spw=[0,1,2,3], intent'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   
        #clearcal(self.msfile, addmodel=True)
       
        correlation_type=['RR', 'LL']
        sd_correlation_type=['0', '1']
        datacolumn_list=['float_data']
        spw_list=['0', '1', '2', '3']
        reporting_axes=['ddid']
        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']
        intent_list=['OBSERVE_TARGET#ON_SOURCE,POSITION_SWITCH']

        tb.open(self.msfile2)
        ref=tb.getcolkeyword('TIME','MEASINFO')['Ref']
        tt=tb.getcol('TIME')
        tb.close()
        
        trange = qa.time(me.epoch('ref','%fs' % tt[0])['m0'], prec=8, form='ymd')[0]

        for dt in datacolumn_list:
            for col, sd_pol in izip(correlation_type, sd_correlation_type):
                num_tt=0
                for time in tt:
                    for spwin in spw_list:
                        trange = qa.time(me.epoch('ref','%fs' % time)['m0'], prec=8, form='ymd')[0]
                        sd = sdstat(infile=self.msfile2_asap, timerange=str(trange), pol=sd_pol, spw=spwin)
                        v2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reporting_axes[0], correlation=col, datacolumn=dt, spw=spwin, intent=intent_list[0])
                        v2_keys=v2.keys()
                        for check in check_list:
                            if(abs((sd[check]-v2[str(v2_keys[0])][check])/sd[check]) > 0.0001):
                                retValue['success']= False
                                retValue['error_msgs']=retValue['error_msgs']
                            # +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='integragtion')"

                        num_tt +=1
                        if num_tt==3:
                            break

        self.assertTrue(retValue['success'],retValue['error_msgs'])



    def test14(self):
        '''Visstat2 14: Test using reportingaxes=integration, datacolumn=float_data, intent=on'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   
       
        correlation_type=['RR', 'LL']
        sd_correlation_type=['0', '1']
        datacolumn_list=['float_data']
        spw_list=['0', '1', '2', '3']
        reporting_axes=['integration']
        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']

        intent_list=['OBSERVE_TARGET#ON_SOURCE,POSITION_SWITCH']
        #intent_list=['OBSERVE_TARGET#OFF_SOURCE,POSITION_SWITCH']
        tb.open(self.msfile2)
        ref=tb.getcolkeyword('TIME','MEASINFO')['Ref']
        tt=tb.getcol('TIME')
        tb.close()
        
        trange = qa.time(me.epoch('ref','%fs' % tt[0])['m0'], prec=8, form='ymd')[0]

        for dt in datacolumn_list:
            for col, sd_pol in izip(correlation_type, sd_correlation_type):
                num_tt=0
                for time in tt:
                    for spwin in spw_list:
                        trange = qa.time(me.epoch('ref','%fs' % time)['m0'], prec=8, form='ymd')[0]
                        sd = sdstat(infile=self.msfile2_asap, timerange=str(trange), pol=sd_pol, spw=spwin)
                        v2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reporting_axes[0], correlation=col, datacolumn=dt, spw=spwin, intent=intent_list[0])
                        v2_keys=v2.keys()
                        for check in check_list:
                            print sd[check]
                            print v2[str(v2_keys[0])][check]
                            if(abs((sd[check]-v2[str(v2_keys[0])][check])/sd[check]) > 0.0001):
                                retValue['success']= False
                                retValue['error_msgs']=retValue['error_msgs']
                            # +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='integragtion')"

                        num_tt +=1
                        if num_tt==3:
                            break

        self.assertTrue(retValue['success'],retValue['error_msgs'])








    """    
    def test15(self):#(<-test11)
        '''Visstat2 15: Test using reportingaxes=field, datacolumn=model, intent=on'''

        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   
       
        correlation_type=['RR', 'LL']
        #sd_correlation_type=['0', '1']
        datacolumn_list=['model']
        spw_list=['0']
        reporting_axes=['field']
        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']
        intent_list=['OBSERVE_TARGET#ON_SOURCE,POSITION_SWITCH']
        field_list=['0','1','2']


        for dt in datacolumn_list:
            for col in correlation_type:
                for fd in field_list:
                    for spwin in spw_list:
                        v1 = visstat(vis=self.msfile, axis='amp',datacolumn=dt,correlation=col, field=fd, spw=spwin)
                        v2 = visstat2(vis=self.msfile, axis='amp', datacolumn=dt,correlation=col, spw=spwin,
                                      intent=intent_list[0],reportingaxes=reporting_axes[0])
                        for check in check_list:
                            print v1[dt.upper()][check], dt, col, fd, spwin, check
                            print v2['FIELD_ID='+ fd][check],'FIELD_ID='+fd
                            if(abs((v1[dt.upper()][check]-v2['FIELD_ID='+ fd][check])) > 0.0001):
                                retValue['success']= False
                                retValue['error_msgs']=retValue['error_msgs']
                             #+"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='integragtion')"


        self.assertTrue(retValue['success'],retValue['error_msgs'])
    """


    """    
    def test16(self):#(<-test10)
        '''Visstat2 16: Test using reportingaxes=field, datacolumn=corrected, intent=on'''

        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   
       
        correlation_type=['RR', 'LL']
        #sd_correlation_type=['0', '1']
        datacolumn_list=['corrected']
        spw_list=['0']
        reporting_axes=['field']
        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']
        intent_list=['OBSERVE_TARGET#ON_SOURCE,POSITION_SWITCH']
        field_list=['0','1','2']


        for dt in datacolumn_list:
            for col in correlation_type:
                for fd in field_list:
                    for spwin in spw_list:
                        v1 = visstat(vis=self.msfile, axis='amp',datacolumn=dt,correlation=col, field=fd, spw=spwin)
                        v2 = visstat2(vis=self.msfile, axis='amp', datacolumn=dt,correlation=col, spw=spwin,
                                      intent=intent_list[0],reportingaxes=reporting_axes[0])
                        for check in check_list:
                            print v1[dt.upper()][check], dt, col, fd, spwin, check
                            print v2['FIELD_ID='+ fd][check],'FIELD_ID='+fd
                            if(abs((v1[dt.upper()][check]-v2['FIELD_ID='+ fd][check])) > 0.0001):
                                retValue['success']= False
                                retValue['error_msgs']=retValue['error_msgs']
                             #+"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='integragtion')"


        self.assertTrue(retValue['success'],retValue['error_msgs'])
"""

    def test17(self):
        '''Visstat2 17: Test using reportingaxes=field'''

        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   

        datacolumn_list=['data', 'corrected', 'model']
        correlation_type=['RR', 'LL']
        field_list=['0','1','2']
        spw_list=['0']
        axis_list=['amp', 'scan_number']
        field_list=['0','1','2']
        useflags_list=[True, False]
        reporting_axes=['field']
        #intent_list=['OBSERVE_TARGET#ON_SOURCE,POSITION_SWITCH']
        #intent_list=['OBSERVE_TARGET#OFF_SOURCE,POSITION_SWITCH']

        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']
                
        for ax in axis_list:
            for col in correlation_type:
                for fd in field_list:
                    for spwin in spw_list:
                        for fg in useflags_list:

                            if(ax=='scan_number'):
                                v1 = visstat(vis=self.msfile, axis=ax,correlation=col, field=fd, spw=spwin, useflags=fg)
                                v2 = visstat2(vis=self.msfile, axis=ax, useflags=fg, correlation=col, 
                                      spw=spwin, reportingaxes=reporting_axes[0])

                                for check in check_list:
                                    v1_first_element = ax

                                    print v1[ v1_first_element.upper()][check], dt, col, fd, spwin, check
                                    print v2['FIELD_ID='+ fd][check],'FIELD_ID='+fd
                                    if(abs((v1[ v1_first_element.upper()][check]-v2['FIELD_ID='+ fd][check])) > 0.0001):
                                        retValue['success']= False
                                        retValue['error_msgs']=retValue['error_msgs']

                            if(ax=='amp'):            
                                for dt in datacolumn_list:    
                                    v1 = visstat(vis=self.msfile, axis=ax,datacolumn=dt,correlation=col, field=fd, spw=spwin, useflags=fg)
                                    v2 = visstat2(vis=self.msfile, axis=ax, useflags=fg, datacolumn=dt, correlation=col, 
                                      spw=spwin, reportingaxes=reporting_axes[0])
                                
                                    for check in check_list:
                                        v1_first_element = dt
                                        print v1[ v1_first_element.upper()][check], dt, col, fd, spwin, check
                                        print v2['FIELD_ID='+ fd][check],'FIELD_ID='+fd
                                        if(abs((v1[ v1_first_element.upper()][check]-v2['FIELD_ID='+ fd][check])) > 0.0001):
                                            retValue['success']= False
                                            retValue['error_msgs']=retValue['error_msgs']
                             #+"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='integragtion')"
                             
        self.assertTrue(retValue['success'],retValue['error_msgs'])









    def test18(self):
        '''Visstat2 18: Test using reportingaxes=field, datacolumn=corrected, intent=on'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   
       
        correlation_type=['RR','LL']
        sd_correlation_type=['0', '1']
        datacolumn_list=['corrected']
        #spw_list=['0', '1', '3']
        spw_list=['0']
        field_list=['1']
        reporting_axes=['field']
        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']

        for dt in datacolumn_list:
            for col, sd_pol in izip(correlation_type, sd_correlation_type):
                    for fd in field_list:
                        v2_intent_on = visstat2(vis=self.msfile3, axis='real',reportingaxes=reporting_axes[0], 
                                         correlation=col, datacolumn=dt, intent='OBSERVE_TARGET#ON_SOURCE',field=fd)
                        sd_intent_on  = sdstat(infile=self.msfile4, pol=sd_pol, field=fd)


                        v2_intent_off = visstat2(vis=self.msfile5, axis='real',reportingaxes=reporting_axes[0], 
                                         correlation=col, datacolumn=dt, intent='OBSERVE_TARGET#OFF_SOURCE',field=fd)
                        sd_intent_off  = sdstat(infile=self.msfile6, pol=sd_pol, field=fd)
                        
                        
                        for check in check_list:
                            print ''
                            print 'check on', check
                            print sd_intent_on[check]
                            print v2_intent_on['FIELD_ID='+ fd][check]
                            
                            print 'check off', check
                            print sd_intent_off[check]
                            print v2_intent_off['FIELD_ID='+ fd][check]
                            
                            
                            if(abs((sd_intent_on[check]-v2_intent_on['FIELD_ID='+ fd][check])/sd_intent_on[check]) > 0.0001):
                                #print abs((sd_intent_on[check]-v2_intent_on['FIELD_ID='+ fd][check])/sd_intent_on[check])
                                retValue['success']= False
                                retValue['error_msgs']=retValue['error_msgs']
                            # +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='integragtion')"
                                

                            if(abs((sd_intent_off[check]-v2_intent_off['FIELD_ID='+ fd][check])/sd_intent_off[check]) > 0.0001):
                                print abs((sd_intent_off[check]-v2_intent_off['FIELD_ID='+ fd][check])/sd_intent_off[check])
                                retValue['success']= False
                                retValue['error_msgs']=retValue['error_msgs']
                            # +"\nError: Failed with visstat2(vis=msfile, axis='amp', reportingaxes='integragtion')"    

        self.assertTrue(retValue['success'],retValue['error_msgs'])








class visstat2_cleanup(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case the files don't exist
        shutil.rmtree(self.msfile,ignore_errors=True)
        
    def test1a(self):
        '''Visstat2: Cleanup'''
        pass

def suite():
    return [visstat2_test]
    
    
