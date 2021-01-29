// Copyright (C) 2011-2020  CEA/DEN, EDF R&D
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

// SMESH SMESHGUI : GUI for the adaptation in the SMESH component
// File   : MG_ADAPTGUI.hxx
//
#ifndef MG_ADAPTGUI_HXX
#define MG_ADAPTGUI_HXX

#include <set>
// SMESH includes

// Qt includes
#include <QDialog>
#include <QTreeWidget>
#include<QItemDelegate>

#include "LightApp_DataOwner.h"
#include "SalomeApp_Application.h"
#include <SALOMEconfig.h>
#include <SALOME_ListIO.hxx>
#include "SalomeApp_Module.h"
#include "SalomeApp_Study.h"
#include <med.h>
#include <map>


// model


//~#include "MG_ADAPT.hxx"

#include CORBA_SERVER_HEADER(MG_ADAPT)

class SUIT_ViewWindow;
class SUIT_Desktop;
class SUIT_Study;
class SUIT_ResourceMgr;

class CAM_Module;

class SALOMEDSClient_Study;
class SALOMEDSClient_SObject;

class SalomeApp_Study;
class SalomeApp_Module;
class LightApp_SelectionMgr;


class QButtonGroup;
class QLineEdit;
class QGroupBox;
class QRadioButton;
class QLabel;
class QCheckBox;
class QGridLayout;
class QTabWidget;
class QDoubleSpinBox;
class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;
class QSpacerItem;
class QHBoxLayout;
class QItemDelegate;
class QComboBox;

// IDL includes
#include <SALOMEconfig.h>

class SVTK_ViewWindow;
class SVTK_Selector;
class SMESHGUI_MgAdaptDlg;
class SMESHGUI_MgAdaptArguments;
class SMESHGUI_SpinBox;
class MgAdaptAdvWidgetTreeWidget;
class MgAdaptAdvWidget;
//~class MgAdapt;
class QHeaderView;
class QFileDialog;


std::map<QString, int> GetListeChamps(QString aFile, bool errorMessage = true);
QString lireNomMaillage(QString aFile, med_int& meshDim);
QString lireNomMaillage2(med_idt medIdt,int meshId, med_int& meshDim);
med_idt OuvrirFichier(QString aFile);
std::string remove_extension(const std::string& filename);


enum ADAPTATION_MODE{
	SURFACE, // surface adaption when meshDim == 2
	VOLUME, //
	BOTH
};
//=================================================================================
// class    : SMESHGUI_MgAdaptDlg
// purpose  :
//=================================================================================
class  SMESHGUI_MgAdaptDlg : public QDialog
{
    Q_OBJECT;
public:
    //! Property type
    enum Mode { Arguments, AdvancedOptions};
    SMESHGUI_MgAdaptDlg( SalomeApp_Module*, SMESH::MG_ADAPT_ptr, QWidget* parent= 0,bool isCreation = true );
    ~SMESHGUI_MgAdaptDlg();

    void  buildDlg();
    void reject();
    bool checkParams(QString& msg) ;
    //~void setModel(MgAdapt*);
    SMESH::MG_ADAPT_ptr getModel() const;

public slots:

protected slots:

    virtual bool clickOnApply();
private slots:
    virtual void clickOnHelp();
    virtual void clickOnOk();
protected :

    SMESHGUI_MgAdaptArguments* myArgs;
    MgAdaptAdvWidget* myAdvOpt;
    bool                readParamsFromHypo( ) const ;
    bool                readParamsFromWidgets( ) ;
    bool                storeParamsToHypo( const SMESH::MgAdaptHypothesisData & ) const;

private:


    SalomeApp_Module*              mySMESHGUI;              /* Current SMESHGUI object */
    QTabWidget*           myTabWidget;


    SMESH::MgAdaptHypothesisData* myData;
    SMESH::MG_ADAPT_ptr model;

};

class  SMESHGUI_MgAdaptArguments : public QWidget
{
    Q_OBJECT;
public:
    //! Property type
    enum Mode { Mesh, Browser};
    enum SIZEMAP { Local, Background, Constant};
    SMESHGUI_MgAdaptArguments( QWidget* parent);
    ~SMESHGUI_MgAdaptArguments();
    void setMode( const Mode, const SIZEMAP );
    med_int getMeshDim() const;

    QString* myFileInDir;
    QString* myFileOutDir;
    QString* myFileSizeMapDir;
    QGroupBox*    aMeshIn ;
    QRadioButton* aMedfile;
    QRadioButton* aBrowser ;
    QLineEdit* aBrowserObject;
    QPushButton* selectMedFilebutton ;
    QSpacerItem* hspacer;
    QLineEdit* selectMedFileLineEdit ;
    QButtonGroup* meshInGroup ;
    QGridLayout* meshIn ;

    QGroupBox*    aMeshOut ;
    QLabel* meshName;
    QLineEdit* meshNameLineEdit;
    QSpacerItem* secondHspacer;
    QCheckBox* medFileCheckBox;
    QPushButton* selectOutMedFilebutton;
    QLineEdit* selectOutMedFileLineEdit;
    QSpacerItem* thirdHspacer;
    QCheckBox* publishOut;
    QGridLayout* meshOut ;

    QGroupBox*    sizeMapDefinition ;
    QRadioButton* localButton;
    QRadioButton* backgroundButton ;
    QRadioButton* constantButton ;
    QLabel* medFileBackground;
    QPushButton* selectMedFileBackgroundbutton;
    QLineEdit* selectMedFileBackgroundLineEdit;
    QLabel* valueLabel;
    QDoubleSpinBox* dvalue;
    QButtonGroup* sizeMapDefGroup ;
    QGridLayout* sizeMapDefGroupLayout;


    QGroupBox* sizeMapField;
    QLabel* fieldName;
    QComboBox* fieldNameCmb;
    QRadioButton* noTimeStep;
    QRadioButton* lastTimeStep ;
    QRadioButton* chosenTimeStep;
    QLabel* timeStepLabel;
    QSpinBox* timeStep;
    QLabel* rankLabel;
    QSpinBox* rankSpinBox;
    QButtonGroup* timeStepGroup;
    QGridLayout* sizeMapFieldGroupLayout;

signals:
    void updateSelection();
    void toExportMED(const char *);
    void meshDimSignal(ADAPTATION_MODE aMode);
public slots:

protected slots:

private slots:
    void modeChanged( int);
    void sizeMapDefChanged(int);
    void timeStepGroupChanged(int timeStepType, bool disableOther = false, int max = 0);
    void onSelectMedFilebuttonClicked();
    void clear();
    void onMedFileCheckBox(int);
    void onPublishOut(int);
    void onSelectOutMedFilebutton();
    void onSelectMedFileBackgroundbutton();
    void onLocalSelected(QString);
    void onNoTimeStep(bool disableOther = false);
    void onLastTimeStep(bool disableOther = false);
    void onChosenTimeStep(bool disableOther = false, int max = 0);
    void visibleTimeStepRankLabel(bool visible);

private:

    QString getMedFileName(bool avertir);
    LightApp_SelectionMgr* selMgr ;
	med_int meshDim;
    std::map<QString, int> myFieldList;


};
enum {
    OPTION_ID_COLUMN = 0,
    OPTION_TYPE_COLUMN,
    OPTION_NAME_COLUMN = 0,
    OPTION_VALUE_COLUMN,
    NB_COLUMNS,
};

//////////////////////////////////////////
// MgAdaptAdvWidget
//////////////////////////////////////////
class  MgAdaptAdvWidget : public QWidget
{
    Q_OBJECT

public:
    MgAdaptAdvWidget( QWidget* = 0, std::vector <std::string> * = nullptr, Qt::WindowFlags = 0 );
    ~MgAdaptAdvWidget();
    std::vector < std::string > * myOptions;
    QGridLayout *gridLayout_4;
    MgAdaptAdvWidgetTreeWidget *myOptionTable;
    QPushButton *addBtn;
    QSpacerItem *horizontalSpacer;
    QGroupBox *logGroupBox;
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QLabel *workingDirectoryLabel;
    QLineEdit *workingDirectoryLineEdit;
    QPushButton *workingDirectoryPushButton;
    QLabel *verboseLevelLabel;
    QSpinBox *verboseLevelSpin;
    QHBoxLayout *horizontalLayout;
    QCheckBox *logInFileCheck;
    QCheckBox *removeLogOnSuccessCheck;
    QCheckBox *keepWorkingFilesCheck;

    void AddOption( const char* name_value_type, bool isCustom = false );
    void GetOptionAndValue( QTreeWidgetItem * tblRow, QString& option, QString& value, bool& dflt );
    void setupWidget();
public slots:

    void                onAddOption();
    void itemChanged(QTreeWidgetItem * tblRow, int column);
    void onMeshDimChanged(ADAPTATION_MODE aMode);
private slots:
    void _onWorkingDirectoryPushButton();
private:
    void setOptionValue(QString& option, QString& value);
    std::map<QString, QTreeWidgetItem *> optionTreeWidgetItem;

    QTreeWidgetItem* getNewQTreeWidgetItem(QTreeWidget* table, const char* option, QString& name, bool isCustom);

};

enum { EDITABLE_ROLE = Qt::UserRole + 1, PARAM_NAME,
       NAME_COL = 0, VALUE_COL
     };



class  ItemDelegate: public QItemDelegate
{
public:

    ItemDelegate(QObject* parent=0): QItemDelegate(parent) {}
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &o, const QModelIndex &index) const;
};

class  MgAdaptAdvWidgetTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    MgAdaptAdvWidgetTreeWidget( QWidget* );

protected:
    QModelIndex moveCursor( CursorAction, Qt::KeyboardModifiers );
    void keyPressEvent( QKeyEvent* );
};


#endif // MG_ADAPTGUI_HXX
