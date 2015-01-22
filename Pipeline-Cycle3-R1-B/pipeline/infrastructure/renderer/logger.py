#*******************************************************************************
# ALMA - Atacama Large Millimeter Array
# Copyright (c) ATC - Astronomy Technology Center - Royal Observatory Edinburgh, 2011
# (in the framework of the ALMA collaboration).
# All rights reserved.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#*******************************************************************************
import collections
import distutils.spawn as spawn
import itertools
import os
import platform
import re
import string
import subprocess

import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


# Set the command used to shrink plots down to thumbnails. If set to None, no
# thumbnails will be generated 
THUMBNAIL_CMD = None 

if platform.system() == 'Darwin':
    # Look for sips rather than ImageMagick on OS X. sips is a system
    # executable that should be available on all OS X systems.
    sips_path = spawn.find_executable('sips')
    if sips_path:
        LOG.trace('Using sips executable at \'%s\' to generate thumbnails' 
                  % sips_path)
        THUMBNAIL_CMD = lambda full, thumb : (sips_path, '-z', '188', '250',
                                              '--out', thumb, full)
else:
    # .. otherwise try to find ImageMagick's 'mogrify' command. We assume that
    # ImageMagick's 'convert' commnand can be found in the same directory. We
    # do not search for 'convert' directly as some utilities also provide a
    # 'convert' command which may come earlier on the PATH.      
    mogrify_path = spawn.find_executable('mogrify')
    if mogrify_path:
        bin_dir = os.path.dirname(mogrify_path)
        convert_path = os.path.join(bin_dir, 'convert')
        if os.path.exists(convert_path):
            LOG.trace('Using convert executable at \'%s\' to generate '
                      'thumbnails' % convert_path)
            THUMBNAIL_CMD = lambda full, thumb : (convert_path, full, 
                    '-thumbnail', '250x188', thumb)
        else:
            LOG.warning('Could not find ImageMagick \'convert\' command. '
                        'Thumbnails will not be generated, leading to slower '
                        'web logs.')
    else:
        LOG.warning('ImageMagick is not installed. Thumbnails will not be '
                    'generated, leading to slower web logs.')

def getPath(filename):
    path = os.path.join(os.path.dirname(__file__), filename)
    return path


class Parameters(object):
    """
    Provides a set of utility functions that describe how the plot parameters
    given as keys of the optional parameters dictionary given to Plot() should
    be interpreted.
    """
    css_ids = collections.defaultdict(itertools.repeat('Unhandled').next,
                                      { 'ant'    : 'ant',
                                        'pol'    : 'pol',
                                        'spw'    : 'spw',
                                        'map'    : 'map',
                                        'intent' : 'intent',
                                        'field'  : 'field',
                                        'type'   : 'type',
                                        'chnl'   : 'chnl',
                                        'solint' : 'solint',
                                        'vis'    : 'vis',
                                        'file'   : 'file'    })
                                                                             
    descriptions = collections.defaultdict(itertools.repeat('Unknown').next,
                                           { 'ant'    : 'Antenna',
                                             'spw'    : 'Spectral Window',
                                             'pol'    : 'Polarisation',
                                             'map'    : 'Map',
                                             'intent' : 'Intent',
                                             'field'  : 'Field',
                                             'type'   : 'Type',
                                             'chnl'   : 'Channels',
                                             'solint' : 'Solution Interval',
                                             'vis'    : 'Measurement Set',
                                             'file'   : 'File'   })

    @staticmethod
    def getCssId(parameter):
        """
        getCssId(s) -> string

        Get the CSS class associated with this parameter in HTML output.
        """
        return Parameters.css_ids[parameter]
    
    @staticmethod
    def getDescription(parameter):
        """
        getCssId(s) -> string

        Get the plain English description of this parameter.
        """
        t = Parameters.descriptions.get(parameter)
        if t == None:
            return str(parameter) + ' (unknown parameter)' 
        else:
            return t


class PlotGroup(object):
    # the full template for the selectors in this series of plots
    full_template = string.Template('<li class="selectorHeading">$description'
        + ':<ul class="selector">\n$selectors</ul>\n</li>\n')

    # the template used to create a parameter selector
    selector_template = \
        string.Template('<li id="$prefix$css_class">'
                        '<a class="button small pill" href="#">$val</a></li>')

    @staticmethod
    def create_plot_groups(plots=[]):
        """
        Returns a list of PlotGroups, each containing a series of plots with
        the same axes.
        """
        # defaultdict requires a callable constructor argument; we pass a
        # lambda function that returns another defaultdict
        grouped = collections.defaultdict(lambda: collections.defaultdict(list))
        for plot in itertools.chain.from_iterable(plots):
            grouped[plot.x_axis][plot.y_axis].append(plot)

        plot_groups = []        
        # create a dictonary like groups{"phase"}{"time"}=[plot1,plot2,plot3]
        for y_axes in grouped.values():
            for plots_with_common_axes in y_axes.values():
                plot_groups.append(PlotGroup(plots_with_common_axes))
        
        return plot_groups

    def __init__(self, plots=[]):
        self.plots = plots

    @property
    def x_axis(self):
        if len(self.plots) > 0:
            return self.plots[0].x_axis
        else:
            return 'Unknown'

    @property
    def y_axis(self):
        if len(self.plots) > 0:
            return self.plots[0].y_axis
        else:
            return 'Unknown'

    @property
    def title(self):
        title = string.join([string.capwords(self.y_axis),
                             'vs',
                             string.capwords(self.x_axis)])
        if title == 'Dec Offset vs Ra Offset':
            title = 'Image Maps (Dec Offset vs RA Offset)'
        
        return title

    @staticmethod
    def numericalSort(selector):
        """
        Sorts strings numerically, eg. [b13, a2, a10] -> [a2, a10, b13]
        
        Based on Recipe 5.5 from Python Cookbook 2nd Edition.
        """
        re_digits = re.compile(r'(\d+)')
        # split into digits/non-digits
        pieces = re_digits.split(selector.value) 
        pieces[1::2] = map(int, pieces[1::2]) # turn digits into numbers
        return pieces

    def _get_selectors(self, parameter):
        values = self._get_parameter_values(parameter)
        selectors = [Selector(parameter, v) for v in values]
        selectors.sort(key=PlotGroup.numericalSort)
        return selectors

    def _get_parameter_values(self, parameter):
        """
        Get the unique set of values for the given parameter.
        """
        # create a list of all the values for the given parameter
        values = [str(plot.parameters.get(parameter)) for plot in self.plots]
        # remove duplicate values by returning a set
        return set(values)

    @property
    def selectors(self):
        parameter_names = collections.defaultdict(int)
        # determine unique parameter names for our plots
        for plot in self.plots:
            for k in plot.parameters.keys():
                parameter_names[k] += 1
        # get a list of selectors for each unique parameter
        selectors = [self._get_selectors(p) for p in parameter_names.keys()]
        # remove any redundant selectors
        selectors = [s for s in selectors if len(s) > 1]
        # numerically sort selectors by value
#        selectors.sort(key=PlotGroup.numericalSort)
        return selectors
    
    @property
    def thumbnails(self):
        html = [plot.getThumbnailHtml() for plot in self.plots]
        html.sort(key=PlotGroup.numericalSort)
        return string.join(html, '\n')

    @property
    def buttons(self):
        # the button HTML used if we have filters added 
        clearFilterButton = ('<input type="button" name="clear" id="clearbutton" '
                            'value="Clear All Filters">')
        backButton = ('<input type="button" name="back" value="Back" '
                      'onClick="javascript:history.back();">')
        if len(self.selectors) > 0:
            return backButton + '\n\t' + clearFilterButton
        else:
            return backButton
                
    def __repr__(self):
        return self.toHtml()
                

class Selector(object):
    # CSS classes have a restricted character sets, so we use a regex to 
    # remove them
    _regex = re.compile('\W')
    
    def __init__(self, parameter, value):
        self.value = string.capwords(value)
        self.prefix = Parameters.getCssId(parameter)
        self.description = Parameters.getDescription(parameter)
        self.css_class = '%s%s' % (self.prefix,
                                   ''.join(self._regex.split(self.value)))

    def __repr__(self):
        return 'Selector(css_class=%s, prefix=%s, description=%s, value=%s)' % (
            self.css_class, self.prefix, self.description, self.value)


class Plot(object):
    def __init__(self, filename, x_axis='Unknown', y_axis='Unknown', 
                 field=None, parameters={}, qa_score=None):
        """
        Plot(filename, x_axis, y_axis, field, parameters)

        filename - the filename of the plot
        x_axis - what the X axis of this plot measures
        y_axis - what the Y axis of this plot measures
        field - the name of the source or field which this data corresponds to
        parameters - a dictionary of parameters, eg. { 'ant' : 1, 'spw' : 2 }. These
            parameters should be known to the logging.Parameters class.
        """
        self.basename = os.path.basename(filename)
        self.abspath = os.path.abspath(filename)
        self.field = field
        self.x_axis = x_axis
        self.y_axis = y_axis
        self.parameters = parameters
        if field is not None:
            self.parameters['field'] = field
        self.qa_score = qa_score

    @property
    def css_class(self):
        """
        The CSS class to be used for this plot.
        """
        regex = re.compile('\W')
        css_classes = [Parameters.getCssId(parameter) + ''.join(regex.split(str(val)))
                       for parameter, val in self.parameters.items()]
        return string.join(css_classes)

    @property
    def title(self):
        """title -> string

        Construct and return a meaningful plot title using the internal state (axis
        names, parameter names etc.) of this plot.
        """
        field = ''
        if self.field != None and len(self.field) > 0:
            # eg. 'M31: '
            field = string.strip(str(self.field)) + ': '

        params = ''
        if len(self.parameters) > 0:
            params = [string.join((Parameters.getDescription(str(k)), str(v)), ' ') 
                      for k, v in self.parameters.items() if k != "field"]
            params = string.join(params, ', ').rstrip(', ')
            # eg. ' for antenna 1, spectral window 2'
            params = ' for ' + params

        title = Plot.title_template.substitute(
            field=field,
            x_axis=string.capwords(str(self.x_axis)),
            y_axis=string.capwords(str(self.y_axis)),
            params=params)

        # eg. 'M31: Phase vs Time for Antenna 1, Spectral Window 2'
        return title

    @property
    def thumbnail(self):
        thumb_dir = os.path.join(os.path.dirname(self.abspath), 'thumbs')
        thumb_file = os.path.join(thumb_dir, os.path.basename(self.abspath))

        if os.path.exists(thumb_file):
            return thumb_file

        return self._create_thumbnail()
    
    def _create_thumbnail(self):
        """
        Create a scaled-down copy of the plot, returning the filename to be
        used as a thumbnail.

        :rtype: string
        """
        if THUMBNAIL_CMD is None:
            return self.abspath

        thumb_dir = os.path.join(os.path.dirname(self.abspath), 'thumbs')
        thumb_file = os.path.join(thumb_dir, os.path.basename(self.abspath))

        if not os.path.exists(thumb_dir):
            os.mkdir(thumb_dir) 

        if not os.path.exists(self.abspath):
            LOG.warning('Cannot create thumbnail. Original image not found: '
                        '%s', self.abspath)
            return self.basename

        # Set the command to perform. The module defines whether to use sips
        # or ImageMagick; all that remains is to append, in order, the output
        # and input files
        LOG.trace('Creating thumbnail with %s(%s, %s)',
                  THUMBNAIL_CMD, self.abspath, thumb_file)
        cmd = THUMBNAIL_CMD(self.abspath, thumb_file)

        try:
            with open(os.devnull, 'w') as dev_null:
                ret = subprocess.call(cmd, stdout=dev_null, stderr=dev_null)
            if ret is 0:
                # return code is 0: thumbnail file successfully created
                return thumb_file
            else:
                LOG.warning('Error creating thumbnail for %s' % 
                            os.path.basename(self.abspath))
                return self.basename   
        except OSError, e:
            # command not available. Return the full-sized image filename
            LOG.warning('Error creating thumbnail for %s: %s' % 
                        (os.path.basename(self.abspath), e))
            return self.basename

    def __repr__(self):
        return '<Plot(\'%s\')>' % self.abspath 
