from __future__ import absolute_import
import os


import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from recipes import tec_maps

LOG = infrastructure.get_logger(__name__)


class ExportvlassdataResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(ExportvlassdataResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.warn('No exportvlassdata results')
            return

    def __repr__(self):
        #return 'ExportvlassdataResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'ExportvlassdataResults:'


class ExportvlassdataInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class Exportvlassdata(basetask.StandardTaskTemplate):
    Inputs = ExportvlassdataInputs

    def prepare(self):

        LOG.info("This Exportvlassdata class is running.")

        imlist = self.inputs.context.subimlist.get_imlist()

        for imageitem in imlist:
            pbcor_image_name = imageitem['imagename'].replace('subim', 'pbcor.tt0.subim')
            task = casa_tasks.exportfits(imagename=pbcor_image_name, fitsimage=pbcor_image_name + '.fits')
            self._executor.execute(task)
            LOG.info('Wrote {ff}'.format(ff=pbcor_image_name + '.fits'))

            rms_image_name = imageitem['imagename'].replace('subim', 'pbcor.tt0.rms.subim')
            task = casa_tasks.exportfits(imagename=rms_image_name, fitsimage=rms_image_name + '.fits')
            self._executor.execute(task)
            LOG.info('Wrote {ff}'.format(ff=rms_image_name + '.fits'))

        return ExportvlassdataResults()

    def analyse(self, results):
        return results

    def _do_somethingexportvlassdata(self):

        task = casa_tasks.exportvlassdatacal(vis=self.inputs.vis, caltable='tempcal.exportvlassdata')

        return self._executor.execute(task)

    def _do_tec_maps(self):

        tec_maps.create(vis=self.vis, doplot=True, imname='iono')
        # gencal_job = casa_tasks.gencal(**gencal_args)
        gencal_job = casa_tasks.gencal(vis=self.vis, caltable='tec.cal', caltype='tecim', infile='iono.IGS_TEC.im')
        self._executor.execute(gencal_job)

