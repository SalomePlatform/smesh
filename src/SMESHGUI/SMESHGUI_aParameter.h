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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_aParameter.h
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_aParameter_H
#define SMESHGUI_aParameter_H

#include <boost/shared_ptr.hpp>
#include <qstring.h>

class QWidget;
class SMESHGUI_aParameter;

typedef boost::shared_ptr<SMESHGUI_aParameter> SMESHGUI_aParameterPtr;

//=================================================================================
// class    : SMESHGUI_aParameter
// purpose  :
//=================================================================================
class SMESHGUI_aParameter
{ 
public:
  SMESHGUI_aParameter(const QString& label):_label(label) {}
  virtual ~SMESHGUI_aParameter();

  enum Type { INT, DOUBLE };
  virtual Type GetType() const = 0;
  virtual bool GetNewInt( int & Value ) const = 0;
  virtual bool GetNewDouble( double & Value ) const = 0;
  virtual void InitializeWidget( QWidget* ) const = 0;
  virtual void TakeValue( QWidget* ) = 0;

  QString & Label() { return _label; }

 private:
  QString _label;
};

//=================================================================================
// class    : SMESHGUI_intParameter
// purpose  :
//=================================================================================
class SMESHGUI_intParameter: public SMESHGUI_aParameter
{ 
public:
  SMESHGUI_intParameter(const int      initValue = 0,
			const QString& label     = QString::null,
			const int      bottom    = 0,
			const int      top       = 1000);
  int & InitValue() { return _initValue; }
  int & Top()       { return _top; }
  int & Bottom()    { return _bottom; }
  virtual Type GetType() const;
  virtual bool GetNewInt( int & Value ) const;
  virtual bool GetNewDouble( double & Value ) const;
  virtual void InitializeWidget( QWidget* ) const;
  virtual void TakeValue( QWidget* );

 private:
  int _top, _bottom;
  int _initValue, _newValue;
};

//=================================================================================
// class    : SMESHGUI_doubleParameter
// purpose  :
//=================================================================================
class SMESHGUI_doubleParameter: public SMESHGUI_aParameter
{ 
public:
  SMESHGUI_doubleParameter(const double   initValue = 0.0,
			   const QString& label     = QString::null,
			   const double   bottom    = -1E6,
			   const double   top       = +1E6,
			   const double   step      = 1.0,
			   const int      decimals  = 3);
  double & InitValue() { return _initValue; }
  double & Top()       { return _top; }
  double & Bottom()    { return _bottom; }
  double & Step()      { return _step; }
  int    & Decimals()  { return _decimals; }
  virtual Type GetType() const;
  virtual bool GetNewInt( int & Value ) const;
  virtual bool GetNewDouble( double & Value ) const;
  virtual void InitializeWidget( QWidget* ) const;
  virtual void TakeValue( QWidget* );

 private:
  double _top, _bottom, _step;
  double _initValue, _newValue;
  int _decimals;
};

#endif // SMESHGUI_aParameter.h
