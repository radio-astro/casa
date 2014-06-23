# scaleweights(), getavweight(), abschanwidth(), adjustweights()
#
# Recipes to manipulate the WEIGHTS column of an MS
#
# To access these functions, type (at the CASA prompt):
#
# from recipes.weights import *
#
# For more information about each function type
#
# help scaleweights
# help getavweight
# help abschanwidth
# help getnch
# help adjustweights
# help adjustweights2
#
# DP, Dec 2013
# GM, 2013Dec19  added chbw/nchan version (adjustweights2)

import taskinit
import numpy

def scaleweights(vis="", field=[], spw="", scale=1., dotime=False):

    """
    Usage: scaleweights(vis, field, spw, scale)
           Scale the weight column for spw and field by scale.
           The field parameter takes a list of fields.
    """

    if(vis=="" or spw=="" or field==[] or scale==1. or not type(field)==list):
        print "Usage: scaleweights(vis, field, spw, scale)"
        print "       The field parameter takes a list of fields."
        return False

    myvis = vis
    myspw = spw
    myscale = scale
    myfields = field
    mytb=taskinit.tbtool()
    
    mytb.open(myvis)
    w = mytb.getcol("WEIGHT")
    dd = mytb.getcol("DATA_DESC_ID")
    ff = mytb.getcol("FIELD_ID")
    tt =[]
    if (dotime):
        tt=mytb.getcol('EXPOSURE')
    mytb.close()

    mytb.open(myvis+"/DATA_DESCRIPTION")
    mydds = []
    
    for i in range(0,mytb.nrows()):
        if(mytb.getcell("SPECTRAL_WINDOW_ID", i)!=myspw):
            continue
        else:
            mydds.append(i)

    mytb.close()

    print "Will change weights for data description ids ", mydds

    changes=0

    if len(mydds)>0:
        for row in range(0,len(dd)):
            if (dd[row] in mydds) and (ff[row] in myfields):
                changes += 1
                for i in range(0, len(w)):
                    w[i][row] *= myscale
                    if dotime:
                        w[i][row] *= (2*tt[row])  # include 2 factor, too
            
        mytb.open(myvis, nomodify=False)
        mytb.putcol("WEIGHT", w)
        mytb.close()

    if changes>0:
        print "Changes applied in ", changes, " rows."
    else:
        print "No changes applied."

    return True


def getavweight(vis="", field=[], spw=""):

    """
    Usage: getavweight(vis, field, spw)
           Get the average weight for the given field and spw.
           The field parameter takes a list of fields.
    """

    if(vis=="" or spw=="" or field==[] or not type(field)==list):
        print "Usage: getavweight(vis, field, spw)"
        print "       The field parameter takes a list of fields."
        return False

    myvis = vis
    myspw = spw
    myfields = field
    mytb=taskinit.tbtool()
    
    mytb.open(myvis)
    w = mytb.getcol("WEIGHT")
    dd = mytb.getcol("DATA_DESC_ID")
    ff = mytb.getcol("FIELD_ID")
    mytb.close()

    mytb.open(myvis+"/DATA_DESCRIPTION")
    mydds = []
    
    for i in range(0,mytb.nrows()):
        if(mytb.getcell("SPECTRAL_WINDOW_ID", i)!=myspw):
            continue
        else:
            mydds.append(i)

    mytb.close()

    mynrows = 0
    mysumw = 0

    npol = len(w)

    if len(mydds)>0:
        for row in range(0,len(dd)):
            if (dd[row] in mydds) and (ff[row] in myfields):
                mynrows += 1
                for i in range(0, npol):
                    mysumw += w[i][row]

    rval = 0.

    if mynrows>0:
        rval = mysumw/float(npol)/float(mynrows)
        print "Average weight is ", rval
    else:
        print "No rows selected."

    return rval

    
def abschanwidth(vis="", spw=""):

    """
    Usage: abschanwidth(vis, spw)
           Get the absolute channel width for the given spw.
           Returns 0 upon error.
    """

    if(vis=="" or spw==""):
        print "Usage: abschanwidth(vis, spw)"
        return 0

    myvis = vis
    myspw = spw
    mytb=taskinit.tbtool()
    
    mytb.open(myvis+"/SPECTRAL_WINDOW")
    if(spw>=mytb.nrows() or spw<0):
        print "Error: spw out of range. Min is 0. Max is ", mytb.nrows()-1
        return 0
        
    mychw = mytb.getcell("CHAN_WIDTH",spw)[0]
    mytb.close()

    return numpy.fabs(mychw)

def getnch(vis="", spw=""):

    """
    Usage: getnch(vis, spw)
           Get the nchan for the given spw.
           Returns 0 upon error.
    """

    if(vis=="" or spw==""):
        print "Usage: abschanwidth(vis, spw)"
        return 0

    myvis = vis
    myspw = spw
    mytb=taskinit.tbtool()
    
    mytb.open(myvis+"/SPECTRAL_WINDOW")
    if(spw>=mytb.nrows() or spw<0):
        print "Error: spw out of range. Min is 0. Max is ", mytb.nrows()-1
        return 0
        
    mynch = mytb.getcell("NUM_CHAN",spw)
    mytb.close()

    return numpy.abs(mynch)


def adjustweights(vis="", field="", refspws=[], spws=[]):

    """
       Usage: adjustweights(vis, field, refspws, spws)
              Scale the weights given by spws by a factor
              derived from the average weight Wref of the refspws
              and the original average weight Worig of the spws
              using

                   Wnew = Wold * (Wref/Worig)*(ChanWidthorig/ChanWidthref)

              where ChanWidthorig is the channel width of the spws,
              and  ChanWidthref is the channel width of the refspws.

              refspws and spws are of type list,
              field should be given as field id.
    """

    myvis = vis
    myfield = int(field)
    mytb=taskinit.tbtool()
    
    if (vis=="" or myfield=="" or refspws==[] or spws==[] or not type(refspws)==list or not type(spws)==list):
        print "Usage: adjustweights(vis, field, refspws, spws)"
        print "       refspws and spws are of type list,"
        print "       field should be given as field id"
        return False

    # check that all ref spws have the same chan width
    refcw = 0.
    for spw in refspws:
        cw = abschanwidth(myvis, spw)
        if cw==0:
            print "Error reading channel width of spw ", spw
            return False
        if(refcw==0):
            refcw=cw
        else:
            if not refcw==cw:
                print "Error: the spws given in the reference list do not all have the same channel width."
                return False

    # get avweight and chanwidth from spws
    cws = []
    avweights = []
    for spw in spws:
        cw = abschanwidth(myvis, spw)
        if cw==0:
            print "Error reading channel width of spw ", spw
            return False
        cws.append(cw)
        avw = getavweight(myvis, [myfield], spw)
        if(avw==0.):
            print "Error: average weight of spw ", spw, " is zero (could also mean no data)."
            return False    
        print "Spw ", spw, ", channelwidth ", cw, ", av. weight ", avw
        avweights.append(avw)

    # get avweight and chanwidth from ref spws    
    ravweight = 0.
    for spw in refspws:
        avw = getavweight(myvis, [myfield], spw)
        print "Reference Spw ", spw, ", channelwidth ", refcw, ", av. weight ", avw
        ravweight += avw

    if(len(refspws)>0):
        ravweight /= len(refspws)
    else:
        print "Error: no reference spws"
        return False

    print "Average weight of reference spws: ", ravweight

    # calculate scale factor and apply scaling to the spws

    for i in range(0,len(spws)):
        myscale = ravweight/avweights[i]*cws[i]/refcw
        print "Scale factor for weights in spw ", spws[i], " is ", myscale
        scaleweights(myvis, [myfield], spws[i], myscale)

    print "Done."

    return True


def adjustweights2(vis="", field="", spws=[]):

    """
       Usage: adjustweights2(vis, field, spws)
              Scale the weights in specified spws by a factor
              2*df*dt/nchan, where df is the channel bandwidth,
              dt is the integration time, and nchan is the number
              of channels in the spw.  This enables imaging of
              mixed mode spws in CASA v4.2 and earlier.  (Note
              that there will be no net effect if the spws
              share the same df, dt, and nchan.

              spws are of type list,
              field should be given as field id.
    """

    myvis = vis
    myfield = int(field)
    mytb=taskinit.tbtool()
    
    if (vis=="" or myfield=="" or spws==[] or not type(spws)==list):
        print "Usage: adjustweights2(vis, field, spws)"
        print "       spws are of type list,"
        print "       field should be given as field id"
        return False

    # get avweight and chanwidth from spws
    chw = []
    nch = []
    for spw in spws:
        cw = abschanwidth(myvis, spw)
        if cw==0:
            print "Error reading channel width of spw ", spw
            return False
        chw.append(cw)


        nc = getnch(myvis, spw)
        if(nc==0):
            print "Error: nch of spw ", spw, " is zero (could also mean no data)."
            return False    
        nch.append(nc)

        print "Spw ", spw, ", channelwidth", cw, ", nchan ", nc

    # calculate scale factor and apply scaling to the spws

    for i in range(0,len(spws)):
        myscale = chw[i]/nch[i]
        print "Scale factor for weights in spw ", spws[i], " is ", myscale
        scaleweights(myvis, [myfield], spws[i], myscale, True) # include integ time

    print "Done."

    return True
