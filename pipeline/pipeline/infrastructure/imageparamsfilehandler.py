#!/usr/bin/env python

'''
Class to handle imaging parameters files.

The text files contain imaging parameters from tasks like hifa_imageprecheck
which are to be used in the imaging stages. They can not be transported via
the context since the calibration and imaging pipelines are usually run
separately.
'''


class ImageParamsFileHandler(object):

    def __init__(self, filename='imageparams.dat'):
        self.filename = filename

    def read(self):
        imageparams = {'robust': 0.5, 'uvtaper': []}
        for item in open(self.filename, 'r').readlines():
            if 'robust=' in item:
                imageparams['robust'] = float(item.replace('\n', '').replace('\r', '').split('=')[1])
            if 'uvtaper=' in item:
                imageparams['uvtaper'] = eval(item.replace('\n', '').replace('\r', '').split('=')[1])

        return imageparams

    def write(self, robust, uvtaper):
        fd = open(self.filename, 'w+')
        fd.write('robust=%.2f\n' % (robust))
        fd.write('uvtaper=%s\n' % (uvtaper))
        fd.close()
