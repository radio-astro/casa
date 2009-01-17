#include <QThread>

class qtListner : public QThread
{
   Q_OBJECT
   public :
      qtListner(int argc, char **argv_mod, char *path);
      ~qtListner(){}
      void run();
      void signal_showviewer();
   signals :
      void showviewer();
};
