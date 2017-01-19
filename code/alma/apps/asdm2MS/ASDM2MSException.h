#ifndef ASDM2MSEXCEPTION
#define ASDM2MSEXCEPTION

//
// A class to describe ASDM2MS Exceptions.
//
class ASDM2MSException {  
public:
  /**
   * An empty contructor.
   */
  ASDM2MSException();
  
  /**
   * A constructor with a message associated with the exception.
   * @param m a string containing the message.
   */
  ASDM2MSException(string m);
  
  /**
   * The destructor.
   */
  virtual ~ASDM2MSException();
  
  /**
   * Returns the message associated to this exception.
   * @return a string.
   */
  string getMessage() const;
  
protected:
  string message;
  
};

inline ASDM2MSException::ASDM2MSException() : message ("ASDM2MSException") {}
inline ASDM2MSException::ASDM2MSException(string m) : message(m) {}
inline ASDM2MSException::~ASDM2MSException() {}
inline string ASDM2MSException::getMessage() const {
  return "ASDM2MSException : " + message;
}

#endif
