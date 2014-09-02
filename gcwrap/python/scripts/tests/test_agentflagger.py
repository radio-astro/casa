import shutil
import unittest
import os
import filecmp
from tasks import *
from taskinit import *
from __main__ import default


class test_base(unittest.TestCase):

    def setUp_bpass_case(self):
        self.vis = "cal.fewscans.bpass"

        if os.path.exists(self.vis):
            print "The CalTable is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                        os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        
        self.unflag_table()


    def setUp_tsys_case(self):
        self.vis = "X7ef.tsys"
         
        if os.path.exists(self.vis):
            print "The CalTable is already around, just unflag"
            
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                        os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')

        self.unflag_table()


    def setUp_4Ants(self):
        self.vis = "Four_ants_3C286.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                         os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')

        self.unflag_table()


    def setUp_CAS_4052(self):
        self.vis = "TwoSpw.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                        os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_table()

    def setUp_alma_ms(self):
        '''ALMA MS, scan=1,8,10 spw=0~3 4,128,128,1 chans, I,XX,YY'''
        self.vis = "uid___A002_X30a93d_X43e_small.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                        os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_table()
        
    def setUp_float_data(self):
        '''Single-dish MS'''
        self.vis = "SDFloatColumn.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                        os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_table()
        
    def unflag_table(self):

        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.init()
        aflocal.run(writeflags=True)
        aflocal.done()


class test_tsys(test_base):
    """AgentFlagger:: Test flagging tool with Tsys-based CalTable """
    
    def setUp(self):
         self.setUp_tsys_case()

    def test_unsupported_elevation_tsys(self):
        '''AgentFlagger: Unsupported elevation mode'''
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        aflocal.parseelevationparameters()
        aflocal.init()
        res = aflocal.run(writeflags=True)
        aflocal.done() 
        self.assertEqual(res, {})

    def test_mixed_agents_tsys(self):
        '''AgentFlagger: supported and unsupported agents in a list'''
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        aflocal.parsemanualparameters(spw='1')
        aflocal.parseagentparameters({'mode':'shadow','spw':'3'}) #unsupported mode
        aflocal.parseelevationparameters()     # unsupported mode
        aflocal.parsemanualparameters(spw='5')
        aflocal.parsesummaryparameters(spw='1,3,5')
        aflocal.init()
        res = aflocal.run(writeflags=True)
        aflocal.done() 
        self.assertEqual(res['report0']['spw']['1']['flagged'], 32256)
        self.assertEqual(res['report0']['spw']['3']['flagged'], 0)
        self.assertEqual(res['report0']['spw']['5']['flagged'], 32256)
        self.assertEqual(res['report0']['flagged'], 32256*2)
        
    def test_manual_field_selection_agent_layer_for_tsys_CalTable(self):
        """AgentFlagger:: Manually flag a Tsys-based CalTable using flag agent selection engine for field """
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','field':'0'}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['field']['3c279']['flagged'], 9216)
        self.assertEqual(summary['report0']['field']['Titan']['flagged'], 0)
        self.assertEqual(summary['report0']['field']['TW Hya']['flagged'], 0)
        self.assertEqual(summary['report0']['field']['J1037-295=QSO']['flagged'], 0)

    def test_manual_antenna_selection_agent_layer_for_tsys_CalTable(self):
        """AgentFlagger:: Manually flag a Tsys-based CalTable using flag agent selection engine for antenna"""
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','antenna':'DV09'}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['antenna']['DV09']['flagged'], 14336)
        self.assertEqual(summary['report0']['antenna']['DV10']['flagged'], 0)

    def test_manual_antenna_selection_agent_layer_for_tsys_CalTable2(self):
        """AgentFlagger:: Manually flag a Tsys-based CalTable using flag agent selection engine for antenna"""

        # Run the previous test but using the specific parsing functions
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        aflocal.parsemanualparameters(apply=False)
        aflocal.parsemanualparameters(antenna='DV09')
        aflocal.parsesummaryparameters()
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['antenna']['DV09']['flagged'], 14336)
        self.assertEqual(summary['report0']['antenna']['DV10']['flagged'], 0)

    def test_manual_field_msSelection_layer_for_tsys_CalTable(self):
        """AgentFlagger:: Manually flag a Tsys-based CalTable using MSSelection for field """
        aflocal = casac.agentflagger()

        aflocal.open(self.vis)
        aflocal.selectdata(field='0')
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        aflocal.open(self.vis)
        aflocal.selectdata()
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['field']['3c279']['flagged'], 9216.0)
        self.assertEqual(summary['report0']['field']['Titan']['flagged'], 0)
        self.assertEqual(summary['report0']['field']['TW Hya']['flagged'], 0)
        self.assertEqual(summary['report0']['field']['J1037-295=QSO']['flagged'], 0)        

    def test_manual_antenna_msSelection_layer_for_tsys_CalTable(self):
        """AgentFlagger:: Manually flag a Tsys-based CalTable using MSSelection for antenna"""
        aflocal = casac.agentflagger()

        aflocal.open(self.vis)
        aflocal.selectdata(antenna='DV09')
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        aflocal.open(self.vis)
        aflocal.selectdata()
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['antenna']['DV09']['flagged'], 14336)
        self.assertEqual(summary['report0']['antenna']['DV10']['flagged'], 0)
        
    def test_clip_minmax_fparm_sol1(self):
        """AgentFlagger:: Test cliping first calibration solution product of FPARAM column using a minmax range """
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        correlation = 'Sol1'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024.0)
        self.assertEqual(summary['report0']['flagged'], 750.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512.0)

    def test_clip_minmax_fparm_sol1_2(self):
        """AgentFlagger:: Test cliping first calibration solution product of FPARAM column using a minmax range """

        # Run the previous test but using the specific parsing functions
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        correlation = 'Sol1'
        aflocal.open(self.vis)
        aflocal.selectdata()
        aflocal.parsemanualparameters(apply=False)
        aflocal.parseclipparameters(clipzeros=True,clipminmax=[0.,600.],datacolumn=datacolumn,
                                     correlation=correlation)
        aflocal.parsesummaryparameters()
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 
        self.assertEqual(summary['report0']['total'], 129024)
        self.assertEqual(summary['report0']['flagged'], 750)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 750)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512)

    def test_clip_minmax_fparm_sol1_extension(self):
        """AgentFlagger:: Test cliping first calibration solution product of FPARAM column using a minmax range, and then extend to the other solution """
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':'Sol1'}
        agentExtension={'apply':True,'mode':'extend','extendpols':True,'growfreq':0.0, 'growtime':0.0}
        agentSummary={'apply':True,'mode':'summary','correlation':'""'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentExtension)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024)
        self.assertEqual(summary['report0']['flagged'], 1500)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 750)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 750)

    def test_clip_minmax_fparm_sol2(self):
        """AgentFlagger:: Test cliping second calibration solution product of FPARAM column using a minmax range """
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        correlation = 'Sol2'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024.0)
        self.assertEqual(summary['report0']['flagged'], 442.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 442.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512.0)

    def test_clip_minmax_fparm_sol1sol2(self):
        """AgentFlagger:: Test cliping first and second calibration solution products of FPARAM column using a minmax range """
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        correlation = 'Sol1,Sol2'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024.0)
        self.assertEqual(summary['report0']['flagged'], 1192.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 442.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512.0)

    def test_clip_minmax_fparm_all(self):
        """AgentFlagger:: Test cliping all calibration solution products of FPARAM column using a minmax range """
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        correlation = ''
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024.0)
        self.assertEqual(summary['report0']['flagged'], 1192.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 442.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512.0)

    def test_clip_zeros_fparm_all(self):
        """AgentFlagger:: Test cliping only zeros in all calibration solution products of FPARAM column"""
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        correlation = ''
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024.0)
        self.assertEqual(summary['report0']['flagged'], 126.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 56.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 70.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512.0)

    def test_clip_nan_and_inf_fparm_all(self):
        """AgentFlagger:: Test cliping only NaNs/Infs in all calibration solution products of FPARAM column"""
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        correlation = ''
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024.0)
        self.assertEqual(summary['report0']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512.0)

    def test_clip_minmax_fparm_error_case_absall(self):
        """AgentFlagger:: Error case test when a complex operator is used with CalTables """
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        correlation = 'ABS_ALL'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024.0)
        self.assertEqual(summary['report0']['flagged'], 1192.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 442.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512.0)

 
    def test_clip_minmax_snr_all_for_tsys_CalTable(self):
        """AgentFlagger:: Test cliping all calibration solution products of SNR column using a minmax range for Tsys CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'SNR'
        correlation = ''
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,2.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024.0)
        self.assertEqual(summary['report0']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512.0)


class test_bpass(test_base):
    """AgentFlagger:: Test flagging tool with Bpass-based CalTable """
    
    def setUp(self):
        self.setUp_bpass_case()

    def test_default_cparam_bpass(self):
        '''Flagdata: flag CPARAM data column'''
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        aflocal.parseclipparameters(clipzeros=True,datacolumn='CPARAM')
        aflocal.parsesummaryparameters()
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 
        self.assertEqual(summary['report0']['flagged'], 11078.0, 'Should use CPARAM as the default column')

    def test_manual_field_selection_agent_layer_for_bpass_CalTable(self):
        """AgentFlagger:: Manually flag a bpass-based CalTable using flag agent selection engine for field """
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','field':'3C286_A'}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['field']['3C286_A']['flagged'], 499200.0)
        self.assertEqual(summary['report0']['field']['3C286_B']['flagged'], 0)
        self.assertEqual(summary['report0']['field']['3C286_C']['flagged'], 0)
        self.assertEqual(summary['report0']['field']['3C286_D']['flagged'], 0)

    def test_manual_antenna_selection_agent_layer_for_bpass_CalTable(self):
        """AgentFlagger:: Manually flag a bpass-based CalTable using flag agent selection engine for antenna"""
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','antenna':'ea09'}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['antenna']['ea09']['flagged'], 48000.0)
        self.assertEqual(summary['report0']['antenna']['ea10']['flagged'], 0.0)

    def test_manual_field_msSelection_layer_for_bpass_CalTable(self):
        """AgentFlagger:: Manually flag a bpass-based CalTable using MSSelection for field """
        aflocal = casac.agentflagger()

        aflocal.open(self.vis)
        aflocal.selectdata({'field':'3C286_A'})
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        aflocal.open(self.vis)
        aflocal.selectdata()
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['field']['3C286_A']['flagged'], 499200.0)
        self.assertEqual(summary['report0']['field']['3C286_B']['flagged'], 0)
        self.assertEqual(summary['report0']['field']['3C286_C']['flagged'], 0)
        self.assertEqual(summary['report0']['field']['3C286_D']['flagged'], 0)

    def test_manual_antenna_msSelection_layer_for_bpass_CalTable(self):
        """AgentFlagger:: Manually flag a bpass-based CalTable using MSSelection for antenna"""
        aflocal = casac.agentflagger()

        aflocal.open(self.vis)
        aflocal.selectdata({'antenna':'ea09'})
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        aflocal.open(self.vis)
        aflocal.selectdata()
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['antenna']['ea09']['flagged'], 48000.0)
        self.assertEqual(summary['report0']['antenna']['ea10']['flagged'], 0.0)

    def test_clip_minmax_cparam_for_bpass(self):
        """AgentFlagger:: Clip all calibration solutions of CPARAM column using a minmax range"""
        aflocal = casac.agentflagger()
        datacolumn = 'CPARAM'
        aflocal.open(self.vis)
        aflocal.selectdata()
        aflocal.parseclipparameters(clipminmax=[0.,0.3],datacolumn=datacolumn,clipzeros=True)
        aflocal.parsesummaryparameters()
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 
        self.assertEqual(summary['report0']['flagged'], 11175.0)
        self.assertEqual(summary['report0']['total'], 1248000)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 11136.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 39)
        
    def test_clip_minmax_snr_all_for_bpass_CalTable(self):
        """AgentFlagger:: Test cliping all calibration solution products of SNR column using a minmax range for bpass CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'SNR'
        correlation = ''
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,550.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 1248000.0)
        self.assertEqual(summary['report0']['flagged'], 74371.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 36327.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 38044.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000.0)

    def test_rflag_cparam_sol2(self):
        """AgentFlagger:: Test rflag solution 2 of CPARAM column for bpass"""
        aflocal = casac.agentflagger()
        correlation = 'Sol2'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentRflag={'apply':True,'mode':'rflag','correlation':correlation,
                    'extendflags':False,'datacolumn':'CPARAM'}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentRflag)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 
        self.assertEqual(summary['report0']['flagged'], 13197)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 13197)

    def test_tfcrop_cparam_sol1_extension(self):
        """AgentFlagger:: Test tfcrop first calibration solution product of CPARAM column, 
        and then extend to the other solution for bpass CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = "CPARAM"
        correlation = 'Sol1'
        aflocal.open(self.vis)
        aflocal.selectdata()
        
        # Pre-clip data to avoid problems with near-zero values
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'datacolumn':datacolumn,'correlation':correlation}
        aflocal.parseagentparameters(agentClip)
        
        aflocal.parsetfcropparameters(datacolumn=datacolumn, correlation=correlation,
                                      extendflags=False)
        aflocal.parsesummaryparameters()
        # Extend to the other solution
        aflocal.parseextendparameters(extendpols=True,growfreq=0.0,growtime=0.0)
        aflocal.parsesummaryparameters()
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done()
        
        # flags from first summary, only tfcrop     
        assert abs(summary['report0']['flagged'] - 30427) <= 5        
        assert abs(summary['report0']['correlation']['Sol1']['flagged'] - 30427) <= 5
        
        # flags from second summary, tfcrop+extend
        assert abs(summary['report2']['flagged'] - 2*30427) <= 10
        assert abs(summary['report2']['correlation']['Sol2']['flagged'] - 30427) <= 5        

class test_MS(test_base):

    def setUp(self):
        self.setUp_alma_ms()

    def test_null_intent_selection1(self):
        '''Agentflagger: handle unknown scan intent in list mode'''
        
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        aflocal.parsemanualparameters(intent='FOCUS') # non-existing intent
        aflocal.parsemanualparameters(intent='CALIBRATE_POINTING*') # scan=1
        aflocal.parsemanualparameters(intent='CALIBRATE_AMPLI_ON_SOURC') # typo
        aflocal.parsemanualparameters(intent='*DELAY*') # non-existing
        aflocal.parsesummaryparameters()
        aflocal.init()
        summary = aflocal.run()
        aflocal.done()
        self.assertEqual(summary['report0']['scan']['1']['flagged'], 192416)
        self.assertEqual(summary['report0']['scan']['8']['flagged'], 0)
        self.assertEqual(summary['report0']['flagged'], 192416)
        
    def test_summarylist1(self):
        '''agentflagger: multiple summaries'''
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        aflocal.parsesummaryparameters(name='summary_1')
        aflocal.parsemanualparameters(intent='FOCUS') # non-existing intent
        aflocal.parsesummaryparameters(name='summary_2')
        aflocal.parsemanualparameters(spw='0') 
        aflocal.parsesummaryparameters(name='summary_3')
        aflocal.init()
        summary = aflocal.run()
        aflocal.done()
        
        # It creates 2 reports per summary agent. One is type='summary',
        # ther other is type='plotpoints'
        self.assertEqual(summary['report0']['name'],'summary_1')
        self.assertEqual(summary['report2']['name'],'summary_2')
        self.assertEqual(summary['report4']['name'],'summary_3')
        self.assertEqual(summary['report0']['flagged'], 0)
        self.assertEqual(summary['report2']['flagged'], 0)
        self.assertEqual(summary['report4']['spw']['0']['flagged'], summary['report4']['spw']['0']['total'])

             
class test_display(test_base):
    """AgentFlagger:: Automatic test to check basic behaviour of display GUI using pause=False option """

    def test_display_data_single_channel_selection(self):
        """AgentFlagger:: Check nominal behaviour for single spw:chan selection """
        self.setUp_4Ants()
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','spw':'*:20~40'}
        agentSummary={'apply':True,'mode':'summary'}
        agentDisplay={'mode':'display','datadisplay':True,'pause':False}
        aflocal.parseagentparameters(agentManual)
        aflocal.parseagentparameters(agentSummary)
        aflocal.parseagentparameters(agentDisplay)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done()

    def test_display_data_multiple_channel_selection(self):
        """AgentFlagger:: Check behaviour for multiple spw:chan selection """
        self.setUp_4Ants()
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','spw':'*:10~20;30~40'}
        agentSummary={'apply':True,'mode':'summary'}
        agentDisplay={'mode':'display','datadisplay':True,'pause':False}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.parseagentparameters(agentSummary)
        aflocal.parseagentparameters(agentDisplay)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done()

    def test_display_data_different_corrs_per_spw(self):
        """AgentFlagger:: Check behaviour when the number of correlation products changes between SPWs """
        self.setUp_CAS_4052()
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','spw':'*:100~200;300~400'}
        agentSummary={'apply':True,'mode':'summary'}
        agentDisplay={'mode':'display','datadisplay':True,'pause':False}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentManual)
        aflocal.parseagentparameters(agentSummary)
        aflocal.parseagentparameters(agentDisplay)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done()

    def test_display_cal_tables(self):
        '''AgentFlagger: Select spws, display and flag cal tables'''
        self.setUp_bpass_case()
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata(spw='1')
        aflocal.parsemanualparameters(spw='1:0~10;60~63')
        aflocal.parsesummaryparameters(spwchan=True)
#        agentManual={'apply':True,'mode':'manual'}
#        agentSummary={'apply':True,'mode':'summary','spwchan':True}
        agentDisplay={'mode':'display','datadisplay':True,'pause':False,'datacolumn':'CPARAM'}
#        aflocal.parseagentparameters(agentManual)
#        aflocal.parseagentparameters(agentSummary)
        aflocal.parseagentparameters(agentDisplay)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done()
        
        self.assertEqual(summary['report0']['spw:channel']['1:0']['flagged'], 1300)
        self.assertEqual(summary['report0']['spw:channel']['1:5']['flagged'], 1300)
        self.assertEqual(summary['report0']['spw:channel']['1:10']['flagged'], 1300)
        self.assertEqual(summary['report0']['spw:channel']['1:11']['flagged'], 0)
        self.assertEqual(summary['report0']['spw:channel']['1:60']['flagged'], 1300)
        self.assertEqual(summary['report0']['spw:channel']['1:59']['flagged'], 0)
        self.assertEqual(summary['report0']['flagged'], 15*1300)
        
    def test_display_float_data(self):
        '''AgentFlagger: Select spw, display and flag single-dish MS'''
        self.setUp_float_data()
        aflocal = casac.agentflagger()
        aflocal.open(self.vis)
        aflocal.selectdata(spw='1~4')
        aflocal.parsemanualparameters()
        aflocal.parsesummaryparameters(name='Single-dish')
        agentDisplay={'mode':'display','datadisplay':True,'pause':False,'datacolumn':'FLOAT_DATA'}
        aflocal.parseagentparameters(agentDisplay)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done()
        self.assertEqual(summary['report0']['flagged'], 2052)
    

# Dummy class which cleans up created files
class cleanup(test_base):
            
    def tearDown(self):
        os.system('rm -rf cal.fewscans.bpass*')
        os.system('rm -rf X7ef.tsys*')
        os.system('rm -rf Four_ants_3C286.ms*')
        os.system('rm -rf TwoSpw.ms*')

    def test1(self):
        '''AgentFlagger: Cleanup'''
        pass


def suite():
    return [test_tsys,
            test_bpass,
            test_display,
            test_MS,
            cleanup]
