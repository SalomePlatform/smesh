#ifndef DIALOGBOX_ADD_QUADRATIC_ELEMENT_H
#define DIALOGBOX_ADD_QUADRATIC_ELEMENT_H

#include "LightApp_SelectionMgr.h"

// QT Includes
#include <qdialog.h>
#include <qtable.h>

class QButtonGroup;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QTable;
class SMESHGUI;
class SMESH_Actor;
class SVTK_Selector;

namespace SMESH{
  struct TElementSimulation;
}

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

enum { QUAD_EDGE, QUAD_TRIANGLE, QUAD_QUADRANGLE, QUAD_TETRAHEDRON, QUAD_PYRAMID, QUAD_PENTAHEDRON, QUAD_HEXAHEDRON };

//=================================================================================
// class    : SMESHGUI_AddQuadraticElementDlg
// purpose  :
//=================================================================================
class SMESHGUI_AddQuadraticElementDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_AddQuadraticElementDlg( SMESHGUI* theModule,
				     const int theType,
				     const char* = 0, 
				     bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_AddQuadraticElementDlg();

private:
    void Init ();
    void closeEvent (QCloseEvent*);
    void hideEvent (QHideEvent*);                 /* ESC key */
    void enterEvent (QEvent*);                    /* mouse enter the QWidget */
    void displaySimulation();
    void UpdateTable( bool theConersValidity = true );
    bool IsValid();

    SMESHGUI*                   mySMESHGUI;       /* Current SMESHGUI object */
    LightApp_SelectionMgr*      mySelectionMgr;   /* User shape selection */
    int                         myNbCorners;      /* The required number of corners */
    bool                        myBusy;
    SVTK_Selector*              mySelector;

    SMESH::SMESH_Mesh_var       myMesh;
    SMESH_Actor*                myActor;
    SMESH::TElementSimulation*  mySimulation;

    int                         myType;
    bool                        myIsEditCorners;
    
    QButtonGroup* GroupConstructors;
    QRadioButton* myRadioButton1;

    QGroupBox*    GroupArguments;
    QLineEdit*    myCornerNodes;
    QPushButton*  mySelectButton;
    QTable*       myTable;
    QCheckBox*    myReverseCB;

    QGroupBox*    GroupButtons;
    QPushButton*  buttonOk;
    QPushButton*  buttonCancel;
    QPushButton*  buttonApply;
   
private slots:
  
    void onTextChange(const QString&);
    void onCellTextChange(int, int);
    void onReverse( int );
    void onCellDoubleClicked(int, int, int, const QPoint&);
  
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCorners() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
};

class SMESHGUI_IdEditItem: public QTableItem
{
public:
    SMESHGUI_IdEditItem(QTable* table, EditType et, const QString& text ):
      QTableItem(table, et, text) {};
    ~SMESHGUI_IdEditItem() {};

    QWidget* createEditor() const;
};


#endif // DIALOGBOX_ADD_QUADRATIC_ELEMENT_H
