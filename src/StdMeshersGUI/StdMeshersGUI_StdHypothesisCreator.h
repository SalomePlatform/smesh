//  SMESH StdMeshersGUI : GUI for plugged-in meshers
//
//  Copyright (C) 2003  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : StdMeshersGUI_StdHypothesisCreator.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header: /home/server/cvs/SMESH/SMESH_SRC/src/StdMeshersGUI/StdMeshersGUI_StdHypothesisCreator.h

#ifndef STDMESHERSGUI_StdHypothesisCreator_HeaderFile
#define STDMESHERSGUI_StdHypothesisCreator_HeaderFile

#include <SMESHGUI_Hypotheses.h>

/*!
 * \brief Class for creation of simple hypotheses (only set of parameters without dependencies)
*/
class StdMeshersGUI_StdHypothesisCreator : public SMESHGUI_GenericHypothesisCreator
{
  Q_OBJECT

public:
  StdMeshersGUI_StdHypothesisCreator( const QString& );
  virtual ~StdMeshersGUI_StdHypothesisCreator();

  virtual bool checkParams() const;

protected:
  virtual QFrame*  buildFrame    ();
  virtual void     retrieveParams() const;
  virtual void     storeParams   () const;
  virtual bool     stdParams     ( ListOfStdParams& ) const;
  virtual void     attuneStdWidget( QWidget*, const int ) const;
  virtual QString  caption() const;
  virtual QPixmap  icon() const;
  virtual QString  type() const;

private:
          QString hypTypeName( const QString& ) const;
};

#endif
