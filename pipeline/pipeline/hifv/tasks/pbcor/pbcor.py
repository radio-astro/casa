from __future__ import absolute_import
import os


import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from recipes import tec_maps

LOG = infrastructure.get_logger(__name__)


class PbcorResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[], pbcornames=[]):
        super(PbcorResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.pbcornames = pbcornames[:]
        self.error = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.warn('No pbcor results')
            return

    def __repr__(self):
        #return 'PbcorResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'PbcorResults:'


class PbcorInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class Pbcor(basetask.StandardTaskTemplate):
    Inputs = PbcorInputs

    def prepare(self):

        imlist = self.inputs.context.sciimlist.get_imlist()
        pbcor_list = []
        for image in imlist:
            imgname = image['imagename']
            pbcorname = imgname+'.pbcor.tt0'
            task = casa_tasks.impbcor(imagename=imgname+'.tt0', pbimage=imgname[:imgname.rfind('.image')]+'.pb.tt0',
                                      outfile=pbcorname, mode='divide', cutoff=-1.0, stretch=False)
            self._executor.execute(task)
            pbcor_list.append(pbcorname)
            LOG.info("PBCOR image names: " + ','.join(pbcor_list))

        return PbcorResults(pbcornames=pbcor_list)

    def analyse(self, results):
        return results

    def _do_somethingpbcor(self):

        task = casa_tasks.pbcorcal(vis=self.inputs.vis, caltable='tempcal.pbcor')

        return self._executor.execute(task)

    def _do_tec_maps(self):

        tec_maps.create(vis=self.vis, doplot=True, imname='iono')
        # gencal_job = casa_tasks.gencal(**gencal_args)
        gencal_job = casa_tasks.gencal(vis=self.vis, caltable='tec.cal', caltype='tecim', infile='iono.IGS_TEC.im')
        self._executor.execute(gencal_job)

