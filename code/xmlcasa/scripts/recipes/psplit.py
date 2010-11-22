def psplit(vis,outputvispattern,nodes,field,nspw,
           outdir="",
           basename="/home/casa-dev-",
           datacolumn="corrected",width="1",antenna="",
           timebin="",timerange="",scan="",uvrange=""):

    nnodes=len(nodes);
    format=str('%2.2d');
    spwPerNode=nspw/nnodes;
    startSPW=0;
    endSPW=0;
    for i in range(nnodes):
        nodename=(format%nodes[i]);
        OutputName=basename+nodename+'/'+outdir+'/'+outputvispattern+'-'+nodename;
        endSPW=startSPW+spwPerNode;
        spwExpr=str(startSPW)+'~'+str(endSPW-1);
        startSPW=endSPW;
        print OutputName,spwExpr;
        
        split(vis=vis,outputvis=OutputName,
              datacolumn=datacolumn,
              field=field,
              spw=spwExpr,
              width=width,
              antenna=antenna,
              timebin=timebin,
              timerange=timerange,
              scan=scan,uvrange=uvrange);
