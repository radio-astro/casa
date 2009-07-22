# SDPipelineRecipe Template (June 18 2009)

# path for the heuristics scripts
sys.path.append('/home/anybody/tmp/PIPELINE/Heuristics/src/heuristics/')

beamArcsec = 1.22 * 2.9979e8 / 45.49e9 / 106. / pi * 180. * 3600. + 1.
#beamArcsec = 20.0
radius = 0.5 * beamArcsec / 3600.0
iter=2
import SDPipelineStagesInterface

p=SDPipelineStagesInterface.SDPipelineStagesInterface()
# set default parameters to the specific telescope (not yet implemented)
#p.setHeuristics('ALMA')
p.setBeamRadius(radius)
p.setBaselineEdge([1,1])
p.setRawDirectory('/home/anybody/SDTest')
p.setOutDirectory('/home/anybody/SDTest/resultPipeline')
p.setRawFile('M16-CO43.ASAP')
p.readData()
p.createPositionOverviewPlots()
ifs=p.getIF()
pols=p.getPOL()
for vIF in ifs:
    for pol in pols:
	p.groupData(vIF,pol)
        p.remove1stOrderBaseline(vIF,pol)
        #p.removeBaseline(vIF,pol,fitOrder=1)
        p.accumulateSpectra(vIF,pol)
        for iteration in xrange(iter):
            p.spectralLineDetect(vIF,pol)
            p.spatialLineDetect(vIF,pol,createResultPlots=True)
            p.removeBaseline(vIF,pol,createResultPlots=True)
            #p.removeBaseline(vIF,pol,createResultPlots=True,fitOrder=3)
            p.flagData(vIF,pol,createResultPlots=True)
            #p.flagData(vIF,pol,createResultPlots=True,FlagRule=['rmsexpectedprefit','rmsexpectedpostfit','xxx'])
            p.regridData(vIF,pol)
            p.plotResultSpectra(vIF,pol)
            p.plotSparseSpectraMap(vIF,pol)
            p.plotChannelMaps(vIF,pol)
p.writeMSDataCube()

