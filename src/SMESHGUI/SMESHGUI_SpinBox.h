//  File      : SMESHGUI_SpinBox.h
//  Created   : 14 august 2002
//  Author    : Lucien PIGNOLONI
//  Project   : SALOME
//  Module    : SMESHGUI
//  Copyright : OPEN CASCADE
//  $Header$

#ifndef  SMESHSPINBOX_H
#define  SMESHSPINBOX_H

#include "QAD_SpinBoxDbl.h" 

//=================================================================================
// class    : SMESHGUI_SpinBox
// purpose  : Derivated from QAD_SpinBoxDbl class 
//=================================================================================
class SMESHGUI_SpinBox : public QAD_SpinBoxDbl
{
  Q_OBJECT

public :

  SMESHGUI_SpinBox( QWidget* parent, const char* name = 0 ) ;
  ~SMESHGUI_SpinBox() ;
  
  void    RangeStepAndValidator( double         min = -1000000.0, 
				 double         max = +1000000.0, 
				 double         step = 100.0, 
				 unsigned short decimals = 3 ) ;
  void    SetValue( double v ) ;
  double  GetValue() ;
  QString GetString() ;

public slots:
  void    SetStep( double newStep );

};
#endif //  SMESHSPINBOX_H
