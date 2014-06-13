from taskinit import casalog

import reductionhelper as rh

DO_TEST=False

def ssdreduce(vis,
             field, spw,
             selectdata, timerange, antenna, scan, pol, observaiton, msselect,
             gaintable, interp, spwmap,
             maskmode, thresh, avg_limit, edge, blmask,
             blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn,
             rejwn, clipthresh, clipniter,
             bloutput, blformat,
             clipminmax, 
             kernel, kwidth, usefft, interpflag,
             statmask, stoutput, stformat):

    if not DO_TEST:
        print locals()
        return

    # register reduction step here when ssdreduce becomes fully flexible
    #rh.register_step('calibration')
    #rh.register_step('flagnan')
    #rh.register_step('baseline')
    #rh.register_step('clipping')
    #rh.register_step('smoothing')
    #rh.register_step('statictics')
    
    # init context
    # I think context is implemented as a set of sub-contexts that 
    # correspond to each reduction step (calibration context, baseline
    # context, etc).
    # context: {spwid: (calibration_context,
    #                   baseline_context,
    #                   convolve1d_context,), ...}
    context = rh.initcontext(vis, spw, gaintable, interp, spwmap,
                             maskmode, thresh, avg_limit, edge, blmask,
                             blfunc, order, npiece, applyfft, fftmethod, fftthresh,
                             addwn, rejwn, clipthresh, clipniter,
                             bloutput, blformat,
                             clipminmax, 
                             kernel, kwidth, usefft, interpflag,
                             statmask, stoutput, stformat)
    
    # generate query string for reduction
    # query_list is a list of TaQL query per DATA_DESC_ID and ANTENNA
    # row_list instead of query_list?
    if selectdata:
        query_list = rh.generate_query(vis, field, spw)
        #row_list = rh.generate_rowlist(vis, field, spw)
    else:
        query_list = rh.generate_query(vis, field, spw, timerange, antenna,
                                       scan, pol, observation, msselect)
        #row_list = rh.generate_rowlist(vis, field, spw, timerange, antenna,
        #                               scan, pol, observation, msselect)

    # get optimized number of threads
    num_record, num_threads = rh.optimize_thread_parameters()

    # start reduction
    with rh.opentable(vis) as table:
        for query in query_list:
        #for query in row_list:
            for results in rh.paraMap(num_threads, rh.reducechunk, rh.readchunk(table, query, num_record)):
                rh.writechunk(table, results)

    
