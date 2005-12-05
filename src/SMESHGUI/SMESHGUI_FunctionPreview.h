// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//

#ifndef SMESHGUI_FUNCTION_PREVIEW_HEADER
#define SMESHGUI_FUNCTION_PREVIEW_HEADER

#include "qwt_plot.h"
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include <ExprIntrp_GenExp.hxx>
#include <Expr_Array1OfNamedUnknown.hxx>
#include <TColStd_Array1OfReal.hxx>

class SMESHGUI_FunctionPreview : public QwtPlot
{
  Q_OBJECT

public:
  SMESHGUI_FunctionPreview( QWidget* );
  virtual ~SMESHGUI_FunctionPreview();

  QString   function() const;
  bool      isTableFunc() const;
  void      tableFunc( SMESH::double_array& ) const;
  void      interval( double&, double& ) const;
  int       pointsCount() const;
  bool      isDone() const;

  bool      setParams( const QString&, const double = 0.0, const double = 1.0, const int = 50, const bool = true );
  bool      setParams( const SMESH::double_array&, const double = 0.0, const double = 1.0, const bool = true );
  void      setIsExp( const bool, const bool = true );

protected:
  virtual   bool   init( const QString& );
  virtual   double funcValue( const double, bool& );
  virtual   bool   createTable( SMESH::double_array& );
  virtual   void   drawContents( QPainter* );

private:
  double calc( bool& );

private:
  QString              myFunction;
  double               myXmin, myXmax;
  int                  myPoints;
  bool                 myIsTable;
  bool                 myIsExp;
  SMESH::double_array  myTableFunc;
  long                 myCurve;
  Handle(ExprIntrp_GenExp)  myExpr;
  Expr_Array1OfNamedUnknown myVars;
  TColStd_Array1OfReal  myValues;
  bool                 myIsDone;
};

#endif
