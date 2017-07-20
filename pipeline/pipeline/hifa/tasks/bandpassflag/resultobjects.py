from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.hif.tasks.bandpass import common
from pipeline.hif.tasks.correctedampflag import resultobjects

LOG = infrastructure.get_logger(__name__)


class BandpassflagResults(basetask.Results):

    def __init__(self):
        super(BandpassflagResults, self).__init__()
        self.bpresult = common.BandpassResults()
        self.cafresult = resultobjects.CorrectedampflagResults()
        self.plots = {}

        # list of antennas that should be moved to the end
        # of the refant list
        self.refants_to_demote = set()

        # list of entirely flagged antennas that should be removed from refants
        self.refants_to_remove = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.api.Results.merge_with_context`
        """
        if not self.bpresult.final:
            LOG.error('No results to merge')
            return

        # Update the calibration library to add the new bandpass caltable.
        for calapp in self.bpresult.final:
            LOG.debug('Adding calibration to callibrary:\n%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

        # Update refant list if necessary.
        if self.refants_to_remove or self.refants_to_demote:

            # Get the MS
            ms = context.observing_run.get_ms(name=self.vis)

            # Fetch list of current refants
            refants = ms.reference_antenna.split(',')

            # Create updated refant list.
            updated_refants = []
            refants_to_move = []
            for ant in refants:
                if ant in self.refants_to_remove:
                    pass
                elif ant in self.refants_to_demote:
                    refants_to_move.append(ant)
                else:
                    updated_refants.append(ant)
            updated_refants.extend(refants_to_move)

            # Update MS with new refant list.
            ms.reference_antenna = ','.join(updated_refants)

    def __repr__(self):
        s = 'BandpassflagResults'
        for calapplication in self.bpresult.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s
