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

        tflocal = casac.casac.testflagger()
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.init()
        tflocal.run(writeflags=True)
        tflocal.done()


class test_tsys(test_base):
    """TestFlagger:: Test flagging tool with Tsys-based CalTable """
    
    def setUp(self):
         self.setUp_tsys_case()

    def test_manual_field_selection_agent_layer_for_tsys_CalTable(self):
        """TestFlagger:: Manually flag a Tsys-based CalTable using flag agent selection engine for field """
        tflocal = casac.casac.testflagger()
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','field':'0'}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['field']['3c279']['flagged'] == 9216.0
        assert summary['report0']['field']['Titan']['flagged'] == 0
        assert summary['report0']['field']['TW Hya']['flagged'] == 0
        assert summary['report0']['field']['J1037-295=QSO']['flagged'] == 0

    def test_manual_antenna_selection_agent_layer_for_tsys_CalTable(self):
        """TestFlagger:: Manually flag a Tsys-based CalTable using flag agent selection engine for antenna"""
        tflocal = casac.casac.testflagger()
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','antenna':'DV09'}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['antenna']['DV09']['flagged'] == 14336.0
        assert summary['report0']['antenna']['DV10']['flagged'] == 0.0

    def test_manual_antenna_selection_agent_layer_for_tsys_CalTable2(self):
        """TestFlagger:: Manually flag a Tsys-based CalTable using flag agent selection engine for antenna"""

        # Run the previous test but using the specific parsing functions
        tflocal = casac.casac.testflagger()
        tflocal.open(self.vis)
        tflocal.selectdata()
        tflocal.parsemanualparameters(apply=False)
        tflocal.parsemanualparameters(antenna='DV09')
        tflocal.parsesummaryparameters()
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        self.assertEqual(summary['report0']['antenna']['DV09']['flagged'], 14336)
        self.assertEqual(summary['report0']['antenna']['DV10']['flagged'], 0)

    def test_manual_field_msSelection_layer_for_tsys_CalTable(self):
        """TestFlagger:: Manually flag a Tsys-based CalTable using MSSelection for field """
        tflocal = casac.casac.testflagger()

        tflocal.open(self.vis)
        tflocal.selectdata(field='0')
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        tflocal.open(self.vis)
        tflocal.selectdata()
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['field']['3c279']['flagged'] == 9216.0
        assert summary['report0']['field']['Titan']['flagged'] == 0
        assert summary['report0']['field']['TW Hya']['flagged'] == 0
        assert summary['report0']['field']['J1037-295=QSO']['flagged'] == 0        

    def test_manual_antenna_msSelection_layer_for_tsys_CalTable(self):
        """TestFlagger:: Manually flag a Tsys-based CalTable using MSSelection for antenna"""
        tflocal = casac.casac.testflagger()

        tflocal.open(self.vis)
        tflocal.selectdata(antenna='DV09')
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        tflocal.open(self.vis)
        tflocal.selectdata()
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['antenna']['DV09']['flagged'] == 14336.0
        assert summary['report0']['antenna']['DV10']['flagged'] == 0.0
        
    def test_clip_minmax_fparm_sol1(self):
        """TestFlagger:: Test cliping first calibration solution product of FPARAM column using a minmax range """
        tflocal = casac.casac.testflagger()
        datacolumn = 'FPARAM'
        correlation = 'Sol1'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 750.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 750.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0

    def test_clip_minmax_fparm_sol1_2(self):
        """TestFlagger:: Test cliping first calibration solution product of FPARAM column using a minmax range """

        # Run the previous test but using the specific parsing functions
        tflocal = casac.casac.testflagger()
        datacolumn = 'FPARAM'
        correlation = 'Sol1'
        tflocal.open(self.vis)
        tflocal.selectdata()
        tflocal.parsemanualparameters(apply=False)
        tflocal.parseclipparameters(clipzeros=True,clipminmax=[0.,600.],datacolumn=datacolumn,
                                     correlation=correlation)
        tflocal.parsesummaryparameters()
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        self.assertEqual(summary['report0']['total'], 129024)
        self.assertEqual(summary['report0']['flagged'], 750)
        self.assertEqual(summary['report0']['correlation']['Sol1']['flagged'], 750)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 64512)
        self.assertEqual(summary['report0']['correlation']['Sol2']['flagged'], 0)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 64512)

    def test_clip_minmax_fparm_sol1_extension(self):
        """TestFlagger:: Test cliping first calibration solution product of FPARAM column using a minmax range, and then extend to the other solution """
        tflocal = casac.casac.testflagger()
        datacolumn = 'FPARAM'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':'Sol1'}
        agentExtension={'apply':True,'mode':'extend','extendpols':True,'correlation':'ALL'}
        agentSummary={'apply':True,'mode':'summary','correlation':'ALL'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentExtension)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 1726.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 863.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 863.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0

    def test_clip_minmax_fparm_sol2(self):
        """TestFlagger:: Test cliping second calibration solution product of FPARAM column using a minmax range """
        tflocal = casac.casac.testflagger()
        datacolumn = 'FPARAM'
        correlation = 'Sol2'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 442.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 442.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0

    def test_clip_minmax_fparm_sol1sol2(self):
        """TestFlagger:: Test cliping first and second calibration solution products of FPARAM column using a minmax range """
        tflocal = casac.casac.testflagger()
        datacolumn = 'FPARAM'
        correlation = 'Sol1,Sol2'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 1192.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 750.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 442.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0

    def test_clip_minmax_fparm_all(self):
        """TestFlagger:: Test cliping all calibration solution products of FPARAM column using a minmax range """
        tflocal = casac.casac.testflagger()
        datacolumn = 'FPARAM'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 1192.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 750.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 442.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0

    def test_clip_zeros_fparm_all(self):
        """TestFlagger:: Test cliping only zeros in all calibration solution products of FPARAM column"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'FPARAM'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0

    def test_clip_nan_and_inf_fparm_all(self):
        """TestFlagger:: Test cliping only NaNs/Infs in all calibration solution products of FPARAM column"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'FPARAM'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0

    def test_clip_minmax_fparm_error_case_absall(self):
        """TestFlagger:: Error case test when a complex operator is used with CalTables """
        tflocal = casac.casac.testflagger()
        datacolumn = 'FPARAM'
        correlation = 'ABS ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,600.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 1192.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 750.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 442.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0

    def test_clip_minmax_paramerr_all_for_tsys_CalTable(self):
        """TestFlagger:: Test cliping all calibration solution products of PARAMERR column using a minmax range for Tsys CalTable"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,0.2],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0

    def test_clip_minmax_snr_all_for_tsys_CalTable(self):
        """TestFlagger:: Test cliping all calibration solution products of SNR column using a minmax range for Tsys CalTable"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'SNR'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,2.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 129024.0
        assert summary['report0']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['total'] == 64512.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol2']['total'] == 64512.0


class test_bpass(test_base):
    """TestFlagger:: Test flagging tool with Bpass-based CalTable """
    
    def setUp(self):
        self.setUp_bpass_case()

    def test_manual_field_selection_agent_layer_for_bpass_CalTable(self):
        """TestFlagger:: Manually flag a bpass-based CalTable using flag agent selection engine for field """
        tflocal = casac.casac.testflagger()
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','field':'3C286_A'}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['field']['3C286_A']['flagged'] == 499200.0
        assert summary['report0']['field']['3C286_B']['flagged'] == 0
        assert summary['report0']['field']['3C286_C']['flagged'] == 0
        assert summary['report0']['field']['3C286_D']['flagged'] == 0

    def test_manual_antenna_selection_agent_layer_for_bpass_CalTable(self):
        """TestFlagger:: Manually flag a bpass-based CalTable using flag agent selection engine for antenna"""
        tflocal = casac.casac.testflagger()
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','antenna':'ea09'}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['antenna']['ea09']['flagged'] == 48000.0
        assert summary['report0']['antenna']['ea10']['flagged'] == 0.0

    def test_manual_field_msSelection_layer_for_bpass_CalTable(self):
        """TestFlagger:: Manually flag a bpass-based CalTable using MSSelection for field """
        tflocal = casac.casac.testflagger()

        tflocal.open(self.vis)
        tflocal.selectdata({'field':'3C286_A'})
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        tflocal.open(self.vis)
        tflocal.selectdata()
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['field']['3C286_A']['flagged'] == 499200.0
        assert summary['report0']['field']['3C286_B']['flagged'] == 0
        assert summary['report0']['field']['3C286_C']['flagged'] == 0
        assert summary['report0']['field']['3C286_D']['flagged'] == 0

    def test_manual_antenna_msSelection_layer_for_bpass_CalTable(self):
        """TestFlagger:: Manually flag a bpass-based CalTable using MSSelection for antenna"""
        tflocal = casac.casac.testflagger()

        tflocal.open(self.vis)
        tflocal.selectdata({'antenna':'ea09'})
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        tflocal.open(self.vis)
        tflocal.selectdata()
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['antenna']['ea09']['flagged'] == 48000.0
        assert summary['report0']['antenna']['ea10']['flagged'] == 0.0
        
    def test_clip_zeros_paramerr_all_for_bpass_CalTable(self):
        """TestFlagger:: Test cliping only zeros in all calibration solution products of PARAMERR column for bpass CalTable"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 1248000.0
        assert summary['report0']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['total'] == 624000.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol2']['total'] == 624000.0

    def test_clip_nan_and_inf_paramerr_all_for_bpass_CalTable(self):
        """TestFlagger:: Test cliping only zeros in all calibration solution products of PARAMERR column for bpass CalTable"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 1248000.0
        assert summary['report0']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol1']['total'] == 624000.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 0.0
        assert summary['report0']['correlation']['Sol2']['total'] == 624000.0

    def test_clip_minmax_paramerr_all_for_bpass_CalTable(self):
        """TestFlagger:: Test cliping all calibration solution products of PARAMERR column using a minmax range for bpass CalTable"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0., 0.001],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 
    
        assert summary['report0']['total'] == 1248000.0
        assert summary['report0']['flagged'] == 253315.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 134252.0
        assert summary['report0']['correlation']['Sol1']['total'] == 624000.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 119063.0
        assert summary['report0']['correlation']['Sol2']['total'] == 624000.0

    def test_rflag_paramerr_all_for_bpass_CalTable(self):
        """TestFlagger:: Test rflag in all calibration solution products of PARAMERR column for bpass CalTable"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'PARAMERR'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentRflag={'apply':True,'mode':'rflag','datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentRflag)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 1248000.0
        assert summary['report0']['flagged'] == 184222.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 93880.0
        assert summary['report0']['correlation']['Sol1']['total'] == 624000.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 90342.0
        assert summary['report0']['correlation']['Sol2']['total'] == 624000.0

    def test_tfcrop_paramerr_all_for_bpass_CalTable(self):
        """TestFlagger:: Test tfcrop in all calibration solution products of PARAMERR column for bpass CalTable"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'PARAMERR'
        correlation = 'REAL ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentTfcrop={'apply':True,'mode':'tfcrop','datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentTfcrop)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 1248000.0
        assert abs(summary['report0']['flagged'] - 63861.0) <= 5
        assert summary['report0']['correlation']['Sol1']['total'] == 624000.0
        assert abs(summary['report0']['correlation']['Sol1']['flagged'] - 32193.0) <= 5
        assert summary['report0']['correlation']['Sol2']['total'] == 624000.0
        assert abs(summary['report0']['correlation']['Sol2']['flagged'] - 31668.0) <=5

    def test_tfcrop_paramerr_sol1_extension_for_bpass_CalTable(self):
        """TestFlagger:: Test tfcrop first calibration solution product of PARAMERR column, and then extend to the other solution for bpass CalTable"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'PARAMERR'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentTfcrop={'apply':True,'mode':'tfcrop','datacolumn':datacolumn,'correlation':'Sol1'}
        agentExtension={'apply':True,'mode':'extend','extendpols':True,'correlation':'ALL'}
        agentSummary={'apply':True,'mode':'summary','correlation':'ALL'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentTfcrop)
        tflocal.parseagentparameters(agentExtension)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 1248000.0
        assert abs(summary['report0']['flagged'] - 72034.0) <= 5
        assert summary['report0']['correlation']['Sol1']['total'] == 624000.0
        assert abs(summary['report0']['correlation']['Sol1']['flagged'] - 36017.0) <= 5
        assert summary['report0']['correlation']['Sol2']['total'] == 624000.0
        assert abs(summary['report0']['correlation']['Sol2']['flagged'] - 36017.0) <= 5

    def test_tfcrop_paramerr_sol1_extension_for_bpass_CalTable2(self):
        """TestFlagger:: Test tfcrop first calibration solution product of PARAMERR column, 
        and then extend to the other solution for bpass CalTable"""

        # Same as the previous test but using the specific parsing functions
        tflocal = casac.casac.testflagger()
        datacolumn = 'PARAMERR'
        tflocal.open(self.vis)
        tflocal.selectdata()
        tflocal.parsemanualparameters(apply=False)
        tflocal.parsetfcropparameters(datacolumn=datacolumn, correlation='Sol1')
        tflocal.parseextendparameters(extendpols=True, correlation='ALL')
        tflocal.parsesummaryparameters(correlation='ALL')
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        self.assertEqual(summary['report0']['total'], 1248000)
        self.assert_(abs(summary['report0']['flagged'] - 72034.0) <= 5)
        self.assertEqual(summary['report0']['correlation']['Sol1']['total'], 624000)
        self.assertTrue(abs(summary['report0']['correlation']['Sol1']['flagged'] - 36017.0) <= 5)
        self.assertEqual(summary['report0']['correlation']['Sol2']['total'], 624000)
        self.assertTrue(abs(summary['report0']['correlation']['Sol2']['flagged'] - 36017.0) <= 5)

    def test_clip_minmax_snr_all_for_bpass_CalTable(self):
        """TestFlagger:: Test cliping all calibration solution products of SNR column using a minmax range for bpass CalTable"""
        tflocal = casac.casac.testflagger()
        datacolumn = 'SNR'
        correlation = 'ALL'
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentClip={'apply':True,'mode':'clip','clipzeros':True,'clipminmax':[0.,550.],'datacolumn':datacolumn,'correlation':correlation}
        agentSummary={'apply':True,'mode':'summary'}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentClip)
        tflocal.parseagentparameters(agentSummary)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done() 

        assert summary['report0']['total'] == 1248000.0
        assert summary['report0']['flagged'] == 63570.0
        assert summary['report0']['correlation']['Sol1']['flagged'] == 25526.0
        assert summary['report0']['correlation']['Sol1']['total'] == 624000.0
        assert summary['report0']['correlation']['Sol2']['flagged'] == 38044.0
        assert summary['report0']['correlation']['Sol2']['total'] == 624000.0


class test_display(test_base):
    """TestFlagger:: Automatic test to check basic behaviour of display GUI using pause=False option """

    def test_display_data_single_channel_selection(self):
        """TestFlagger:: Check nominal behaviour for single spw:chan selection """
        self.setUp_4Ants()
        tflocal = casac.casac.testflagger()
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','spw':'*:20~40'}
        agentSummary={'apply':True,'mode':'summary'}
        agentDisplay={'mode':'display','datadisplay':True,'pause':False}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.parseagentparameters(agentSummary)
        tflocal.parseagentparameters(agentDisplay)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done()

    def test_display_data_multiple_channel_selection(self):
        """TestFlagger:: Check behaviour for multiple spw:chan selection """
        self.setUp_4Ants()
        tflocal = casac.casac.testflagger()
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','spw':'*:10~20;30~40'}
        agentSummary={'apply':True,'mode':'summary'}
        agentDisplay={'mode':'display','datadisplay':True,'pause':False}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.parseagentparameters(agentSummary)
        tflocal.parseagentparameters(agentDisplay)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done()

    def test_display_data_different_corrs_per_spw(self):
        """TestFlagger:: Check behaviour when the number of correlation products changes between SPWs """
        self.setUp_CAS_4052()
        tflocal = casac.casac.testflagger()
        tflocal.open(self.vis)
        tflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        agentManual={'apply':True,'mode':'manual','spw':'*:100~200;300~400'}
        agentSummary={'apply':True,'mode':'summary'}
        agentDisplay={'mode':'display','datadisplay':True,'pause':False}
        tflocal.parseagentparameters(agentUnflag)
        tflocal.parseagentparameters(agentManual)
        tflocal.parseagentparameters(agentSummary)
        tflocal.parseagentparameters(agentDisplay)
        tflocal.init()
        summary = tflocal.run(writeflags=True)
        tflocal.done()


# Dummy class which cleans up created files
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf cal.fewscans.bpass*')
        os.system('rm -rf X7ef.tsys*')
        os.system('rm -rf Four_ants_3C286.ms*')
        os.system('rm -rf TwoSpw.ms*')

    def test1(self):
        '''TestFlagger: Cleanup'''
        pass


def suite():
    return [test_tsys,
            test_bpass,
            test_display,
            cleanup]
