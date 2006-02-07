
#ifndef NB_SEGMENTS_CREATOR_HEADER
#define NB_SEGMENTS_CREATOR_HEADER

#include "StdMeshersGUI_StdHypothesisCreator.h"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QtxIntSpinBox;
class QtxComboBox;
class SMESHGUI_SpinBox;
class StdMeshersGUI_DistrTableFrame;
class StdMeshersGUI_DistrPreview;
class QLineEdit;
class QButtonGroup;
class QGridLayout;
class QRadioButton;

typedef struct
{
  int                 myNbSeg, myDistrType, myConv;
  double              myScale;
  SMESH::double_array myTable;
  QString             myName, myExpr;

} NbSegmentsHypothesisData;

class StdMeshersGUI_NbSegmentsCreator : public StdMeshersGUI_StdHypothesisCreator
{
  Q_OBJECT

public:
  StdMeshersGUI_NbSegmentsCreator();
  virtual ~StdMeshersGUI_NbSegmentsCreator();

  virtual bool checkParams() const;

protected:
  virtual QFrame*  buildFrame();
  virtual void     retrieveParams() const;
  virtual QString  storeParams() const;

protected slots:
  virtual void     onValueChanged();

private:
  bool readParamsFromHypo( NbSegmentsHypothesisData& ) const;
  bool readParamsFromWidgets( NbSegmentsHypothesisData& ) const;
  bool storeParamsToHypo( const NbSegmentsHypothesisData& ) const;

private:
  QtxIntSpinBox*   myNbSeg;
  QtxComboBox*     myDistr;
  SMESHGUI_SpinBox*   myScale;
  StdMeshersGUI_DistrTableFrame*  myTable;
  StdMeshersGUI_DistrPreview* myPreview;
  QLineEdit       *myName, *myExpr;
  QButtonGroup*    myConv;
  QLabel          *myLScale, *myLTable, *myLExpr, *myLConv, *myInfo;
  QGridLayout*     myGroupLayout;
  int              myTableRow, myPreviewRow;
  QRadioButton*    myCutNeg;
};

#endif
