from parallel.pimager import pimager

def flagratio(msname,spw='*',timerange='',field='*',obsid=''):
    flag_summary=flagdata(vis=msname,mode='summary',field=field,timerange=timerange,spw=spw,observation=obsid)
    print "percentage data flagged in time range"+timerange+"is",((100*flag_summary['flagged'])/flag_summary['total']);

def checkpimagersetup(msname, npartition, spw='*', timerange='', field='*', obsid=''):
    tlist=pimager.findtimerange(msname=msname, field=field,spwids=spw,numpartition=npartition);
    print "Time range list: ";
    print tlist;
    n=len(tlist)
    for i in range(n):
        flagratio(msname=MSNAME, field=field, spw=spw,timerange=tlist[i]);
