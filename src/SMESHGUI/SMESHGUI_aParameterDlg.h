//  File      : SMESHGUI_aParameterDlg.h
//  Created   : Wed Jun 12 21:04:41 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef SMESHGUI_aParameterDlg_H
#define SMESHGUI_aParameterDlg_H

// QT Includes
#include <qdialog.h>

class QLabel;
class QPushButton;
class QSpinBox;
class SMESHGUI;
class QAD_SpinBoxDbl;

//=================================================================================
// class    : SMESHGUI_aParameterDlg
// purpose  :
//=================================================================================
class SMESHGUI_aParameterDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_aParameterDlg( QWidget*     parent   = 0,
			    QString      title    = QString::null,
			    QString      label    = QString::null,
			    const double bottom   = -1E6,
			    const double top      = +1E6,
			    const int    decimals = 3,
			    bool         modal    = TRUE );
    SMESHGUI_aParameterDlg( QWidget*     parent   = 0,
			    QString      title    = QString::null,
			    QString      label    = QString::null,
			    const int    bottom   = 0,
			    const int    top      = 1000,
			    bool         modal    = TRUE );

    ~SMESHGUI_aParameterDlg();

    void   setValue( const double val );
    void   setValue( const int    val );
    double getDblValue();
    int    getIntValue();

protected:
    void init( bool isDouble );

private:
    SMESHGUI*       mySMESHGUI ;

    QPushButton*    myButtonOk;
    QPushButton*    myButtonCancel;
    QLabel*         myTextLabel;
    QSpinBox*       myIntSpin;
    QAD_SpinBoxDbl* myDblSpin;
};

#endif // SMESHGUI_aParameterDlg.h
