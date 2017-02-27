import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.api as api
import pipeline.infrastructure as infrastructure
LOG = infrastructure.get_logger(__name__)


class GroupByPosition2(api.Heuristic):
    """
    Grouping by RA/DEC position
    """

    @staticmethod
    def translate_pos_dict(pos_dict, rows, ids):
        translated = {}
        for (pos,val) in pos_dict.items():
            key = rows[pos]
            if val[0] == -1:
                translated[key] = [[-1,rows[val[1]]],[ids[val[1]]]]
            else:
                translated[key] = [[rows[v] for v in val],
                                   [ids[v] for v in val]]
        return translated

    @staticmethod
    def translate_pos_gap(pos_gap, rows, ids):
        return [ids[p] for p in pos_gap]


    
    def calculate(self, ra, dec, r_combine, r_allowance):
        """
        Grouping by RA/DEC position
            ra: list of R.A.
            dec: list of DEC.
            r_combine: inside r_combine will be grouped together
            r_allowance: inside r_allowance are assumed to be the same position
        Return(PosDict, PosGap):
            PosDict: [ID1, ID2,..., IDN] if not referenced before
                     [-1, IDK] if has a reference to rowK
            PosGap: [IDX1, IDX2,...,IDXN]
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
        CombineDiameter = 2.0 * CombineRadius

        # 2009/2/10 Quicker Method
        Nrows = len(ra)
        PosDict = {}
        SelectDict = {}
        MinRA = ra.min()
        MaxRA = ra.max()
        MinDEC = dec.min()
        MaxDEC = dec.max()
        # Calculate the lattice position (sRA, sDEC) for each pointings
        # Create list of pointings (dictionary) for each lattice position
        for x in xrange(Nrows):
           sRA = int((ra[x] - MinRA) / CombineDiameter)
           sDEC = int((dec[x] - MinDEC) / CombineDiameter)
           if not SelectDict.has_key(sRA): SelectDict[sRA] = {}
           if not SelectDict[sRA].has_key(sDEC): SelectDict[sRA][sDEC] = [x]
           else:
               SelectDict[sRA][sDEC].append(x)

        # Create PosDict
        # make a list of spectra inside each lattice grid
        # store the list in PosDict[i] where 'i' is the smallest row number in the list
        # Other spectra have a reference to 'i'
        LOG.debug('SelectDict.keys() : %s' % SelectDict.keys())
        for sRA in SelectDict.keys():
            LOG.debug('len(SelectDict[%s].keys()) : %s' % (sRA, len(SelectDict[sRA].keys())))
            for sDEC in SelectDict[sRA].keys():
                PosDict[SelectDict[sRA][sDEC][0]] = SelectDict[sRA][sDEC]
                if len(SelectDict[sRA][sDEC]) != 1:
                    for x in SelectDict[sRA][sDEC][1:]:
                        PosDict[x] = [-1,SelectDict[sRA][sDEC][0]]
        del SelectDict

        # Calculate thresholds to determine gaps
        DeltaP = numpy.sqrt((ra[1:] - ra[:-1])**2.0 + (dec[1:] - dec[:-1])**2.0)
        DeltaQ = numpy.take(DeltaP, numpy.nonzero(DeltaP > ThresholdAR)[0])
        if len(DeltaQ) != 0:
            ThresholdP = numpy.median(DeltaQ) * 10.0
        else:
            ThresholdP = 0.0
        LOG.info('threshold:%s deg' % ThresholdP)

        # List rows which distance from the previous position is larger than the threshold
        PosGap = []
        for x in range(1, Nrows):
            if DeltaP[x - 1] > ThresholdP:
                PosGap.append(x)
                LOG.info('Position Gap %s deg at row=%d' % (DeltaP[x-1], x))


        # Print information
        if len(PosGap) == 0:
            PosGapMsg = 'Found no position gap'
        else:
            PosGapMsg = 'Found %d position gap(s)' % len(PosGap)
        LOG.info(PosGapMsg)

        #print '\nPosGap', PosGap
        #print '\nPosDict', PosDict
        return (PosDict, PosGap)


class GroupByTime2(api.Heuristic):
    """
    Grouping by time sequence
    """

    @staticmethod
    def translate_time_table(time_table, rows, ids):
        translated = [[],[]]
        for i in (0,1):
            for group in time_table[i]:
                translated[i].append([[rows[v] for v in group],
                                      [ids[v] for v in group]])
        return translated

    @staticmethod
    def translate_time_gap(time_gap, rows, ids):
        return [[ids[t] for t in time_gap[0]],
                [ids[t] for t in time_gap[1]]]


    def calculate(self, timebase, time_diff):
        """
        Grouping by time sequence
            timebase: base list of time stamps for threshold estimation
            time_diff: difference from the previous time stamp
                time_diff is generated from timebase in most of the cases
                timebase contains all time stamps and time_diff is created
                from selected time stamps in other case
        Return (TimeTable, TimeGap)
            TimeTable: [[[index0,..,indexN],[indexXX..],..,[..]], [[],[],[]]]
            TimeTable[0]: separated by small gaps (for process5 and 7)
            TimeTable[1]: separated by large gaps (for process5 and 7)
            TimeGap: [[rowX1, rowX2,...,rowXN], [rowY1, rowY2,...,rowYN]]
            TimeGap[0]: small gap (for plot)
            TimeGap[1]: large gap (for plot)
        """
        LOG.info('Grouping by Time...')
        
        # Threshold for grouping
        h = ThresholdForGroupByTime()
        (Threshold1,Threshold2) = h(timebase)

        TimeTable = [[],[]]
        SubTable1 = [0]
        SubTable2 = [0]
        TimeGap = [[],[]]

        # Detect small and large time gaps
        for index in xrange(len(time_diff)):
            indexp1 = index + 1
            if time_diff[index] <= Threshold1:
                SubTable1.append(indexp1)
            else:
                TimeTable[0].append(SubTable1)
                SubTable1 = [indexp1]
                TimeGap[0].append(indexp1)
                LOG.info('Small Time Gap: %s sec at row=%d' % (time_diff[index], indexp1))
            if time_diff[index] <= Threshold2:
                SubTable2.append(indexp1)
            else:
                TimeTable[1].append(SubTable2)
                SubTable2 = [indexp1]
                TimeGap[1].append(indexp1)
                LOG.info('Large Time Gap: %s sec at row=%d' % (time_diff[index], indexp1))

        if len(SubTable1) > 0: TimeTable[0].append(SubTable1)
        if len(SubTable2) > 0: TimeTable[1].append(SubTable2)
        del SubTable1, SubTable2

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
        

class ThresholdForGroupByTime(api.Heuristic):
    def calculate(self, timebase):
        """
        Estimate thresholds for large and small time gaps using
        base list of time stamps.
        """
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

        return (Threshold1,Threshold2)
    
        
class MergeGapTables2(api.Heuristic):
    def calculate(self, TimeGap, TimeTable, PosGap, tBEAM):
        """
        Merge TimeGap Table and PosGap Table. PosGap is merged into TimeGap Table[0]: Small gap
            TimeTable, TimeGap: output from GroupByTime2()
            PosGap: output from GroupByPosition2()
            DataTable: DataTable
        Return (TimeTable, TimeGap)
            TimeTable[0]: separated by small gaps (for process5 and 7)
            TimeTable[1]: separated by large gaps (for process5 and 7)
            format: [[[row1,...,rowN],[index1,...,indexN]],[[row2,...,rowM],[index2,...,indexM]]]
        """

        LOG.info('Merging Position and Time Gap tables...')

        idxs = []
        for i in xrange(len(TimeTable[0])):
            idxs += TimeTable[0][i]
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
                TimeTable[0].append(SubTable1)
                SubTable1 = [n]
                LOG.info('Small Time Gap at row=%d' % n)
            else:
                SubTable1.append(n)
        if len(SubTable1) > 0: TimeTable[0].append(SubTable1)

        # 2009/2/6 Divide TimeTable in accordance with the Beam
        TimeTable2 = TimeTable[:]
        TimeTable = [[],[]]
        for i in range(len(TimeTable2)):
            for index in range(len(TimeTable2[i])):
                #rows = TimeTable2[i][index][0]
                idxs = TimeTable2[i][index]
                BeamDict = {}
                for index2 in range(len(idxs)):
                    #row = rows[index2]
                    idx = idxs[index2]
                    if BeamDict.has_key(tBEAM[idx]):
                        #BeamDict[tBEAM[row]][0].append(row)
                        BeamDict[tBEAM[idx]].append(idx)
                    else:
                        BeamDict[tBEAM[idx]] = [idx]
                BeamList = BeamDict.values()
                for beam in BeamList:
                    TimeTable[i].append(beam)

        #print TimeTable[0]
        del BeamDict, BeamList, TimeTable2

        LOG.debug('TimeTable = %s' % (TimeTable))

        #print '\nTimeGap', TimeGap
        #print '\nTimeTable', TimeTable
        return(TimeTable, TimeGap)
