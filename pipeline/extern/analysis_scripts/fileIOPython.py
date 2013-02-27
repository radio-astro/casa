"""The script allows files of data to be read into a table format.
It also allows basic sorting and shifting through the file.  
"""
import os

def fileToTable(fileName,splitVal=None,ignoreEmpty=False,keepType=False,commentChar=None) :
    f=file('%s' % fileName,'r')
    linesInFile = []
    for line in f:
        if len(line.split()) == 0 and ignoreEmpty==True : continue
        if (line[0] == commentChar and commentCar <> None) : continue
        if splitVal == None : linesInFile.append(line.split())
        else : linesInFile.append(line.split(splitVal))
    if not keepType : return linesInFile
    else :
        for i in range(len(linesInFile)) :
            for j in range(len(linesInFile[i])) :
                didNotDo = 1
                if didNotDo :
                    try: 
                        linesInFile[i][j] = int(linesInFile[i][j])
                        didNotDo = 0
                    except: didNotDo = 1
                if didNotDo :
                    try: 
                        linesInFile[i][j] = float(linesInFile[i][j])
                        didNotDo = 0
                    except: didNotDo = 1
        return linesInFile

def getLineNumbers(fileName,splitVal=None,formatString=None,beStrict=True,startStrict=True) :
    tempTable = fileToTable(fileName,splitVal,False,True)
    if formatString==None :
        formatString = getRowFormat(tempTable[0])
    formatString.lower()
    for i in formatString.split(',') :
        if i <> 'i' and i <> 'f' and i <> 's' :
            print "Invalid format string, give me a string of i, f and s separated by commas! Like 's,i,f,f,s'"
            return
    finalTable = list()
    counter = 0
    for i in tempTable :
        rowFormat = getRowFormat(i)
        if beStrict :
            if formatString == rowFormat : 
                finalTable.append(counter)
                counter+=1
            else : 
                counter+=1
                continue
        else :
            if formatString in rowFormat :
                [low,high] = whereStringIn(formatString,rowFormat)
                low = (low+1)/2
		high = (high+1)/2
                if low <> 0 and startStrict : continue
                else : 
                    finalTable.append(counter)
                    counter+=1
            else : 
                counter+=1
                continue
    return finalTable
    
def fileToFormatedTable(fileName,formatString=None,splitVal=None,beStrict=True,startStrict=True) :
    tempTable=fileToTable(fileName,splitVal,True,True)
    if formatString==None :
        formatString = getRowFormat(tempTable[0])
    formatString.lower()
    for i in formatString.split(',') :
        if i <> 'i' and i <> 'f' and i <> 's' :
            print "Invalid format string, give me a string of i, f and s separated by commas! Like 's,i,f,f,s'"
            return
    finalTable = list()
    for i in tempTable :
        rowFormat = getRowFormat(i)
        if beStrict :
            if formatString == rowFormat : finalTable.append(i)
            else : continue
        else :
            if formatString in rowFormat :
                [low,high] = whereStringIn(formatString,rowFormat)
                low = (low+1)/2
		high = (high+1)/2
                if low <> 0 and startStrict : continue
                else : finalTable.append(i[low:high])
            else : continue
    return finalTable

def readcolPy(fileName,formatString=None,splitVal=None,beStrict=True,startStrict=True) :
    finalTable = fileToFormatedTable(fileName,formatString,splitVal,beStrict,startStrict)
#    return finalTable
    return getInvertTable(finalTable)

def getUnique(dataTable,paramRequest) :
    lengthFile = len(dataTable)
    usedVals = []
    uniqueVals = []
    for i in range(lengthFile) :
        if len(dataTable[i]) <= paramRequest : continue
        usedVals.append(dataTable[i][paramRequest])
        if dataTable[i][paramRequest] not in uniqueVals :
            uniqueVals.append(dataTable[i][paramRequest])
    return uniqueVals

def getRestrictTable(dataTable,restrictParam,restrictValue,beStrict=False) :
    restrictTable = []
    locoRestrict = []
    lengthFile = len(dataTable)
    for i in range(lengthFile) :
        try:
            if not beStrict : 
                if restrictValue in dataTable[i][restrictParam] :
                    restrictTable.append(dataTable[i])
                    locoRestrict.append(int(i))
            else :
                if restrictValue == dataTable[i][restrictParam] :
                    restrictTable.append(dataTable[i])
                    locoRestrict.append(int(i))
        except: continue
    return [restrictTable,locoRestrict]

def getInvertTable(tableIn) :
    invertTable = []
    for i in range(len(tableIn[0])) :
        invertCol = []
        for j in range(len(tableIn)) :
            invertCol.append(tableIn[j][i])
        invertTable.append(invertCol)
    return invertTable

def changeTableToFloat(tableIn) :
    newTable = []
    for i in range(len(tableIn)) :
        newColumn = []
        for j in range(len(tableIn[0])) :
            try: new = float(tableIn[i][j])
            except: new=tableIn[i][j]
            newColumn.append(new)
        newTable.append(newColumn)
    return newTable


def getTableColumn(tableIn,column) :
    lengthFile = len(tableIn)
    columnVals = []
    for i in range(lengthFile) : columnVals.append(float(tableIn[i][column]))
    return columnVals
    
def getRowFormat(row,asList=False) :
    formatString = ''
    for i in row :
        if type(i) == int : formatString+='i,' 
        elif type(i) == float : formatString+='f,'
	else : formatString+='s,'
    formatString=formatString[:-1]
    if asList : formatString.split(',')
    return formatString

def whereStringIn(str1,str2) :
    if str1 in str2 :
        lenStr1 = len(str1) ; lenStr2 = len(str2)
        for i in range(lenStr2-lenStr1+1) :
            if str1 == str2[i:i+lenStr1] : return [i,i+lenStr1]
    else : return [-1,-1]
