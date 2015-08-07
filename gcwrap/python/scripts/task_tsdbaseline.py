import numpy
import os
#from taskinit import *
from taskinit import gentools, casalog
import sdutil
from collections import Counter
ms,sdms,tb = gentools(['ms','sdms','tb'])

def tsdbaseline(infile=None, datacolumn=None, antenna=None, field=None, spw=None, timerange=None, scan=None, pol=None, maskmode=None, thresh=None, avg_limit=None, minwidth=None, edge=None, blmode=None, dosubtract=None, blformat=None, bloutput=None, bltable=None, blfunc=None, order=None, npiece=None, applyfft=None, fftmethod=None, fftthresh=None, addwn=None, rejwn=None, clipthresh=None, clipniter=None, blparam=None, verify=None, verbose=None, showprogress=None, minnrow=None, outfile=None, overwrite=None):

    casalog.origin('tsdbaseline')
    try:
        if (outfile == '') or not isinstance(outfile, str):
            print("type=%s, value=%s" % (type(outfile), str(outfile)))
            raise ValueError, "outfile name is empty."
        if os.path.exists(outfile) and not overwrite:
            raise Exception(outfile + ' exists.')
        if (maskmode == 'interact'):
            raise ValueError, "maskmode='%s' is not supported yet" % maskmode
        if (blfunc == 'variable' and not os.path.exists(blparam)):
            raise ValueError, "input file '%s' does not exists" % blparam
        
        if (spw == ''): spw = '*'

        if blmode == 'apply':
            if not os.path.exists(bltable):
                raise ValueError, "file specified in bltable '%s' does not exist." % bltable

            with sdutil.tbmanager(infile + '/DATA_DESCRIPTION') as tb:
                spw_ids = tb.getcol('SPECTRAL_WINDOW_ID')
            with sdutil.tbmanager(infile + '/ANTENNA') as tb:
                ant_ids = range(tb.nrows())
            with sdutil.tbmanager(infile + '/FEED') as tb:
                feed_ids = numpy.unique(tb.getcol('FEED_ID'))

            sorttab_info = remove_sorted_table_keyword(infile)

            if overwrite and os.path.exists(outfile):
                os.system('rm -rf %s' % outfile)

            selection = ms.msseltoindex(vis=infile, spw=spw, field=field, 
                                        baseline=str(antenna), time=timerange, 
                                        scan=scan)
            sdms.open(infile)
            sdms.set_selection(spw=sdutil.get_spwids(selection), field=field, 
                               antenna=str(antenna), timerange=timerange, 
                               scan=scan)
            sdms.apply_baseline_table(bltable=bltable,
                                      datacolumn=datacolumn,
                                      spw=spw,
                                      outfile=outfile)
            sdms.close()
            
            restore_sorted_table_keyword(infile, sorttab_info)
            
        elif blmode == 'fit':
            blout_exists = False
            if (isinstance(bloutput, str) and os.path.exists(bloutput)):
                blout_exists = True
            elif isinstance(bloutput, list):
                for blout in bloutput:
                    if os.path.exists(blout):
                        blout_exists = True
                        break
            if blout_exists and not overwrite:
                raise ValueError, "file(s) specified in bloutput exists."

            selection = ms.msseltoindex(vis=infile, spw=spw, field=field, 
                                        baseline=str(antenna), time=timerange, 
                                        scan=scan)

            if blfunc == 'variable':
                sorttab_info = remove_sorted_table_keyword(infile)
        
            sdms.open(infile)
            sdms.set_selection(spw=sdutil.get_spwids(selection), field=field, 
                               antenna=str(antenna), timerange=timerange, 
                               scan=scan)

            
            #if(outfile==''):
            #    outfile=infile


            

            

#            if isinstance(blformat, str):
#                if('' == blformat):
#                    #blformat=',,'
#                    bloutput=',,'
#                    print 'blformat'
#                if('text' == blformat):
#                    bloutput_tmp = infile + '_blparam.txt'
#                    bloutput = ',' + bloutput_tmp + ','
#                    if os.path.exists(bloutput):
#                        raise Exception(fname + ' exists.') 





            new_bloutput=[]


            if isinstance(blformat, str):
                blformat = [blformat]
        
            

            if isinstance(bloutput, str):
                bloutput = [bloutput] 




            if isinstance(blformat, list):
                if('' in blformat):
                    blformat = ',,'
                #elif(len(blformat) > 3 ):
                #    raise ValueError, "The maximum size of blformat must be three."
                elif(len(blformat) != len(bloutput)):
                    raise ValueError, "The size of blfomat must be the same as that of bloutput."
                elif [key for key, val in Counter(blformat).items() if val>1]:
                    raise ValueError, "Same multiple blformats are not allowed."
                else:
                    if('csv' in blformat):
                        if(''!= bloutput[blformat.index('csv')]):
                            fname=bloutput[blformat.index('csv')]
                            if not overwrite and os.path.exists(fname):
                                raise Exception(fname + ' exists.') 
                            new_bloutput.append(bloutput[blformat.index('csv')])
                        else:
                            fname= infile + '_blparam.csv'
                            if not overwrite and os.path.exists(fname):
                                raise Exception(fname + ' exists. ')######################################################
                            new_bloutput.append(infile + '_blparam.csv')
                    else:
                         new_bloutput.append('')

                    

                    

                    if('text' in blformat):
                        if(''!= bloutput[blformat.index('text')]):
                            new_bloutput.append(bloutput[blformat.index('text')])
                        
                            fname = bloutput[blformat.index('text')]

                            
                            if not overwrite and os.path.exists(fname):
                                raise Exception(fname + ' exists.')


                            f = open(fname, "w")

                            blf = blfunc.lower()
                            mm = maskmode.lower()
                            if blf == 'poly':
                                ftitles = ['Fit order']
                            elif blf == 'chebyshev':
                                ftitles = ['Fit order']
                            elif blf == 'cspline':
                                ftitles = ['nPiece']
                                #print 'task 2'
                            else: # sinusoid
                                ftitles = ['applyFFT', 'fftMethod', 'fftThresh', 'addWaveN', 'rejWaveN']
                            if mm == 'auto':
                                mtitles = ['Threshold', 'avg_limit', 'Edge']
                            elif mm == 'list':
                                mtitles = []
                            else: # interact
                                mtitles = []
                            ctitles = ['clipThresh', 'clipNIter']

                            #fkeys = getattr(self, '%s_keys'%(blfunc))
                            #mkeys = getattr(self, '%s_keys'%(maskmode))
                            
                            outfile2=''
                            if(outfile==''): 
                                outfile2 = infile
                            else:
                                outfile2 = outfile
                            

                            info = [['Source Table', infile],
                                    ['Output File', outfile2]]
                                    #['Function', blfunc],
                                    #['Fit order', order]] 
                            #for i in xrange(len(ftitles)):
                            #    info.append([ftitles[i],getattr(self,fkeys[i])])
                            #if blf != 'poly':
                            #    for i in xrange(len(ctitles)):
                            #        info.append([ctitles[i],clip_keys[i]])
                            info.append(['Mask mode', maskmode])
                            #for i in xrange(len(mtitles)):
                            #    info.append([mtitles[i],getattr(mkeys[i])])

                            separator = "#"*60 + "\n"

                            f.write(separator)
                            for i in xrange(len(info)):
                                f.write('%12s: %s\n'%tuple(info[i]))
                            f.write(separator)
                            f.close()

                        
                        
                        else:
                            #new_bloutput.append(infile + '_blparam.txt')
                            new_bloutput.append(infile + '_blparam.txt')
                            fname2  = infile + '_blparam.txt'
                            if not overwrite and os.path.exists(fname2):
                                raise Exception(fname2 + ' exists.')

                            f = open(fname2, "w")

                            blf = blfunc.lower()
                            mm = maskmode.lower()
                            if blf == 'poly':
                                ftitles = ['Fit order']
                            elif blf == 'chebyshev':
                                ftitles = ['Fit order']
                            elif blf == 'cspline':
                                ftitles = ['nPiece']
                            else: # sinusoid
                                ftitles = ['applyFFT', 'fftMethod', 'fftThresh', 'addWaveN', 'rejWaveN']
                            if mm == 'auto':
                                mtitles = ['Threshold', 'avg_limit', 'Edge']
                            elif mm == 'list':
                                mtitles = []
                            else: # interact
                                mtitles = []
                            ctitles = ['clipThresh', 'clipNIter']

                            #fkeys = getattr(self, '%s_keys'%(blfunc))
                            #mkeys = getattr(self, '%s_keys'%(maskmode))

                            outfile2=''
                            if(outfile==''):
                                outfile2 = infile

                            info = [['Source Table', infile],
                                    ['Output File', outfile]]
                                    #['Function', blfunc],
                                    #['Fit order', order]] 
                            #for i in xrange(len(ftitles)):
                            #    info.append([ftitles[i],getattr(self,fkeys[i])])
                            #if blf != 'poly':
                            #    for i in xrange(len(ctitles)):
                            #        info.append([ctitles[i],clip_keys[i]])
                            info.append(['Mask mode', maskmode])
                            #for i in xrange(len(mtitles)):
                            #    info.append([mtitles[i],getattr(mkeys[i])])

                            separator = "#"*60 + "\n"

                            f.write(separator)
                            for i in xrange(len(info)):
                                f.write('%12s: %s\n'%tuple(info[i]))
                            f.write(separator)
                            f.close()       
                    
                   
                    else:
                        new_bloutput.append('')



                    if('table' in blformat):
                        #print 'blformat->',type(blformat), blformat
                        #print "blformat.index('table')", blformat.index('table')
                        #print "bloutput[blformat.index('table')])", bloutput[blformat.index('table')]

                        if(''!= bloutput[blformat.index('table')]):
                            fname = bloutput[blformat.index('table')]
                            if not overwrite and os.path.exists(fname):
                                raise Exception(fname + ' exists.')#############################################

                            new_bloutput.append(bloutput[blformat.index('table')])
                       
                        else:
                            fname = infile + '_blparam.bltable'
                            if not overwrite and os.path.exists(fname):
                                raise Exception(fname + ' exists.')#############################################
                                print ''
                            new_bloutput.append(fname)



                    else:
                        new_bloutput.append('')

                    blformat = ",".join(blformat)
                    bloutput = ",".join(new_bloutput)

            
            #print 'task blformat',type(blformat), blformat
            #print 'task bloutput',type(bloutput), bloutput
                



            params, func = prepare_for_baselining(blfunc=blfunc,
                                                  datacolumn=datacolumn,
                                                  outfile=outfile,
                                                  bltable=bloutput, # remove this line once text/csv output becomes available (2015/7/1 WK)
                                                  blformat=blformat,
                                                  bloutput=bloutput,
                                                  dosubtract=dosubtract,
                                                  spw=spw,
                                                  pol=pol,
                                                  order=order,
                                                  npiece=npiece,
                                                  blparam=blparam,
                                                  clip_threshold_sigma=clipthresh,
                                                  num_fitting_max=clipniter+1,
                                                  linefinding=(maskmode=='auto'),
                                                  threshold=thresh,
                                                  avg_limit=avg_limit,
                                                  minwidth=minwidth,
                                                  edge=edge)
            if overwrite:
                if os.path.exists(outfile):
                    os.system('rm -rf %s' % outfile)
                for bloutfile in new_bloutput:
                    if os.path.exists(bloutfile):
                        os.system('rm -rf %s' % bloutfile)

            #print params
            if(blformat != ',,'):
                func(**params)
            
            if (blfunc == 'variable'):
                restore_sorted_table_keyword(infile, sorttab_info)

    except Exception, instance:
        raise Exception, instance



#def __init_blfile(self):
#    if self.bloutput:
#        self.blfile = self.bl + "_blparam.txt"
#
#        if (self.blformat.lower() != "csv"):
#            f = open(self.blfile, "w")
#            
#            blf = self.blfunc.lower()
#            mm = self.maskmode.lower()
#            if blf == 'poly':
#                ftitles = ['Fit order']
#            elif blf == 'chebyshev':
#                ftitles = ['Fit order']
#            elif blf == 'cspline':
#                ftitles = ['nPiece']
#            else: # sinusoid
#                ftitles = ['applyFFT', 'fftMethod', 'fftThresh', 'addWaveN', 'rejWaveN']
#            if mm == 'auto':
#                mtitles = ['Threshold', 'avg_limit', 'Edge']
#            elif mm == 'list':
#                mtitles = []
#            else: # interact
#                mtitles = []
#            ctitles = ['clipThresh', 'clipNIter']
#
#            fkeys = getattr(self, '%s_keys'%(self.blfunc))
#            mkeys = getattr(self, '%s_keys'%(self.maskmode))
#
#            info = [['Source Table', self.infile],
#                    ['Output File', self.outfile],
#                    ['Function', self.blfunc]]
#            for i in xrange(len(ftitles)):
#                info.append([ftitles[i],getattr(self,fkeys[i])])
#            if blf != 'poly':
#                for i in xrange(len(ctitles)):
#                    info.append([ctitles[i],self.clip_keys[i]])
#            info.append(['Mask mode', self.maskmode])
#            for i in xrange(len(mtitles)):
#                info.append([mtitles[i],getattr(self,mkeys[i])])
#
#            separator = "#"*60 + "\n"
#
#            f.write(separator)
#            for i in xrange(len(info)):
#                f.write('%12s: %s\n'%tuple(info[i]))
#            f.write(separator)
#            f.close()
#        else:
#            self.blfile = ""










def prepare_for_baselining(**keywords):
    params = {}
    funcname = 'subtract_baseline'

    blfunc = keywords['blfunc']
    keys = ['datacolumn', 'outfile', 'bltable', 'blformat','bloutput', 'dosubtract', 'spw', 'pol']
    if blfunc in ['poly', 'chebyshev']:
        keys += ['blfunc', 'order', 'clip_threshold_sigma', 'num_fitting_max']
    elif blfunc == 'cspline':
        keys += ['npiece', 'clip_threshold_sigma', 'num_fitting_max']
        funcname += ('_' + blfunc)
    elif blfunc == 'variable':
        keys += ['blparam']
        funcname += ('_' + blfunc)
    else:
        raise ValueError, "Unsupported blfunc = %s" % blfunc
    if blfunc!= 'variable':
        keys += ['linefinding', 'threshold', 'avg_limit', 'minwidth', 'edge']
    for key in keys: params[key] = keywords[key]

    baseline_func = getattr(sdms, funcname)

    return params, baseline_func
    
    
def remove_sorted_table_keyword(infile):
    res = {'is_sorttab': False, 'sorttab_keywd': '', 'sorttab_name': ''}
    with sdutil.tbmanager(infile, nomodify=False) as tb:
        try:
            sorttab_keywd = 'SORTED_TABLE'
            if sorttab_keywd in tb.keywordnames():
                res['is_sorttab'] = True
                res['sorttab_keywd'] = sorttab_keywd
                res['sorttab_name'] = tb.getkeyword(sorttab_keywd)
                tb.removekeyword(sorttab_keywd)
        except Exception, e:
            raise Exception, e

    return res

def restore_sorted_table_keyword(infile, sorttab_info):
    if sorttab_info['is_sorttab'] and (sorttab_info['sorttab_name'] != ''):
        with sdutil.tbmanager(infile, nomodify=False) as tb:
            try:
                tb.putkeyword(sorttab_info['sorttab_keywd'],
                              sorttab_info['sorttab_name'])
            except Exception, e:
                raise Exception, e
