from __future__ import absolute_import

from . import logging

LOG = logging.get_logger(__name__)


# This class contains a list ImageItem objects encoded as dictionaries.

class ImageLibrary(object):
    def __init__(self):
        self._images = []

    # Return the image list.
    def get_imlist(self):
        return self._images

    # Clear the image list
    def clear_imlist (self):
        del self._images[:]

    # Add image item to the list as a dictionary
    def add_item (self, imageitem):
        if self.find_imageitem (imageitem) < 0:
            self._images.append(dict(imageitem))
        else:
            LOG.warning('Image item %s already in list' % \
                    (imageitem.imagename))

    # Remove image item from the list
    def delete_item(self, imageitem):
        index = self.find_imageitem (imageitem)
        if index >= 0:
            del self._images[index]

    # Return the index of the item in the list or -1.
    def find_imageitem(self, imageitem):
        for i in range(len(self._images)):
            if imageitem.imagename == self._images[i]['imagename']:
                return i
        return -1

# This class contains information for image data product

class ImageItem (object):
    def __init__(self, imagename, sourcename, spwlist, sourcetype,
        imageplot ='', metadata={}):
        self.imagename = imagename
        self.sourcename = sourcename
        self.spwlist = spwlist
        self.sourcetype = sourcetype
        self.imageplot = imageplot
        self._metadata = metadata
    def __iter__(self):
        return vars(self).iteritems()

# This class defines the image meta data that will be stored for
# each final cleaned image.

class ImageMetadata (dict):
    # This is a test.
    _keys = ['IMCENTER', 'IMRES']
    def __init__(self):
        dict.__init__(self)
        for key in ImageMetadata._keys:
            self[key] = None
    def __setitem__(self, key, val):
        if key not in ImageMetadata._keys:
            raise KeyError
        dict.__setitem__(self, key, val)
