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

#include "HomardQtCommun.h"
#include "HOMARDGUI_Utils.h"

#include <utilities.h>

#include <qmessagebox.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qstring.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <sys/stat.h>


#include "SalomeApp_Tools.h"

using namespace std;

#include <med.h>

// ============================================================================
QString HOMARD_QT_COMMUN::SelectionArbreEtude(QString commentaire, int option )
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
  int nbSel = HOMARD_UTILS::IObjectCount() ;
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
  Handle(SALOME_InteractiveObject) aIO = HOMARD_UTILS::firstIObject();
  if ( aIO->hasEntry() )
  {
//     MESSAGE("aIO->getEntry() = " << aIO->getEntry());
    _PTR(Study) aStudy = HOMARD_UTILS::getStudy();
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
QString HOMARD_QT_COMMUN::SelectionCasEtude()
// =======================================================================
{
  QString aName    = QString("");
  int nbSel = HOMARD_UTILS::IObjectCount() ;
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
  Handle(SALOME_InteractiveObject) aIO = HOMARD_UTILS::firstIObject();
  if ( aIO->hasEntry() )
  {
	_PTR(Study) aStudy = HOMARD_UTILS::getStudy();
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
QString HOMARD_QT_COMMUN::PushNomFichier(bool avertir, QString TypeFichier)
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
  int nbSel = HOMARD_UTILS::IObjectCount() ;
//   MESSAGE("nbSel ="<<nbSel);
  // B.1. Rien n'est selectionne
  if ( nbSel == 0 )
  {
//     aFile = QFileDialog::getOpenFileName(0, QObject::tr("HOM_SELECT_FILE_0"), QString(""), QString("Med files (*.med);;all (*) ") );
    aFile = QFileDialog::getOpenFileName(0, QObject::tr("HOM_SELECT_FILE_0"), QString(""), filtre );
  }
  // B.2. Un objet est selectionne
  else if (nbSel == 1)
  {
    Handle(SALOME_InteractiveObject) aIO = HOMARD_UTILS::firstIObject();
    if ( aIO->hasEntry() )
    {
      _PTR(Study) aStudy = HOMARD_UTILS::getStudy();
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
med_idt HOMARD_QT_COMMUN::OuvrirFichier(QString aFile)
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
QString HOMARD_QT_COMMUN::LireNomMaillage(QString aFile)
// ========================================================
{
  QString nomMaillage = "" ;
  int erreur = 0 ;
  med_idt medIdt ;
  while ( erreur == 0 )
  {
    //  Ouverture du fichier
    medIdt = HOMARD_QT_COMMUN::OuvrirFichier(aFile);
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

    nomMaillage = HOMARD_QT_COMMUN::LireNomMaillage2(medIdt,1);
    break ;
  }
  // Fermeture du fichier
  if ( medIdt > 0 ) MEDfileClose(medIdt);

  return nomMaillage;
}
// =======================================================================
QString HOMARD_QT_COMMUN::LireNomMaillage2(med_idt medIdt ,int meshId)
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
std::list<QString> HOMARD_QT_COMMUN::GetListeChamps(QString aFile)
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
    medIdt = HOMARD_QT_COMMUN::OuvrirFichier(aFile);
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
std::list<QString> HOMARD_QT_COMMUN::GetListeComposants(QString aFile, QString aChamp)
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
    medIdt = HOMARD_QT_COMMUN::OuvrirFichier(aFile);
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
