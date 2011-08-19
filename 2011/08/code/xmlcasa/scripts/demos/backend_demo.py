#If ~/.matplotlib/matplotlibrc contains 'backend : TkAgg'
#plotxy (tableplot) can do either interactive of non-interactive
#If ~/.matplotlib/matplotlibrc contains 'backend : Agg'
#plotxy (tableplot) can only do non-interactive
import sys
#sys.path.insert(2,'/home/casa-dev-09/hye/gnuactive/linux_64b/python/2.5')
import casac
import matplotlib
__tableplothome__ = casac.homefinder.find_home_by_name('tableplotHome')
mytp=__tableplothome__.create( )
#mytp.setgui(False)
mytp.setgui(True)
mytp.open(tabnames='ngc5921.ms')
pop={'timeplot':'x', 'overplot':True, 'plotsymbol':'o','multicolour':True,'nrows':1}
xystr=['(TIME/86400.0)+678576.0','UVW[1]'];
mytp.plotdata(poption=pop,datastr=xystr)
mytp.savefig(filename='lastSave.png')

