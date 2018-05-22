from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class PbcorResults(basetask.Results):
    def __init__(self, pbcorimagenames=[]):
        super(PbcorResults, self).__init__()
        self.pbcorimagenames = pbcorimagenames[:]

    def __repr__(self):
        # return 'PbcorResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'PbcorResults:'


class PbcorInputs(vdp.StandardInputs):
    def __init__(self, context, vis=None):
        super(PbcorInputs, self).__init__()
        self.context = context
        self.vis = vis


@task_registry.set_equivalent_casa_task('hifv_pbcor')
class Pbcor(basetask.StandardTaskTemplate):
    Inputs = PbcorInputs

    def prepare(self):

        imlist = self.inputs.context.sciimlist.get_imlist()
        pbcor_list = []
        for image in imlist:
            imgname = image['imagename']
            outname = imgname + '.pbcor'
            pbname = imgname[:imgname.rfind('.image')] + '.pb'

            impbcor_imgname = imgname
            if image['multiterm']:
                outname += '.tt0'
                pbname += '.tt0'
                impbcor_imgname += '.tt0'

            task = casa_tasks.impbcor(imagename=impbcor_imgname, pbimage=pbname,
                                      outfile=outname, mode='divide', cutoff=-1.0, stretch=False)
            self._executor.execute(task)
            pbcor_list.append(outname)

            pbcor_list.append(pbname)

            outname = imgname+'.residual.pbcor'
            impbcor_imagename = imgname[:imgname.rfind('.image')] + '.residual'

            if image['multiterm']:
                outname += '.tt0'
                impbcor_imagename += '.tt0'

            task = casa_tasks.impbcor(imagename=impbcor_imagename, pbimage=pbname,
                                      outfile=outname, mode='divide', cutoff=-1.0, stretch=False)
            self._executor.execute(task)
            pbcor_list.append(outname)

            LOG.info("PBCOR image names: " + ','.join(pbcor_list))

        return PbcorResults(pbcorimagenames=pbcor_list)

    def analyse(self, results):
        return results



