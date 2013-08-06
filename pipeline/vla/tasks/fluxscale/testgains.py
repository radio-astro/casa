from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary

import itertools

from pipeline.hif.tasks import gaincal
from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import applycal
from pipeline.vla.heuristics import getCalFlaggedSoln, getBCalStatistics


LOG = infrastructure.get_logger(__name__)

class TestgainsInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        
        self.gain_solint1 = 'int'
        self.gain_solint2 = 'int'

class TestgainsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[], shortsol2=None, short_solint=None, new_gain_solint1=None):
        super(TestgainsResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.shortsol2 = shortsol2
        self.short_solint = short_solint
        self.new_gain_solint1 = new_gain_solint1
        
    def merge_with_context(self, context):    
        m = context.observing_run.measurement_sets[0]
        context.evla['msinfo'][m.name].shortsol2 = self.shortsol2
        context.evla['msinfo'][m.name].short_solint = self.short_solint
        context.evla['msinfo'][m.name].new_gain_solint1 = self.new_gain_solint1
    
        
class Testgains(basetask.StandardTaskTemplate):
    Inputs = TestgainsInputs

    def prepare(self):

        context = self.inputs.context
        tablebase = 'testgaincal'
        table_suffix = ['.g','3.g','10.g']
        soltimes = [1.0,3.0,10.0] 
        m = self.inputs.context.observing_run.measurement_sets[0]
        soltimes = [self.inputs.context.evla['msinfo'][m.name].int_time * x for x in soltimes]
        
        solints = ['int', '3.0s', '10.0s']
        soltime = soltimes[0]
        solint = solints[0]
        shortsol1 = context.evla['msinfo'][m.name].shortsol1
        combtime = 'scan'

        

        bpdgain_touse = tablebase + table_suffix[0]
        testgains_result = self._do_gtype_testgains('calibrators.ms', 'testgaincal.g', solint=solint, context=context, combtime=combtime)

        flaggedSolnResult1 = getCalFlaggedSoln('testgaincal.g')


        if (flaggedSolnResult1['all']['total'] > 0):
            fracFlaggedSolns1=flaggedSolnResult1['antmedian']['fraction']
        else:
            fracFlaggedSolns1=1.0

        shortsol2=soltime

        if (fracFlaggedSolns1 > 0.05):
            soltime = soltimes[1]
            solint = solints[1]

            testgains_result = self._do_gtype_testgains('calibrators.ms', 'testgaincal3.g', solint=solint, context=context, combtime=combtime)
            flaggedSolnResult3 = getCalFlaggedSoln('testgaincal3.g')

            if (flaggedSolnResult3['all']['total'] > 0):
                fracFlaggedSolns3=flaggedSolnResult3['antmedian']['fraction']
            else:
                fracFlaggedSolns3=1.0

            if (fracFlaggedSolns3 < fracFlaggedSolns1):
                shortsol2 = soltime
                bpdgain_touse = tablebase + table_suffix[1]
            
                if (fracFlaggedSolns3 > 0.05):
                    soltime = soltimes[2]
                    solint = solints[2]

                    testgains_result = self._do_gtype_testgains('calibrators.ms', 'testgaincal10.g', solint=solint, context=context, combtime=combtime)
                    flaggedSolnResult10 = getCalFlaggedSoln(tablebase + table_suffix[2])

                    if (flaggedSolnResult10['all']['total'] > 0):
                        fracFlaggedSolns10 = flaggedSolnResult10['antmedian']['fraction']
                    else:
                        fracFlaggedSolns10 = 1.0

                    if (fracFlaggedSolns10 < fracFlaggedSolns3):
                        shortsol2=soltime
                        bpdgain_touse = tablebase + table_suffix[2]

                        if (fracFlaggedSolns10 > 0.05):
                            solint='inf'
                            combtime=''
                            testgains_result = self._do_gtype_testgains('calibrators.ms', 'testgaincalscan.g', solint=solint, context=context, combtime=combtime)
                            flaggedSolnResultScan = getCalFlaggedSoln('testgaincalscan.g')
                            
                            if (flaggedSolnResultScan['all']['total'] > 0):
                                fracFlaggedSolnsScan=flaggedSolnResultScan['antmedian']['fraction']
                            else:
                                fracFlaggedSolnsScan=1.0
                                
                            if (fracFlaggedSolnsScan < fracFlaggedSolns10):
                                shortsol2=longsolint
                                bpdgain_touse = 'testgaincalscan.g'
                                
                                if (fracFlaggedSolnsScan > 0.05):
                                    LOG.warn("Warning, large fraction of flagged solutions, there might be something wrong with your data")

        
        short_solint=max(shortsol1,shortsol2)
        new_gain_solint1=str(short_solint)+'s'
        
        LOG.info("Using short solint = " + new_gain_solint1)
        

        return TestgainsResults(shortsol2=shortsol2, short_solint=short_solint, new_gain_solint1=new_gain_solint1)                        

    def analyse(self, results):
	return results
    
 

    def _check_flagSolns(self, flaggedSolnResult):
        
        if (flaggedSolnResult['all']['total'] > 0):
            fracFlaggedSolns=flaggedSolnResult['antmedian']['fraction']
        else:
            fracFlaggedSolns=1.0

        refant_csvstring = self.inputs.context.observing_run.measurement_sets[0].reference_antenna
        refantlist = [x for x in refant_csvstring.split(',')]

        m = self.inputs.context.observing_run.measurement_sets[0]
        critfrac = self.inputs.context.evla['msinfo'][m.name].critfrac

        if (fracFlaggedSolns > critfrac):
            refantlist.pop(0)
            self.inputs.context.observing_run.measurement_sets[0].reference_antenna = ','.join(refantlist)
            LOG.info("Not enough good solutions, trying a different reference antenna.")
            LOG.info("The pipeline will use antenna "+refantlist[0]+" as the reference.")

        return fracFlaggedSolns

    def _do_gtype_testgains(self, calMs, caltable, solint='int', context=None, combtime='scan'):

        m = context.observing_run.measurement_sets[0]

        calibrator_scan_select_string=context.evla['msinfo'][m.name].calibrator_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal

        #Do this to get the reference antenna string
        temp_inputs = gaincal.GTypeGaincal.Inputs(context)
        refant = temp_inputs.refant.lower()

        task_args = {'vis'          : calMs,
                     'caltable'     : caltable,
                     'field'        : '',
                     'spw'          : '',
                     'intent'       : '',
                     'selectdata'   : True,
                     'scan'         : calibrator_scan_select_string,
                     'solint'       : solint,
                     'combine'      : combtime,
                     'preavg'       : -1.0,
                     'refant'       : refant,
                     'minblperant'  : minBL_for_cal,
                     'minsnr'       : 5.0,
                     'solnorm'      : False,
                     'gaintype'     : 'G',
                     'smodel'       : [],
                     'calmode'      : 'ap',
                     'append'       : False,
                     'gaintable'    : [''],
                     'gainfield'    : [''],
                     'interp'       : [''],
                     'spwmap'       : [],
                     'gaincurve'    : False,
                     'opacity'      : [],
                     'parang'       : False,
                     'async'        : False}

        
        job = casa_tasks.gaincal(**task_args)
            
        return self._executor.execute(job)
