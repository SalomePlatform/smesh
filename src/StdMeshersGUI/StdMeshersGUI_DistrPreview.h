
#ifndef STD_MESHERS_GUI_DISTR_PREVIEW_HEADER
#define STD_MESHERS_GUI_DISTR_PREVIEW_HEADER

#include <qwt_plot.h>
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)
#include <ExprIntrp_GenExp.hxx>
#include <Expr_Array1OfNamedUnknown.hxx>
#include <TColStd_Array1OfReal.hxx>

class StdMeshersGUI_DistrPreview : public QwtPlot
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
  long                      myCurve1, myCurve2, myMsg;
  Handle(ExprIntrp_GenExp)  myExpr;
  Expr_Array1OfNamedUnknown myVars;
  TColStd_Array1OfReal      myValues;
  bool                      myIsDone;
  StdMeshers::StdMeshers_NumberOfSegments_var  myHypo;
};

#endif
