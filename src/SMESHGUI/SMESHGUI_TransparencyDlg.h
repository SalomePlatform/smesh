//  File      : SMESHGUI_TransparencyDlg.h
//  Created   : Thu Jun 06 16:41:57 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$


#ifndef DIALOGBOX_TRANSPARENCYDLG_H
#define DIALOGBOX_TRANSPARENCYDLG_H

#include "SALOME_Selection.h"
#include "SALOME_InteractiveObject.hxx"

// QT Includes
#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QFrame;
class QLabel;
class QPushButton;
class QSlider;
class SMESHGUI;

//=================================================================================
// class    : SMESHGUI_TransparencyDlg
// purpose  :
//=================================================================================
class SMESHGUI_TransparencyDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_TransparencyDlg( QWidget* parent = 0,
				 const char* name = 0,
				 SALOME_Selection* Sel = 0,
				 bool modal = TRUE,
				 WFlags fl = 0 );

    ~SMESHGUI_TransparencyDlg();

private :

    SMESHGUI*                        mySMESHGUI ;     /* Current GeomGUI object     */
    bool                             myFirstInit ;    /* Inform for the first init  */
    SALOME_Selection*                mySel;           /* User selection             */
    
    QPushButton* buttonOk;
    QLabel* TextLabelOpaque;
    QLabel* TextLabelTransparent;
    QSlider* Slider1;

public slots:
      
    void ClickOnOk();
    void ClickOnClose();
    void ValueHasChanged( int newValue ) ;
    
protected:
    QGridLayout* SMESHGUI_TransparencyDlgLayout;
    QHBoxLayout* Layout1;
    QHBoxLayout* Layout2;
};

#endif // DIALOGBOX_TRANSPARENCYDLG_H

