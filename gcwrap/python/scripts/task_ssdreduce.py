from taskinit import casalog

import libsakurapy
import reductionhelper as rh

DO_TEST=True

def ssdreduce(vis,
             field, spw,
             selectdata, timerange, antenna, scan, pol, observation, msselect,
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
    context = rh.initcontext(vis, spw, antenna, gaintable, interp, spwmap,
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
        query_list = rh.generate_query(vis, field, spw, timerange, antenna,
                                       scan, pol, observation, msselect)
    else:
        query_list = rh.generate_query(vis, field, spw)

    # start reduction
    with rh.opentable(vis) as table:
        spwidmap = rh.spw_id_map(vis)
        for query in query_list:
            num_record, num_threads = rh.optimize_thread_parameters(table, query, spwmap)
            #print 'opt_th_param : num_record='+str(num_record)+', num_threads='+str(num_threads)

            if num_record > 0:
                for results in rh.paraMap(num_threads, rh.reducechunk, rh.readchunk(table, query[0], num_record, rh.get_context(query, spwidmap, context))):
                    rh.writechunk(table, results)
