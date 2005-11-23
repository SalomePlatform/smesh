
#include "SMESHGUI_FunctionPreview.h"
#include <Expr_NamedUnknown.hxx>
#include <Expr_GeneralExpression.hxx>
#include <CASCatch_CatchSignals.hxx>
#include <CASCatch_Failure.hxx> 
#include <CASCatch_ErrorHandler.hxx>
#include <OSD.hxx>

SMESHGUI_FunctionPreview::SMESHGUI_FunctionPreview( QWidget* p )
: QwtPlot( p ),
  myXmin( 0.0 ),
  myXmax( 1.0 ),
  myPoints( 50 ),
  myIsTable( false ),
  myVars( 1, 1 ),
  myValues( 1, 1 ),
  myIsExp( false ),
  myIsDone( true )
{
  myVars.ChangeValue( 1 ) = new Expr_NamedUnknown( "t" );
  myCurve = insertCurve( QString() );
  setCurvePen( myCurve, QPen( Qt::red, 1 ) );
}

SMESHGUI_FunctionPreview::~SMESHGUI_FunctionPreview()
{
}

bool SMESHGUI_FunctionPreview::isTableFunc() const
{
  return myIsTable;
}

void SMESHGUI_FunctionPreview::tableFunc( SMESH::double_array& f ) const
{
  f = myTableFunc;
}

QString SMESHGUI_FunctionPreview::function() const
{
  return myFunction;
}

void SMESHGUI_FunctionPreview::interval( double& xmin, double& xmax ) const
{
  xmin = myXmin;
  xmax = myXmax;
}

int SMESHGUI_FunctionPreview::pointsCount() const
{
  return myPoints;
}

void SMESHGUI_FunctionPreview::setIsExp( const bool exp, const bool update )
{
  myIsExp = exp;
  if( update )
    repaint();
}

bool SMESHGUI_FunctionPreview::setParams( const QString& func,
					       const double xmin, const double xmax,
					       const int points, const bool update )
{
  myIsTable = false;
  myTableFunc = SMESH::double_array();
  myFunction = func.isEmpty() ? "0" : func;
  myXmin = xmin;
  myXmax = xmax<myXmin ? myXmax : xmax;
  myPoints = points>0 ? points : 2;
  bool res = init( func );
  if( update )
    repaint();
  return res;
}

bool SMESHGUI_FunctionPreview::setParams( const SMESH::double_array& f, const double xmin, const double xmax,
					       const bool update )
{
  myIsTable = true;
  myTableFunc = f;
  if( myTableFunc.length()%2==1 )
    myTableFunc.length( myTableFunc.length()-1 );

  myFunction = "0";
  myXmin = xmin;
  myXmax = xmax<myXmin ? myXmax : xmax;
  myPoints = myTableFunc.length()/2;

  if( update )
    repaint();

  return myTableFunc.length()>0;
}

bool SMESHGUI_FunctionPreview::createTable( SMESH::double_array& func )
{
  if( myExpr.IsNull() )
  {
    func.length( 0 );
    return false;
  }

  double d = (myXmax-myXmin)/double(myPoints-1);
  func.length( 2*myPoints );
  int err = 0;
  for( int i=0, j=0; i<myPoints; j++ )
  {
    bool ok;
    double t = myXmin + d*j, f = funcValue( t, ok );
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

void SMESHGUI_FunctionPreview::drawContents( QPainter* p )
{
  SMESH::double_array f;
  if( isTableFunc() )
  {
    myIsDone = true;
    f = myTableFunc;
  }
  else
    myIsDone = createTable( f );

  int size = f.length()/2;
  if( size==0 )
  {
    setAxisScale( curveXAxis( myCurve ), 0.0, 0.0 );
    setAxisScale( curveYAxis( myCurve ), 0.0, 0.0 );
    setCurveData( myCurve, 0, 0, 0 );
    replot();
    QwtPlot::drawContents( p );
    return;
  }

  double* x = new double[size], *y = new double[size];
  double min_x, max_x, min_y, max_y;
  for( int i=0; i<size; i++ )
  {
    x[i] = f[2*i];
    y[i] = myIsExp ? pow( 10.0, f[2*i+1] ) : f[2*i+1];
    if( i==0 || y[i]<min_y )
      min_y = y[i];
    if( i==0 || y[i]>max_y )
      max_y = y[i];
    if( i==0 || x[i]<min_x )
      min_x = x[i];
    if( i==0 || x[i]>max_x )
      max_x = x[i];
  }

  setAxisScale( curveXAxis( myCurve ), min_x, max_x );
  setAxisScale( curveYAxis( myCurve ), min_y, max_y );
  setCurveData( myCurve, x, y, size );
  delete[] x;
  delete[] y;

  replot();
  QwtPlot::drawContents( p );
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

bool SMESHGUI_FunctionPreview::init( const QString& str )
{
  myExpr = ExprIntrp_GenExp::Create();
  myExpr->Process( ( Standard_CString ) str.latin1() );

  bool syntax = false, args = false;
  if( myExpr->IsDone() )
  {
    syntax = true;
    args = isCorrectArg( myExpr->Expression() );
  }

  bool res = syntax && args;
  if( !res )
    myExpr.Nullify();
  return res;
}

double SMESHGUI_FunctionPreview::funcValue( const double t, bool& ok )
{
  if( myExpr.IsNull() )
    return 0;

  myValues.ChangeValue( 1 ) = t;

  ok = true;
  double res = calc( ok );

  return res;
}

double SMESHGUI_FunctionPreview::calc( bool& ok )
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

bool SMESHGUI_FunctionPreview::isDone() const
{
  return myIsDone;
}
