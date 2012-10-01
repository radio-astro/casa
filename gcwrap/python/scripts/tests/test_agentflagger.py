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
        agentExtension={'apply':True,'mode':'extend','extendpols':True,'correlation':'ALL'}
        agentSummary={'apply':True,'mode':'summary','correlation':'ALL'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentExtension)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024)
        self.assertEqual(summary['report0']['flagged'], 1726)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 863)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 863)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512)

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
        correlation = 'ALL'
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
        correlation = 'ALL'
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
        self.assertEqual(summary['report0']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512.0)

    def test_clip_nan_and_inf_fparm_all(self):
        """AgentFlagger:: Test cliping only NaNs/Infs in all calibration solution products of FPARAM column"""
        aflocal = casac.agentflagger()
        datacolumn = 'FPARAM'
        correlation = 'ALL'
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
        correlation = 'ABS ALL'
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

    def test_clip_minmax_paramerr_all_for_tsys_CalTable(self):
        """AgentFlagger:: Test cliping all calibration solution products of PARAMERR column using a minmax range for Tsys CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,0.2],'datacolumn':datacolumn,'correlation':correlation}
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

    def test_clip_minmax_snr_all_for_tsys_CalTable(self):
        """AgentFlagger:: Test cliping all calibration solution products of SNR column using a minmax range for Tsys CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'SNR'
        correlation = 'ALL'
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
        
    def test_clip_zeros_paramerr_all_for_bpass_CalTable(self):
        """AgentFlagger:: Test cliping only zeros in all calibration solution products of PARAMERR column for bpass CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
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

        self.assertEqual(summary['report0']['total'], 1248000.0)
        self.assertEqual(summary['report0']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000.0)

    def test_clip_nan_and_inf_paramerr_all_for_bpass_CalTable(self):
        """AgentFlagger:: Test cliping only zeros in all calibration solution products of PARAMERR column for bpass CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
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

        self.assertEqual(summary['report0']['total'], 1248000.0)
        self.assertEqual(summary['report0']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000.0)

    def test_clip_minmax_paramerr_all_for_bpass_CalTable(self):
        """AgentFlagger:: Test cliping all calibration solution products of PARAMERR column using a minmax range for bpass CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0., 0.001],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentClip)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 
    
        self.assertEqual(summary['report0']['total'], 1248000.0)
        self.assertEqual(summary['report0']['flagged'], 253315.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 134252.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 119063.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000.0)

    def test_rflag_paramerr_all_for_bpass_CalTable(self):
        """AgentFlagger:: Test rflag in all calibration solution products of PARAMERR column for bpass CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentRflag={'apply':True,'mode':'rflag','datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentRflag)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 1248000.0)
        self.assertEqual(summary['report0']['flagged'], 184222.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 93880.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 90342.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000.0)

    def test_tfcrop_paramerr_all_for_bpass_CalTable(self):
        """AgentFlagger:: Test tfcrop in all calibration solution products of PARAMERR column for bpass CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'PARAMERR'
        correlation = 'REAL ALL'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentTfcrop={'apply':True,'mode':'tfcrop','datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentTfcrop)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 1248000.0)
        self.assertTrue(abs(summary['report0']['flagged'] - 63861.0) <= 5)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000.0)
        assert abs(summary['report0']['correlation']['Sol1']['flagged'] - 32193.0) <= 5
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000.0)
        assert abs(summary['report0']['correlation']['Sol2']['flagged'] - 31668.0) <=5

    def test_tfcrop_paramerr_sol1_extension_for_bpass_CalTable(self):
        """AgentFlagger:: Test tfcrop first calibration solution product of PARAMERR column, and then extend to the other solution for bpass CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'PARAMERR'
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentTfcrop={'apply':True,'mode':'tfcrop','datacolumn':datacolumn,'correlation':'Sol1'}
        agentExtension={'apply':True,'mode':'extend','extendpols':True,'correlation':'ALL'}
        agentSummary={'apply':True,'mode':'summary','correlation':'ALL'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.parseagentparameters(agentTfcrop)
        aflocal.parseagentparameters(agentExtension)
        aflocal.parseagentparameters(agentSummary)
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 1248000.0)
        assert abs(summary['report0']['flagged'] - 72034.0) <= 5
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000.0)
        assert abs(summary['report0']['correlation']['Sol1']['flagged'] - 36017.0) <= 5
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000.0)
        assert abs(summary['report0']['correlation']['Sol2']['flagged'] - 36017.0) <= 5

    def test_tfcrop_paramerr_sol1_extension_for_bpass_CalTable2(self):
        """AgentFlagger:: Test tfcrop first calibration solution product of PARAMERR column, 
        and then extend to the other solution for bpass CalTable"""

        # Same as the previous test but using the specific parsing functions
        aflocal = casac.agentflagger()
        datacolumn = 'PARAMERR'
        aflocal.open(self.vis)
        aflocal.selectdata()
        aflocal.parsemanualparameters(apply=False)
        aflocal.parsetfcropparameters(datacolumn=datacolumn, correlation='Sol1')
        aflocal.parseextendparameters(extendpols=True, correlation='ALL')
        aflocal.parsesummaryparameters(correlation='ALL')
        aflocal.init()
        summary = aflocal.run(writeflags=True)
        aflocal.done() 

        self.assertEqual(summary['report0']['total'], 1248000)
        self.assert_(abs(summary['report0']['flagged'] - 72034.0) <= 5)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000)
        self.assertTrue(abs(summary['report0']['correlation']['Sol1']['flagged'] - 36017.0) <= 5)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000)
        self.assertTrue(abs(summary['report0']['correlation']['Sol2']['flagged'] - 36017.0) <= 5)

    def test_clip_minmax_snr_all_for_bpass_CalTable(self):
        """AgentFlagger:: Test cliping all calibration solution products of SNR column using a minmax range for bpass CalTable"""
        aflocal = casac.agentflagger()
        datacolumn = 'SNR'
        correlation = 'ALL'
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
        self.assertEqual(summary['report0']['flagged'], 63570.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 25526.0)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 38044.0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000.0)


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
        aflocal.parseagentparameters(agentUnflag)
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
            cleanup]
