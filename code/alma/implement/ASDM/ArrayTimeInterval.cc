#include <ArrayTimeInterval.h>
namespace asdm {
  
#ifndef WITHOUT_ACS
  /**
   * Create a ArrayTimeInterval from an IDLArrayTimeInterval object.
   * @param t The IDL ArrayTimeInterval object.
   */
  ArrayTimeInterval::ArrayTimeInterval (IDLArrayTimeInterval t) {
    start = ArrayTime(t.start);
    duration = Interval(t.duration); 
  }
  
  // inline to IDL conversion.
  const asdmIDLTypes::IDLArrayTimeInterval ArrayTimeInterval::toIDLArrayTimeInterval()const {
    IDLArrayTimeInterval x;
    x.start = start.get();
    x.duration = duration.get();
    return x;
  }
#endif

  void ArrayTimeInterval::toBin(EndianOSStream& eoss) {
    eoss.writeLongLong(getStart().get());
    eoss.writeLongLong(getDuration().get());
  }

  void ArrayTimeInterval::toBin(const vector<ArrayTimeInterval>& arrayTimeInterval,  EndianOSStream& eoss) {
    eoss.writeInt((int) arrayTimeInterval.size());
    for (unsigned int i = 0; i < arrayTimeInterval.size(); i++) {
      eoss.writeLongLong(arrayTimeInterval.at(i).getStart().get());
      eoss.writeLongLong(arrayTimeInterval.at(i).getDuration().get());		
    }
  }

  void ArrayTimeInterval::toBin(const vector<vector<ArrayTimeInterval> >& arrayTimeInterval,  EndianOSStream& eoss) {
    eoss.writeInt((int) arrayTimeInterval.size());
    eoss.writeInt((int) arrayTimeInterval.at(0).size());
    for (unsigned int i = 0; i < arrayTimeInterval.size(); i++)
      for (unsigned int j = 0; j < arrayTimeInterval.at(0).size(); j++) {
	eoss.writeLongLong(arrayTimeInterval.at(i).at(j).getStart().get());
	eoss.writeLongLong(arrayTimeInterval.at(i).at(j).getDuration().get());		
      }
  }

  void ArrayTimeInterval::toBin(const vector< vector<vector<ArrayTimeInterval> > >& arrayTimeInterval,  EndianOSStream& eoss) {
    eoss.writeInt((int) arrayTimeInterval.size());
    eoss.writeInt((int) arrayTimeInterval.at(0).size());
    eoss.writeInt((int) arrayTimeInterval.at(0).at(0).size());	
    for (unsigned int i = 0; i < arrayTimeInterval.size(); i++)
      for (unsigned int j = 0; j < arrayTimeInterval.at(0).size(); j++)
	for (unsigned int k = 0; k < arrayTimeInterval.at(0).at(0).size(); j++) {
	  eoss.writeLongLong(arrayTimeInterval.at(i).at(j).at(k).getStart().get());
	  eoss.writeLongLong(arrayTimeInterval.at(i).at(j).at(k).getDuration().get());		
	}
  }

  ArrayTimeInterval ArrayTimeInterval::fromBin(EndianISStream & eiss) {
    int64_t start = eiss.readLongLong();
    int64_t duration = eiss.readLongLong();
    return ArrayTimeInterval(start, duration);
  }

  vector<ArrayTimeInterval> ArrayTimeInterval::from1DBin(EndianISStream & eiss) {
    int64_t start;
    int64_t duration;
    int dim1 = eiss.readInt();
    vector<ArrayTimeInterval> result;
    for (int i = 0; i < dim1; i++) {
      start = eiss.readLongLong();
      duration = eiss.readLongLong();		
      result.push_back(ArrayTimeInterval(start, duration));
    }
    return result;	
  }

  vector<vector<ArrayTimeInterval > > ArrayTimeInterval::from2DBin(EndianISStream & eiss) {
    int64_t start;
    int64_t duration;
    int dim1 = eiss.readInt();
    int dim2 = eiss.readInt();
    vector< vector<ArrayTimeInterval> >result;
    vector <ArrayTimeInterval> aux;
    for (int i = 0; i < dim1; i++) {
      aux.clear();
      for (int j = 0; j < dim2; j++) {
	start = eiss.readLongLong();
	duration = eiss.readLongLong();	
	aux.push_back(ArrayTimeInterval(start, duration));
      }
      result.push_back(aux);
    }
    return result;	
  }

  vector<vector<vector<ArrayTimeInterval > > > ArrayTimeInterval::from3DBin(EndianISStream & eiss) {
    int64_t start;
    int64_t duration;
    int dim1 = eiss.readInt();
    int dim2 = eiss.readInt();
    int dim3 = eiss.readInt();
    vector<vector< vector<ArrayTimeInterval> > >result;
    vector < vector<ArrayTimeInterval> >aux1;
    vector <ArrayTimeInterval> aux2;
    for (int i = 0; i < dim1; i++) {
      aux1.clear();
      for (int j = 0; j < dim2; j++) {
	aux2.clear();
	for (int k = 0; k < dim3; k++) {
	  start = eiss.readLongLong();
	  duration = eiss.readLongLong();	
	  aux2.push_back(ArrayTimeInterval(start, duration));
	}
	aux1.push_back(aux2);
      }
      result.push_back(aux1);
    }
    return result;	
  }

  string ArrayTimeInterval::toString() const {
    ostringstream oss;
    oss << "start=" << start.toFITS() << ",duration=" << duration / 1.e9 << "s." ;
    return oss.str();
  }
} // end name space
