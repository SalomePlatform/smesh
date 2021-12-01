// Copyright (C) 2011-2021  CEA/DEN, EDF R&D
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


#include "SMESHGUI_HomardUtils.h"

#include <utilities.h>

#include "OB_Browser.h"

#include "SUIT_Desktop.h"
#include "SUIT_Application.h"
#include "SUIT_Session.h"

#include "LightApp_SelectionMgr.h"
#include "SalomeApp_Application.h"
#include "SalomeApp_Module.h"
#include "SalomeApp_Study.h"

#include "SALOME_ListIO.hxx"

#include "SALOMEconfig.h"
#include <string>

#include <qmessagebox.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qstring.h>
#include <stdlib.h>
#include <qstringlist.h>

#include <sys/stat.h>
#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#endif

#include "SalomeApp_Tools.h"

#include <med.h>

SALOME_ListIO SMESH_HOMARD_UTILS::mySelected;

//================================================================
// Function : GetActiveStudy
// Returne un pointeur sur l'etude active
//================================================================
SUIT_Study* SMESH_HOMARD_UTILS::GetActiveStudy()
{
    SUIT_Application* app = SUIT_Session::session()->activeApplication();
    if (app)
      return app->activeStudy();
    else
      return NULL;
}

//================================================================
// Function : getStudy
// Returne un pointeur sur l'etude active
//================================================================
_PTR(Study) SMESH_HOMARD_UTILS::getStudy()
{
  static _PTR(Study) _study;
  if(!_study)
    _study = SalomeApp_Application::getStudy();
  return _study;
}

//================================================================
// Function : updateObjBrowser
// Purpose  : met a jour l arbre d 'etude pour Homard
//================================================================
void SMESH_HOMARD_UTILS::updateObjBrowser()
{
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>(SUIT_Session::session()->activeApplication());
    if (app) {
      //  Le nom identifiant doit etre la valeur du parametre
      // name de la section HOMARD du fichier SalomeApp.xml
      CAM_Module* module = app->module("Homard" );
      SalomeApp_Module* appMod = dynamic_cast<SalomeApp_Module*>( module );
      if ( appMod ) {
        app->updateObjectBrowser();
        appMod->updateObjBrowser( true );
      }
      else
        MESSAGE( "---   SMESHHOMARD::updateObjBrowser: appMod = NULL");
    }
}

//================================================================
// Function : selectedIO
// Return the list of selected SALOME_InteractiveObject's
//================================================================
const SALOME_ListIO& SMESH_HOMARD_UTILS::selectedIO()
{
      SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* > ( SUIT_Session::session()->activeApplication() );
      LightApp_SelectionMgr* aSelectionMgr = app->selectionMgr();
      if( aSelectionMgr )
      {
	      aSelectionMgr->selectedObjects( mySelected );
        for (SALOME_ListIteratorOfListIO it (mySelected); it.More(); it.Next())
        SCRUTE(it.Value()->getEntry());
      };
      return mySelected;
}

//================================================================
// Function : IObjectCount
// Return the number of selected objects
//================================================================
int SMESH_HOMARD_UTILS::IObjectCount()
{
      SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
      LightApp_SelectionMgr* aSelectionMgr = app->selectionMgr();
      if( aSelectionMgr )
      {
	      aSelectionMgr->selectedObjects( mySelected );
	      SCRUTE(mySelected.Extent());
        return mySelected.Extent();
      }
      return 0;
}

//================================================================
// Function : firstIObject
// Purpose  :  Return the first selected object in the selected object list
//================================================================
Handle(SALOME_InteractiveObject) SMESH_HOMARD_UTILS::firstIObject()
{
      const SALOME_ListIO& aList = selectedIO();
      return aList.Extent() > 0 ? aList.First() : Handle(SALOME_InteractiveObject)();
}

//================================================================
// Function : lastIObject
// Return the last selected object in the selected object list
//================================================================
Handle(SALOME_InteractiveObject) SMESH_HOMARD_UTILS::lastIObject()
{
      const SALOME_ListIO& aList = selectedIO();
      return aList.Extent() > 0 ? aList.Last() : Handle(SALOME_InteractiveObject)();
}

//================================================================
// Retourne vrai si l'objet est du type voulu
// . Dans le cas d'un cas, d'une hypothese, d'une zone, on se contente
// d'une comparaison simple entre le type stocke et TypeObject.
// . Pour l'iteration, le type stocke en attribut est sous la forme
// "IterationHomard" + le nom de l'iteration precedente.
// Il faut donc regarder si la chaine commence par TypeObject.
// . Idem pour le fichier de messages : "Mess " + numero d'iteration
//
// On filtre ce genre de situation avec option :
//  -1 : comparaison totale
//   0 : comparaison sur le debut de la chaine
//================================================================
bool SMESH_HOMARD_UTILS::isObject(_PTR(SObject) MonObj, QString TypeObject, int option)
{
//   MESSAGE("isObject, TypeObject = "<<TypeObject.toStdString().c_str()<<", option = "<<option);
// Existence d'un attribut ?
  _PTR(GenericAttribute) anAttr;
  if ( !MonObj->FindAttribute(anAttr, "AttributeComment") ) return false;
// Quel type ?
  _PTR(AttributeComment) aComment (anAttr);
  QString Type = QString(aComment->Value().c_str());
//   MESSAGE("Type = "<<Type.toStdString().c_str());
// Est-ce le bon ?
  bool bOK = false ;
  if ( option == 0 )
  {
    int position = Type.lastIndexOf(TypeObject);
//     MESSAGE("position = "<<position);
    if ( position == 0 ) { bOK = true ; }
  }
  else
  {
    if ( Type == TypeObject ) { bOK = true ; }
  }
  return bOK ;
}
//================================================================
// Retourne vrai si l'objet est une frontiere CAO
//================================================================
bool SMESH_HOMARD_UTILS::isBoundaryCAO(_PTR(SObject) MonObj)
{
   return isObject( MonObj, QString("BoundaryCAOHomard"), -1 ) ;
}
//================================================================
// Retourne vrai si l'objet est une frontiere analytique
//================================================================
bool SMESH_HOMARD_UTILS::isBoundaryAn(_PTR(SObject) MonObj)
{
   return isObject( MonObj, QString("BoundaryAnHomard"), -1 ) ;
}
//================================================================
// Retourne vrai si l'objet est une frontiere discrete
//================================================================
bool SMESH_HOMARD_UTILS::isBoundaryDi(_PTR(SObject) MonObj)
{
   return isObject( MonObj, QString("BoundaryDiHomard"), -1 ) ;
}
//================================================================
// Retourne vrai si l'objet est un cas
//================================================================
bool SMESH_HOMARD_UTILS::isCase(_PTR(SObject) MonObj)
{
   return isObject( MonObj, QString("CasHomard"), -1 ) ;
}
//================================================================
// Retourne vrai si l'objet est une Hypothese
//================================================================
bool SMESH_HOMARD_UTILS::isHypo(_PTR(SObject) MonObj)
{
   return isObject( MonObj, QString("HypoHomard"), -1 ) ;
}
//================================================================
// Retourne vrai si l'objet est une iteration
//================================================================
bool SMESH_HOMARD_UTILS::isIter(_PTR(SObject) MonObj)
{
   return isObject( MonObj, QString("IterationHomard"), 0 ) ;
}
//================================================================
// Retourne vrai si l'objet est un fichier de type TypeFile
//================================================================
bool SMESH_HOMARD_UTILS::isFileType(_PTR(SObject) MonObj, QString TypeFile)
{
   return isObject( MonObj, TypeFile, 0 ) ;
}

// ============================================================================
QString SMESH_HOMARD_QT_COMMUN::SelectionArbreEtude(QString commentaire, int option )
// ============================================================================
// Retourne l'objet selectionne dans l'arbre d'etudes
// commentaire :
// . si le commentaire est une chaine vide, on ne tient pas compte du type de l'objet
//   et on retourne le nom de cet objet
// . sinon :
//   . si l'objet est du type defini par commentaire, retourne le nom de cet objet
//   . sinon on retourne une QString("")
// option :
// . Si option = 0, ce n'est pas grave de ne rien trouver ; aucun message n'est emis
// . Si option = 1, ce n'est pas grave de ne rien trouver mais on emet un message
{
//   MESSAGE("SelectionArbreEtude : commentaire = " << commentaire.toStdString().c_str() << " et option = " << option);
  int nbSel = SMESH_HOMARD_UTILS::IObjectCount() ;
  if ( nbSel == 0 )
  {
    if ( option == 1 )
    {
      QMessageBox::warning( 0, QObject::tr("HOM_WARNING"),
                               QObject::tr("HOM_SELECT_OBJECT_1") );
    }
    return QString("");
  }
  if ( nbSel > 1 )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_SELECT_OBJECT_2") );
    return QString("");
  }
//
  Handle(SALOME_InteractiveObject) aIO = SMESH_HOMARD_UTILS::firstIObject();
  if ( aIO->hasEntry() )
  {
//     MESSAGE("aIO->getEntry() = " << aIO->getEntry());
    _PTR(Study) aStudy = SMESH_HOMARD_UTILS::getStudy();
    _PTR(SObject) aSO ( aStudy->FindObjectID( aIO->getEntry() ) );
    _PTR(GenericAttribute) anAttr;
    if (aSO->FindAttribute(anAttr, "AttributeComment") )
    {
      if ( commentaire != "" )
      {
        _PTR(AttributeComment) attributComment = anAttr;
        QString aComment= QString(attributComment->Value().data());
//         MESSAGE("... aComment = " << aComment.toStdString().c_str());
        int iaux = aComment.lastIndexOf(commentaire);
//         MESSAGE("... iaux = " << iaux);
        if ( iaux !=0  )
        {
          QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                    QObject::tr("HOM_SELECT_OBJECT_3").arg(commentaire) );
          return QString("");
        }
      }
      if (aSO->FindAttribute(anAttr, "AttributeName") )
      {
        _PTR(AttributeName) attributName = anAttr;
        QString aName= QString(attributName->Value().data());
        return aName;
      }
    }
  }
//
  return QString("");
}

// =======================================================================
QString SMESH_HOMARD_QT_COMMUN::SelectionCasEtude()
// =======================================================================
{
  QString aName    = QString("");
  int nbSel = SMESH_HOMARD_UTILS::IObjectCount() ;
  if ( nbSel == 0 )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_SELECT_OBJECT_1") );
    return QString("");
  }
  if ( nbSel > 1 )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_SELECT_OBJECT_2") );
    return QString("");
  }
  Handle(SALOME_InteractiveObject) aIO = SMESH_HOMARD_UTILS::firstIObject();
  if ( aIO->hasEntry() )
  {
	_PTR(Study) aStudy = SMESH_HOMARD_UTILS::getStudy();
	_PTR(SObject) aSO ( aStudy->FindObjectID( aIO->getEntry() ) );
        _PTR(SObject) aSObjCas = aSO->GetFather();
	_PTR(GenericAttribute) anAttr;
	if (aSObjCas->FindAttribute(anAttr, "AttributeName") )
        {
            _PTR(AttributeName) attributName = anAttr;
            aName= QString(attributName->Value().data());
        }
        return aName;
    }
    return QString("");
}

// =======================================================================
QString SMESH_HOMARD_QT_COMMUN::PushNomFichier(bool avertir, QString TypeFichier)
// =======================================================================
// Gestion les boutons qui permettent  de
// 1) retourne le nom d'un fichier par une fenetre de dialogue si aucun
//    objet est selectionne dans l arbre d etude
// 2) retourne le nom du fichier asocie a l objet
//    selectionne dans l arbre d etude
{
//   MESSAGE("PushNomFichier avec avertir "<<avertir<<" et TypeFichier = "<<TypeFichier.toStdString().c_str());
  QString aFile = QString::null;
//
  // A. Filtre
  QString filtre  ;
//
  if ( TypeFichier == "med" )     { filtre = QString("Med") ; }
  else if ( TypeFichier == "py" ) { filtre = QString("Python") ; }
  else                            { filtre = TypeFichier ; }
  //
  if ( TypeFichier != "" ) { filtre += QString(" files (*.") + TypeFichier + QString(");;") ; }
  //
  filtre += QString("all (*) ") ;
  //
  // B. Selection
  //int nbSel = SMESH_HOMARD_UTILS::IObjectCount() ;
  int nbSel = 0;
  //   MESSAGE("nbSel ="<<nbSel);
  // B.1. Rien n'est selectionne
  if ( nbSel == 0 )
  {
    //aFile = QFileDialog::getOpenFileName(0, QObject::tr("HOM_SELECT_FILE_0"), QString(""), QString("Med files (*.med);;all (*) ") );
    aFile = QFileDialog::getOpenFileName(0, QObject::tr("HOM_SELECT_FILE_0"), QString(""), filtre );
  }
  // B.2. Un objet est selectionne
  else if (nbSel == 1)
  {
    Handle(SALOME_InteractiveObject) aIO = SMESH_HOMARD_UTILS::firstIObject();
    if ( aIO->hasEntry() )
    {
      _PTR(Study) aStudy = SMESH_HOMARD_UTILS::getStudy();
      _PTR(SObject) aSO ( aStudy->FindObjectID( aIO->getEntry() ) );
      _PTR(GenericAttribute) anAttr;
      _PTR(AttributeFileType) aFileType;
      _PTR(AttributeExternalFileDef) aFileName;
      if (aSO) {
        if (aSO->FindAttribute(anAttr, "AttributeFileType") ) {
          aFileType=anAttr;
          QString fileType=QString(aFileType->Value().data());
          if ( fileType==QString("FICHIERMED")) {
            if (aSO->FindAttribute(anAttr,"AttributeExternalFileDef")) {
            aFileName=anAttr;
            aFile= QString(aFileName->Value().data()); }
          }
        }
      }
    }

    if ( aFile==QString::null )
    {
      if ( avertir ) {
        QMessageBox::warning( 0, QObject::tr("HOM_WARNING"),
                                 QObject::tr("HOM_SELECT_STUDY") );
      }
      aFile = QFileDialog::getOpenFileName(0, QObject::tr("HOM_SELECT_FILE_0"), QString(""), filtre );
    }
  }
  // B.3. Bizarre
  else
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_SELECT_FILE_2") );
  }

  return aFile;
}
// =======================================================================
med_idt SMESH_HOMARD_QT_COMMUN::OuvrirFichier(QString aFile)
// =======================================================================
// renvoie le medId associe au fichier Med apres ouverture
{
  med_idt medIdt = MEDfileOpen(aFile.toStdString().c_str(),MED_ACC_RDONLY);
  if (medIdt <0)
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_MED_FILE_1") );
  }
  return medIdt;
}

// ======================================================
QString SMESH_HOMARD_QT_COMMUN::LireNomMaillage(QString aFile)
// ========================================================
{
  QString nomMaillage = "" ;
  int erreur = 0 ;
  med_idt medIdt ;
  while ( erreur == 0 )
  {
    //  Ouverture du fichier
    medIdt = SMESH_HOMARD_QT_COMMUN::OuvrirFichier(aFile);
    if ( medIdt < 0 )
    {
      erreur = 1 ;
      break ;
    }
    med_int numberOfMeshes = MEDnMesh(medIdt) ;
    if (numberOfMeshes == 0 )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_MED_FILE_2") );
      erreur = 2 ;
      break ;
    }
    if (numberOfMeshes > 1 )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_MED_FILE_3") );
      erreur = 3 ;
      break ;
    }

    nomMaillage = SMESH_HOMARD_QT_COMMUN::LireNomMaillage2(medIdt,1);
    break ;
  }
  // Fermeture du fichier
  if ( medIdt > 0 ) MEDfileClose(medIdt);

  return nomMaillage;
}
// =======================================================================
QString SMESH_HOMARD_QT_COMMUN::LireNomMaillage2(med_idt medIdt ,int meshId)
// =======================================================================
{
  QString NomMaillage=QString::null;
  char meshname[MED_NAME_SIZE+1];
  med_int spacedim,meshdim;
  med_mesh_type meshtype;
  char descriptionription[MED_COMMENT_SIZE+1];
  char dtunit[MED_SNAME_SIZE+1];
  med_sorting_type sortingtype;
  med_int nstep;
  med_axis_type axistype;
  int naxis = MEDmeshnAxis(medIdt,1);
  char *axisname=new char[naxis*MED_SNAME_SIZE+1];
  char *axisunit=new char[naxis*MED_SNAME_SIZE+1];
  med_err aRet = MEDmeshInfo(medIdt,
                          meshId,
                          meshname,
                          &spacedim,
                          &meshdim,
                          &meshtype,
                          descriptionription,
                          dtunit,
                          &sortingtype,
                          &nstep,
                          &axistype,
                          axisname,
                          axisunit);

  if ( aRet < 0 ) { QMessageBox::critical( 0, QObject::tr("HOM_ERROR"), \
                                              QObject::tr("HOM_MED_FILE_4") );  }
  else            { NomMaillage=QString(meshname); }

  delete[] axisname ;
  delete[] axisunit ;

  return NomMaillage;
}


// =======================================================================
std::list<QString> SMESH_HOMARD_QT_COMMUN::GetListeChamps(QString aFile)
// =======================================================================
{
// Il faut voir si plusieurs maillages

  MESSAGE("GetListeChamps");
  std::list<QString> ListeChamp ;

  med_err erreur = 0 ;
  med_idt medIdt ;

  while ( erreur == 0 )
  {
    // Ouverture du fichier
    SCRUTE(aFile.toStdString());
    medIdt = SMESH_HOMARD_QT_COMMUN::OuvrirFichier(aFile);
    if ( medIdt < 0 )
    {
      erreur = 1 ;
      break ;
    }
  // Lecture du nombre de champs
    med_int ncha = MEDnField(medIdt) ;
    if (ncha < 1 )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_MED_FILE_5") );
      erreur = 2 ;
      break ;
    }
  // Lecture des caracteristiques des champs
    for (int i=0; i< ncha; i++)
    {
//       Lecture du nombre de composantes
      med_int ncomp = MEDfieldnComponent(medIdt,i+1);
//       Lecture du type du champ, des noms des composantes et du nom de l'unite
      char nomcha  [MED_NAME_SIZE+1];
      char meshname[MED_NAME_SIZE+1];
      char * comp = (char*) malloc(ncomp*MED_SNAME_SIZE+1);
      char * unit = (char*) malloc(ncomp*MED_SNAME_SIZE+1);
      char dtunit[MED_SNAME_SIZE+1];
      med_bool local;
      med_field_type typcha;
      med_int nbofcstp;
      erreur = MEDfieldInfo(medIdt,i+1,nomcha,meshname,&local,&typcha,comp,unit,dtunit,&nbofcstp) ;
      free(comp);
      free(unit);
      if ( erreur < 0 )
      {
        QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                  QObject::tr("HOM_MED_FILE_6") );
        break ;
      }
      ListeChamp.push_back(QString(nomcha));
    }
    break ;
  }
  // Fermeture du fichier
  if ( medIdt > 0 ) MEDfileClose(medIdt);

  return ListeChamp;
}

// ======================================================================================
std::list<QString> SMESH_HOMARD_QT_COMMUN::GetListeComposants(QString aFile, QString aChamp)
// ======================================================================================
{
  MESSAGE ( "GetListeComposants pour le fichier " << aFile.toStdString().c_str());
  MESSAGE ( "GetListeComposants pour le champ " << aChamp.toStdString().c_str());

  std::list<QString> ListeComposants;

  med_err erreur = 0 ;
  med_idt medIdt ;

  while ( erreur == 0 )
  {
    // Ouverture du fichier
    SCRUTE(aFile.toStdString());
    medIdt = SMESH_HOMARD_QT_COMMUN::OuvrirFichier(aFile);
    if ( medIdt < 0 )
    {
      erreur = 1 ;
      break ;
    }
  // Lecture du nombre de champs
    med_int ncha = MEDnField(medIdt) ;
    if (ncha < 1 )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_MED_FILE_5") );
      erreur = 2 ;
      break ;
    }
  // Lecture des caracteristiques des champs
    for (int i=0; i< ncha; i++)
    {
//       Lecture du nombre de composantes
      med_int ncomp = MEDfieldnComponent(medIdt,i+1);
//       Lecture du type du champ, des noms des composantes et du nom de l'unite
      char nomcha  [MED_NAME_SIZE+1];
      char meshname[MED_NAME_SIZE+1];
      char * comp = (char*) malloc(ncomp*MED_SNAME_SIZE+1);
      char * unit = (char*) malloc(ncomp*MED_SNAME_SIZE+1);
      char dtunit[MED_SNAME_SIZE+1];
      med_bool local;
      med_field_type typcha;
      med_int nbofcstp;
      erreur = MEDfieldInfo(medIdt,i+1,nomcha,meshname,&local,&typcha,comp,unit,dtunit,&nbofcstp) ;
      free(unit);
      if ( erreur < 0 )
      {
        free(comp);
        QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                  QObject::tr("HOM_MED_FILE_6") );
        break ;
      }
      // Lecture des composantes si c'est le bon champ
      if ( QString(nomcha) == aChamp )
      {
        for (int j = 0; j <ncomp; j++)
        {
          char cible[MED_SNAME_SIZE +1];
          strncpy(cible,comp+j*MED_SNAME_SIZE,MED_SNAME_SIZE );
          cible[MED_SNAME_SIZE ]='\0';
          ListeComposants.push_back(QString(cible));
        }
      }
      // Menage
      free(comp);
      // Sortie si c'est bon
      if ( QString(nomcha) == aChamp ) { break ; }
    }
    break ;
  }
  // Fermeture du fichier
  if ( medIdt > 0 ) MEDfileClose(medIdt);

  return ListeComposants;
}
