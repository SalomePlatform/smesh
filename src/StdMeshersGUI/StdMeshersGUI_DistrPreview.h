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

// File   : StdMeshersGUI_DistrPreview.h
// Author : Open CASCADE S.A.S.
//
#ifndef STDMESHERSGUI_DISTRPREVIEW_H
#define STDMESHERSGUI_DISTRPREVIEW_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"

// Qwt includes
#include <qwt_plot.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

// OCCT includes
#include <ExprIntrp_GenExp.hxx>
#include <Expr_Array1OfNamedUnknown.hxx>
#include <TColStd_Array1OfReal.hxx>

class QwtPlotCurve;
class QwtPlotMarker;

class STDMESHERSGUI_EXPORT StdMeshersGUI_DistrPreview : public QwtPlot
{
  Q_OBJECT

public:
  typedef enum { EXPONENT, CUT_NEGATIVE }  Conversion;

  StdMeshersGUI_DistrPreview( QWidget*, StdMeshers::StdMeshers_NumberOfSegments_ptr );
  virtual ~StdMeshersGUI_DistrPreview();

  QString   function() const;
  bool      isTableFunc() const;
  void      tableFunc( SMESH::double_array& ) const;
  int       pointsCount() const;
  int       nbSeg() const;
  bool      isDone() const;

  bool      setParams( const QString&, const int, const int = 50, const bool = true );
  bool      setParams( const SMESH::double_array&, const int, const bool = true );
  void      setConversion( Conversion, const bool = true );

protected:
  virtual   bool   init( const QString& );
  virtual   double funcValue( const double, bool& );
  virtual   bool   createTable( SMESH::double_array& );
  virtual   bool   convert( double& ) const;
            void   update();

private:
  double calc( bool& );
  void   showError();

private:
  QString                   myFunction;
  int                       myPoints, myNbSeg;
  bool                      myIsTable;
  Conversion                myConv;
  SMESH::double_array       myTableFunc;
  QwtPlotCurve*             myDensity;
  QwtPlotCurve*             myDistr;
  QwtPlotMarker*            myMsg;
  Handle(ExprIntrp_GenExp)  myExpr;
  Expr_Array1OfNamedUnknown myVars;
  TColStd_Array1OfReal      myValues;
  bool                      myIsDone;
  StdMeshers::StdMeshers_NumberOfSegments_var  myHypo;
};

#endif // STDMESHERSGUI_DISTRPREVIEW_H
