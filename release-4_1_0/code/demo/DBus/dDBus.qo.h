#ifndef _DDBUS_QO_H_
#define _DDBUS_QO_H_
#include <string>
#include <QtCore/QObject>
#include <QtGui/QMainWindow>
#include <QtGui/QLineEdit>
#include <QtDBus/QDBusAbstractAdaptor>
#include <casaqt/QtUtilities/QtDBusApp.h>


namespace casa {

    class EditlineGuiAdaptor;

    class EditlineGui : public QMainWindow {
	Q_OBJECT
	public:
	    EditlineGui( const std::string &dbus_name="", QWidget *parent = 0 );
	    ~EditlineGui( );

	    void set( const QString &txt ) { le->setText(txt); }
	    QString get( ) { return le->text( ); }

	private:
	    QLineEdit *le;
	    EditlineGuiAdaptor *adaptor;
    };


    class EditlineGuiAdaptor: public QDBusAbstractAdaptor, public QtDBusApp {
	    Q_OBJECT
	    Q_CLASSINFO("D-Bus Interface", "edu.nrao.casa.editlinegui")
	public:
	    QString dbusName( ) const { return "editlinegui"; }
	    bool connectToDBus( const QString &dbus_name="" )
			{ return QtDBusApp::connectToDBus( parent(), dbus_name ); }

	    EditlineGuiAdaptor( EditlineGui *elg ) : QDBusAbstractAdaptor(new QObject()), editline_(elg) { }
	    ~EditlineGuiAdaptor( ) { }

	public slots:
	    void set( const QString &txt ) { editline_->set(txt); }
	    QString get( ) { return editline_->get( ); }

	private:
	    EditlineGui *editline_;
    };

    inline EditlineGui::~EditlineGui( ) { delete adaptor; }

    inline EditlineGui::EditlineGui( const std::string &dbus_name, QWidget *parent ) : QMainWindow(parent),
							le(new QLineEdit("*some*text*",this)),
							adaptor(new EditlineGuiAdaptor(this)) {
	le->setMinimumWidth(200);
	setCentralWidget(le);
	setFixedHeight(30);

	adaptor->connectToDBus( dbus_name.c_str( ) );
    }

}

#endif
