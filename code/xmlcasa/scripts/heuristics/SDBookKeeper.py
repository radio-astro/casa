import os
import pickle

class SDbookkeeper:
    '''
    BK = SDB.SDbookkeeper()
    BK.show()
    BK.write('ProcessName1', a=5, b=3, c=2, d=7)
    BK.write('ProcessName2', [0,1], [range(100),range(200)], a=3, b=1)
    BK.write('ProcessName3', [1,2], [range(20),range(10)], b=2, c=5)
    BK.read(3, [1], 'a','b','c','d') => (range(20), 3, 2, 5, 7)
    BK.read(2, [1], 'a','b','c','d') => (range(200), 3, 1, 2, 7)
    BK.clean()

    History: [ProcessName, [Data], [keys]]
    '''
    def __init__(self, HistoryDir):
        self.HistoryDir = HistoryDir
        self.ID = 0
        self.Counter = 0
        self.HistoryFile = HistoryDir + '/History'
        self.BackupData = HistoryDir + '/Data'
        self.BackupParam = HistoryDir + '/Param'
        if not os.access(self.HistoryDir, os.F_OK):
            os.mkdir(self.HistoryDir)
            self.History = []
            print 'Created History Directory'
        elif os.access(self.HistoryFile, os.F_OK):
            h_file = open(self.HistoryFile, 'r')
            self.History = pickle.load(h_file)
            h_file.close()
            for ProcessName in self.History:
                self.ID += 1
                print self.ID, ProcessName
        else:
            self.History = []
            print 'No History File'

    def clean(self):
        if os.access(self.HistoryDir, os.F_OK):
            for root, dirs, files in os.walk(self.HistoryDir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
        else: os.mkdir(self.HistoryDir)
        self.History = []
        self.ID = 0

    def countup(self):
        self.Counter += 1

    def write(self, ProcessName, DataID=[], Data=[], **args):
        if len(self.History) > self.ID: del self.History[self.ID:]
        for i in range(len(DataID)):
            d_file = open(self.BackupData + '%d_%d' % (self.ID, DataID[i]), 'w')
            pickle.dump(Data[i], d_file)
            d_file.close()
        self.History.append([ProcessName, DataID, args.keys()])
        h_file = open(self.HistoryFile, 'w')
        pickle.dump(self.History, h_file)
        h_file.close()
        p_file = open(self.BackupParam + '%d' % self.ID,  'w')
        pickle.dump(args, p_file)
        p_file.close()
        self.ID += 1
        print self.History[-1]

    def show(self):
        ID = 0
        for ProcessName, DataID, Keys in self.History:
            ID += 1
            print ID, ProcessName, DataID, Keys

    def read(self, ID, DataID, *args):
        self.ID = ID
        ID = ID - 1
        Item = len(args)
        DataItem = [DataID[:], []]
        DataRet = {}
        KeyItem = [list(args), []]
        KeyRet = {}
        print 'Retrieving...', DataID, args
        #print DataItem, KeyItem
        while ID >= 0 and (len(DataItem[0]) > 0 or len(KeyItem[0]) > 0):
            print 'Checking repository...', self.History[ID]
            for i in range(len(DataItem[0])-1, -1, -1):
                #print ID, '*****', DataItem[0][i], self.History[ID][1]
                if DataItem[0][i] in self.History[ID][1]:
                    DataItem[1].append(DataItem[0][i])
                    d_file = open(self.BackupData + '%d_%d' % (ID, DataItem[0][i]), 'r')
                    Data = pickle.load(d_file)
                    d_file.close()
                    DataRet[DataItem[0][i]] = Data
                    #print '-----', DataRet.keys(), DataItem[0][i], Data
                    #print 'xxxxx', DataRet
                    del DataItem[0][i]

            Loaded = False
            for i in range(len(KeyItem[0])-1, -1, -1):
                #print KeyItem
                if KeyItem[0][i] in self.History[ID][2]:
                    KeyItem[1].append(KeyItem[0][i])
                    if Loaded == False:
                        p_file = open(self.BackupParam + '%d' % ID, 'r')
                        Param = pickle.load(p_file)
                        p_file.close()
                        Loaded = True
                    KeyRet[KeyItem[0][i]] = Param[KeyItem[0][i]]
                    del KeyItem[0][i]
            ID = ID - 1

        Ret = []
        #print 'yyyyy', DataID, args
        #print 'zzzzz', DataRet, KeyRet
        for j in DataID:
            Ret.append(DataRet[j])
        for j in args:
            Ret.append(KeyRet[j])
        print 'Data recovered successfully. ', DataID, args
        return Ret

    
