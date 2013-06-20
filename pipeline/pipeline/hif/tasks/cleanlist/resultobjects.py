from __future__ import absolute_import

import os.path

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.imagelibrary as imagelibrary


class CleanListResult(basetask.Results):
    def __init__(self):
        super(CleanListResult, self).__init__()
        self.targets = []
        self.results = []
        self.plot_path = None

    def add_result(self, result, target, outcome):
        target['outcome'] = outcome
        self.targets.append(target)
        self.results.append(result)

    def merge_with_context(self, context):
        # add the cleaned targets to the context
        for result in self.results:
            try:
                imageitem = imagelibrary.ImageItem(
                  imagename=result.image, sourcename=result.sourcename,
                  spwlist=result.spw, sourcetype=result.intent,
                  imageplot=result.imageplot)
                if 'TARGET' in result.intent:
                    context.sciimlist.add_item(imageitem)
                else:
                    context.calimlist.add_item(imageitem)
            except:
                pass

#        for item in context.sciimlist.get_imlist():
#            print 'science'
#            print item
#            print item['imagename']
#            print item['sourcename'], item['spwlist'], item['sourcetype']
#            print item['imageplot']

#        for item in context.calimlist.get_imlist():
#            print 'calib'
#            print item
#            print item['imagename']
#            print item['sourcename'], item['spwlist'], item['sourcetype']
#            print item['imageplot']

        # empty the pending list
        context.clean_list_pending = []
        
    def __repr__(self):
        repr = 'CleanList:'

        field_width = len('field')
        intent_width = len('intent')
        spw_width = len('spw')
        phasecenter_width = len('phasecenter')
        cell_width = len('cell')
        imsize_width = len('imsize')
        imagename_width = len('imagename')
        outcome_width = len('outcome')
        for target in self.targets:
            field_width = max(field_width, len(target['field']))
            intent_width = max(intent_width, len(target['intent']))
            spw_width = max(spw_width, len(target['spw']))
            phasecenter_width = max(phasecenter_width,
             len(target['phasecenter']))
            cell_width = max(cell_width, len(str(target['cell'])))
            imsize_width = max(imsize_width, len(str(target['imsize'])))
            imagename = os.path.basename(target['imagename'])
            imagename_width = max(imagename_width, len(imagename))
            outcome = target['outcome']
            outcome_width = max(outcome_width, len(outcome))

        field_width += 1
        intent_width += 1
        spw_width += 1
        phasecenter_width += 1
        cell_width += 1
        imsize_width += 1
        imagename_width += 1
        outcome_width += 1

        repr += '\n'
        repr += '{0:{1}}'.format('field', field_width)
        repr += '{0:{1}}'.format('intent', intent_width)
        repr += '{0:{1}}'.format('spw', spw_width)
        repr += '{0:{1}}'.format('phasecenter', phasecenter_width)
        repr += '{0:{1}}'.format('cell', cell_width)
        repr += '{0:{1}}'.format('imsize', imsize_width)
        repr += '{0:{1}}'.format('imagename', imagename_width)
        repr += '{0:{1}}'.format('outcome', outcome_width)

        for target in self.targets:
            repr += '\n'
            repr += '{0:{1}}'.format(target['field'], field_width)
            repr += '{0:{1}}'.format(target['intent'], intent_width)
            repr += '{0:{1}}'.format(target['spw'], spw_width)
            repr += '{0:{1}}'.format(target['phasecenter'], phasecenter_width)
            repr += '{0:{1}}'.format(target['cell'], cell_width)
            repr += '{0:{1}}'.format(target['imsize'], imsize_width)
            repr += '{0:{1}}'.format(os.path.basename(target['imagename']),
              imagename_width)
            repr += '{0:{1}}'.format(target['outcome'], outcome_width)

        return repr

