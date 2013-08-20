
#include <stdio.h>

#include "DBusBase.h"

#include <casa/Containers/Record.h>
#include <casadbus/session/DBusSession.h>
#include <casadbus/utilities/BusAccess.h>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace casa {

  DBusThreadedBase::DBusThreadedBase():
    itsThread(NULL) 
  {
    itsThread=new boost::thread(boost::bind(&DBusThreadedBase::serviceLoop,
                                            this));
    /* This prevents a race condition that occurs when the service
       is destroyed as soon as it starts */
    boost::this_thread::yield(); 
  }

  DBusThreadedBase::~DBusThreadedBase() {
    if (itsThread != NULL) {
      stopService();
    }
  }

  void DBusThreadedBase::stopService() {
    std::cout << "Leaving dispatcher loop: " << time(0) << std::endl;
    casa::DBusSession::instance().dispatcher( ).leave( );
    std::cout << "Waiting for timed join: " << time(0) <<std::endl;
    if (!itsThread->timed_join(boost::posix_time::time_duration(0,0,30,0))) {
      std::cout << "Error Closing service thread" << std::endl;
    }
    std::cout << "Timed Join Passed: " << time(0) <<std::endl;
    itsThread = NULL;
  }

  void DBusThreadedBase::serviceLoop() {
    casa::DBusSession::instance().dispatcher( ).enter( );
    std::cout << "Service Loop Exited: " << time(0) << std::endl;
    itsThread->detach();
  }

  std::map<std::string,DBus::Variant> 
  DBusThreadedBase::fromRecord(Record record){

    std::map<std::string,DBus::Variant> returnMap;
    
    for (unsigned int idx = 0; idx < record.nfields(); ++idx) {
      RecordFieldId id(idx);      
      switch (record.dataType(id)) {
      case TpBool:
        {
          DBus::Variant v;
          v.writer().append_bool(record.asBool(id));
          returnMap[record.name(id)] = v;
        }
        break;
      case TpInt:
        {
          DBus::Variant v;
          v.writer().append_int32(record.asInt(id));
          returnMap[record.name(id)] = v;
        }
        break;
      case TpFloat:
        {
          DBus::Variant v;
          v.writer().append_double(record.asFloat(id));
          returnMap[record.name(id)] = v;
        }
        break;
      case TpDouble:
        {
          DBus::Variant v;
          v.writer().append_double(record.asDouble(id));
          returnMap[record.name(id)] = v;
        }
        break;
      default:
        std::cout << "Error: Unsupported Type" << std::endl;
      }
    }
    return returnMap;
  }

  Record DBusThreadedBase::toRecord(std::map<std::string,DBus::Variant> mapIn) {
    Record returnRecord;
    
    std::map<std::string,DBus::Variant>::iterator iter;
    for ( iter = mapIn.begin(); iter != mapIn.end(); ++iter) {
      DBus::MessageIter dbusMI = (*iter).second.reader();
      
      switch (dbusMI.type()) {
//       case DBus::Type::TYPE_BOOLEAN:
//         returnRecord.define(RecordFieldId((*iter).first),dbusMI.get_bool());
//         break;
      case 105: //DBus::Type::Type_INT32:
        returnRecord.define(RecordFieldId((*iter).first),dbusMI.get_int32());
        break;
      case 100: //DBus::Type::TYPE_DOUBLE:
        returnRecord.define(RecordFieldId((*iter).first),dbusMI.get_double());
        break;
//       case DBus::Type::TYPE_STRING:
//         returnRecord.define(RecordFieldId((*iter).first),dbusMI.get_string());
//         break;
      default:
        std::cout << "Unsupported type: " << dbusMI.type() << std::endl;
      }
    }
    return returnRecord;
  }  


  /* -------------------------------------------------------- */
  DBusService::DBusService(const std::string &bus_name, const std::string &object_path):
    DBus::ObjectAdaptor(DBusSession::instance().connection( ), 
                        dbus::adaptor_object(bus_name,object_path).c_str( )),
    DBusThreadedBase()
  {
  }

  DBusService::~DBusService() {}
  /* -------------------------------------------------------- */
  ServiceProxy::ServiceProxy(const std::string& serviceName):
    DBus::ObjectProxy(DBusSession::instance().connection( ), 
                      dbus::object(serviceName).c_str(),
                      dbus::path(serviceName).c_str() ),
    DBusThreadedBase() {}

  ServiceProxy::~ServiceProxy() {}

}
