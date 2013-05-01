#if     !defined(_ERROR_H)

using namespace std;
#include <string>
#include <stdarg.h>

enum ErrorLevel { NOERROR=0, WARNING, MINOR, SERIOUS, FATAL };

class Error
{
 public:

  Error();
  Error(ErrorLevel errlev);
  Error(ErrorLevel errlev, string message);
  Error(ErrorLevel errlev, char* fmt, ... );
  Error(int errlev);
  Error(int errlev, string message);
  Error(int errlev, char* fmt, ... );
  ~Error();

  void   notify(ErrorLevel errlev, string message);
  void   notify(string message);

  static void       setAcceptableLevel(ErrorLevel errlev);
  static ErrorLevel getAcceptableLevel();
  static string     getAcceptableLevelToString();

  static ErrorLevel getLevel();
  static string     getLevelToString();
  static string     getErrorMessage();

  static void       clearMessage();
  static void       clearErrLev();

 private:
  static ErrorLevel acceptableErrorLevel;
  static string     errorMessage_;
  static ErrorLevel errlev_;
  string            errorLog;
  void              printMessage( string message);
};

#define _ERROR_H
#endif
