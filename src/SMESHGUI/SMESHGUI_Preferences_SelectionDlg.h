//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.salome-platorm.org or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_Preferences_SelectionDlg.h
//  Author : Natalia KOPNOVA
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_PREFERENCES_SELECTIONDLG_H
#define SMESHGUI_PREFERENCES_SELECTIONDLG_H


// QT Includes
#include <qdialog.h>
#include <qlineedit.h>
#include <qvalidator.h>

class QPushButton;
class QSpinBox;
class QColor;

class SMESHGUI_LineEdit : public QLineEdit
{
  Q_OBJECT

  public:
    SMESHGUI_LineEdit(QWidget* parent, const char* name = 0);
    SMESHGUI_LineEdit(const QString& text, QWidget* parent, const char* name = 0);

    ~SMESHGUI_LineEdit() {};

  protected:
    void focusOutEvent(QFocusEvent* e);
};

class SMESHGUI_DoubleValidator : public QDoubleValidator
{
  Q_OBJECT

  public:
    SMESHGUI_DoubleValidator(QObject* parent, const char* name = 0);
    SMESHGUI_DoubleValidator(double bottom, double top, int decimals,
			    QObject* parent, const char* name = 0);

    ~SMESHGUI_DoubleValidator() {};

    void fixup(QString& text) const;
};

class SMESHGUI_Preferences_SelectionDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_Preferences_SelectionDlg( QWidget* parent = 0, const char* name = 0 );
    ~SMESHGUI_Preferences_SelectionDlg();

    void   SetColor(int type, QColor color);
    QColor GetColor(int type);
    void   SetWidth(int type, int value);
    int    GetWidth(int type);
    void   SetPrecision(int type, double value);
    double GetPrecision(int type);

private:
    void closeEvent( QCloseEvent* e ) ;

private slots:
    void onSelectColor();

private:
    QPushButton*          myColor[3];
    QSpinBox*             myWidth[2];
    QLineEdit*            myPrecision[2];
};

#endif // SMESHGUI_PREFERENCES_SELECTIONDLG_H
