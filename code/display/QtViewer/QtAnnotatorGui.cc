//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
//#

#include <display/QtViewer/QtAnnotatorGui.qo.h>

#include <iostream>
namespace casa {

	QtAnnotatorGui::QtAnnotatorGui(QtDisplayPanel *parent)
		:activeTool(0), activeMarker(0), activeKey(0)
		,gridLayout2(0), moveMarker(0), pDP(parent), annot(0) {

		setWindowTitle("Annotating...");
		setup();
		//setupUi(this);
		canvasType = canvastype[0];
		try {
			annot = new Annotations(pDP->panelDisplay());
		} catch(...) {
			cout << "Failed to create Annotations"
			     << __FILE__ << " " << __LINE__ << endl;
		}
	}
	QtAnnotatorGui::~QtAnnotatorGui()
	{}


	const QString QtAnnotatorGui::toolshorthands[] = {
		"pointer", "move", "delete",
		"arrow", "text", "rectangle", "square",
		"polyline", "polygon", "ellipse", "circle", "marker"
	};

	const QString QtAnnotatorGui::canvastype[] = {
		"relative screen", "world coordinates", "absolute screen"
	};
	const QString QtAnnotatorGui::basebitmaps[] = {
		"pointer0.png", "dsmove.png" , "delete.png",
		"dsarrow.png", "dstext.png", "dsrectangle.png", "dssquare.png",
		"dspolyline.png", "dspoly.png",  "dsellipse.png", "dscircle.png",
		"dsmarker.png"
	};

	const QString QtAnnotatorGui::markerbitmaps[] = {
		"markercircx.png","markersqr.png",  "markercross.png","markerx.png",
		"markerdia.png",  "markercirc.png",  "markertri.png", "markeritri.png",
		"markerfcirc.png", "markerfsqr.png", "markerfdia.png",   "markerftri.png",
		"markerfitri.png",  "markercircc.png"
	};

	const QString QtAnnotatorGui::helptext[] = {
		"Click on this button with a key to assign that "
		"\nmouse key to control of annotations",
		"Select this tool to move / manipulate shapes that "
		"\nhave already been created and to cancel creation of "
		"\na current shape. The control key also acts as "
		"\na 'modifier', which alters the action the mouse performs."
		"\nHold control and drag a handle to rotate a shape or"
		"\nor hold control inside a shape and drag to scale an object about its"
		" center"
		"\n(not all shapes support this)",
		"Click this button to delete the currently selected"
		"\nshape from the displaypanel.",
		"Select a tool from the panel, and then click on the displaypanel"
		"\nto draw that object / shape",
		"Clicking here will bring up a list (a pop up menu) of "
		"\nmarkers you can use. Select a marker, and then "
		"\nclick on the displaypanel to draw the marker.",
		"What to use to \'lock\' shapes in position"
		"\nThe following options are available for selecting a "
		"\nmethod by which to \'lock\' shapes in position."
		"\n\n- Selecting \'rel. screen\' will lock the shapes onto"
		"\nthe screen via their relative positions. For example, "
		"\na shape which is one quarter of the way from the top of"
		"\n the screen will always remain one quarter of the way "
		"\nfrom the top. This means that upon resizing the window,"
		"\n shapes will tend to stay in the correct position. This "
		"\nmethod also produces good results when printing."
		"\n\n- Selecting \'world co-ords\' will *attempt* to "
		"\nposition the shape using world co-ordinates. "
		"\nThis means that regardless "
		"\nof zoom, resizing etc the annotations will stay locked "
		"\nat the specified world co-ordinates. This obviously means"
		"\n that a shape must be inside a worldcanvas"
		"\n. Creation of a shape will fail if this is not true."
		"\n\n- Selecting \'abs. screen\' will lock purely on pixel "
		"\nposition, relative from the bottom left corner. This is"
		"\n included more for completeness than anything else. "
		"\nObviously resizing the window, zooming etc will "
		"\nleave the shapes in their exact same position, relative"
		"\n to the bottom left of the window."
		"\n\n\nGenerally speaking, if you wish the shape to remain"
		"\n locked to a worldcanvas"
		"\n and are happy for that shape to stay within the bounds"
		"\n of the worldcanvas, then choose worldcanvas as the lock "
		"\nmethod."
		"\n If you require a shape outside a worldcanvas (anywhere"
		"\n else on the screen), then use relative screen as the lock method."
	};

	const int QtAnnotatorGui::dlkeys[] = {
		0, Qt::LeftButton, Qt::MidButton, Qt::RightButton
	};

	void QtAnnotatorGui::setup() {

		setObjectName(QString::fromUtf8("Annotating"));
		resize(QSize(445, 184).expandedTo(minimumSizeHint()));
		QSizePolicy sizePolicy((QSizePolicy::Policy)1, (QSizePolicy::Policy)1);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		setSizePolicy(sizePolicy);

		setMinimumSize(QSize(445, 144));
		verticalLayout_4 = new QWidget(this);
		verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
		verticalLayout_4->setGeometry(QRect(10, 10, 431, 165));

		bGroup = new QButtonGroup(this);
		bGroup->setExclusive(true);

		vboxLayout2 = new QVBoxLayout();
		vboxLayout2->setSpacing(6);
		vboxLayout2->setMargin(0);
		vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
		for (int i = 0; i < 3; i++) {
			button[i] = new QToolButton(verticalLayout_4);
			button[i]->setObjectName(toolshorthands[i]);
			button[i]->setIcon(QIcon(":/icons/" + basebitmaps[i]));
			button[i]->setToolTip(helptext[i]);
			vboxLayout2->addWidget(button[i]);
			bGroup->addButton(button[i]);
			//button[i]->setCheckable(true);
		}
		//button[1]->setCheckable(true);
		activeTool = 0;
		spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum,
		                             QSizePolicy::Expanding);
		vboxLayout2->addItem(spacerItem);
		hboxLayout1 = new QHBoxLayout();
		hboxLayout1->setSpacing(6);
		hboxLayout1->setMargin(0);
		hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
		spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding,
		                              QSizePolicy::Minimum);
		hboxLayout1->addItem(spacerItem1);
		for (int i = 3; i < 12; i++) {
			button[i] = new QToolButton(verticalLayout_4);
			button[i]->setObjectName(toolshorthands[i]);
			button[i]->setIcon(QIcon(":/icons/" + basebitmaps[i]));
			button[i]->setToolTip(helptext[(i == 11) ? 4 : 3] );
			hboxLayout1->addWidget(button[i]);
			bGroup->addButton(button[i]);
			button[i]->setCheckable(true);
			if (i == 5 || i == 6 || i == 9)
				button[i]->setEnabled(false);
		}
		connect(bGroup, SIGNAL(buttonClicked(QAbstractButton *)),
		        this, SLOT(buttonClicked(QAbstractButton*)));
		connect(this, SIGNAL(toolKitChange(const QString,
		                                   const Qt::MouseButton, const bool)),
		        this,SLOT(confirmToolKitChange(const QString,
		                                       const Qt::MouseButton, const bool)));

		mGroup = new QButtonGroup(this);
		mGroup->setExclusive(true);

		gridLayout2 = new QtMarkerGui(this, Qt::Popup);
		gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
		gridLayout2->setGeometry(QRect(90, 220, 160, 151));

		gridLayout1 = new QGridLayout(gridLayout2);
		gridLayout1->setSpacing(6);
		gridLayout1->setMargin(0);
		gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));

		for (int i = 0; i < 14; i++) {
			marker[i] = new QToolButton(gridLayout2);
			marker[i]->setObjectName(markerbitmaps[i]);
			marker[i]->setIcon(QIcon(":/icons/" + markerbitmaps[i]));
			gridLayout1->addWidget(marker[i], i / 4, i  % 4, 1, 1);
			mGroup->addButton(marker[i]);
		}
		activeMarker = 0;
		gridLayout2->setVisible(false);
		connect(mGroup, SIGNAL(buttonClicked(QAbstractButton *)),
		        this, SLOT(markerClicked(QAbstractButton*)));

		hboxLayout2 = new QHBoxLayout();
		hboxLayout2->setSpacing(6);
		hboxLayout2->setMargin(0);
		hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));

		label0 = new QLabel(verticalLayout_4);
		label0->setObjectName(QString::fromUtf8("label0"));
		label0->setText("Lock to: ");
		hboxLayout2->addWidget(label0);

		spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Fixed,
		                              QSizePolicy::Minimum);
		hboxLayout2->addItem(spacerItem2);

		comboBox = new QComboBox(verticalLayout_4);
		comboBox->setObjectName(QString::fromUtf8("comboBox"));
		comboBox->setCurrentIndex(0);
		comboBox->setMaxCount(4);
		comboBox->addItem(canvastype[0]);
		comboBox->addItem(canvastype[1]);
		comboBox->addItem(canvastype[2]);
		comboBox->setToolTip(helptext[5]);
		hboxLayout2->addWidget(comboBox);
		connect(comboBox, SIGNAL(activated(const QString&)),
		        this, SLOT(canvasLockChanged(const QString& )));
		connect(comboBox, SIGNAL(activated(int )),
		        this, SLOT(canvasLockChanged(int )));

		hboxLayout3 = new QHBoxLayout();
		hboxLayout3->setSpacing(6);
		hboxLayout3->setMargin(0);
		hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));

		pushButton2 = new QPushButton(verticalLayout_4);
		pushButton2->setObjectName(QString::fromUtf8("pushButton2"));
		pushButton2->setIcon(QIcon(QString::fromUtf8(":/icons/downarrow.png")));
		pushButton2->setText("Option");
		pushButton2->setCheckable(true);
		pushButton2->setChecked(false);
		connect(pushButton2, SIGNAL(clicked()),
		        this, SLOT(getShapeOptions()));
		hboxLayout3->addWidget(pushButton2);

		groupBox_2 = new QGroupBox(verticalLayout_4);
		groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));

		vboxLayout3 = new QVBoxLayout();
		vboxLayout3->setSpacing(6);
		vboxLayout3->setMargin(0);
		vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
		vboxLayout3->addLayout(hboxLayout1);
		vboxLayout3->addLayout(hboxLayout2);
		vboxLayout3->addLayout(hboxLayout3);
		vboxLayout3->addWidget(groupBox_2);

		hboxLayout = new QHBoxLayout();
		hboxLayout->setSpacing(6);
		hboxLayout->setMargin(0);
		hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
		hboxLayout->addLayout(vboxLayout2);
		hboxLayout->addLayout(vboxLayout3);

		hboxLayout4 = new QHBoxLayout();
		hboxLayout4->setSpacing(6);
		hboxLayout4->setMargin(0);
		hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));

		pushButton1 = new QPushButton(verticalLayout_4);
		pushButton1->setObjectName(QString::fromUtf8("pushButton1"));
		hboxLayout4->addWidget(pushButton1);

		pushButton3 = new QPushButton(verticalLayout_4);
		pushButton3->setObjectName(QString::fromUtf8("pushButton3"));
		hboxLayout4->addWidget(pushButton3);

		lineEdit2 = new QLineEdit(verticalLayout_4);
		lineEdit2->setObjectName(QString::fromUtf8("lineEdit2"));
		hboxLayout4->addWidget(lineEdit2);

		toolButton13 = new QToolButton(verticalLayout_4);
		toolButton13->setObjectName(QString::fromUtf8("toolButton13"));
		toolButton13->setIcon(QIcon(QString::fromUtf8(":/icons/spanner.png")));
		hboxLayout4->addWidget(toolButton13);

		label2 = new QLabel(verticalLayout_4);
		label2->setObjectName(QString::fromUtf8("label2"));
		label2->setPixmap(QPixmap(QString::fromUtf8(":/icons/tick.png")));
		hboxLayout4->addWidget(label2);

		spacerItem3 = new QSpacerItem(40, 20, QSizePolicy::Expanding,
		                              QSizePolicy::Minimum);
		hboxLayout4->addItem(spacerItem3);

		pushButton7 = new QPushButton(verticalLayout_4);
		pushButton7->setObjectName(QString::fromUtf8("pushButton7"));
		hboxLayout4->addWidget(pushButton7);

		spacerItem4 = new QSpacerItem(20, 40, QSizePolicy::Minimum,
		                              QSizePolicy::Expanding);

		vboxLayout1 = new QVBoxLayout();
		vboxLayout1->setSpacing(6);
		vboxLayout1->setMargin(0);
		vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
		vboxLayout1->addLayout(hboxLayout);
		vboxLayout1->addLayout(hboxLayout4);
		vboxLayout1->addItem(spacerItem4);


		vboxLayout = new QVBoxLayout(verticalLayout_4);
		vboxLayout->setSpacing(6);
		vboxLayout->setMargin(0);
		vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
		vboxLayout->addLayout(vboxLayout1);


		QMetaObject::connectSlotsByName(this);

	}

	void QtAnnotatorGui::buttonClicked(QAbstractButton* butto) {
		//cout << "Annotator clicked" << endl;
		QToolButton* bt = dynamic_cast<QToolButton*>(butto);
		if(bt!=0) {
			for (int i = 0; i < bGroup->buttons().count(); i++) {
				if (bt == bGroup->buttons().at(i)) {
					emit toolKitChange(toolshorthands[i],
					                   Qt::LeftButton, pushButton2->isChecked());
				}
			}
		}
	}
	void QtAnnotatorGui::markerClicked(QAbstractButton* butto) {
		//cout << "marker clicked" << endl;
		QToolButton* bt = dynamic_cast<QToolButton*>(butto);
		if(bt!=0) {
			for (int i = 0; i < mGroup->buttons().count(); i++) {
				if (bt == mGroup->buttons().at(i)) {
					activeMarker = i;
					activeTool = 11;
					button[activeTool]->setChecked(true);
					emit toolKitChange(toolshorthands[activeTool],
					                   Qt::LeftButton, pushButton2->isChecked());
					break;
				}
			}
		}
	}
	void QtAnnotatorGui::markerReleased(QMouseEvent *event) {
		//cout << "marker released" << endl;
		int x = event->x();
		int y = event->y();
		if (mGroup != 0) {
			for (int i = 0; i < mGroup->buttons().count(); i++) {
				QToolButton* bt = (QToolButton*)(mGroup->buttons().at(i));
				QRect rect = bt->geometry();
				if (rect.contains(x, y)) {
					activeMarker = i;
					activeTool = 11;
					button[activeTool]->setChecked(true);
					emit toolKitChange(toolshorthands[activeTool],
					                   event->button(), pushButton2->isChecked());
					break;
				}
			}
		}
	}

	void QtAnnotatorGui::confirmToolKitChange(const QString tool,
	        const Qt::MouseButton key, const bool /*getOption*/) {
		//The receiver of this signal can decide whether is a command 'pointer',
		//'move', 'delete' and mouseKey id
		//or a shape or marker, do whatever on requested on displaypanel. If asked
		//for options, get options and
		//send back options such that this piece can add it the panel
		//if we create this panel use a panelDisplay as parent, these functions can
		//be moved into this class
		static bool markerShown = false;

		//determine the mouseKey
		int currentKey = 0;
		if (key == Qt::LeftButton) currentKey = 1;
		if (key == Qt::RightButton) currentKey = 3;
		if (key == Qt::MidButton) currentKey = 2;

		//determine the currentTool
		int currentTool = 0;
		for (int i = 0; i < bGroup->buttons().count(); i++) {
			if (tool == toolshorthands[i]) {
				currentTool = i;
				break;
			}
		}

		//cout << "received toolKitChange signal:  ";
		//if (activeTool != 11)
		//    cout << "tool= " << tool.toStdString();
		//else
		//    cout << " marker= " << markerbitmaps[activeMarker].toStdString() ;
		cout << " activeTool=" << activeTool;
		cout << " currentTool=" << currentTool;
		cout << " activeKey=" << activeKey;
		cout << " currentKey=" << currentKey;
		cout << " activeMarker=" << activeMarker;
		cout << endl;
		//cout << " request shape options=" << getOption << endl;

		QString str;
		if (activeKey == 0 || activeTool == 0) {
			//first time, can determine the mouse key
			//cout << "activeKey==0" << endl;
			activeKey = currentKey;
			button[0]->setIcon(QIcon(":/icons/" + toolshorthands[0] +
			                         str.setNum(activeKey) + ".png"));
			annot->setKey((Display::KeySym)activeKey);
		}

		if (currentKey != activeKey) {
			//cout << "currentKey != activeKey" << endl;
			button[activeTool]->setChecked(true);
			if (currentTool == 0) {
				activeKey = currentKey;
				button[0]->setIcon(QIcon(":/icons/" + toolshorthands[0] +
				                         str.setNum(activeKey) + ".png"));
				annot->setKey((Display::KeySym)activeKey);
			}
			return;
		}

		if (activeTool == 0 && currentTool < 2) {
			//cout << "activeTool==0 && currentTool=" << currentTool  << endl;
			currentTool = 3;
		}

		if (currentTool == 1) {
			cout << "currentTool==1" << endl;
			//tool == toolshorthands[1]
			QPalette palette(button[1]->palette() );
			moveMarker = !moveMarker;
			cout << "moveMarker=" << moveMarker << endl;
			if (moveMarker) {
				palette.setColor(QPalette::Base, Qt::green);
			} else
				palette.setColor(QPalette::Base, Qt::gray);
			//button[1]->setForegroundRole(QPalette::Dark);
			//button[1]->setPalette(palette);
			button[1]->setChecked(true);
			cout << toolshorthands[1].toStdString() << endl;
		} else if (currentTool == 2) {
			//cout << "currentTool==2" << endl;
			//toolshorthands[2]
			cout << toolshorthands[2].toStdString() << endl;
		} else if (currentTool == 11) {
			//cout << "currentTool==11" << endl;
			//tool == toolshorthands[11]
			markerShown = !markerShown;
			gridLayout2->move(x() + width() - 150, y() + 60);
			gridLayout2->setVisible(markerShown);
			button[11]->setChecked(markerShown);
			activeTool = 11;
			cout << "activeTool=" << activeTool
			     << " activeMarker=" << activeMarker << endl;
			button[activeTool]->setIcon(QIcon(":/icons/" +
			                                  markerbitmaps[activeMarker]));
			//return;
		} else {
			//tool == toolshorthands[activeTool]
			//cout << "currentTool==" << currentTool << endl;
			activeTool = currentTool;
			cout << toolshorthands[activeTool].toStdString() << endl;
			button[activeTool]->setIcon(QIcon(":/icons/" +
			                                  basebitmaps[activeTool]));
			button[activeTool]->setChecked(true);
		}

		//#dk QtXmlRecord xmlRec;
		//#dk Record rec = annot->availableShapes();
		//#dk xmlRec.printRecord(&rec);

		//#dk Use this instead (if you really need it...).
		// cout << annot->availableShapes();


		return;
	}

	void QtAnnotatorGui::mouseReleaseEvent(QMouseEvent *event) {
		//cout << "Annotator released" << endl;
		int x = event->x();
		int y = event->y();
		for (int i = 0; i < bGroup->buttons().count(); i++) {
			QRect rect = button[i]->geometry();
			if (rect.contains(x, y)) {
				button[i]->setChecked(true);
				emit toolKitChange(toolshorthands[i], event->button(),
				                   pushButton2->isChecked());
				break;
			}
		}
	}
	void QtAnnotatorGui::canvasLockChanged(const QString &tool) {
		canvasType = tool;
		//cout << " canvas lock changed= "
		//     << canvasType.toStdString()
		//     << endl;
	}
	void QtAnnotatorGui::canvasLockChanged(int idx) {
		canvasType = canvasType[idx];
		//cout << " canvas lock changed= "
		//     << canvastype[idx].toStdString()
		//     << endl;
	}
	void QtAnnotatorGui::buttonClicked(int /*i*/) {
		// std::cout << "Qt 4.1 -- left button clicked" << i << " -> " <<
		// tools[i].toStdString() << std::endl;
		// emit toolKitChange(tools[i], 1);
	}

	QButtonGroup* QtAnnotatorGui::mgroup() {
		return mGroup;
	}


	QtMarkerGui::QtMarkerGui(QtAnnotatorGui *parent, Qt::WFlags f)
		:QWidget(parent, f), parent(parent) {
		//setWindowTitle("Marker");
	}

	QtMarkerGui::~QtMarkerGui()
	{}

	void QtMarkerGui::mouseReleaseEvent ( QMouseEvent * event ) {
		//cout << "marker mouse release event" << endl;
		parent->markerReleased(event);
	}


}

