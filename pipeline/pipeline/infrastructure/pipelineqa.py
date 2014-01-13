from __future__ import absolute_import
import abc
import operator

from . import logging

LOG = logging.get_logger(__name__)


class QAScore(object):
    def __init__(self, score, longmsg='', shortmsg=''):
        self.score = score
        self.longmsg = longmsg
        self.shortmsg = shortmsg

    def __repr__(self):
        return 'QAScore(%s, "%s", "%s")' % (self.score, self.longmsg, self.shortmsg)


class QAScorePool(object):
    all_unity_longmsg = 'All QA completed successfully'
    # Use empty string as shortmsg if all scores = 1.0 to draw attention to
    # tasks with scores other than 1.0. 
    all_unity_shortmsg = ''
    
    def __init__(self):
        self.pool = []
        self._representative = None

    @property
    def representative(self):
        if self._representative is not None:
            return self._representative
        
        if not self.pool:
            return QAScore(None, 'No QA scores registered for this task', '')
        
        if all([s.score == 1.0 for s in self.pool]):
            return QAScore(1.0, self.all_unity_longmsg, self.all_unity_shortmsg)
        
        # maybe have different algorithms here. for now just return the
        # QAScore with minimum score
        return min(self.pool, key=operator.attrgetter('score'))

    @representative.setter
    def representative(self, value):
        self._representative = value


class QAResultHandler(object):
    __metaclass__ = abc.ABCMeta

    # the class this handler is expected to handle
    result_cls = None
    # if result_cls is a list, the type of classes it is expected to contain
    child_cls = None

    def is_handler_for(self, result):
        # if the result is not a list or the expected results class,
        # return False
        if not isinstance(result, self.result_cls):
            return False
        
        # this is the expected class and we weren't expecting any
        # children, so we should be able to handle the result
        if self.child_cls is None:
            return True

        try:
            if all([isinstance(r, self.child_cls) for r in result]):
                return True
            return False
        except:
            # catch case when result does not have a task attribute
            return False

    @abc.abstractmethod
    def handle(self, context, result):
        pass


class QAHandler(object):
    def __init__(self):
        self.__handlers = []

    def add_handler(self, handler):
        LOG.info('Registering %s as new pipeline QA handler' % handler.__class__.__name__)
        self.__handlers.append(handler)

    def do_qa(self, context, result):
        # if this result is a list, process the lower-level scalar results
        # first
        if isinstance(result, list):
            for r in result:
                self.do_qa(context, r)

        # so that the upper-level handler can collate the lower-level scores
        # or process as a group
        for handler in self.__handlers:
            if handler.is_handler_for(result):
                LOG.info('%s handling QA analysis for %s' % (handler.__class__.__name__,
                                                             result.__class__.__name__))
                handler.handle(context, result)


registry = QAHandler()
