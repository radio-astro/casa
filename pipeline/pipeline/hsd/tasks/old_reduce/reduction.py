from __future__ import absolute_import

import os
import re

import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from pipeline.infrastructure.basetask import timestamp

#import pipeline.hsd.heuristics as heuristics

loglevelMap = { logging.CRITICAL: 0,
                logging.ERROR: 0,
                logging.WARNING: 1,
                logging.INFO: 2,
                logging.DEBUG: 3,
                logging.TRACE: 4 }

fitfuncMap = { 'cspline': 'spline',
               'poly': 'polynomial' }

def _parse_loglevel( loglevel ):
    import types
    if loglevel is None:
        key = logging.logging_level
        sdloglevel = loglevelMap[key]
    elif type(loglevel) is types.StringType:
        key = logging.LOGGING_LEVELS[loglevel]
        sdloglevel = loglevelMap[key]
    else:
        sdloglevel = loglevel
    return sdloglevel

class SDReductionInputs(api.Inputs):
    """
    Inputs for single dish processing
    """
    def __init__(self,
                 context,
                 input_dir=None,
                 output_dir=None,
                 infiles=None,
                 loglevel=None,
                 antennalist=None,
                 spwlist=None,
                 pollist=None,
                 rowlist=None,
                 rowbase=None,
                 scanlist=None,
                 scanbase=None,
                 continuum=None,
                 contsub=None,
                 edge=None,
                 linewindow=None,
                 broadline=None,
                 plotcluster=None,
                 plotflag=None,
                 fitorder=None,
                 fitfunc=None,
                 gridsize=None,
                 docombine=None,
                 plotfit=None,
                 plotspectra=None,
                 plotchmap=None,
                 plotspmap=None,
                 plotcombine=None,
                 moments=None,
                 iteration=None,
                 flag_tsys=None,
                 tsys_thresh=None,
                 flag_weath=None,
                 weath_thresh=None,
                 flag_prfre=None,
                 prfre_thresh=None,
                 flag_pofre=None,
                 pofre_thresh=None,
                 flag_prfr=None,
                 prfr_thresh=None,
                 flag_pofr=None,
                 pofr_thresh=None,
                 flag_prfrm=None,
                 prfrm_thresh=None,
                 prfrm_nmean=None,
                 flag_pofrm=None,
                 pofrm_thresh=None,
                 pofrm_nmean=None,
                 flag_user=None,
                 user_thresh=None):
        self.context = context
        self.output_dir = output_dir if output_dir is not None else context.output_dir
        if infiles is None:
            self._dataset_from_context()
        elif input_dir is None:
            self._dataset_from_context( infiles )
        else:
            self.input_dir = input_dir 
            self.dataset = infiles 
        self.loglevel = _parse_loglevel( loglevel )
        self.antennalist = self.__parse(antennalist,'all')
        self.spwlist = self.__parse(spwlist,'all')
        self.pollist = self.__parse(pollist,'all')
        self.rowlist = self.__parse(rowlist,'all')
        self.rowbase = self.__parse(rowbase,'all')
        self.scanlist = self.__parse(scanlist,'all')
        self.scanbase = self.__parse(scanbase,'all')
        self.continuum = self.__tobool(continuum,True)
        self.contsub = self.__tobool(contsub,False)
        self.edge = self.__parse(edge,[0,0])
        self.linewindow = self.__parse(linewindow,None)
        self.broadline = self.__tobool(broadline,False)
        self.plotcluster = self.__tobool(plotcluster,True)
        self.plotflag = self.__tobool(plotflag,True)
        #self.fitorder = fitorder if fitorder is not None else 'automatic'
        self.fitorder = self.__parse(fitorder,'automatic')
        self.fitfunc = fitfunc if fitfunc is not None else 'spline'
        #self.gridsize = gridsize if gridsize is not None else 'automatic'
        #if self.gridsize < 0.0:
        #    self.gridsize = 'automatic'
        self.gridsize = self.__parse(gridsize,'automatic')
        self.docombine = self.__tobool(docombine,True)
        self.plotfit = self.__tobool(plotfit,True)
        self.plotspectra = self.__tobool(plotspectra,True)
        self.plotchmap = self.__tobool(plotchmap,True)
        self.plotspmap = self.__tobool(plotspmap,True)
        self.plotcombine = self.__tobool(plotcombine,True)
        self.moments = self.__parse(moments,[0])
        self.iteration = iteration if iteration is not None else 2
        self.flag_tsys = flag_tsys
        self.tsys_thresh = tsys_thresh
        self.flag_weath = flag_weath
        self.weath_thresh = weath_thresh
        self.flag_prfre = flag_prfre
        self.prfre_thresh = prfre_thresh
        self.flag_pofre = flag_pofre
        self.pofre_thresh = pofre_thresh
        self.flag_prfr = flag_prfr
        self.prfr_thresh = prfr_thresh
        self.flag_pofr = flag_pofr
        self.pofr_thresh = pofr_thresh
        self.flag_prfrm = flag_prfrm
        self.prfrm_thresh = prfrm_thresh
        self.prfrm_nmean = prfrm_nmean
        self.flag_pofrm = flag_pofrm
        self.pofrm_thresh = pofrm_thresh
        self.pofrm_nmean = pofrm_nmean
        self.flag_user = flag_user
        self.user_thresh = user_thresh

    def _dataset_from_context(self,names=[]):
        observing_run = self.context.observing_run

        ms_names = [(name.rstrip('/') if re.match('.*\.ms/*$',name) \
                     else name.rstrip('/')+'.ms') for name in names]
        
        if len(ms_names) == 0:
            measurement_sets = observing_run.measurement_sets
        else:
            measurement_sets = [ms for ms in observing_run.measurement_sets \
                                if ms.basename in ms_names]
            
        self.dataset = [f.basename for f in measurement_sets]
        if len(measurement_sets) == 0:
            self.input_dir = './'
        else:
            self.input_dir = os.path.split(measurement_sets[0].name)[0]
        
    def __tobool(self,param,default=True):
        if param is None:
            #print '__tobool:return default'
            return default
        else:
            return (True if param == 1 else False)

    def __parse(self,param,default=None):
        if param is None:
            #print '__parse:return default'
            return default
        elif type(param) is list:
            return param
        else:
            ret = None
            try:
                ret = int(param)
                if ret < 0:
                    ret = default
            except:
                try:
                    ret = eval(param)
                except:
                    ret = str(param)
                    if ret.lower() == 'all':
                        ret = 'all'
                    elif ret.lower() == 'none':
                        ret = default
                    elif ret[0] == '[' and ret[-1] == ']':
                        # would be a string list
                        # 1d list assumed
                        ret2 = ret.replace('[','[\'').replace(']','\']').replace(',','\',\'')
                        #print ret
                        try:
                            ret = eval(ret2)
                        except:
                            pass
            return ret
                        

    def to_casa_args(self):
        # No corresponding CASA task so return an empty dictionary
        return {}

    def as_dict(self):
        # Our convention is that properties - as in, methods decorated with
        # the @property decorator - hide instance values of the same name
        # beginning with underscore. We detect this case and ask for the 
        # result of the property method instead. This gives the calculated
        # value rather than, say, the name of the heuristic handling that
        # property.   
        properties = {}
        for k, v in self.__dict__.items():
            if k in ('_context', '_my_vislist'):
                continue
            v = getattr(self, k[1:]) if k[0] is '_' else v
            k = k[1:] if k[0] is '_' else k
            properties[k] = v
        return properties


    def __repr__(self):
        properties = self.as_dict()
        header  = '<SDReductionInputs( '
        footer = ' )>'
        initial_sep = ''
        followon_sep = '\n' + ' ' * len(header)
        sep = initial_sep
        param_string = ''
        keys = [ 'input_dir', 'output_dir', 'dataset',
                 'loglevel', 'iteration',
                 'antennalist', 'spwlist', 'pollist',
                 'rowlist', 'rowbase',
                 'scanlist', 'scanbase',
                 'continuum', 'contsub',
                 'edge', 'linewindow', 'broadline',
                 'fitorder', 'fitfunc',
                 'gridsize', 'docombine',
                 'flag_tsys', 'tsys_thresh',
                 'flag_weath', 'weath_thresh',
                 'flag_prfre', 'prfre_thresh',
                 'flag_pofre', 'pofre_thresh',
                 'flag_prfr', 'prfr_thresh',
                 'flag_pofr', 'pofr_thresh',
                 'flag_prfrm', 'prfrm_thresh', 'prfrm_nmean',
                 'flag_pofrm', 'pofrm_thresh', 'pofrm_nmean',
                 'flag_user', 'user_thresh',
                 'moments',
                 'plotcluster', 'plotflag',
                 'plotfit', 'plotspectra', 'plotchmap',
                 'plotspmap', 'plotcombine' ]
        for k in keys:
            try:
                param_string += '%s%s=%s'%(sep,k,properties[k])
            except Exception, e:
                pass
            sep = followon_sep
        ret = header + param_string + footer
        return ret
        

class SDReductionResults(basetask.Results):
    def __init__(self, success):
        super(SDReductionResults,self).__init__()
        self.success = success
        self.error = set()
        # TODO: relative path must be obtained in some way
        #       hard-code here at the moment
        self.url = '../sd/index.html'
    
    def merge_with_context(self, context):
        # TODO: self.status should be registered to context
        self.error.clear()

    def __repr__(self):
        return ('Single dish results' )


class SDReduction(api.Task):
    Inputs = SDReductionInputs

    def __init__(self, inputs):
        super(SDReduction,self).__init__(inputs)
    
    def prepare(self):
        return []

    @timestamp
    def execute(self, dry_run=True, **parameters):
        print 'SDReduction.execute()...'

        # increment task_counter
        self.inputs.context.task_counter += 1

        # remember original logfile for casalog
        # since sd heuristics manipulates it
        from taskinit import casalog
        logfile = casalog.logfile()
        casalog.post('Create StagesInterface instance')
        casalog.post(str(self.inputs))
        #casalog.post('type(spwlist)=%s'%(type(self.inputs.spwlist)))

        if dry_run is True:
            results = None
        else:
            # single dish heuristics run
            print 'Run single dish script...'
            try:
                results = self.__execute()
            except Exception, e:
                results = SDReductionResults(success=False)
                results.error.add( str(e) )

            # set logfile for casalog to original
            casalog.setlogfile(logfile)
            casalog.post('logfile is back to original')
            if not results.success:
                for item in results.error:
                    casalog.post(item,'ERROR')
            results.task = self.__class__
        
        # no follow-on jobs        
        return results

    def __execute(self):
        inputs = self.inputs
        
        # parse fitfunc
        if inputs.fitfunc in fitfuncMap.keys():
            fitfunc = fitfuncMap[inputs.fitfunc]
        else:
            fitfunc = inputs.fitfunc

        # root of single dish weblog is '<context.report_dir>/sd'
        html_dir = os.path.join( inputs.context.report_dir, 'sd' )
        
        #import heuristics.hsd.SDPipelineStagesInterface as SI
        #from pipeline.hsd.tasks.common import SDPipelineStagesInterface as SI
        from . import SDPipelineStagesInterface as SI
        si = SI.SDPipelineStagesInterface(rawDir=inputs.input_dir,
                                          outDir=inputs.output_dir,
                                          htmlDir=html_dir,
                                          LogLevel=inputs.loglevel)
        self.__configureFlagRule( si )
        si.setRawFile(inputs.dataset)
        si.setContinuumOutput(inputs.continuum)
        # check linewindow format
        if inputs.linewindow is not None:
            status=self.__setLineWindow(si, inputs.linewindow)
            if status is False:
                del si
                return 'Invalid window format'
        si.setBaselineEdge(inputs.edge)
        si.setBroadLineSpectralComponent(inputs.broadline)
        si.setupSDReducer(ANTENNA=inputs.antennalist,
                          IF=inputs.spwlist,
                          POL=inputs.pollist,
                          ROW=inputs.rowlist,
                          ROWbase=inputs.rowbase,
                          SCAN=inputs.scanlist,
                          SCANbase=inputs.scanbase,
                          overwrite=True)
        si.calibrate()
        si.readData()
        si.createPositionOverviewPlots()
        loop_elements = si.getLoopElementsPerSpw()
        for (SWconfig) in loop_elements:
            si.groupData(SWconfig)
            if inputs.contsub:
                si.subtractBaselineAtRasterEdgeRunThrough()
            for iter in xrange(inputs.iteration):
                si.simpleGridding()
                si.spectralLineDetect()
                si.spatialLineDetect(createResultPlots=inputs.plotcluster)
                si.removeBaselineRunThrough(fitOrder=inputs.fitorder,
                                            fitFunc=fitfunc,
                                            createResultPlots=False)
                si.flagDataRunThrough(createResultPlots=inputs.plotflag)
                si.regridDataRunThrough(gridsize=inputs.gridsize)
                if inputs.docombine:
                    si.combineGriddedData()
                if inputs.plotfit:
                    si.plotFitSpectraWithFlagResultRunThrough()
                if inputs.plotspectra:
                    si.plotResultSpectraRunThrough()
                    if inputs.plotcombine:
                        si.plotResultSpectraForCombine()
                if inputs.plotchmap:
                    si.plotChannelMapsRunThrough()
                    if inputs.plotcombine:
                        si.plotChannelMapForCombine()
                if inputs.plotspmap:
                    si.plotSparseSpectraMapRunThrough()
                    if inputs.plotcombine:
                        si.plotSparseSpectraMapForCombine()
        si.writeMSDataCube(moments=inputs.moments)
        si.cleanup()
        del si

        results = SDReductionResults(success=True)
        return results
    
    def __configureFlagRule( self, si ):
        inputs = self.inputs
        d = { 'TsysFlag': (inputs.flag_tsys, [inputs.tsys_thresh]),
              'WeatherFlag': (inputs.flag_weath, [inputs.weath_thresh]),
              'UserFlag': (inputs.flag_user, [inputs.user_thresh]),
              'RmsPreFitFlag': (inputs.flag_prfr, [inputs.prfr_thresh]),
              'RmsPostFitFlag': (inputs.flag_pofr, [inputs.pofr_thresh]),
              'RmsExpectedPreFitFlag': (inputs.flag_prfre, [inputs.prfre_thresh]),
              'RmsExpectedPostFitFlag': (inputs.flag_pofre, [inputs.pofre_thresh]),
              'RunMeanPreFitFlag': (inputs.flag_prfrm, [inputs.prfrm_thresh, inputs.prfrm_nmean]),
              'RunMeanPostFitFlag': (inputs.flag_pofrm, [inputs.pofrm_thresh, inputs.pofrm_nmean]) }
        keys = ['Threshold', 'Nmean']
        for (k,v) in d.items():
            (b,p) = v
            if b is None:
                pass
            elif b == True:
                si.activateFlagRule( k )
                for i in xrange(len(p)):
                    si.FlagRuleDictionary[k][p[i]] = p[i]
            elif b == False:
                si.deactivateFlagRule( k )

    def __setLineWindow(self, obj, linewindow):
        status = True
        if(len(linewindow)>0):
            print "setting spectral window for line",linewindow
            import numpy
            if type(linewindow[0]) == list or type(linewindow[0]) == numpy.ndarray:
                if ( len(linewindow[0]) == 2 ):
                    # channel window
                    print 'channel window'
                    obj.setChannelWindow(linewindow)
                elif ( len(linewindow[0]) == 3 ):
                    print 'spectral window'
                    obj.setSpectrumWindow(linewindow)
                else:
                    print 'Invalid window format'
                    status = False
            else:
                if ( len(linewindow) == 2 ):
                    print 'channel window'
                    obj.setChannelWindow([linewindow])
                elif ( len(linewindow) == 3 ):
                    print 'spectral window'
                    obj.setSpectrumWindow([linewindow])
                else:
                    print 'Invalid window format'
                    status = False
        return status
