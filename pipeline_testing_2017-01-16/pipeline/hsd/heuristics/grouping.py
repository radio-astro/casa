import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.api as api
import pipeline.infrastructure as infrastructure
LOG = infrastructure.get_logger(__name__)


class GroupByPosition(api.Heuristic):
    """
    Grouping by RA/DEC position
    """
    def calculate(self, ids, rows, ra, dec, r_combine, r_allowance):
        """
        Grouping by RA/DEC position
            ids: list of ids
            rows: list of rows
            ra: list of R.A.
            dec: list of DEC.
            r_combine: inside r_combine will be grouped together
            r_allowance: inside r_allowance are assumed to be the same position
        Return(PosDict, PosGap):
            PosDict: [[row1, row2,..., rowN],[ID1, ID2,..., IDN]] if not referenced before
                     [[-1, rowK],[IDK]] if has a reference to rowK
            PosGap: [rowX1, rowX2,...,rowXN]
        """
        qa = casatools.quanta
        if isinstance(r_combine, dict):
            # r_combine should be quantity
            CombineRadius = qa.convert(r_combine,'deg')['value']
        else:
            CombineRadius = r_combine
        if isinstance(r_allowance, dict):
            # r_allowance should be quantity
            AllowanceRadius = qa.convert(r_allowance,'deg')['value']
        else:
            AllowanceRadius = r_allowance

        ThresholdR = CombineRadius * CombineRadius
        ThresholdAR = AllowanceRadius * AllowanceRadius

        # 2009/2/10 Quicker Method
        Nrows = len(rows)
        PosDict = {}
        SelectDict = {}
        MinRA = ra.min()
        MaxRA = ra.max()
        MinDEC = dec.min()
        MaxDEC = dec.max()
        # Calculate the lattice position (sRA, sDEC) for each pointings
        # Create list of pointings (dictionary) for each lattice position
        for x in xrange(Nrows):
           sRA = int((ra[x] - MinRA) / CombineRadius / 2.0)
           sDEC = int((dec[x] - MinDEC) / CombineRadius / 2.0)
           if not SelectDict.has_key(sRA): SelectDict[sRA] = {}
           if not SelectDict[sRA].has_key(sDEC): SelectDict[sRA][sDEC] = [[rows[x]],[ids[x]]]
           else:
               SelectDict[sRA][sDEC][0].append(rows[x])
               SelectDict[sRA][sDEC][1].append(ids[x])
               #SelectDict[sRA][sDEC][1].append(x)

        # Create PosDict
        # make a list of spectra inside each lattice grid
        # store the list in PosDict[i] where 'i' is the smallest row number in the list
        # Other spectra have a reference to 'i'
        LOG.debug('SelectDict.keys() : %s' % SelectDict.keys())
        for sRA in SelectDict.keys():
            LOG.debug('len(SelectDict[%s].keys()) : %s' % (sRA, len(SelectDict[sRA].keys())))
            for sDEC in SelectDict[sRA].keys():
                PosDict[SelectDict[sRA][sDEC][0][0]] = SelectDict[sRA][sDEC]
                if len(SelectDict[sRA][sDEC][0]) != 1:
                    for x in SelectDict[sRA][sDEC][0][1:]:
                        PosDict[x] = [[-1,SelectDict[sRA][sDEC][0][0]],[SelectDict[sRA][sDEC][1][0]]]
        del SelectDict

        # Calculate thresholds to determine gaps
        DeltaP = numpy.sqrt((ra[1:] - ra[:-1])**2.0 + (dec[1:] - dec[:-1])**2.0)
        DeltaQ = numpy.take(DeltaP, numpy.nonzero(DeltaP > ThresholdAR)[0])
        if len(DeltaQ) != 0:
            ThresholdP = numpy.median(DeltaQ) * 10.0
        else:
            ThresholdP = 0.0
        LOG.info('threshold:%s deg' % ThresholdP)
        #del row, dec, ra

        # List rows which distance from the previous position is larger than the threshold
        PosGap = []
        for x in range(1, Nrows):
            if DeltaP[x - 1] > ThresholdP:
                PosGap.append(ids[x])
                #PosGap.append(self.DataTable[id[x]][DT_ROW])
                #LOG.info('Position Gap %s deg at row=%d' % (DeltaP[x-1], self.DataTable[id[x]][DT_ROW]))
                LOG.info('Position Gap %s deg at row=%d' % (DeltaP[x-1], rows[x]))


        # Print information
        if len(PosGap) == 0:
            PosGapMsg = 'Found no position gap'
        else:
            PosGapMsg = 'Found %d position gap(s)' % len(PosGap)
        LOG.info(PosGapMsg)

        #print '\nPosGap', PosGap
        #print '\nPosDict', PosDict
        return (PosDict, PosGap)
        # PosGap is index
        # PosDict[row][0]: row, PosDict[row][1]: index


class GroupByTime(api.Heuristic):
    """
    Grouping by time sequence
    """
    def calculate(self, timebase, timelist, rows, ids):
        """
        Grouping by time sequence
            basedata: thresholds are determined by basadata (list of rows)
            rows: thresholds are applied to rows
                basedata and rows are the same list in most of the cases
                basedata contains all spectra and rows are selections in other case
        Return (TimeTable, TimeGap)
            TimeTable: [[[[row0,..,rowN],[index0,..,indexN]],[[rowXX,..,rowXXX],[indexXX..]],..,[[,,]]], [[],[],[]]]]
            TimeTable[0]: separated by small gaps (for process5 and 7)
            TimeTable[1]: separated by large gaps (for process5 and 7)
            TimeGap: [[rowX1, rowX2,...,rowXN], [rowY1, rowY2,...,rowYN]]
            TimeGap[0]: small gap (for plot)
            TimeGap[1]: large gap (for plot)
        """
        LOG.info('Grouping by Time...')

        # Read time stamp for all basedata (list of rows) to determine thresholds
        ArrayTime = numpy.array(timebase)

        # 2009/2/5 adapted for multi beam which assumes to have identical time stamps
        # identical time stamps are rejected before determining thresholds
        # DeltaT: difference from the previous time stamp
        DeltaT = ArrayTime[1:] - ArrayTime[:-1]
        DeltaT1 = numpy.take(DeltaT, numpy.nonzero(DeltaT)[0])
        Threshold1 = numpy.median(DeltaT1) * 5.0
        DeltaT2 = numpy.take(DeltaT1, numpy.nonzero(DeltaT1 > Threshold1)[0])
        if len(DeltaT2) > 0:
            Threshold2 = numpy.median(DeltaT2) * 5.0
        else:
            Threshold2 = Threshold1

        # print information
        LOG.info('Threshold1 = %s sec' % Threshold1)
        LOG.info('Threshold2 = %s sec' % Threshold2)
        LOG.info('MaxDeltaT = %s sec' % DeltaT1.max())
        LOG.info('MinDeltaT = %s sec' % DeltaT1.min())
        del DeltaT2, DeltaT1, DeltaT, ArrayTime

        # Read time stamp for rows
        #ArrayTime = numpy.take(times, ids)
        ArrayTime = numpy.array(timelist)

        # DeltaT: difference from the previous time stamp
        DeltaT = ArrayTime[1:] - ArrayTime[:-1]
        del ArrayTime

        TimeTable = [[],[]]
        SubTable1 = [rows[0]]
        SubTable2 = [rows[0]]
        IdxTable1 = [ids[0]]
        IdxTable2 = [ids[0]]
        TimeGap = [[],[]]

        # Detect small and large time gaps
        for index in xrange(len(ids)-1):
            indexp1 = index + 1
            if DeltaT[index] <= Threshold1:
                SubTable1.append(rows[indexp1])
                IdxTable1.append(ids[indexp1])
            else:
                TimeTable[0].append([SubTable1, IdxTable1])
                SubTable1 = [rows[indexp1]]
                IdxTable1 = [ids[indexp1]]
                TimeGap[0].append(ids[indexp1])
                LOG.info('Small Time Gap: %s sec at row=%d' % (DeltaT[index], rows[indexp1]))
            if DeltaT[index] <= Threshold2:
                SubTable2.append(rows[indexp1])
                IdxTable2.append(ids[indexp1])
            else:
                TimeTable[1].append([SubTable2, IdxTable2])
                SubTable2 = [rows[indexp1]]
                IdxTable2 = [ids[indexp1]]
                TimeGap[1].append(ids[indexp1])
                LOG.info('Large Time Gap: %s sec at row=%d' % (DeltaT[index], rows[indexp1]))

        if len(SubTable1) > 0: TimeTable[0].append([SubTable1, IdxTable1])
        if len(SubTable2) > 0: TimeTable[1].append([SubTable2, IdxTable2])
        del SubTable1, SubTable2, DeltaT, IdxTable1, IdxTable2

        # print information
        if len(TimeGap[0])==0: 
            TimeGapMsg = 'Found no time gap'
            LOG.info(TimeGapMsg)
        else:
            TimeGapMsg1 = 'Found %d small time gap(s)' % len(TimeGap[0])
            TimeGapMsg2 = 'Found %d large time gap(s)' % len(TimeGap[1])
            LOG.info(TimeGapMsg1)
            LOG.info(TimeGapMsg2)

        #print '\nTimeGap', TimeGap
        #print '\nTimeTable', TimeTable
        return (TimeTable, TimeGap)
        # TimeGap is index
        # TimeTable[][0] is row, TimeTable[][1] is index

        
class MergeGapTables(api.Heuristic):
    def calculate(self, TimeGap, TimeTable, PosGap, DataTable):
        """
        Merge TimeGap Table and PosGap Table. PosGap is merged into TimeGap Table[0]: Small gap
            TimeTable, TimeGap: output from GroupByTime()
            PosGap: output from GroupByPosition()
            DataTable: DataTable
        Return (TimeTable, TimeGap)
            TimeTable[0]: separated by small gaps (for process5 and 7)
            TimeTable[1]: separated by large gaps (for process5 and 7)
            format: [[[row1,...,rowN],[index1,...,indexN]],[[row2,...,rowM],[index2,...,indexM]]]
        """

        LOG.info('Merging Position and Time Gap tables...')

        tROW = DataTable.getcol('ROW')
        tBEAM = DataTable.getcol('BEAM')

        idxs = []
        for i in xrange(len(TimeTable[0])):
            idxs += TimeTable[0][i][1]
        IDX = list(numpy.sort(numpy.array(idxs)))
        tmpGap = list(numpy.sort(numpy.array(TimeGap[0] + PosGap)))
        NewGap = []
        if len( tmpGap ) != 0: 
            t = n = tmpGap[0]
            for n in tmpGap[1:]:
                if t != n and t in IDX:
                    NewGap.append(t)
                    t = n
            if n in IDX:
                NewGap.append(n)
        TimeGap[0] = NewGap

        SubTable1 = []
        SubTable2 = []
        TimeTable[0] = []
        for index in range(len(IDX)):
            n = IDX[index]
            if n in TimeGap[0]:
                TimeTable[0].append([SubTable1, SubTable2])
                SubTable1 = [tROW[n]]
                LOG.info('Small Time Gap at row=%d' % tROW[n])
                SubTable2 = [n]
            else:
                SubTable1.append(tROW[n])
                SubTable2.append(n)
        if len(SubTable1) > 0: TimeTable[0].append([SubTable1, SubTable2])

        # 2009/2/6 Divide TimeTable in accordance with the Beam
        TimeTable2 = TimeTable[:]
        TimeTable = [[],[]]
        for i in range(len(TimeTable2)):
            for index in range(len(TimeTable2[i])):
                rows = TimeTable2[i][index][0]
                idxs = TimeTable2[i][index][1]
                BeamDict = {}
                for index2 in range(len(rows)):
                    row = rows[index2]
                    idx = idxs[index2]
                    if BeamDict.has_key(tBEAM[row]):
                        BeamDict[tBEAM[row]][0].append(row)
                        BeamDict[tBEAM[row]][1].append(idx)
                    else:
                        BeamDict[tBEAM[row]] = [[row],[idx]]
                BeamList = BeamDict.values()
                for beam in BeamList:
                    TimeTable[i].append(beam)

        #print TimeTable[0]
        del BeamDict, BeamList, TimeTable2

        LOG.debug('TimeTable = %s' % (TimeTable))

        #print '\nTimeGap', TimeGap
        #print '\nTimeTable', TimeTable
        return(TimeTable, TimeGap)
