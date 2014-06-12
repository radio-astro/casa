import contextlib
import multiprocessing
import os
import Queue
from threading import *
import time

from taskinit import gentools, ms
#import mtpy
import reductionhelper_util as rhutil

def dbgPrint(msg):
	#print(msg)
	pass

class Context(object):
	# Attributes are inQ, inCv, outQ, outCv, qLen, pendingItems
	pass

class EndOfDataException(BaseException):
	pass

EOD = EndOfDataException() # singleton instance. Use 'is' to compare.

def worker(func, context):
	try:
		while True:
			item = None
			with context.inCv:
				while True:
					try:
						item = context.inQ.get(False)
						if item is EOD:
							raise item
						break
					except Queue.Empty:
						context.inCv.wait()
			try:
				result = func(item)
			except Exception as e:
				result = e
			with context.outCv:
				context.outQ.put(result)
				context.outCv.notify()
	except EndOfDataException:
		pass
	finally:
		thr_id = current_thread().ident
		dbgPrint("{0} terminated".format(thr_id))

# out of order and parallel execution generator
def paraMap(numThreads, func, dataSource):
	assert numThreads > 0
	context = Context()
	context.qLen = int(numThreads * 1.5)
	assert context.qLen >= numThreads
	context.inQ = Queue.Queue(maxsize=context.qLen)
	context.inCv = Condition()
	context.outQ = Queue.Queue(maxsize=context.qLen)
	context.outCv = Condition()
	context.pendingItems = 0
	threads = []
	for i in range(numThreads):
		thr = Thread(target=worker, args=(func, context))
		thr.daemon = True
		thr.start()
		threads.append(thr)
	def fillInQ(context):
		try:
			while context.pendingItems < context.qLen:
				item = dataSource.next()
				with context.inCv:
					context.inQ.put(item, False)
					context.pendingItems += 1
					context.inCv.notify()
		except Queue.Full:
			assert False
	def putEODIntoInQ(context):
		try:
			with context.inCv:
				context.inQ.put(EOD, False)
				context.pendingItems += 1
				context.inCv.notify()
		except Full:
			assert False
	def getFromOutQ(context):
		assert 0 < context.pendingItems and context.pendingItems <= context.qLen
		with context.outCv:
			while True:
				try:
					item = context.outQ.get(False)
					context.pendingItems -= 1
					return item
				except Queue.Empty:
					context.outCv.wait()
	try:
		fillInQ(context)
		assert 0 < context.pendingItems and context.pendingItems <= context.qLen
		while context.pendingItems > 0:
			item = getFromOutQ(context)
			assert 0 <= context.pendingItems and context.pendingItems < context.qLen
			try:
				fillInQ(context)
				assert 0 < context.pendingItems and context.pendingItems <= context.qLen
			finally:
				yield item
	except StopIteration as e:
		while context.pendingItems > 0:
			yield getFromOutQ(context)
	assert context.pendingItems == 0
	for i in range(numThreads):
		assert context.pendingItems < context.qLen
		putEODIntoInQ(context)

def generate_query(vis, field=None, spw=None, timerange=None, antenna=None, scan=None, pol=None, observation=None, msselect=None):
    tb = gentools(['tb'])[0]
    try:
        tb.open(os.path.join(vis, 'DATA_DESCRIPTION'))
        num_data_desc_id = tb.nrows()
        data_desc_id_by_spw = {}
        for i in xrange(num_data_desc_id):
            data_desc_id_by_spw[tb.getcell('SPECTRAL_WINDOW_ID', i)] = i
    finally:
        tb.close()

    try:
        tb.open(os.path.join(vis, 'ANTENNA'))
        num_antenna_id = tb.nrows()
    finally:
        tb.close()

    try:
        ms.open(vis)
        for data_desc_id in xrange(num_data_desc_id):
            for antenna_id in xrange(num_antenna_id):
                valid_selection = True
                ms.selectinit(datadescid=data_desc_id)
                ms.selecttaql('ANTENNA1==' + str(antenna_id)+' && ANTENNA2==' + str(antenna_id))

                try:
                    if field is not None: ms.msselect({'field':field})
                except:
                    valid_selection = False

                try:
                    if spw is not None: ms.msselect({'spw':spw})
                except:
                    valid_selection = False

                try:
                    if timerange is not None: ms.msselect({'time':timerange})
                except:
                    valid_selection = False

                try:
                    if antenna is not None: ms.msselect({'taql':'ANTENNA1==' + str(antenna)})
                except:
                    valid_selection = False

                try:
                    if scan is not None: ms.msselect({'scan':scan})
                except:
                    valid_selection = False

                """
                try:
                    if pol is not None: ms.msselect({'polarization':pol})
                except:
                    valid_selection = False
                """

                try:
                    if observation is not None: ms.msselect({'observation':observation})
                except:
                    valid_selection = False

                try:
                    if msselect is not None: ms.msselect({'taql':msselect})
                except:
                    valid_selection = False

                if valid_selection:
                    res = 'DATA_DESC_ID==' + str(data_desc_id)
                    res += ' && ANTENNA1==' + str(antenna_id)
                    res += ' && ANTENNA2==' + str(antenna_id)

                    selected_idx = ms.msselectedindices()

                    dd_matches = True
                    if len(selected_idx['spw']) > 0:
                        data_desc_id_list = []
                        for i in xrange(len(selected_idx['spw'])):
                            data_desc_id_list.append(data_desc_id_by_spw[selected_idx['spw'][i]])
                        try:
                            idx = data_desc_id_list.index(data_desc_id)
                        except:
                            dd_matches = False
                            
                    if dd_matches:
                        ant_matches = True
                        if len(selected_idx['antenna1']) > 0:
                            try:
                                selected_idx['antenna1'].index(antenna_id)
                            except:
                                ant_matches = False
                        if len(selected_idx['antenna2']) > 0:
                            try:
                                selected_idx['antenna2'].index(antenna_id)
                            except:
                                ant_matches = False

                        if ant_matches:
                            if len(selected_idx['field']) > 0:
                                res += ' && FIELD_ID IN ' + str(selected_idx['field'])
                            if timerange is not None:
                                res += ' && ' + rhutil.select_by_timerange(vis, timerange)
                            if len(selected_idx['scan']) > 0:
                                res += ' && SCAN_NUMBER IN ' + str(selected_idx['scan'])
                            if len(selected_idx['observationid']) > 0:
                                res += ' && OBSERVATION_ID IN ' + str(selected_idx['observationid'])
                            if msselect is not None:
                                res += ' && ' + str(msselect)

                            idx_channel = selected_idx['channel']
                            res_channel = str(idx_channel) if len(idx_channel) > 0 else ''
                            res_pol = pol if pol is not None else ''

                            yield res, res_channel, res_pol

                ms.reset()
    finally:
        ms.close()

@contextlib.contextmanager
def opentable(vis):
    print 'open table %s'%(vis)
    tb = gentools(['tb'])[0]
    tb.open(vis, nomodify=False)
    yield tb
    print 'closing table %s'%(vis)
    tb.close()

def optimize_thread_parameters(vis, data_desc_id, antenna_id):
    try:
        tb = gentools(['tb'])[0]
        tb.open(vis)
        query_str = 'DATA_DESC_ID==' + str(data_desc_id) + ' && ANTENNA1==' + str(antenna_id) + ' && ANTENNA2==' + str(antenna_id)
        subt = tb.query(query_str)
        num_rows = subt.nrows()
        data = subt.getcell('FLOAT_DATA', 0)
        num_pols = len(data)
        num_channels = len(data[0])
        data_size_per_record = num_pols * num_channels * (8 + 1) * 2 #dummy
        assert data_size_per_record > 0
    finally:
        tb.close()
        subt.close()

    num_cores = multiprocessing.cpu_count()
    num_threads = num_cores - 1 if (num_cores > 1) else 1
    assert num_threads > 0

    mem_size = 8*1024*1024*1024 #to be replaced with an appropriate function
    num_record = mem_size / num_threads / data_size_per_record
    if (num_record > num_rows): num_record = num_rows
    assert num_record > 0

    return num_record, num_threads

def readchunk(table, criteria, nrecord):
    tb = table.query(criteria)
    nrow = tb.nrows()
    rownumbers = tb.rownumbers()
    tb.close()
    nchunk = nrow / nrecord 
    for ichunk in xrange(nchunk):
        start = ichunk * nrecord
        end = start + nrecord
        chunk =  _readchunk(table, rownumbers[start:end])
        #print 'readchunk:',chunk
        yield chunk

    # residuals
    residual = nrow % nrecord
    if residual > 0:
        start = nrow - residual
        end = nrow
        chunk = _readchunk(table, rownumbers[start:end])
        #print 'readchunk:',chunk
        yield chunk
        
def _readchunk(table, rows):
    print '_readchunk: reading rows %s...'%(rows)
    return tuple((_readrow(table, irow) for irow in rows))

def _readrow(table, row):
    get = lambda col: table.getcell(col, row)
    return (row, get('FLOAT_DATA'), get('FLAG'), get('TIME_CENTROID'))
            
def reducechunk(chunk):
    return tuple((reducerecord(record) for record in chunk))

def reducerecord(record):
    print 'reducing row %s'%(record[0])
    data, flag, stats = reducedata(record[0], record[1], record[2], record[3])
    return (record[0], data, flag, record[3], stats)

def reducerecord2(record):
    data, mask = tosakura(record[1], record[2])
    data, mask = calibratedata(data, mask, record[3])
    mask = masknanorinf(data, mask)
    mask = maskedge(data, mask)
    data, mask = baselinedata(data, mask)
    mask = clipdata(data, mask)
    data = smoothdata(data, mask)
    stats = calcstats(data, mask)
    data, flag = tocasa(data, mask)
    yield (record[0], data, flag, record[3], stats)
    
def writechunk(table, results):
    put = lambda row, col, val: table.putcell(col, row, val)
    for record in results:
        row = int(record[0])
        data = record[1]
        flag = record[2]
        print 'writing result to table %s at row %s...'%(table.name(), row)
        put(row, 'FLOAT_DATA', data)
        put(row, 'FLAG', flag)

###
def reducedata(row, data, flag, timestamp):
    data[:] = float(row)
    print 'reducing row %s...'%(row)
    #mtpy.wait_for(5, 'row%s'%(row))
    print 'done reducing row %s...'%(row)
    return data, flag, {'statistics': data.real.mean()}

