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
import numpy as np

#     Functional tests of visstat2 

epsilon = 0.0001

# Path for data
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/visstat2/"

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/visstat2/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR

print 'visstat2 tests will use data from '+datapath         

class visstat2_test(unittest.TestCase):
    def setUp(self):
        self.msfile = "ngc5921_add_corect_model.ms"
        self.msfile2 ="OrionS_rawACSmod_calave.ms"
        self.msfile2_asap="OrionS_rawACSmod_calave.asap"
        self.msfile3="OrionS_rawACSmod_calave_intent_on1_off3.ms"
        self.msfile4="OrionS_rawACSmod_calave_intent_on1.ms"
        self.msfile5="OrionS_rawACSmod_calave_intent_on3_off1.ms"
        self.msfile6="OrionS_rawACSmod_calave_intent_off1.ms"
        self.msfile7="visstat2_test6_scan.txt"
        self.msfile8="visstat2_test6_amp.txt"
        self.msfile9="visstat2_test5.txt"
        self.msfile10="visstat2_test7.txt"
        self.msfile11="visstat2_test8.txt"
        self.msfile13='visstat2_test10_check_on.txt'
        self.msfile14='visstat2_test10_check_off.txt'

        shutil.copytree(datapath+self.msfile, self.msfile)
        shutil.copytree(datapath+self.msfile2, self.msfile2)
        shutil.copytree(datapath+self.msfile2_asap, self.msfile2_asap)
        shutil.copytree(datapath+self.msfile3, self.msfile3)
        shutil.copytree(datapath+self.msfile4, self.msfile4)
        shutil.copytree(datapath+self.msfile5, self.msfile5)
        shutil.copytree(datapath+self.msfile6, self.msfile6)
        shutil.copyfile(datapath+self.msfile7, self.msfile7)
        shutil.copyfile(datapath+self.msfile8, self.msfile8)
        shutil.copyfile(datapath+self.msfile9, self.msfile9)
        shutil.copyfile(datapath+self.msfile10, self.msfile10)
        shutil.copyfile(datapath+self.msfile11, self.msfile11)
        shutil.copyfile(datapath+self.msfile13, self.msfile13)
        shutil.copyfile(datapath+self.msfile14, self.msfile14)

        default('visstat2')

    def tearDown(self):
        shutil.rmtree(self.msfile)
        shutil.rmtree(self.msfile2)
        shutil.rmtree(self.msfile2_asap)
        shutil.rmtree(self.msfile3)
        shutil.rmtree(self.msfile4) 
        shutil.rmtree(self.msfile5)
        shutil.rmtree(self.msfile6)
        os.remove(self.msfile7)
        os.remove(self.msfile8)
        os.remove(self.msfile9)
        os.remove(self.msfile10)
        os.remove(self.msfile11)
        os.remove(self.msfile13) 
        os.remove(self.msfile14)

    def compare(self, a, b):
        for d1, d2 in izip(a,b):
            if(d1.split(':')[0]==d2.split(':')[0]):
                if(not np.allclose(np.array([float(d1.split(':')[1])]), np.array([float(d2.split(':')[1])]))):
                    raise Exception(d1.split(':')[0] + ' ' + 'values are not consistent!')


    def test01(self):
        '''Visstat2 01: Default values'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
    
        expected = {self.msfile:
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
                                        'quartile': 0.3291134536266327,
                                        'rms': 17.081207832906546,
                                        'stddev': 16.382008276126726,
                                        'sum': 12871502.415939873,
                                        'sumsq': 776391995.3973862,
                                        'variance': 268.3701951590845}}}

        v2 = visstat2(vis=self.msfile, axis='amp', datacolumn='data', reportingaxes='ddid')

        if v2.keys() != expected[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], v2)
            raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], v2))

                            


        if not v2.has_key('DATA_DESC_ID=0'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat2 does not have key DATA_DESC_ID=0"
            raise Exception("Dictionary returned from visstat2 does not have key DATA_DESC_ID=0")

        for e in expected[self.msfile]['DATA_DESC_ID=0'].keys():
            print ''
            print "Checking %s: %s vs %s" % \
                   (e, expected[self.msfile]['DATA_DESC_ID=0'][e], v2['DATA_DESC_ID=0'][e])
            failed = False
            
            if type(expected[self.msfile]['DATA_DESC_ID=0'][e])==bool:
                if expected[self.msfile]['DATA_DESC_ID=0'][e] != v2['DATA_DESC_ID=0'][e]:
                    failed = True
            elif abs((expected[self.msfile]['DATA_DESC_ID=0'][e] - v2['DATA_DESC_ID=0'][e])/expected[self.msfile]['DATA_DESC_ID=0'][e]) > epsilon:
                failed = True

            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Numbers differ, expected %s, got %s" % \
                        ( str(v2['DATA_DESC_ID=0'][e]), str(expected[self.msfile]['DATA_DESC_ID=0'][e]) )

        self.assertTrue(retValue['success'],retValue['error_msgs'])



    def test02(self):     
        '''Visstat2 02: Check channel selections, useflags=True, repotingaxes='ddid',correlation=corr, datacolumn=data, axis=amp'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        for ch in [1, 2, 4, 7, 13, 62]:
          for corr in ['ll', 'rr', 'll,rr']:
            print "Call with spw='0:1~"+str(ch)+"', correlation="+corr
            s2 = visstat2(vis=self.msfile, axis='amp', datacolumn='data', spw='0:1~'+str(ch), correlation=corr, reportingaxes='ddid', useflags=True)
            print ''
            print 's2', s2
            n_expected = 2660994/63 * ch   
            if corr in ['ll', 'rr']:
                n_expected /= 2
            n = int(s2['DATA_DESC_ID=0']['npts'])
            print "Checking npts: %s vs %s" % (n, n_expected)
            if n != n_expected:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError:"+str(n_expected) + " points expected, but npts = " + str(n)
                raise Exception(str(n_expected) + " points expected, but npts = " + str(n))
        self.assertTrue(retValue['success'],retValue['error_msgs']) 



    def test03(self):
        '''Visstat2 03: Default values with datacolum=model, reporingaxis=ddid'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        expected = {self.msfile:
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

        v2 = visstat2(vis=self.msfile, axis='amp', datacolumn='model', reportingaxes='ddid')
        if v2.keys() != expected[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], v2)

        if not v2.has_key('DATA_DESC_ID=0'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat2 does not have key DATA_DESC_ID=0"
            raise Exception("Dictionary returned from visstat2 does not have key DATA_DESC_ID=0")

        for e in expected[self.msfile]['DATA_DESC_ID=0'].keys():
            print ''
            print "Checking %s: %s vs %s" % \
                   (e, expected[self.msfile]['DATA_DESC_ID=0'][e], v2['DATA_DESC_ID=0'][e])
            failed = False
            if expected[self.msfile]['DATA_DESC_ID=0'][e] == 0:
                if v2['DATA_DESC_ID=0'][e] != 0:
                    failed = True
            else:
                if abs((expected[self.msfile]['DATA_DESC_ID=0'][e] - v2['DATA_DESC_ID=0'][e])/expected[self.msfile]['DATA_DESC_ID=0'][e]) > epsilon:
                    failed = True
            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected2 %s, got %s" % \
                      (str(expected[self.msfile]['DATA_DESC_ID=0'][e]), str(v2['DATA_DESC_ID=0'][e]))

        self.assertTrue(retValue['success'],retValue['error_msgs'])



    def test04(self):
        '''Visstat2 04: Test of special cases'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }     

        for a in range(1, 5):
            s2 = visstat2(vis=self.msfile, axis='ANTENNA1', antenna=str(a)+'&26')
            print ''
            print "antenna =", a, "; mean = ", s2['DATA_DESC_ID=0']['mean']

            # Note there's a counting from 0 or 1 issue here
            # with the antenna numbering
            if self.msfile == self.msfile:
                offset = 1
            else:
                offset = 0

            if abs((s2['DATA_DESC_ID=0']['mean']+offset) - a) > epsilon:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Failed when antenna= "+str(a)+'&26'
                raise Exception("Error!")

        for scan in range(1, 8):
            s2 = visstat2(vis=self.msfile, axis='scan_number', scan=str(scan)) 
            if abs(s2['DATA_DESC_ID=0']['mean'] - scan) > epsilon:        
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Failed for scan = "+str(scan)
                raise Exception("Error!")

        self.assertTrue(retValue['success'],retValue['error_msgs'])



    def test05(self):
        '''Visstat2 05: Test using reportingaxes=integration, datacolumn=float_data'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   
       
        correlation_type=['RR', 'LL']
        sd_correlation_type=['0', '1']
        datacolumn_list=['float_data']
        spw_list=['0', '1', '2', '3']
        reporting_axes=['integration']
        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']
        intent_list=['']

        tb.open(self.msfile2)
        ref=tb.getcolkeyword('TIME','MEASINFO')['Ref']
        tt=tb.getcol('TIME')
        tb.close()

        result_list=[]

        for dt in datacolumn_list:
            for col, sd_pol in izip(correlation_type, sd_correlation_type):
                num_tt=0
                for time in tt:
                    for spwin in spw_list:
                        trange = qa.time(me.epoch('ref','%fs' % time)['m0'], prec=8, form='ymd')[0]
                        v2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reporting_axes[0], 
                                      correlation=col, datacolumn=dt, spw=spwin, intent=intent_list[0])
                        v2_keys=v2.keys()
                        for check in check_list:
                            print check, v2[str(v2_keys[0])][check]
                            result_list.append(check+':' + str(v2[str(v2_keys[0])][check]))

                        num_tt +=1
                        if num_tt==3:
                            break

        f=open('visstat2_test5.txt','r').read()
        self.compare(np.array(result_list), np.array(f[:-1].split(' ')))



    def test06(self):
        '''Visstat2 06: Test using reportingaxes=field'''

        datacolumn_list=['data', 'corrected', 'model']
        correlation_type=['RR', 'LL']
        field_list=['0','1','2']
        spw_list=['0']
        axis_list=['amp', 'scan_number']
        field_list=['0','1','2']
        useflags_list=[True, False]
        reporting_axes=['field']
        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']

        ax_scan=[]
        ax_amp=[]

        for ax in axis_list:
            for col in correlation_type:
                for fd in field_list:
                    for spwin in spw_list:
                        for fg in useflags_list:

                            if(ax=='scan_number'):
                                v2 = visstat2(vis=self.msfile, axis=ax, useflags=fg, correlation=col, 
                                      spw=spwin, reportingaxes=reporting_axes[0])
                                for check in check_list:
                                    ax_scan.append(check+':'+str(v2['FIELD_ID='+ fd][check]))

                            if(ax=='amp'):            
                                for dt in datacolumn_list:    
                                    v2 = visstat2(vis=self.msfile, axis=ax, useflags=fg, datacolumn=dt, correlation=col, 
                                      spw=spwin, reportingaxes=reporting_axes[0])
                                    for check in check_list:
                                        ax_amp.append(check+':'+str(v2['FIELD_ID='+ fd][check]))

        f_scan=open('visstat2_test6_scan.txt','r').read()
        f_amp=open('visstat2_test6_amp.txt','r').read()

        #check when ax=scan
        self.compare(np.array(ax_scan),np.array(f_scan[:-1].split(' ')))
        #check when ax~amp
        self.compare(np.array(ax_amp),np.array(f_amp[:-1].split(' ')))



    def test07(self):
        '''Visstat2 07: Default values with datacolum=corrected, reporingaxis=ddid'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        
        expected = {self.msfile:
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
                                        'quartile': 0.3291134536266327,
                                        'rms': 17.081207832906546,
                                        'stddev': 16.382008276126726,
                                        'sum': 12871502.415939873,
                                        'sumsq': 776391995.3973862,
                                        'variance': 268.3701951590845}}}
        
        v2 = visstat2(vis=self.msfile, axis='amp', datacolumn='corrected', reportingaxes='ddid')
        if v2.keys() != expected[self.msfile].keys():
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], v2)
            raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[self.msfile], v2))
 
        if not v2.has_key('DATA_DESC_ID=0'):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Dictionary returned from visstat2 does not have key DATA_DESC_ID=0"
            raise Exception("Dictionary returned from visstat does not have key DATA_DESC_ID=0")

        for e in expected[self.msfile]['DATA_DESC_ID=0'].keys():
            print ''
            print "Checking %s: %s vs %s" % \
                   (e, expected[self.msfile]['DATA_DESC_ID=0'][e], v2['DATA_DESC_ID=0'][e])
            failed = False
            if expected[self.msfile]['DATA_DESC_ID=0'][e] == 0:
                if v2['DATA_DESC_ID=0'][e] != 0:
                    failed = True
            else:
                if abs((expected[self.msfile]['DATA_DESC_ID=0'][e] - v2['DATA_DESC_ID=0'][e])/expected[self.msfile]['DATA_DESC_ID=0'][e]) > epsilon:
                    failed = True
            if failed:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Numbers differ, expected2 %s, got %s" % \
                      (str(expected[self.msfile]['DATA_DESC_ID=0'][e]), str(v2['DATA_DESC_ID=0'][e]))


        self.assertTrue(retValue['success'],retValue['error_msgs'])



    def test08(self):
        '''Visstat2 08: Test when using reportingaxes='integration, datacolumn=data,corrected,model'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   

        clearcal(self.msfile, addmodel=True)
        correlation_type=['','LL','RR']
        datacolumn_list=['data', 'corrected', 'model']

        tb.open(self.msfile)
        ref=tb.getcolkeyword('TIME','MEASINFO')['Ref']
        tt=tb.getcol('TIME')
        tb.close()
        
        trange = qa.time(me.epoch('ref','%fs' % tt[0])['m0'], prec=8, form='ymd')[0]
        s2 = visstat2(vis=self.msfile, axis='amp', timerange=str(trange),reportingaxes='integration')
        s2_keys=s2.keys()

        check_list=['rms', 'medabsdevmed', 'min', 'max', 'sum', 'median', 'sumsq', 'stddev', 'variance', 'npts', 'mean']
        result_list=[]

        for dt in datacolumn_list:
            for col in correlation_type:
                num_tt=0
                for time in tt:
                    trange = qa.time(me.epoch('ref','%fs' % time)['m0'], prec=8, form='ymd')[0]
                    s2 = visstat2(vis=self.msfile, axis='amp', timerange=str(trange),reportingaxes='integration', correlation=col, datacolumn=dt)
                    for check in check_list:
                        result_list.append(check+':' + str(s2[str(s2_keys[0])][check]))

                    num_tt +=1
                    if num_tt==10:
                        break

        f=open('visstat2_test8.txt','r').read()
        self.compare(np.array(result_list), np.array(f[:-1].split(' ')))



    def test09(self):
        '''Visstat2 09: Test using reportingaxes=ddid, correlation=[LL,RR], datacolumn=float_data spw=[0,1,2,3]'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   

        correlation_type=['RR', 'LL']
        sd_correlation_type=['0', '1']
        datacolumn_list=['float_data']
        spw_list=['0', '1', '2', '3']
        reporting_axes=['ddid']
        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']
        intent_list=['']

        tb.open(self.msfile2)
        ref=tb.getcolkeyword('TIME','MEASINFO')['Ref']
        tt=tb.getcol('TIME')
        tb.close()

        result_list=[]

        for dt in datacolumn_list:
            for col, sd_pol in izip(correlation_type, sd_correlation_type):
                num_tt=0
                for time in tt:
                    for spwin in spw_list:
                        trange = qa.time(me.epoch('ref','%fs' % time)['m0'], prec=8, form='ymd')[0]
                        v2 = visstat2(vis=self.msfile2, axis='amp', timerange=str(trange),reportingaxes=reporting_axes[0], 
                                      correlation=col, datacolumn=dt, spw=spwin, intent=intent_list[0])
                        v2_keys=v2.keys()
                        for check in check_list:
                            result_list.append(check+':' + str(v2[str(v2_keys[0])][check]))
                        num_tt +=1
                        if num_tt==3:
                            break

        f=open('visstat2_test7.txt','r').read()
        self.compare(np.array(result_list), np.array(f[:-1].split(' ')))



    def test10(self):
        '''Visstat2 10: Test using reportingaxes=field, datacolumn=corrected, intent=[on,off]'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }   
       
        correlation_type=['RR','LL']
        sd_correlation_type=['0', '1']
        datacolumn_list=['corrected']
        spw_list=['0']
        field_list=['1']
        reporting_axes=['field']
        check_list=['rms', 'max', 'min', 'sum', 'median', 'stddev', 'mean']

        intent_on=[]
        intent_off=[]

        for dt in datacolumn_list:
            for col, sd_pol in izip(correlation_type, sd_correlation_type):
                    for fd in field_list:
                        v2_intent_on = visstat2(vis=self.msfile3, axis='real',reportingaxes=reporting_axes[0], 
                                         correlation=col, datacolumn=dt, intent='OBSERVE_TARGET#ON_SOURCE',field=fd)
                        v2_intent_off = visstat2(vis=self.msfile5, axis='real',reportingaxes=reporting_axes[0], 
                                         correlation=col, datacolumn=dt, intent='OBSERVE_TARGET#OFF_SOURCE',field=fd)
                        for check in check_list:
                            print ''
                            print 'check intent on', check
                            print v2_intent_on['FIELD_ID='+ fd][check]
                           
                            print 'check intent off', check
                            print v2_intent_off['FIELD_ID='+ fd][check]
                        
                            intent_on.append(check+':'+str(v2_intent_on['FIELD_ID='+ fd][check]))
                            intent_off.append(check+':'+str(v2_intent_off['FIELD_ID='+ fd][check]))

        f_intent_on=open('visstat2_test10_check_on.txt','r').read()
        f_on_split=f_intent_on[:-1].split(' ')
        self.compare(np.array(f_on_split), np.array(intent_on))
        
        f_intent_off=open('visstat2_test10_check_off.txt','r').read()
        f_off_split=f_intent_off[:-1].split(' ')
        self.compare(np.array(f_off_split), np.array(intent_off))


def suite():
    return [visstat2_test]
    
    
