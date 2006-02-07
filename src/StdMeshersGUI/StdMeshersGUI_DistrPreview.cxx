
#include "StdMeshersGUI_DistrPreview.h"
#include <Expr_NamedUnknown.hxx>
#include <Expr_GeneralExpression.hxx>
#include <CASCatch_CatchSignals.hxx>
#include <CASCatch_Failure.hxx> 
#include <CASCatch_ErrorHandler.hxx>
#include <OSD.hxx>

StdMeshersGUI_DistrPreview::StdMeshersGUI_DistrPreview( QWidget* p, StdMeshers::StdMeshers_NumberOfSegments_ptr h )
: QwtPlot( p ),
  myPoints( 50 ),
  myIsTable( false ),
  myVars( 1, 1 ),
  myValues( 1, 1 ),
  myConv( CUT_NEGATIVE ),
  myIsDone( true ),
  myNbSeg( 1 )
{
  myHypo = StdMeshers::StdMeshers_NumberOfSegments::_duplicate( h );
  myVars.ChangeValue( 1 ) = new Expr_NamedUnknown( "t" );
  myDensity = insertCurve( QString() );
  myDistr = insertCurve( QString() );
  myMsg = insertMarker( new QwtPlotMarker( this ) );
  setMarkerPos( myMsg, 0.5, 0.5 );
  setMarkerLabelPen( myMsg, QPen( Qt::red, 1 ) );
  QFont f = markerFont( myMsg );
  f.setPointSize( 14 );
  f.setBold( true );
  setMarkerFont( myMsg, f );
  setCurvePen( myDensity, QPen( Qt::red, 1 ) );

  QColor dc = Qt::blue;
  setCurvePen( myDistr, QPen( dc, 1 ) );
  setCurveSymbol( myDistr, QwtSymbol( QwtSymbol::XCross, QBrush( dc ), QPen( dc ), QSize( 5, 5 ) ) );
  setAutoLegend( true );
  enableLegend( true );
  setLegendPos( Qwt::Bottom );
  setCurveTitle( myDensity, tr( "SMESH_DENSITY_FUNC" ) );
  setCurveTitle( myDistr, tr( "SMESH_DISTR" ) );
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
	arr = h->BuildDistributionExpr( myFunction.latin1(), myNbSeg, ( int )myConv );
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
    setMarkerLabel( myMsg, QString() );

  int size = graph.length()/2;
  double* x = new double[size], *y = new double[size];
  double min_x, max_x, min_y, max_y;
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
    if( i==0 || y[i]<min_y )
      min_y = y[i];
    if( i==0 || y[i]>max_y )
      max_y = y[i];
    if( i==0 || x[i]<min_x )
      min_x = x[i];
    if( i==0 || x[i]>max_x )
      max_x = x[i];
  }

  setAxisScale( curveXAxis( myDensity ), min_x, max_x );
  setAxisScale( curveYAxis( myDensity ), min( 0.0, min_y ), max( 0.0, max_y ) );
  setCurveData( myDensity, x, y, size );
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
  setCurveData( myDistr, x, y, size );
  delete[] x;
  delete[] y;
  x = y = 0;

  OSD::SetSignal( true );
  CASCatch_CatchSignals aCatchSignals;
  aCatchSignals.Activate();

  CASCatch_TRY
  {   
    replot();
  }
  CASCatch_CATCH(CASCatch_Failure)
  {
    aCatchSignals.Deactivate();
    Handle(CASCatch_Failure) aFail = CASCatch_Failure::Caught();
  }
  aCatchSignals.Deactivate();
}

void StdMeshersGUI_DistrPreview::showError()
{
  setAxisScale( curveXAxis( myDensity ), 0.0, 1.0 );
  setAxisScale( curveYAxis( myDensity ), 0.0, 1.0 );
  setCurveData( myDensity, 0, 0, 0 );
  setCurveData( myDistr, 0, 0, 0 );
  setMarkerLabel( myMsg, tr( "SMESH_INVALID_FUNCTION" ) );
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
  CASCatch_CatchSignals aCatchSignals;
  aCatchSignals.Activate();

  bool parsed_ok = true;
  CASCatch_TRY
  {
    myExpr = ExprIntrp_GenExp::Create();
    myExpr->Process( ( Standard_CString ) str.latin1() );
  }
  CASCatch_CATCH(CASCatch_Failure)
  {
    aCatchSignals.Deactivate();
    Handle(CASCatch_Failure) aFail = CASCatch_Failure::Caught();
    parsed_ok = false;
  }
  aCatchSignals.Deactivate();

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
  OSD::SetSignal( true );
  double res = 0.0;

  CASCatch_CatchSignals aCatchSignals;
  aCatchSignals.Activate();

  ok = true;
  CASCatch_TRY {   
    res = myExpr->Expression()->Evaluate( myVars, myValues );
  }
  CASCatch_CATCH(CASCatch_Failure) {
    aCatchSignals.Deactivate();
    Handle(CASCatch_Failure) aFail = CASCatch_Failure::Caught();
    ok = false;
    res = 0.0;
  }
  aCatchSignals.Deactivate();
  return res;
}

bool StdMeshersGUI_DistrPreview::isDone() const
{
  return myIsDone;
}

bool StdMeshersGUI_DistrPreview::convert( double& v ) const
{
  CASCatch_CatchSignals aCatchSignals;
  aCatchSignals.Activate();

  bool ok = true;
  switch( myConv )
  {
  case EXPONENT:
    {
      CASCatch_TRY
      { 
	v = pow( 10.0, v );
      }
      CASCatch_CATCH(CASCatch_Failure)
      {
	aCatchSignals.Deactivate();
	Handle(CASCatch_Failure) aFail = CASCatch_Failure::Caught();
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
  aCatchSignals.Deactivate();
  return ok;
}
