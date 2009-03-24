//# QtProgressWidget.qo.h: Qt widget for displaying progress information.
//# Copyright (C) 2009
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
//# $Id: $
#ifndef QTPROGRESSWIDGET_QO_H_
#define QTPROGRESSWIDGET_QO_H_

#include <casaqt/QtUtilities/QtProgressWidget.ui.h>

#include <QMutex>

namespace casa {

// GUI for displaying progress information.  It also shows "background",
// "cancel", and "pause/resume" buttons for the user.  The parent of this
// widget is responsible for implementing these behaviors as needed.
class QtProgressWidget : public QWidget, Ui::ProgressWidget {
    Q_OBJECT
    
public:
    // Constructor which takes the PlotMS parent an optional parent widget.
    QtProgressWidget(bool allowBackground = true,
            bool allowPauseResume = true, bool allowCancel = true,
            bool isDialog = true, QWidget* parent = NULL);
    
    // Destructor.
    ~QtProgressWidget();
    
    
    // Sets which operations the user can request using this widget.
    void setAllowedOperations(bool background, bool pauseResume, bool cancel);
    
    // Returns the mutex used for this widget.
    QMutex& mutex();
    
public slots:
    // Initializes the GUI with the given operation name.  Should be called
    // before the operation starts.
    void initialize(const QString& operationName);

    // Sets the status to the given.
    void setStatus(const QString& status);
    
    // Sets the progress percentage (0 - 100) to the given.
    void setProgress(unsigned int progress);
    
    // Sets the progress percentage (0 - 100) and the status to the given.
    void setProgress(unsigned int progress, const QString& status);
    
    // Finalizes the GUI.  Should be called after the operation ends.
    void finalize();
    
signals:
    // Signal for when the user requests "background" for the thread.
    void backgroundRequested();
    
    // Signal for when the user requests "pause" for the thread.
    void pauseRequested();
    
    // Signal for when the user requests "resume" for the thread.
    void resumeRequested();
    
    // Signal for when the user requests "cancel" for the thread.
    void cancelRequested();
    
protected:
    // Overrides QWidget::closeEvent().  Connects closing to the "background"
    // signal.
    void closeEvent(QCloseEvent* event);
    
private:    
    // Mutex.
    QMutex itsMutex_;
    
private slots:
    // For the "background" button.
    void background();
    
    // For the "pause"/"resume" button.
    void pauseResume(bool pause);
    
    // For the "cancel" button.
    void cancel();
};

}

#endif /* QTPROGRESSWIDGET_QO_H_ */
