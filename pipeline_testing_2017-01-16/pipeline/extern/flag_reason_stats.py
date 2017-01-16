########################################
#
#   Function to count statistics as a function of 'reason'.
#
######################################
def task_reason_stats(msname='',field='',spw='',writeflags=True):

    # Run the task with rflag
    tf.open(msname=msname,ntime=100.0);
    tf.selectdata(field=field, spw=spw); # loose union...

    # Unflag
    ag0 = {'apply':True, 'sequential':True, 'mode':'unflag'}
    tf.parseagentparameters(ag0);

    # Flag Zeros
    ag1 = {'apply':True,'sequential':True,'mode':'clip', 'clipzeros':True}
    tf.parseagentparameters(ag1);
    # Summary
    ag2 = {'apply':True,'sequential':True,'mode':'summary','name':'Zeros'}
    tf.parseagentparameters(ag2);
    # TFCrop
    ag3 = {'apply':True,'sequential':True,'mode':'tfcrop'}
    tf.parseagentparameters(ag3);
    # Summary
    ag4 = {'apply':True,'sequential':True,'mode':'summary', 'name':'TFCrop'}
    tf.parseagentparameters(ag4);


    # Run all these agents, and get the combined-report.
    tf.init();
    summary_stats_list = tf.run(writeflags=False);
    tf.done();

    # Parse the output summary lists and extract only 'type==summary'
    ## Iterate through the list in the correct order. Do not follow default 'dictionary-key' ordering.
    summary_reps=[];
    for rep in range(0,summary_stats_list['nreport']):
        repname = 'report'+str(rep);
        if(summary_stats_list[repname]['type']=='summary'):
              summary_reps.append(summary_stats_list[repname]);

    # Step through the summary list and print a few things.
    ## SUBTRACT flag counts from previous agents, because the counts are cumulative.
    for ind in range(0,len(summary_reps)):

        flagcount = summary_reps[ind]['flagged'];
        totalcount = summary_reps[ind]['total'];
     
        # From the second summary onwards, subtract counts from the previous one :)
        if ( ind > 0 ):
             flagcount = flagcount - summary_reps[ind-1]['flagged'];

        print "Summary ", ind , "(" , summary_reps[ind]['name']  , ") :  Flagged : " , flagcount , " out of " , totalcount ;

    return summary_reps; 


##############################################


