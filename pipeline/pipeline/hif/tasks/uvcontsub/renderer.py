import collections
import os

import pipeline.infrastructure
#import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

#LOG = logging.get_logger(__name__)

# Reuse this definition
UVcontFitApplication = collections.namedtuple('UVcontFitApplication', 
                                            #'ms scispw freqrange solint fitorder source scispws gaintable') 
                                            'ms freqrange solint fitorder source scispw gaintable') 


class T2_4MDetailsUVcontFitRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='uvcontfit.mako',
                 description='UV continuum subtraction',
                 always_rerender=False):
        super(T2_4MDetailsUVcontFitRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        rows = []

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)

            rows.extend(self.get_uvtable_rows(context, result, ms))

        table_rows = utils.merge_td_columns(rows)
        ctx.update({
            'table_rows': table_rows
        })

    def get_uvtable_rows(self, context, result, ms):
        table_rows = []
        
        # Construct the rows. All entries must be strings.
        for calapp in result.final:

            # Define solint
            solint = result.inputs['solint']
            if solint == 'inf':
                solint = 'Infinite'
            
            # Convert solint=int to a real integration time. 
            # solint is spw dependent; science windows usually have the same
            # integration time, though that's not guaranteed.
            #    Leave out this for now
            #if solint == 'int':
                #in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6) 
                           #for dt in utils.get_intervals(context, calapp)]
                #solint = 'Per integration (%s)' % utils.commafy(in_secs, quotes=False, conjunction='or')
            
            gaintable = os.path.basename(calapp.gaintable)

            #to_spws = ', '.join(calapp.spw.split(','))

            # Source is composed of source name and intent
            to_intent = ', '.join(calapp.intent.split(','))
            if to_intent == '':
                to_intent = 'ALL'
            to_field = ', '.join(calapp.field.split(','))
            if to_field == '':
                to_field = 'ALL'
            to_source = to_field + ' ' + to_intent

            # Get the fit order
            fitorder = '%d' % result.inputs['fitorder']

            # Get the input spws and associated frequency ranges
            spws, freqranges = self._get_spw_ranges(result.spwstr)

            # Create the table
            for spw, freqrange in zip(spws, freqranges):
                frange = ', '.join(freqrange)
                row = UVcontFitApplication(ms.basename,  frange, solint, fitorder,
                    to_source, spw, gaintable)
                table_rows.append(row)

        return table_rows

    def _get_spw_ranges (self, spwranges):

        '''
        Decode the spw / frequency ranges string int a list of spws and
        frequency ranges
        '''
        spws = []; freqranges = []
        for spwrange in spwranges.split(','):
            tspwrange = tuple (spwrange.split(':'))
            spws.append(tspwrange[0])
            if len(tspwrange) > 1:
                freqranges.append(tspwrange[1].split(';'))
            else:
                freqranges.append([])

        return spws, freqranges
