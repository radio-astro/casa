from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.extern.asizeof as asizeof
import cPickle as pickle
import bz2
import time

LOG = logging.get_logger(__name__)

# object compression/decopmpression utility
class CompressedObj(object):
    def __init__(self, obj, protocol=pickle.HIGHEST_PROTOCOL, compresslevel=9):
        self.compressed = compress_object(obj, protocol=protocol, compresslevel=compresslevel)
        
    def decompress(self):
        return decompress_object(self.compressed)
        
def compress_object(obj, protocol=pickle.HIGHEST_PROTOCOL, compresslevel=9):
    size_org = asizeof.asizeof(obj)
    start = time.time()
    try:
        compressed = bz2.compress(pickle.dumps(obj, protocol), compresslevel=compresslevel)
    except:
        compressed = obj
    end = time.time()
    size_comp = asizeof.asizeof(compressed)
    LOG.info('compress: size before {0} after {1} ({2} %)'.format(size_org, size_comp, float(size_comp)/float(size_org) * 100))
    LOG.info('elapsed {0} sec'.format(end - start))
    return compressed

def decompress_object(obj):
    size_comp = asizeof.asizeof(obj)
    start = time.time()
    decompressed = pickle.loads(bz2.decompress(obj))
    end = time.time()
    size_org = asizeof.asizeof(decompressed)
    LOG.info('decompress: size before {0} after {1} ({2} %)'.format(size_org, size_comp, size_comp/size_org))
    LOG.info('elapsed {0} sec'.format(end - start))
    return decompressed

class CompressedIter(object):
    def __init__(self, obj):
        self.obj = obj
        self._count = 0
        
    def next(self):
        if self._count < len(self.obj):
            v = self.obj[self._count]
            self._count += 1
            if hasattr(v, 'decompress'):
                return v.decompress()
            else:
                return v
        else:
            raise StopIteration()
        
class CompressedList(list):
    def __iter__(self):
        return CompressedIter(self)