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

// File   : StdMeshersGUI_DistrPreview.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "StdMeshersGUI_DistrPreview.h"

// Qwt includes
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>

// OCCT includes
#include <Expr_NamedUnknown.hxx>
#include <Expr_GeneralExpression.hxx>

#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
#define NO_CAS_CATCH
#endif

#include <Standard_Failure.hxx>

#ifdef NO_CAS_CATCH
#include <Standard_ErrorHandler.hxx>
#endif

#ifdef WIN32
# include <algorithm>
#endif
#include <math.h>
#include <limits>

// SALOME KERNEL includes
#include <Basics_Utils.hxx>
// SALOME GUI includes
#include <Plot2d_PlotItems.h>

Plot2d_QwtLegendLabel* getLegendLabel( QwtPlotCurve* plotCurve )
{
  const QVariant itemInfo = plotCurve->plot()->itemToInfo( plotCurve );
  QwtLegend* legend = dynamic_cast<QwtLegend*>( plotCurve->plot()->legend() );
  QWidget* widget = legend->legendWidget( itemInfo );
  QwtLegendLabel* label = dynamic_cast<QwtLegendLabel*>( widget );
  Plot2d_QwtLegendLabel* anItem = (Plot2d_QwtLegendLabel*)label;
  return anItem;
}

StdMeshersGUI_DistrPreview::StdMeshersGUI_DistrPreview( QWidget* p, StdMeshers::StdMeshers_NumberOfSegments_ptr h )
  : QwtPlot( p ),
    myPoints( 50 ),
    myNbSeg( 1 ),
    myIsTable( false ),
    myConv( CUT_NEGATIVE ),
    myVars( 1, 1 ),
    myValues( 1, 1 ),
    myIsDone( true )
{
  Kernel_Utils::Localizer loc;
  myHypo = StdMeshers::StdMeshers_NumberOfSegments::_duplicate( h );
  myVars.ChangeValue( 1 ) = new Expr_NamedUnknown( "t" );

  QwtAbstractLegend* absLegend = legend();
  QwtLegend* legend = 0;
  if ( !absLegend ) {
    legend = new Plot2d_QwtLegend( this );
    legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
  }
  else
    legend = dynamic_cast<QwtLegend*>( absLegend );
  if( legend )
    insertLegend( legend, QwtPlot::BottomLegend );

  myDensity = new QwtPlotCurve( QString() );
  myDensity->attach( this );
  QPen densityPen = QPen( Qt::red, 1 );
  myDensity->setPen( QPen( Qt::red, 1 ) );
  if( Plot2d_QwtLegendLabel* anItem = getLegendLabel( myDensity ) ) {
    anItem->setPen( densityPen );
    anItem->repaint();
  }

  myDistr = new QwtPlotCurve( QString() );
  myDistr->attach( this );
  QPen distrPen = QPen( Qt::blue, 1 );
  QwtSymbol* distrSymbol = new QwtSymbol( QwtSymbol::XCross, QBrush( Qt::blue ),
                                                  QPen( Qt::blue ), QSize( 5, 5 ) );
  myDistr->setPen( distrPen );
  myDistr->setSymbol( distrSymbol );
  if( Plot2d_QwtLegendLabel* anItem = getLegendLabel( myDistr ) ) {
    anItem->setPen( distrPen );
    anItem->setSymbol( distrSymbol );
    anItem->repaint();
  }

  myMsg = new QwtPlotMarker();
  myMsg->attach( this );
  myMsg->setValue( 0.5, 0.5 );
  QwtText mt = myMsg->label();
  mt.setBorderPen( QPen( Qt::red, 1 ) );
  QFont f = mt.font();
  f.setPointSize( 14 ); //f.setBold( true );
  mt.setFont( f );
  myMsg->setLabel( mt );

  enableAxis(QwtPlot::yLeft, false);
  enableAxis(QwtPlot::yRight, true);
  
  QFont axisFont;
  axisFont.setPointSize( 8 );
  setAxisFont(QwtPlot::yRight, axisFont); 
  setAxisFont(QwtPlot::xBottom, axisFont); 
  
  myDensity->setYAxis(QwtPlot::yRight);
  myDistr->setYAxis(QwtPlot::yRight);
  myMsg->setYAxis(QwtPlot::yRight);
  myDensity->setTitle( tr( "SMESH_DENSITY_FUNC" ) );
  myDistr->setTitle( tr( "SMESH_DISTR" ) );
  
  QwtPlotGrid* aGrid = new QwtPlotGrid();
  QPen aMajPen = aGrid->majorPen();
  aMajPen.setStyle( Qt::DashLine );
  aGrid->setPen( aMajPen );

  aGrid->enableX( true );
  aGrid->enableY( true );

  aGrid->attach( this );
}

StdMeshersGUI_DistrPreview::~StdMeshersGUI_DistrPreview()
{
}

bool StdMeshersGUI_DistrPreview::isTableFunc() const
{
  return myIsTable;
}

void StdMeshersGUI_DistrPreview::tableFunc( SMESH::double_array& f ) const
{
  f = myTableFunc;
}

QString StdMeshersGUI_DistrPreview::function() const
{
  return myFunction;
}

int StdMeshersGUI_DistrPreview::nbSeg() const
{
  return myNbSeg;
}

int StdMeshersGUI_DistrPreview::pointsCount() const
{
  return myPoints;
}

void StdMeshersGUI_DistrPreview::setConversion( Conversion conv, const bool upd )
{
  myConv = conv;
  if( upd )
    update();
}

bool StdMeshersGUI_DistrPreview::setParams( const QString& func, const int nbSeg, const int points, const bool upd )
{
  myIsTable = false;
  myTableFunc = SMESH::double_array();
  myFunction = func.isEmpty() ? "0" : func;
  myPoints = points>0 ? points : 2;
  myNbSeg = nbSeg>0 ? nbSeg : 1;
  bool res = init( func );
  if( upd )
    update();
  return res;
}

bool StdMeshersGUI_DistrPreview::setParams( const SMESH::double_array& f, const int nbSeg, const bool upd )
{
  myIsTable = true;
  myTableFunc = f;
  if( myTableFunc.length()%2==1 )
    myTableFunc.length( myTableFunc.length()-1 );

  myFunction = "0";
  myPoints = myTableFunc.length()/2;
  myNbSeg = nbSeg>0 ? nbSeg : 1;

  if( upd )
    update();

  return myTableFunc.length()>0;
}

bool StdMeshersGUI_DistrPreview::createTable( SMESH::double_array& func )
{
  if( myExpr.IsNull() )
  {
    func.length( 0 );
    return false;
  }

  const double xmin = 0.0, xmax = 1.0;

  double d = (xmax-xmin)/double(myPoints-1);
  func.length( 2*myPoints );
  int err = 0;
  for( int i=0, j=0; i<myPoints; j++ )
  {
    bool ok;
    double t = xmin + d*j, f = funcValue( t, ok );
    if( ok )
    {
      func[2*i] = t;
      func[2*i+1] = f;
      i++;
    }
    else
      err++;
  }
  func.length( func.length()-2*err );
  return err==0;
}

void StdMeshersGUI_DistrPreview::update()
{
  Kernel_Utils::Localizer loc;
  SMESH::double_array graph, distr;
  if( isTableFunc() )
  {
    myIsDone = true;
    graph = myTableFunc;
  }
  else
    myIsDone = createTable( graph );

  if( graph.length()>=2 )
  {
    StdMeshers::StdMeshers_NumberOfSegments_var h = 
      StdMeshers::StdMeshers_NumberOfSegments::_narrow( myHypo );

    if( !CORBA::is_nil( h.in() ) )
    {
      SMESH::double_array* arr = 0;
      if( isTableFunc() )
        arr = h->BuildDistributionTab( myTableFunc, myNbSeg, ( int )myConv );
      else
        arr = h->BuildDistributionExpr( myFunction.toLatin1().data(), myNbSeg, ( int )myConv );
      if( arr )
      {
        distr = *arr;
        delete arr;
      }
    }
  }

  bool correct = graph.length()>=2 && distr.length()>=2;
  if( !correct )
  {
    showError();
    return;
  }
  else
  {
    QwtText mt = myMsg->label();
    mt.setText( QString() );
    myMsg->setLabel( mt );
  }

  int size = graph.length()/2;
  double* x = new double[size], *y = new double[size];
  double min_x = 0, max_x = 0, min_y = 0, max_y = 0;
  for( int i=0; i<size; i++ )
  {
    x[i] = graph[2*i];
    y[i] = graph[2*i+1];
    if( !convert( y[i] ) )
    {
      min_x = 0.0; max_x = 1.0; min_y = 0.0; max_y = 1.0;
      delete[] x; delete[] y;
      x = y = 0;
      showError();
      return;
    }
#ifdef WIN32
    if ( std::fabs(y[i]) >= HUGE_VAL)
      y[i] = HUGE_VAL/100.;
#else
    if ( std::isinf(y[i]))
      y[i] = std::numeric_limits<double>::max()/100.;
#endif
//     if ( y[i] > 1e3 )
//       y[i] = 1e3;
    if( i==0 || y[i]<min_y )
      min_y = y[i];
    if( i==0 || y[i]>max_y )
      max_y = y[i];
    if( i==0 || x[i]<min_x )
      min_x = x[i];
    if( i==0 || x[i]>max_x )
      max_x = x[i];
  }

  setAxisScale( myDensity->xAxis(), min_x, max_x );
  setAxisScale( myDensity->yAxis(),
#ifdef WIN32
    min( 0.0, min_y ),
    max( 0.0, max_y )
#else
    std::min( 0.0, min_y ),
    std::max( 0.0, max_y )
#endif
    );
  myDensity->setSamples( x, y, size );
  if( x )
    delete[] x;
  if( y )
    delete[] y;
  x = y = 0;

  size = distr.length();
  x = new double[size];
  y = new double[size];
  for( int i=0; i<size; i++ )
  {
    x[i] = distr[i];
    y[i] = 0;
  }
  myDistr->setSamples( x, y, size );
  delete[] x;
  delete[] y;
  x = y = 0;

  try {   
#ifdef NO_CAS_CATCH
    OCC_CATCH_SIGNALS;
#endif
    replot();
  } catch(Standard_Failure) {
    Handle(Standard_Failure) aFail = Standard_Failure::Caught();
  }
}

void StdMeshersGUI_DistrPreview::showError()
{
  setAxisScale( myDensity->xAxis(), 0.0, 1.0 );
  setAxisScale( myDensity->yAxis(), 0.0, 1.0 );
  myDensity->setSamples( 0, 0, 0 );
  myDistr->setSamples( 0, 0, 0 );
  QwtText mt = myMsg->label();
  mt.setText( tr( "SMESH_INVALID_FUNCTION" ) );
  myMsg->setLabel( mt );
  replot();
}

bool isCorrectArg( const Handle( Expr_GeneralExpression )& expr )
{
  Handle( Expr_NamedUnknown ) sub = Handle( Expr_NamedUnknown )::DownCast( expr );
  if( !sub.IsNull() )
    return sub->GetName()=="t";

  bool res = true;
  for( int i=1, n=expr->NbSubExpressions(); i<=n && res; i++ )
  {
    Handle( Expr_GeneralExpression ) sub = expr->SubExpression( i );
    Handle( Expr_NamedUnknown ) name = Handle( Expr_NamedUnknown )::DownCast( sub );
    if( !name.IsNull() )
    {
      if( name->GetName()!="t" )
        res = false;
    }
    else
      res = isCorrectArg( sub );
  }
  return res;
}

bool StdMeshersGUI_DistrPreview::init( const QString& str )
{
  Kernel_Utils::Localizer loc;
  bool parsed_ok = true;
  try {
#ifdef NO_CAS_CATCH
    OCC_CATCH_SIGNALS;
#endif
    myExpr = ExprIntrp_GenExp::Create();
    myExpr->Process( ( Standard_CString ) str.toLatin1().data() );
  } catch(Standard_Failure) {
    Handle(Standard_Failure) aFail = Standard_Failure::Caught();
    parsed_ok = false;
  }

  bool syntax = false, args = false;
  if( parsed_ok && myExpr->IsDone() )
  {
    syntax = true;
    args = isCorrectArg( myExpr->Expression() );
  }

  bool res = parsed_ok && syntax && args;
  if( !res )
    myExpr.Nullify();
  return res;
}

double StdMeshersGUI_DistrPreview::funcValue( const double t, bool& ok )
{
  if( myExpr.IsNull() )
    return 0;

  myValues.ChangeValue( 1 ) = t;

  ok = true;
  double res = calc( ok );

  return res;
}

double StdMeshersGUI_DistrPreview::calc( bool& ok )
{
  double res = 0.0;

  ok = true;
  try {   
#ifdef NO_CAS_CATCH
    OCC_CATCH_SIGNALS;
#endif
    res = myExpr->Expression()->Evaluate( myVars, myValues );
  } catch(Standard_Failure) {
    Handle(Standard_Failure) aFail = Standard_Failure::Caught();
    ok = false;
    res = 0.0;
  }

  return res;
}

bool StdMeshersGUI_DistrPreview::isDone() const
{
  return myIsDone;
}

bool StdMeshersGUI_DistrPreview::convert( double& v ) const
{
  bool ok = true;
  switch( myConv )
  {
  case EXPONENT:
    {
      try { 
#ifdef NO_CAS_CATCH
        OCC_CATCH_SIGNALS;
#endif
        // in StdMeshers_NumberOfSegments.cc
        // const double PRECISION = 1e-7;
        //
        if(v < -7) v = -7.0;
        v = pow( 10.0, v );
      } catch(Standard_Failure) {
        Handle(Standard_Failure) aFail = Standard_Failure::Caught();
        v = 0.0;
        ok = false;
      }
    }
    break;

  case CUT_NEGATIVE:
    if( v<0 )
      v = 0;
    break;
  }

  return ok;
}
