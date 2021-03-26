// Copyright (C) 2020-2021  CEA/DEN, EDF R&D
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

#include "SMESHGUI_MgAdaptDlg.h"

#include "MED_Factory.hxx"

#include <SalomeApp_Tools.h>
#include <SalomeApp_Module.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_FileDlg.h>

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QString>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

const int SPACING = 6;            // layout spacing
const int MARGIN  = 9;            // layout margin

namespace
{

  // ======================================================
  QString lireNomDimMaillage(QString aFile, int& meshdim)
  // ========================================================
  {
    QString nomMaillage = QString::null ;

    try {
      while ( true )
      {

        MED::PWrapper aMed = MED::CrWrapperR( aFile.toUtf8().data() );
        MED::TInt numberOfMeshes = aMed->GetNbMeshes();

        if (numberOfMeshes == 0 )
        {
          QMessageBox::critical( 0, QObject::tr("MG_ADAPT_ERROR"),
                                 QObject::tr("MG_ADAPT_MED_FILE_2") );
          break ;
        }
        if (numberOfMeshes > 1 )
        {
          QMessageBox::critical( 0, QObject::tr("MG_ADAPT_ERROR"),
                                 QObject::tr("MG_ADAPT_MED_FILE_3") );
          break ;
        }

        MED::PMeshInfo aMeshInfo = aMed->GetPMeshInfo( 1 );
        nomMaillage = aMeshInfo->GetName().c_str();
        meshdim = (int) aMeshInfo->GetDim();

        break ;
      }
    }
    catch ( const SALOME::SALOME_Exception & S_ex )
    {
      SalomeApp_Tools::QtCatchCorbaException(S_ex);
    }

    return nomMaillage;
  }

  // =======================================================================
  std::map<QString, int> GetListeChamps(QString aFile, bool errorMessage=true)
  // =======================================================================
  {
    // Il faut voir si plusieurs maillages

    std::map<QString, int> ListeChamp ;

    try
    {
      while ( true )
      {
        MED::PWrapper aMed = MED::CrWrapperR( aFile.toUtf8().data() );
        MED::TInt jaux = aMed->GetNbFields();
        if (jaux < 1 )
        {
          if(errorMessage)
          {
            QMessageBox::critical( 0, QObject::tr("_ERROR"),
                                   QObject::tr("HOM_MED_FILE_5") );
          }
          break ;
        }
        // nbofcstp inutile pour le moment
        MED::PMeshInfo aMeshInfo = aMed->GetPMeshInfo( 1 );
        int nbofcstp = 1;
        for( MED::TInt j=0;j<jaux;j++)
        {
          MED::PFieldInfo aFiledInfo = aMed->GetPFieldInfo( aMeshInfo, j + 1 );
          ListeChamp.insert({ QString( aFiledInfo->GetName().c_str()), nbofcstp });
        }
        break ;
      }
    }
    catch ( const SALOME::SALOME_Exception & S_ex )
    {
      SalomeApp_Tools::QtCatchCorbaException(S_ex);
    }

    return ListeChamp;
  }

  // =======================================================================
  std::string remove_extension(const std::string& filename)
  // =======================================================================
  {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
  }
}

//=================================================================================
// function : SMESHGUI_MgAdaptDlg()
// purpose  :
//=================================================================================
SMESHGUI_MgAdaptDlg::SMESHGUI_MgAdaptDlg( SalomeApp_Module* theModule, SMESH::MG_ADAPT_ptr myModel, QWidget* parent, bool isCreation )
  : QDialog(parent), mySMESHGUI( theModule )
{
  //~model = new MgAdapt(*myModel);
  model = SMESH::MG_ADAPT::_duplicate(myModel);
  model->Register();
  myData = model->getData();
  buildDlg();
  if (!isCreation) readParamsFromHypo();
}

void SMESHGUI_MgAdaptDlg::buildDlg()
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "ADAPT_PREF_MG_ADAPT" ) );
  setSizeGripEnabled( true );

  myTabWidget = new QTabWidget( this );

  // Arguments

  myArgs = new SMESHGUI_MgAdaptArguments( myTabWidget );
  SMESH::string_array_var str = model->getOptionValuesStrVec();
  SMESH::string_array_var str2 = model->getCustomOptionValuesStrVec();
  std::vector<std::string> s;
  for (CORBA::ULong i = 0; i< str->length(); i++) s.push_back( str[i].in());
  for (CORBA::ULong j = str->length(); j< str2->length(); j++) s.push_back(str[ j - str->length() ].in() );
  //~str.insert( str.end(), str2.begin(), str2.end() );

  myAdvOpt = new MgAdaptAdvWidget(myTabWidget, &s);

  /*int argsTab =*/ myTabWidget->addTab( myArgs, tr( "Args" ) );
  /*int advTab  =*/ myTabWidget->addTab( myAdvOpt, tr( "ADVOP" ) );

  myAdvOpt->workingDirectoryLabel         ->setText (tr( "WORKING_DIR" ));
  myAdvOpt->workingDirectoryPushButton    ->setText (tr( "SELECT_DIR" ));
  myAdvOpt->keepWorkingFilesCheck         ->setText (tr( "KEEP_WORKING_FILES" ));
  myAdvOpt->verboseLevelLabel             ->setText (tr( "VERBOSE_LEVEL" ));
  myAdvOpt->removeLogOnSuccessCheck       ->setText (tr( "REMOVE_LOG_ON_SUCCESS" ));
  myAdvOpt->logInFileCheck                ->setText (tr( "LOG_IN_FILE" ));
  myAdvOpt->logGroupBox                   ->setTitle(tr( "LOG_GROUP_TITLE" ));

  // buttons
  QPushButton* buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), this);
  buttonOk->setAutoDefault(false);
  QPushButton* buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), this);
  buttonApply->setAutoDefault(false);
  QPushButton* buttonCancel = new QPushButton( tr( "SMESH_BUT_CANCEL" ), this );
  buttonCancel->setAutoDefault( false );
  QPushButton* buttonHelp = new QPushButton( tr( "SMESH_BUT_HELP" ), this );
  buttonHelp->setAutoDefault( false );

  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->setSpacing( SPACING );
  btnLayout->setMargin( 0 );
  btnLayout->addWidget( buttonOk );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( buttonApply );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( buttonCancel );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( buttonHelp );

  QVBoxLayout* l = new QVBoxLayout ( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );
  l->addWidget( myTabWidget );
  l->addStretch();
  l->addLayout( btnLayout );

  connect( buttonOk,       SIGNAL(pressed()), this, SLOT(PushOnOK()));
  connect( buttonApply,    SIGNAL(pressed()), this, SLOT(PushOnApply()));
  connect( buttonCancel,   SIGNAL(pressed()), this, SLOT(close()));
  connect( buttonHelp,     SIGNAL(pressed()), this, SLOT(PushOnHelp()));

  connect( myArgs, SIGNAL(meshDimSignal(ADAPTATION_MODE)), myAdvOpt, SLOT( onMeshDimChanged(ADAPTATION_MODE))  );
}


//=================================================================================
// function : ~SMESHGUI_MgAdaptDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_MgAdaptDlg::~SMESHGUI_MgAdaptDlg()
{
    //~delete model;
}

//~void SMESHGUI_MgAdaptDlg::setModel(MgAdapt* mg)
//~{
    //~model = mg;
//~}
SMESH::MG_ADAPT_ptr SMESHGUI_MgAdaptDlg::getModel() const
{
  return model;
}
/*!
\brief Perform clean-up actions on the dialog box closing.
*/
bool SMESHGUI_MgAdaptDlg::PushOnApply()
{
  bool ret = readParamsFromWidgets();
  return ret;
}
void SMESHGUI_MgAdaptDlg::PushOnOK()
{
  bool ret = PushOnApply();
  if ( ret ) reject();
}
void SMESHGUI_MgAdaptDlg::reject()
{
  QDialog::reject();
}
bool SMESHGUI_MgAdaptDlg::readParamsFromHypo( ) const
{
  bool ret = true;
  myArgs->aMedfile->setChecked(myData->fromMedFile) ;
  if (myData->fromMedFile)
  {

    myArgs->myFileInDir = myData->myFileInDir;
    myArgs->selectMedFileLineEdit->setText(QString(myData->myMeshFileIn)) ;
    // myData->myInMeshName = // TODO

  }
  else
  {
    myArgs->aBrowserObject->setText(QString(myData->myInMeshName));
    //~ myArgs->myFileInDir =""; // TODO
    //~ myArgs->selectMedFileLineEdit->setText(); // TODO
  }
  myArgs->meshNameLineEdit->setText(QString(myData->myOutMeshName));
  myArgs->medFileCheckBox->setChecked(myData->myMeshOutMed);

  if(myData->myMeshOutMed)
  {
    myArgs->myFileOutDir = QString(myData->myFileOutDir);
    myArgs->selectOutMedFileLineEdit->setText(myData->myMeshFileOut.in());
  }
  else
  {
    myArgs->myFileOutDir = ""; //TODO
  }

  myArgs->publishOut->setChecked(myData->myPublish);

  myArgs->localButton->setChecked(myData->myUseLocalMap);
  myArgs->backgroundButton->setChecked(myData->myUseBackgroundMap);
  myArgs->constantButton->setChecked(myData->myUseConstantValue);

  if (myData->myUseConstantValue)
  {
    myArgs->dvalue->setValue(myData->myConstantValue);
  }
  else
  {
    myArgs->dvalue->setValue(0.0);
  }

  if (myData->myUseBackgroundMap)
  {

    myArgs->myFileSizeMapDir = QString(myData->myFileSizeMapDir) ;
    myArgs->selectMedFileBackgroundLineEdit->setText(QString(myData->myMeshFileBackground));
  }
  else
  {
    myArgs->myFileSizeMapDir = "";  //TODO
    myArgs->selectMedFileBackgroundLineEdit->setText(""); //TODO
  }

  myArgs->fieldNameCmb->setCurrentText(QString(myData->myFieldName));
  myArgs->noTimeStep->setChecked(myData->myUseNoTimeStep);
  myArgs->lastTimeStep->setChecked( myData->myUseLastTimeStep);
  myArgs->chosenTimeStep->setChecked(myData->myUseChosenTimeStep);
  if (myData->myUseChosenTimeStep)
  {
    myArgs->rankSpinBox->setValue(myData->myRank);
    myArgs->timeStep->setValue(myData->myTimeStep);
  }

  myAdvOpt->workingDirectoryLineEdit->setText(QString(myData->myWorkingDir));
  myAdvOpt->logInFileCheck->setChecked(myData->myPrintLogInFile);

  myAdvOpt->verboseLevelSpin->setValue(myData->myVerboseLevel);
  myAdvOpt->removeLogOnSuccessCheck->setChecked(myData->myRemoveLogOnSuccess);
  myAdvOpt->keepWorkingFilesCheck->setChecked(myData->myKeepFiles);

  return ret;

}

bool SMESHGUI_MgAdaptDlg::readParamsFromWidgets()
{
  bool ret = true ;
  SMESH::MgAdaptHypothesisData data, *aData = &data;
  while ( ret )
  {
    // 1. Fichier du maillage de départ
    aData->fromMedFile = myArgs->aMedfile->isChecked();
    if (aData->fromMedFile)
    {
      aData->myFileInDir = CORBA::string_dup(myArgs->myFileInDir.toUtf8().data());
      aData->myMeshFileIn = CORBA::string_dup(myArgs->selectMedFileLineEdit->text().toUtf8().data());
        // aData->myInMeshName = // TODO
    }
    else // TODO browser
    {
      QMessageBox::critical( 0, QObject::tr("MG_ADAPT_ERROR"),
                                QObject::tr("MG_ADAPT_MED_FILE_4") );
      ret = false ;
      break ;
  //     aData->myInMeshName = CORBA::string_dup(myArgs->aBrowserObject->text().toStdString().c_str());
  //     aData->myFileInDir = CORBA::string_dup(myAdvOpt->workingDirectoryLineEdit->text().toStdString().c_str());
  //
  //     TCollection_AsciiString aGenericName = (char*)aData->myFileInDir;
  //     TCollection_AsciiString aGenericName2 = "MgAdapt_";
  //     aGenericName2 += getpid();
  //     aGenericName2 += "_";
  //     aGenericName2 += Abs((Standard_Integer)(long) aGenericName.ToCString());
  //     aGenericName2 += ".med";
  //     aGenericName+=aGenericName2;
  //     emit myArgs->toExportMED(aGenericName.ToCString());
  //     aData->myMeshFileIn = aGenericName2.ToCString();
    }
    // 2. Fichier du maillage de sortie
    aData->myOutMeshName = CORBA::string_dup(myArgs->meshNameLineEdit->text().toStdString().c_str());
    aData->myMeshOutMed = myArgs->medFileCheckBox->isChecked();
    if(aData->myMeshOutMed)
    {
      aData->myFileOutDir = CORBA::string_dup(myArgs->myFileOutDir.toUtf8().data());
      aData->myMeshFileOut = CORBA::string_dup(myArgs->selectOutMedFileLineEdit->text().toUtf8().data());
    }
    else
    {
      aData->myMeshFileOut = "";
    }
    aData->myPublish = myArgs->publishOut->isChecked();

    // 3. Type de carte de tailles
    aData->myUseLocalMap = myArgs->localButton->isChecked();
    aData->myUseBackgroundMap = myArgs->backgroundButton->isChecked();
    aData->myUseConstantValue = myArgs->constantButton->isChecked();
    // 3.1. Constante
    if (aData->myUseConstantValue)
    {
      aData->myConstantValue = myArgs->dvalue->value();
    }
    else
    {
      aData->myConstantValue = 0.0;
    }
    // 3.2. Arrière-plan
    if (aData->myUseBackgroundMap)
    {
      aData->myFileSizeMapDir = CORBA::string_dup(myArgs->myFileSizeMapDir.toUtf8().data());
      aData->myMeshFileBackground = CORBA::string_dup(myArgs->selectMedFileBackgroundLineEdit->text().toUtf8().data());
    }
    else
    {
      aData->myMeshFileBackground = "";
    }

  // 4. Le champ
    if ( ! aData->myUseConstantValue )
    {
      if ( strlen(myArgs->fieldNameCmb->currentText().toStdString().c_str()) == 0 )
      {
        QMessageBox::critical( 0, QObject::tr("MG_ADAPT_ERROR"),
                                  QObject::tr("MG_ADAPT_MED_FILE_5") );
        ret = false ;
        break ;
      }
      {
        aData->myFieldName = CORBA::string_dup(myArgs->fieldNameCmb->currentText().toStdString().c_str());
        aData->myUseNoTimeStep = myArgs->noTimeStep->isChecked();
        aData->myUseLastTimeStep = myArgs->lastTimeStep->isChecked();
        aData->myUseChosenTimeStep = myArgs->chosenTimeStep->isChecked();
        if (aData->myUseChosenTimeStep)
        {
          aData->myRank = myArgs->rankSpinBox->value();
          aData->myTimeStep = myArgs->timeStep->value();
        }
      }
    }

    // 5. Options avancées
    aData->myWorkingDir = CORBA::string_dup(myAdvOpt->workingDirectoryLineEdit->text().toStdString().c_str());
    aData->myPrintLogInFile = myAdvOpt->logInFileCheck->isChecked();
    aData->myVerboseLevel = myAdvOpt->verboseLevelSpin->value();
    aData->myRemoveLogOnSuccess = myAdvOpt->removeLogOnSuccessCheck->isChecked();
    aData->myKeepFiles = myAdvOpt->keepWorkingFilesCheck->isChecked();
    model->setData(*aData);

    QString msg;
    checkParams(msg);
    break ;
  }

  return ret;
}
bool SMESHGUI_MgAdaptDlg::storeParamsToHypo( const SMESH::MgAdaptHypothesisData& ) const
{
  return true;
}
/*!
  \brief Show help page
*/
void SMESHGUI_MgAdaptDlg::PushOnHelp()
{
//   QString aHelpFile;
  // if ( myTabWidget->currentIndex() == MinDistance ) {
  //   aHelpFile = "measurements.html#min-distance-anchor";
  // } else if ( myTabWidget->currentIndex() == BoundingBox ) {
  //   aHelpFile = "measurements.html#bounding-box-anchor";
  // } else if ( myTabWidget->currentWidget() == myAngle ) {
  //   aHelpFile = "measurements.html#angle-anchor";
  // } else {
  //   aHelpFile = "measurements.html#basic-properties-anchor";
  // }

//   SMESH::ShowHelpFile( aHelpFile );
}
bool SMESHGUI_MgAdaptDlg::checkParams(QString& msg)
{
  if ( !QFileInfo( myAdvOpt->workingDirectoryLineEdit->text().trimmed() ).isWritable() )
  {
    SUIT_MessageBox::warning( this,
                              tr( "SMESH_WRN_WARNING" ),
                              tr( "NO_PERMISSION" ) );
    return false;
  }


  myAdvOpt->myOptionTable->setFocus();
  QApplication::instance()->processEvents();

  QString name, value;
  bool isDefault, ok = true;
  int iRow = 0, nbRows = myAdvOpt->myOptionTable->topLevelItemCount();
  for ( ; iRow < nbRows; ++iRow )
  {
    QTreeWidgetItem* row = myAdvOpt->myOptionTable->topLevelItem( iRow );
    myAdvOpt->GetOptionAndValue( row, name, value, isDefault );

    if ( name.simplified().isEmpty() )
        continue; // invalid custom option

    if ( isDefault ) // not selected option
        value.clear();

    try
    {
      model->setOptionValue( name.toLatin1().constData(), value.toLatin1().constData() );
    }
    catch ( const SALOME::SALOME_Exception& ex )
    {
      msg = ex.details.text.in();
      ok = false;
      break;
    }
  }

  return ok;
}

//=================================================================================
// function : SMESHGUI_MgAdaptArguments()
// purpose  :
//=================================================================================
SMESHGUI_MgAdaptArguments::SMESHGUI_MgAdaptArguments( QWidget* parent )
  :QWidget(parent)
{

  if ( SUIT_FileDlg::getLastVisitedPath().isEmpty() )
  {
    myFileInDir = QDir::currentPath();
    myFileOutDir = QDir::currentPath();
    myFileSizeMapDir = QDir::currentPath();
  }
  else
  {
    myFileInDir = SUIT_FileDlg::getLastVisitedPath();
    myFileOutDir = SUIT_FileDlg::getLastVisitedPath();
    myFileSizeMapDir = SUIT_FileDlg::getLastVisitedPath();
  }

  meshDim = 0;
  meshDimBG = 0;
  // Mesh in
  aMeshIn = new QGroupBox( tr( "MeshIn" ), this );
  aMedfile       = new QRadioButton( tr( "MEDFile" ),    aMeshIn );
  aBrowser       = new QRadioButton( tr( "Browser" ), aMeshIn );
  aBrowserObject = new QLineEdit(  aMeshIn );
  selectMedFilebutton = new QPushButton("...", aMeshIn);
  selectMedFileLineEdit = new QLineEdit(  aMeshIn );

  meshIn = new QGridLayout( aMeshIn );

  meshIn->setMargin( MARGIN );
  meshIn->setSpacing( SPACING );
  meshIn->addWidget( aMedfile,     0, 0, 1,1 );
  meshIn->addWidget( aBrowser,     0, 1,1,1);
  meshIn->addWidget( aBrowserObject,     0, 2, 1, 1 );
  meshIn->addWidget( selectMedFilebutton,  1, 0,1, 1);
  meshIn->addWidget( selectMedFileLineEdit,  1, 1, 1, 1 );
  hspacer = new QSpacerItem(188, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  meshInGroup = new QButtonGroup( this );
  meshInGroup->addButton( aMedfile,   0 );
  meshInGroup->addButton( aBrowser,   1 );

  //Mesh out

  aMeshOut = new QGroupBox( tr( "MeshOut" ), this );
  meshName = new QLabel(tr("MeshName"), aMeshOut);
  secondHspacer = new QSpacerItem(100, 30);
  meshNameLineEdit = new QLineEdit(aMeshOut) ;
  medFileCheckBox = new QCheckBox(tr("MEDFile"), aMeshOut);
  selectOutMedFilebutton = new QPushButton("...", aMeshOut);
  thirdHspacer = new QSpacerItem(188, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  selectOutMedFileLineEdit = new QLineEdit(aMeshOut) ;
  publishOut = new  QCheckBox(tr("Publish_MG_ADAPT"), aMeshOut);

  meshOut = new QGridLayout( aMeshOut );

  meshOut->setMargin( MARGIN );
  meshOut->setSpacing( SPACING );
  meshOut->addWidget( meshName,  0, 0, 1,1 );
  meshOut->addItem( secondHspacer,  0, 1, 1, 1 );
  meshOut->addWidget( meshNameLineEdit, 0, 2,1,1);
  meshOut->addWidget( medFileCheckBox,  1, 0,1,1 );
  meshOut->addWidget( selectOutMedFilebutton,  1, 1,1,1 );
  meshOut->addWidget( selectOutMedFileLineEdit,  1, 2,1,1);
  meshOut->addWidget( publishOut,  2, 0,1,1 );

  //size map definition

  sizeMapDefinition  = new QGroupBox(tr("SIZE_MAP_DEF"), this);
  localButton = new QRadioButton(tr("LOCAL_MG_ADAPT"), sizeMapDefinition);
  backgroundButton = new QRadioButton(tr("BACKGRND_MG_ADAPT"), sizeMapDefinition);
  constantButton = new QRadioButton(tr("CNST_MG_ADAPT"), sizeMapDefinition);
  medFileBackground = new QLabel(tr("MED_FILE_BCKG"), sizeMapDefinition);
  selectMedFileBackgroundbutton = new QPushButton("...", sizeMapDefinition);
  selectMedFileBackgroundLineEdit = new QLineEdit(sizeMapDefinition);
  valueLabel = new QLabel(tr("VALUE_MG_ADAPT"), sizeMapDefinition);
  dvalue = new QDoubleSpinBox(sizeMapDefinition);
  sizeMapDefGroup = new QButtonGroup( this );
  sizeMapDefGroup->addButton( localButton,   0 );
  sizeMapDefGroup->addButton( backgroundButton,   1 );
  sizeMapDefGroup->addButton( constantButton,   2 );

  sizeMapDefGroupLayout = new QGridLayout(sizeMapDefinition);
  sizeMapDefGroupLayout->addWidget(localButton, 0,0);
  sizeMapDefGroupLayout->addWidget(backgroundButton, 0,1);
  sizeMapDefGroupLayout->addWidget(constantButton, 0,2);
  sizeMapDefGroupLayout->addWidget(medFileBackground, 1,0);
  sizeMapDefGroupLayout->addWidget(selectMedFileBackgroundbutton, 1,1);
  sizeMapDefGroupLayout->addWidget(selectMedFileBackgroundLineEdit, 1,2);
  sizeMapDefGroupLayout->addWidget(valueLabel, 2,0);
  sizeMapDefGroupLayout->addWidget(dvalue, 2,1);

  // size Map field
  sizeMapField = new QGroupBox(tr("SIZE_MAP_FIELD"), this);
  fieldName = new QLabel(tr("MG_ADAPT_FIELD_NAME"), sizeMapField);
  fieldNameCmb = new QComboBox(sizeMapField);
  noTimeStep = new QRadioButton(tr("MG_ADAPT_NO_T_ST"), sizeMapField);
  lastTimeStep = new QRadioButton(tr("MG_ADAPT_L_ST"), sizeMapField);
  chosenTimeStep = new QRadioButton(tr("MG_ADAPT_CH_ST"), sizeMapField);
  timeStepLabel = new QLabel(tr("MG_ADAPT_TSTP"), sizeMapField);
  timeStep = new QSpinBox(sizeMapField);
  timeStep->setMinimum(-1);
  rankLabel = new QLabel(tr("MG_ADAPT_RANK"), sizeMapField);
  rankSpinBox = new QSpinBox(sizeMapField);
  rankSpinBox->setMinimum(-1);

  timeStepGroup = new QButtonGroup(this);
  timeStepGroup->addButton(noTimeStep, 0);
  timeStepGroup->addButton(lastTimeStep, 1);
  timeStepGroup->addButton(chosenTimeStep, 2);

  sizeMapFieldGroupLayout = new QGridLayout(sizeMapField);

  sizeMapFieldGroupLayout->addWidget(fieldName, 0,0);
  sizeMapFieldGroupLayout->addWidget(fieldNameCmb, 0,1);
  sizeMapFieldGroupLayout->addWidget(noTimeStep, 1,0);
  sizeMapFieldGroupLayout->addWidget(lastTimeStep, 1,1);
  sizeMapFieldGroupLayout->addWidget(chosenTimeStep, 1,2);
  sizeMapFieldGroupLayout->addWidget(timeStepLabel, 2,0);
  sizeMapFieldGroupLayout->addWidget(timeStep, 2,1);
  sizeMapFieldGroupLayout->addWidget(rankLabel, 2,2);
  sizeMapFieldGroupLayout->addWidget(rankSpinBox, 2,3);

  QGridLayout* argumentsLayout = new QGridLayout( this );
  argumentsLayout->setMargin( MARGIN );
  argumentsLayout->setSpacing( SPACING );

  argumentsLayout->addWidget( aMeshIn,  0, 0, 1, 3 );
  argumentsLayout->addWidget( aMeshOut, 1, 0, 1, 3 );
  argumentsLayout->addWidget( sizeMapDefinition, 2, 0, 1, 3 );
  argumentsLayout->addWidget( sizeMapField, 3, 0, 1, 3 );
  argumentsLayout->setColumnStretch( 1, 5 );
  argumentsLayout->setRowStretch( 4, 5 );

  // Initial state
  setMode( Mesh, Local);
  medFileCheckBox->setChecked(true);
  visibleTimeStepRankLabel (false);

  // Connections
  connect( meshInGroup,            SIGNAL( buttonClicked( int ) ),  this, SLOT( modeChanged( int ) ) );
  connect( sizeMapDefGroup,        SIGNAL( buttonClicked( int ) ),  this, SLOT( sizeMapDefChanged( int ) ) );
  connect( selectMedFilebutton,    SIGNAL( pressed(  ) ),           this, SLOT( onSelectMedFilebuttonClicked(  ) ) );
  connect( medFileCheckBox,        SIGNAL (stateChanged(int)),      this, SLOT(onMedFileCheckBox(int) ) );
  connect( publishOut,             SIGNAL (stateChanged(int)),      this, SLOT(onPublishOut(int) ) );
  connect( selectOutMedFilebutton, SIGNAL( pressed()),              this, SLOT(onSelectOutMedFilebutton()));
  connect( selectMedFileBackgroundbutton, SIGNAL(pressed()),        this, SLOT(onSelectMedFileBackgroundbutton()) );
  connect( timeStepGroup,          SIGNAL( buttonClicked( int ) ),  this, SLOT( timeStepGroupChanged( int ) ) );
  emit updateSelection();
}

//=================================================================================
// function : ~SMESHGUI_MgAdaptArguments()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_MgAdaptArguments::~SMESHGUI_MgAdaptArguments()
{
}

void SMESHGUI_MgAdaptArguments::onNoTimeStep(bool disableOther)
{
  noTimeStep->setChecked(true);

  visibleTimeStepRankLabel (false);
  rankSpinBox->setValue(-2);
  timeStep->setValue(-2);

  lastTimeStep->setDisabled(disableOther);
  chosenTimeStep->setDisabled(disableOther);
}
void SMESHGUI_MgAdaptArguments::onLastTimeStep(bool disableOther)
{
  lastTimeStep->setChecked(true);

  visibleTimeStepRankLabel (false);
  rankSpinBox->setValue(-1);
  timeStep->setValue(-1);
  noTimeStep->setDisabled(disableOther);
}
void SMESHGUI_MgAdaptArguments::onChosenTimeStep(bool /*disableOther*/, int vmax)
{
  chosenTimeStep->setChecked(true);

  visibleTimeStepRankLabel (true);
  rankSpinBox->setValue(-1);
  timeStep->setValue(-1);
  if (vmax) timeStep->setMaximum(vmax);
}

void SMESHGUI_MgAdaptArguments::visibleTimeStepRankLabel(bool visible)
{
  rankLabel->setVisible(visible);
  rankSpinBox->setVisible(visible);

  timeStepLabel->setVisible(visible);
  timeStep->setVisible(visible);
}

void SMESHGUI_MgAdaptArguments::onSelectOutMedFilebutton()
{

  QString filtre = QString("Med") ;
  filtre += QString(" files (*.") + QString("med") + QString(");;");
  QString fileName = QFileDialog::getSaveFileName(this, tr("SAVE_MED"), QString(""), filtre);
  QFileInfo myFileInfo(fileName);
  selectOutMedFileLineEdit->setText(myFileInfo.fileName());
  myFileOutDir = myFileInfo.path();

}
void SMESHGUI_MgAdaptArguments::onSelectMedFileBackgroundbutton()
{
  QString fileName0 = selectMedFileBackgroundbutton->text().trimmed();

  QString fileName = getMedFileName(false);
  if (fileName != QString::null)
  {
    myFieldList = GetListeChamps(fileName);
    if (myFieldList.empty())
    {
      fileName = fileName0;
      fieldNameCmb->clear();
    }
    else
    {
      // fill field name Combobox
      fieldNameCmb->clear();
      std::map<QString, int>::const_iterator it;
      for ( it=myFieldList.begin() ; it != myFieldList.end(); it++)
      {
        fieldNameCmb->insertItem(0,QString(it->first));
        int typeStepInField = it->second > 2 ?  2 : it->second ;
        timeStepGroupChanged(typeStepInField, false);
      }
      // Dimension du maillage de fonds
      lireNomDimMaillage( fileName, meshDimBG );
      valueAdaptation ();
    }
  }
  else
  {
    fileName = fileName0;
    fieldNameCmb->clear();
  }

  QFileInfo myFileInfo(fileName);
  myFileSizeMapDir = myFileInfo.path();
  selectMedFileBackgroundLineEdit->setText(myFileInfo.fileName());

}
void SMESHGUI_MgAdaptArguments::onMedFileCheckBox(int state)
{
  if (state == Qt::Checked)
  {
    selectOutMedFilebutton->show();
    selectOutMedFileLineEdit->show();
    selectOutMedFilebutton->setEnabled(true);
    selectOutMedFileLineEdit->setEnabled(true);
  }
  else
  {
    selectOutMedFilebutton->setEnabled(false);
    selectOutMedFileLineEdit->setEnabled(false);
    publishOut->setChecked(true);
  }
}
void SMESHGUI_MgAdaptArguments::onPublishOut(int state)
{
  if (state == Qt::Unchecked)
  {
    medFileCheckBox->setChecked(true);
  }
}

void SMESHGUI_MgAdaptArguments::onSelectMedFilebuttonClicked()
{
  // bool keep = false;
  QString fileName0 = selectMedFileLineEdit->text().trimmed();

  QString fileName = getMedFileName(false);
  if(fileName != QString::null)
  {
    QString aMeshName = lireNomDimMaillage(fileName.trimmed(), meshDim);
    if (aMeshName.isEmpty() )
    {
      QMessageBox::critical( 0, QObject::tr("MG_ADAPT_ERROR"),
                                QObject::tr("MG_ADAPT_MED_FILE_2") );
      fileName = fileName0;
    }
    else
    {
      meshNameLineEdit->setText(aMeshName);
      valueAdaptation ();
//       ADAPTATION_MODE aMode = meshDim == 3 ? ADAPTATION_MODE::BOTH : ADAPTATION_MODE::SURFACE; // and when dimesh 3 without 2D mesh?
//       emit meshDimSignal(aMode);
    }
  }
  else
  {
      return;
  }

  QFileInfo myFileInfo(fileName);
  myFileInDir = myFileInfo.path();
  myFileOutDir = myFileInfo.path();
  selectMedFileLineEdit->setText(myFileInfo.fileName());
  QString outF = fileName == QString::null ? myFileInfo.fileName() :
  QString( remove_extension(myFileInfo.fileName().toStdString() ).c_str() )+ QString(".adapt.med");
  selectOutMedFileLineEdit->setText(outF);
  onLocalSelected(myFileInfo.filePath());

}

void SMESHGUI_MgAdaptArguments::valueAdaptation()
{
  ADAPTATION_MODE aMode ;
  if ( meshDimBG < 3 )
  {
    aMode = meshDim == 3 ? ADAPTATION_MODE::BOTH : ADAPTATION_MODE::SURFACE;
  }
  else
  {
    aMode = ADAPTATION_MODE::BOTH;
  }
  emit meshDimSignal(aMode);
}

void SMESHGUI_MgAdaptArguments::onLocalSelected(QString filePath)
{
  myFieldList = GetListeChamps(filePath, false);
  if (myFieldList.empty())
  {
    if (localButton->isChecked())
    {
      fieldNameCmb->clear();
    }
  }
  else
  {
    // fill field name Combobox
    fieldNameCmb->clear();
    std::map<QString, int>::const_iterator it;
    for ( it = myFieldList.begin() ; it != myFieldList.end(); it++)
    {
      fieldNameCmb->insertItem(0,QString(it->first));
      // Je ne comprends pas le rapport entre pas de temps et apparition d'un nouveau champ... GN
      int typeStepInField = it->second > 2 ?  2 : it->second ;
//             std::cout << "SMESHGUI_MgAdaptArguments::onLocalSelected typeStepInField : " << typeStepInField << std::endl;
      timeStepGroupChanged(typeStepInField, false);
    }
  }
}
// =======================================================================
// Gestion les boutons qui permettent  de
// 1) retourne le nom d'un fichier par une fenetre de dialogue si aucun
//    objet est selectionne dans l arbre d etude
// 2) retourne le nom du fichier asocie a l objet
//    selectionne dans l arbre d etude
// =======================================================================
QString SMESHGUI_MgAdaptArguments::getMedFileName(bool /*avertir*/)
{

  QString aFile = QString::null;
  QString filtre = QString("Med") ;
  filtre += QString(" files (*.") + QString("med") + QString(");;");
  aFile = SUIT_FileDlg::getOpenFileName(0, QObject::tr("MG_ADAPT_SELECT_FILE_0"), QString(""), filtre );

  return aFile;

}
void SMESHGUI_MgAdaptArguments::setMode(const Mode theMode, const SIZEMAP theSizeMap )
{
  QRadioButton* aButton = qobject_cast<QRadioButton*>( meshInGroup->button( theMode ) );
  QRadioButton* bButton = qobject_cast<QRadioButton*>( sizeMapDefGroup->button( theSizeMap ) );
  if ( aButton )
  {
    aButton->setChecked( true );
    modeChanged( theMode );
  }
  if ( bButton )
  {
    bButton->setChecked( true );
    sizeMapDefChanged( theSizeMap );
  }
}

void SMESHGUI_MgAdaptArguments::modeChanged( int theMode )
{
  clear();
  if(theMode == Mesh)
  {
    aBrowserObject->hide();
    selectMedFileLineEdit->show();
    selectMedFilebutton->show();
    localButton->setEnabled(true);
  }
  else
  {
    selectMedFileLineEdit->hide();
    selectMedFilebutton->hide();
    localButton->setEnabled(false);
    aBrowserObject->show();
    sizeMapDefChanged(Background);
    emit updateSelection();
  }
}

void SMESHGUI_MgAdaptArguments::sizeMapDefChanged( int  theSizeMap )
{
  fieldNameCmb->clear();
  if(theSizeMap == Local)
  {
    localButton->setEnabled(true);
    localButton->setChecked(true);
    medFileBackground->hide();
    selectMedFileBackgroundbutton->hide();
    selectMedFileBackgroundLineEdit->hide();
    selectMedFileBackgroundLineEdit->clear();
    valueLabel->hide();
    dvalue->hide();

    sizeMapField->setEnabled(true);
    if (!selectMedFileLineEdit->text().isEmpty())
    {
      QFileInfo myFileInfo(QDir(myFileInDir), selectMedFileLineEdit->text());
      onLocalSelected(myFileInfo.filePath());
    }
  }
  else if (theSizeMap == Background)
  {
    medFileBackground->show();
    backgroundButton->setChecked(true);
    selectMedFileBackgroundbutton->show();
    selectMedFileBackgroundLineEdit->show();
    valueLabel->hide();
    dvalue->hide();
    sizeMapField->setEnabled(true);
  }
  else
  {
    medFileBackground->hide();
    constantButton->setChecked(true);
    selectMedFileBackgroundbutton->hide();
    selectMedFileBackgroundLineEdit->clear();
    selectMedFileBackgroundLineEdit->hide();
    valueLabel->show();
    dvalue->show();
    sizeMapField->setEnabled(false);
  }
  meshDimBG = 0;
  valueAdaptation();
}
void SMESHGUI_MgAdaptArguments::timeStepGroupChanged(int timeStepType, bool disableOther, int vmax)
{
  switch (timeStepType)
  {
  case 0 :
    onNoTimeStep(disableOther);
    break;
  case 1 :
    onLastTimeStep(disableOther);
    break;
  case 2 :
    onChosenTimeStep(disableOther, vmax);
  default:
    break;
  }
}

void SMESHGUI_MgAdaptArguments::clear()
{
  selectMedFileLineEdit->clear();
  aBrowserObject->clear();

  meshNameLineEdit->clear();
  selectOutMedFileLineEdit->clear();
}
// med_int SMESHGUI_MgAdaptArguments::getMeshDim() const
// {
//   return meshDim;
// }
QWidget* ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &o, const QModelIndex &index) const
{
  bool editable = index.data( EDITABLE_ROLE ).toInt();
  return editable ? QItemDelegate::createEditor( parent, o, index ) : 0;
}

//////////////////////////////////////////
// MgAdaptAdvWidget
//////////////////////////////////////////

MgAdaptAdvWidget::MgAdaptAdvWidget( QWidget* parent, std::vector <std::string>* options, Qt::WindowFlags f )
  : QWidget( parent, f ), myOptions(options)
{
  setupWidget();
  myOptionTable->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
  myOptionTable->setItemDelegate( new ItemDelegate( myOptionTable ) );

  for ( size_t i = 0, nb = myOptions->size(); i < nb; ++i )
  {
    AddOption( (*myOptions)[i].c_str() );
  }

  connect( myOptionTable, SIGNAL( itemChanged(QTreeWidgetItem *, int)), SLOT( itemChanged(QTreeWidgetItem *, int )));
  connect( addBtn,        SIGNAL(clicked()),       this, SLOT( onAddOption() ) );
  connect(workingDirectoryPushButton, SIGNAL(pressed()),  this, SLOT(_onWorkingDirectoryPushButton()));
}

MgAdaptAdvWidget::~MgAdaptAdvWidget()
{
}

void MgAdaptAdvWidget::AddOption( const char* option, bool isCustom )
{
  QString name, value;
  bool isDefault = false;
  if ( option )
  {
    QStringList name_value_type = QString(option).split( ":", QString::KeepEmptyParts );
    if ( name_value_type.size() > 0 )
        name = name_value_type[0];
    if ( name_value_type.size() > 1 )
        value = name_value_type[1];
    if ( name_value_type.size() > 2 )
        isDefault = !name_value_type[2].toInt();
  }
  QTreeWidget* table = myOptionTable;
  //table->setExpanded( true );

  QTreeWidgetItem* row;
  if (optionTreeWidgetItem.size())
  {
    std::map<QString, QTreeWidgetItem *>::iterator it = optionTreeWidgetItem.find(name);
    if(it != optionTreeWidgetItem.end()) return; // option exist
    else
    {
      row = getNewQTreeWidgetItem(table, option, name, isCustom);
    }
  }
  else
  {
    row = getNewQTreeWidgetItem(table, option, name, isCustom);
  }
  row->setText( 0, tr( name.toLatin1().constData() ));
  row->setText( 1, tr( value.toLatin1().constData() ));
  row->setCheckState( 0, isDefault ? Qt::Unchecked : Qt::Checked);
  row->setData( NAME_COL, PARAM_NAME, name );

  if ( isCustom )
  {
    myOptionTable->scrollToItem( row );
    myOptionTable->setCurrentItem( row );
    myOptionTable->editItem( row, NAME_COL );
  }
}

QTreeWidgetItem* MgAdaptAdvWidget::getNewQTreeWidgetItem(QTreeWidget* table, const char* option, QString& name, bool isCustom)
{
  QTreeWidgetItem* row = new QTreeWidgetItem( table );
  row->setData( NAME_COL, EDITABLE_ROLE, int( isCustom && !option ));
  row->setFlags( row->flags() | Qt::ItemIsEditable );
  optionTreeWidgetItem.insert(std::pair <QString, QTreeWidgetItem*> (name, row));

  return row;
}

void MgAdaptAdvWidget::onAddOption()
{
  AddOption( NULL, true );
}
void MgAdaptAdvWidget::GetOptionAndValue( QTreeWidgetItem * tblRow,
        QString&          option,
        QString&          value,
        bool&             isDefault)
{
  option    = tblRow->data( NAME_COL, PARAM_NAME ).toString();
  value     = tblRow->text( VALUE_COL );
  isDefault = ! tblRow->checkState( NAME_COL );

}

void MgAdaptAdvWidget::itemChanged(QTreeWidgetItem* tblRow, int column)
{
  if ( tblRow )
  {
    myOptionTable->blockSignals( true );

    tblRow->setData( VALUE_COL, EDITABLE_ROLE, int( tblRow->checkState( NAME_COL )));

    int c = tblRow->checkState( NAME_COL ) ? 0 : 150;
    tblRow->setForeground( VALUE_COL, QBrush( QColor( c, c, c )));

    if ( column == NAME_COL && tblRow->data( NAME_COL, EDITABLE_ROLE ).toInt() ) // custom table
    {
      tblRow->setData( NAME_COL, PARAM_NAME, tblRow->text( NAME_COL ));
    }

    myOptionTable->blockSignals( false );
  }
}
void MgAdaptAdvWidget::setupWidget()
{
  if (this->objectName().isEmpty())
      this->setObjectName(QString(tr("MG-ADAPT-ADV")));
  this->resize(337, 369);
  gridLayout_4 = new QGridLayout(this);
  gridLayout_4->setObjectName(QString("gridLayout_4"));
  myOptionTable = new MgAdaptAdvWidgetTreeWidget(this);
  QFont font;
  font.setBold(false);
  font.setWeight(50);
  QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
  __qtreewidgetitem->setFont(1, font);
  __qtreewidgetitem->setFont(0, font);
  __qtreewidgetitem->setText(1, tr("OPTION_VALUE_COLUMN"));
  __qtreewidgetitem->setText(0, tr("OPTION_NAME_COLUMN"));
  myOptionTable->setHeaderItem(__qtreewidgetitem);
  myOptionTable->setObjectName(QString("myOptionTable"));
  myOptionTable->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
  myOptionTable->setTabKeyNavigation(true);

  gridLayout_4->addWidget(myOptionTable, 0, 0, 1, 2);

  addBtn = new QPushButton(this);
  addBtn->setText(QApplication::translate("SMESH_AdvOptionsWdg", "ADD_OPTION_BTN", Q_NULLPTR));
  addBtn->setObjectName(QString("addBtn"));

  gridLayout_4->addWidget(addBtn, 1, 0, 1, 1);

  horizontalSpacer = new QSpacerItem(188, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  gridLayout_4->addItem(horizontalSpacer, 1, 1, 1, 1);

  logGroupBox = new QGroupBox(this);
  logGroupBox->setObjectName(QString("logGroupBox"));
  gridLayout_2 = new QGridLayout(logGroupBox);
  gridLayout_2->setObjectName(QString("gridLayout_2"));
  gridLayout = new QGridLayout();
  gridLayout->setObjectName(QString("gridLayout"));
  workingDirectoryLabel = new QLabel(logGroupBox);
  workingDirectoryLabel->setObjectName(QString("workingDirectoryLabel"));

  gridLayout->addWidget(workingDirectoryLabel, 0, 0, 1, 1);

  workingDirectoryLineEdit = new QLineEdit(logGroupBox);
  workingDirectoryLineEdit->setObjectName(QString("workingDirectoryLineEdit"));

  gridLayout->addWidget(workingDirectoryLineEdit, 0, 1, 1, 1);

  workingDirectoryPushButton = new QPushButton(logGroupBox);
  workingDirectoryPushButton->setObjectName(QString("workingDirectoryPushButton"));

  gridLayout->addWidget(workingDirectoryPushButton, 0, 2, 1, 1);

  verboseLevelLabel = new QLabel(logGroupBox);
  verboseLevelLabel->setObjectName(QString("verboseLevelLabel"));

  gridLayout->addWidget(verboseLevelLabel, 1, 0, 1, 1);

  verboseLevelSpin = new QSpinBox(logGroupBox);
  verboseLevelSpin->setObjectName(QString("verboseLevelSpin"));

  gridLayout->addWidget(verboseLevelSpin, 1, 1, 1, 1);


  gridLayout_2->addLayout(gridLayout, 0, 0, 1, 1);

  horizontalLayout = new QHBoxLayout();
  horizontalLayout->setObjectName(QString("horizontalLayout"));
  logInFileCheck = new QCheckBox(logGroupBox);
  logInFileCheck->setObjectName(QString("logInFileCheck"));
  logInFileCheck->setChecked(true);

  horizontalLayout->addWidget(logInFileCheck);

  removeLogOnSuccessCheck = new QCheckBox(logGroupBox);
  removeLogOnSuccessCheck->setObjectName(QString("removeLogOnSuccessCheck"));
  removeLogOnSuccessCheck->setChecked(true);

  horizontalLayout->addWidget(removeLogOnSuccessCheck);


  gridLayout_2->addLayout(horizontalLayout, 1, 0, 1, 1);

  keepWorkingFilesCheck = new QCheckBox(logGroupBox);
  keepWorkingFilesCheck->setObjectName(QString("keepWorkingFilesCheck"));
  keepWorkingFilesCheck->setAutoExclusive(false);

  gridLayout_2->addWidget(keepWorkingFilesCheck, 2, 0, 1, 1);


  gridLayout_4->addWidget(logGroupBox, 3, 0, 1, 2);

}
void MgAdaptAdvWidget::_onWorkingDirectoryPushButton()
{
  QString aDirName=QFileDialog::getExistingDirectory ();
  if (!(aDirName.isEmpty()))workingDirectoryLineEdit->setText(aDirName);
}
void MgAdaptAdvWidget::onMeshDimChanged(ADAPTATION_MODE aMode)
{
/* default adaptation mode
  * assume that if meshDim == 2 and no 3D backgrounmesh-->adaptation surface
  * if meshDim == 3 and  if there is not 2D mesh -->VOLUME
  * else BOTH
  */

  QString adaptation("adaptation"), value;
  switch(aMode)
  {
    case ADAPTATION_MODE::SURFACE:
    {
      value ="surface";
      setOptionValue(adaptation, value);
      break;
    }
    case ADAPTATION_MODE::BOTH :
    {
      value = "both";
      setOptionValue(adaptation, value);
      break;
    }
    case ADAPTATION_MODE::VOLUME :
    {
      value = "volume";
      setOptionValue(adaptation, value);
      break;
    }
  }
}
void MgAdaptAdvWidget::setOptionValue(QString& option, QString& value)
{
  std::map<QString, QTreeWidgetItem *>::iterator it = optionTreeWidgetItem.find(option);
  if (it != optionTreeWidgetItem.end())
  {
    it->second->setText( 0, tr( option.toLatin1().constData() ));
    it->second->setText( 1, tr( value.toLatin1().constData() ));
    it->second->setCheckState( 0,  Qt::Checked );
    it->second->setData( NAME_COL, PARAM_NAME, option );
    myOptionTable->editItem( it->second, NAME_COL );
  }
}
namespace
{
bool isEditable( const QModelIndex& index )
{
  return index.isValid() &&
          index.flags() & Qt::ItemIsEditable &&
          index.flags() & Qt::ItemIsEnabled &&
          ( !index.data( Qt::UserRole + 1 ).isValid() || index.data( Qt::UserRole + 1 ).toInt() != 0 );
}
}

MgAdaptAdvWidgetTreeWidget::MgAdaptAdvWidgetTreeWidget( QWidget* parent )
    : QTreeWidget( parent )
{
}

QModelIndex MgAdaptAdvWidgetTreeWidget::moveCursor( CursorAction action, Qt::KeyboardModifiers modifiers )
{
  QModelIndex current = currentIndex();
  int column = current.column();
  if ( action == MoveNext )
  {
    if ( column < columnCount()-1 )
    {
      QModelIndex next = current.sibling( current.row(), column+1 );
      if ( isEditable( next ) ) return next;
    }
    else
    {
      QModelIndex next = current.sibling( current.row()+1, 0 );
      if ( isEditable( next ) ) return next;
    }
  }
  else if ( action == MovePrevious )
  {
    if ( column == 0 ) {
      QModelIndex next = current.sibling( current.row()-1, columnCount()-1 );
      if ( isEditable( next ) ) return next;
    }
    else {
      QModelIndex next = current.sibling( current.row(), column-1 );
       if ( isEditable( next ) ) return next;
    }
  }
  return QTreeWidget::moveCursor( action, modifiers );
}

void MgAdaptAdvWidgetTreeWidget::keyPressEvent( QKeyEvent* e )
{
  switch ( e->key() )
  {
    case Qt::Key_F2:
    {
      QModelIndex index = currentIndex();
      if ( !isEditable( index ) ) {
        for ( int i = 0; i < columnCount(); i++ ) {
          QModelIndex sibling = index.sibling( index.row(), i );
          if ( isEditable( sibling ) ) {
            if ( !edit( sibling, EditKeyPressed, e ) ) e->ignore();
          }
        }
      }
    }
    break;
    default:
      break;
  }
  QTreeWidget::keyPressEvent( e );
}
