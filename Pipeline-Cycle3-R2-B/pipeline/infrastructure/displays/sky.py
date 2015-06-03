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
"""Module to plot sky images."""

# History:

# package modules
import os
import numpy as np
import matplotlib.pyplot as plt
import string

# alma modules
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger

LOG = infrastructure.get_logger(__name__)

_valid_chars = "_.%s%s" % (string.ascii_letters, string.digits)
def _char_replacer(s):
    '''A small utility function that echoes the argument or returns '_' if the
    argument is in a list of forbidden characters.
    '''
    if s not in _valid_chars:
        return '_'
    return s

def sanitize(text):
    filename = ''.join(_char_replacer(c) for c in text)
    return filename

def plotfilename(image, reportdir):
    name = '%s.sky.png' % (os.path.basename(image))
    name = sanitize(name)
    name = os.path.join(reportdir, name)
    return name


class SkyDisplay(object):
    """Class to plot sky images."""

    def plot(self, context, result, reportdir, intent=None):

        if not result:
            return []

        plotfile, coord_names, field = self._plot_panel(reportdir, result)

        # field names may not be unique, which leads to incorrectly merged
        # plots in the weblog output. As a temporary fix, change to field +
        # intent - which is better but again, not guaranteed unique.
        if intent:
            field = '%s (%s)' % (field, intent)

        with casatools.ImageReader(result) as image:
            info = image.miscinfo()
            parameters = {}
            if info.has_key('spw'): parameters['spw'] = info['spw']
            if info.has_key('pol'): parameters['pol'] = info['pol']
            if info.has_key('field'): parameters['field'] = info['field']
            if info.has_key('type'): parameters['type'] = info['type']
            if info.has_key('iter'): parameters['iter'] = info['iter']
            parameters['ant'] = None

        plot = logger.Plot(plotfile,
          x_axis=coord_names[0], y_axis=coord_names[1], field=field,
          parameters=parameters)

        return plot

    def _plot_panel(self, reportdir, result, vmin=None, vmax=None,
     channelMap=False):
        """Method to plot a map."""
        plotfile = plotfilename(image=os.path.basename(result),
          reportdir=reportdir)

        with casatools.ImageReader(result) as image:
            collapsed = image.collapse(function='mean', axes=[2,3])
            name = image.name(strippath=True)
            coordsys = collapsed.coordsys()
            coord_names = coordsys.names()
            coordsys.setunits(type='direction', value='arcsec arcsec')
            coord_units = coordsys.units()
            coord_refs = coordsys.referencevalue(format='s')
            beam = collapsed.restoringbeam()
            brightness_unit = collapsed.brightnessunit()
            miscinfo = collapsed.miscinfo()

            # don't replot if a file of the required name already exists
            if os.path.exists(plotfile):
                LOG.info('plotfile already exists: %s', plotfile) 
                return plotfile, coord_names, miscinfo.get('field')

            # otherwise do the plot
            data = collapsed.getchunk() 
            shape = np.shape(data)
            data = data.reshape(shape[0], shape[1])

            collapsed.done()

            # get x and y axes from coordsys of image
            xpix = np.arange(shape[0])
            x = np.zeros(np.shape(xpix))
            for pix in xpix:
                world = coordsys.toworld([float(pix),0,0,0])
                relative = coordsys.torel(world)
                x[pix] = relative['numeric'][0]

            ypix = np.arange(shape[1])
            y = np.zeros(np.shape(ypix))
            for pix in ypix:
                world = coordsys.toworld([0,float(pix),0,0])
                relative = coordsys.torel(world)
                y[pix] = relative['numeric'][1]

            # remove any incomplete matplotlib plots, if left these can cause
            # weird errors
            plt.close('all')
            plt.figure(1)

            # plot data
            plt.jet()
            plt.imshow(np.transpose(data), interpolation='nearest',
              origin='lower', aspect='equal', extent=[x[0], x[-1], y[0],
              y[-1]], vmin=vmin, vmax=vmax)

            plt.axis('image')
            lims = plt.axis()

            # make ticks and labels white
            ax = plt.gca()
            for line in ax.xaxis.get_ticklines() + ax.yaxis.get_ticklines():
                line.set_color('white')
            for label in ax.xaxis.get_ticklabels() + ax.yaxis.get_ticklabels():
                label.set_fontsize(0.5 * label.get_fontsize())

            # colour bar
            cb = plt.colorbar(shrink=0.5)
            fontsize = 8
            for label in cb.ax.get_yticklabels() + cb.ax.get_xticklabels():
                label.set_fontsize(fontsize)

            cb.set_label(brightness_unit, fontsize=fontsize)

            # image reference pixel    
            yoff = 0.10
            yoff = self.plottext(1.05, yoff, 'Reference position:', 40)
            for i,k in enumerate(coord_refs['string']):
                yoff = self.plottext(1.05, yoff, '%s: %s' %
                  (coord_names[i], k), 40, mult=0.8)

            # plot beam
            cqa = casatools.quanta
            if beam.has_key('major'):
                bpa = beam['positionangle']
                bpa = cqa.quantity('%s%s' % (bpa['value'], bpa['unit']))
                bpa = cqa.convert(bpa, 'rad')
                bpa = bpa['value']
                bpa += np.pi/2.0
                bmaj = beam['major']
                bmaj = cqa.quantity('%s%s' % (bmaj['value'], bmaj['unit']))
                bmaj = cqa.convert(bmaj, 'arcsec')
                bmaj = bmaj['value']
                bmin = beam['minor']
                bmin = cqa.quantity('%s%s' % (bmin['value'], bmin['unit']))
                bmin = cqa.convert(bmin, 'arcsec')
                bmin = bmin['value']

                xbeam = []
                ybeam = []
                for i in range(37):
                    theta = i*10.0*np.pi / 180.0
                    xbeam.append(0.5 * (
                      bmaj*np.sin(theta)*np.cos(bpa) + 
                      bmin*np.cos(theta)*np.sin(bpa)))
                    ybeam.append(0.5 * (
                      -bmaj*np.sin(theta)*np.sin(bpa) +
                      bmin*np.cos(theta)*np.cos(bpa)))

                xbeam = np.array(xbeam) + lims[0] + 0.1 * (lims[1]-lims[0])
                ybeam = np.array(ybeam) + lims[2] + 0.1 * (lims[3]-lims[2])
                plt.plot(xbeam, ybeam, color='yellow')

            # print title
            plt.xlabel('%s (%s)' % (coord_names[0], coord_units[0]))
            plt.ylabel('%s (%s)' % (coord_names[1], coord_units[1]))

            label = ''.join('%s:%s ' % (key,miscinfo[key]) for key in ['type',
              'field', 'spw', 'pol', 'iter'] if miscinfo.get(key) is not None)
            plt.figtext(0.1, 0.95, label, ha='left')

            # make axis fit snugly around image
            plt.axis([lims[0], lims[1], lims[2], lims[3]])

            # save the image
            plt.savefig(plotfile)

            plt.clf()
            plt.close(1)

            return plotfile, coord_names, miscinfo.get('field')

    def plottext(self, xoff, yoff, text, maxchars, ny_subplot=1, mult=1):
        """Utility method to plot text and put line breaks in to keep the
        text within a given limit.

        Keyword arguments:
        xoff       -- world x coord where text is to start.
        yoff       -- world y coord where text is to start.
        text       -- Text to print.
        maxchars   -- Maximum number of characters before a newline is
                      inserted.
        ny_subplot -- Number of sub-plots along the y-axis of the page.
        mult       -- Factor by which the text fontsize is to be multiplied.
        """

        words = text.rsplit()
        words_in_line = 0
        line = ''
        ax = plt.gca()
        for i in range(len(words)):
            temp = line + words[i] + ' '
            words_in_line += 1
            if len(temp) > maxchars:
                if words_in_line == 1:
                    ax.text(xoff, yoff, temp, va='center', fontsize=mult*8,
                      transform=ax.transAxes, clip_on=False)
                    yoff -= 0.03 * ny_subplot * mult
                    words_in_line = 0
                else:
                    ax.text(xoff, yoff, line, va='center', fontsize=mult*8,
                      transform=ax.transAxes, clip_on=False)
                    yoff -= 0.03 * ny_subplot * mult
                    line = words[i] + ' '
                    words_in_line = 1
            else:
                line = temp
        if len(line) > 0:
            ax.text(xoff, yoff, line, va='center', fontsize=mult*8,
              transform=ax.transAxes, clip_on=False)
            yoff -= 0.03 * ny_subplot * mult
        yoff -= 0.01 * ny_subplot * mult
        return yoff
