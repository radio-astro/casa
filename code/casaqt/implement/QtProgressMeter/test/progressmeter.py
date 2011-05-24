import time
import datetime

progressFile = "/tmp/xidjapdfs"

class ProgressMeter:
     def __init__(self, min, max, title):
          self.min = min
          self.max = max
          self.title = title
          self.value = min
          t = datetime.datetime.now()
          self.id = int(time.mktime(t.timetuple()))
     
     def _update(self, val) :
          self.value = val
          fileHandle = open(progressFile, 'a')
          fileHandle.write(str(self.id))
          fileHandle.write(',')
          fileHandle.write(self.title)
          fileHandle.write(',')
          fileHandle.write(str(self.min))
          fileHandle.write(',')
          fileHandle.write(str(self.max))
          fileHandle.write(',')
          fileHandle.write(str(self.value))
          fileHandle.write('\n')
          fileHandle.close() 

     def busy(self) :
          #self.min = 0
          #self.max = 0
          #self.value = 1
          fileHandle = open(progressFile, 'a')
          fileHandle.write(str(self.id))
          fileHandle.write(',')
          fileHandle.write(self.title)
          fileHandle.write(',0,0,1\n')
          fileHandle.close() 

     def done(self) :
          #self.min = 0
          #self.max = 1
          #self.value = 1
          fileHandle = open(progressFile, 'a')
          fileHandle.write(str(self.id))
          fileHandle.write(',')
          fileHandle.write(self.title)
          fileHandle.write(',0,1,1\n')
          fileHandle.close() 

