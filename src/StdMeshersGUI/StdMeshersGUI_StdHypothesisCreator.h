// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// File   : StdMeshersGUI_StdHypothesisCreator.h
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
//
#ifndef STDMESHERSGUI_STDHYPOTHESISCREATOR_H
#define STDMESHERSGUI_STDHYPOTHESISCREATOR_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"
#include <SMESHGUI_Hypotheses.h>


/*!
 * \brief Class for creation of standard hypotheses
*/
class STDMESHERSGUI_EXPORT StdMeshersGUI_StdHypothesisCreator : public SMESHGUI_GenericHypothesisCreator
{
  Q_OBJECT

public:
  StdMeshersGUI_StdHypothesisCreator( const QString& );
  virtual ~StdMeshersGUI_StdHypothesisCreator();

  virtual bool checkParams( QString& ) const;

protected:
  virtual QFrame*  buildFrame    ();
  virtual void     retrieveParams() const;
  virtual QString  storeParams   () const;
  virtual bool     stdParams     ( ListOfStdParams& ) const;
  virtual void     attuneStdWidget( QWidget*, const int ) const;
  virtual QString  caption() const;
  virtual QPixmap  icon() const;
  virtual QString  type() const;
  virtual QWidget* getCustomWidget( const StdParam&, QWidget*, const int ) const;
  virtual QWidget* getHelperWidget() const { return myHelperWidget; }
  virtual bool     getParamFromCustomWidget( StdParam& , QWidget* ) const;

  virtual QString  hypTypeName( const QString& ) const;
  virtual QWidget* getWidgetForParam( int paramIndex ) const;
  virtual ListOfWidgets* customWidgets() const;
  virtual void     onReject();
  virtual void     valueChanged( QWidget* );

  bool             initVariableName(SMESH::SMESH_Hypothesis_var theHyp, StdParam& theParams, const char* theMethod) const;
  QWidget*         makeReverseEdgesWdg( SMESH::long_array_var edgeIDs,
                                        CORBA::String_var     shapeEntry) const;
  


  template<class T>
    T* widget(int i) const {
    return dynamic_cast< T* >( getWidgetForParam( i ));
  }

  ListOfWidgets    myCustomWidgets;
  QWidget*         myHelperWidget;
};

#endif // STDMESHERSGUI_STDHYPOTHESISCREATOR_H
