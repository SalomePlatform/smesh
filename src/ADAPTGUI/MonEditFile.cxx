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

#include "MonEditFile.h"

#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include <utilities.h>

using namespace std;

/* ---------------------------------------------------------
 * MonEditFile classe derivee de EditFile
 *               elle meme generee par uic
 * Ouvre le fichier passe en parametre
 * et affiche le texte correspondant dans la fenetre de log
 * ---------------------------------------------------------
 */

/* ---------------------------------------------------------
 * MonEditFile Constructeur
 * ---------------------------------------------------------
 */
MonEditFile::MonEditFile( QWidget* parent,  bool modal,
                          HOMARD::HOMARD_Gen_var myHomardGen,
                          QString aFileName, int option):
//    QWidget(0),
    Ui_EditFile(),
    _aFileName (aFileName),
    _option (option),
    _codret (0)
{
  MESSAGE("Debut de MonEditFile " << aFileName.toStdString().c_str());
  setupUi(this);
  InitConnect();
  EditText();
}
/*
 *  Destroys the object and frees any allocated resources
 */
MonEditFile::~MonEditFile()
{
   MESSAGE("Destructeur de ~MonEditFile");
}
// ------------------------------------------------------------------------
void MonEditFile::InitConnect()
// ------------------------------------------------------------------------
{
    connect( buttonQuit,     SIGNAL(pressed()), this, SLOT(close()));
    connect( buttonPrint,    SIGNAL(pressed()), this, SLOT(PushOnPrint()));
}
// ------------------------------------------------------------------------
void MonEditFile::EditText()
// ------------------------------------------------------------------------
{
// Creation de l'objet fichier QT associe
  QFile file( _aFileName );
// Ouverture
  bool bOpen = file.open( QIODevice::ReadOnly | QIODevice::Text ) ;
//
  if ( bOpen )
  {
// Lecture
//    Remarque : il serait plus clair de tout lire d'un coup mais cela ne marche pas !
//               alors on fait ligne par ligne et on cumule en ajoutant un saut de ligne.
    QTextStream stream( &file );
    QString tout;
    while ( !stream.atEnd() )
    {
      tout = tout + stream.readLine() + "\n" ;
    }
//       tout = stream.readAll() ;
    QTBEditFile->setPlainText( tout );
  }
  else
  {
    // Option = 0 : emission d'un message d'erreur
    if ( _option == 0 )
    {
      MESSAGE( "EditText " << _aFileName.toStdString().c_str() << " est impossible a ouvrir ");
      QMessageBox::warning( 0, QObject::tr("HOM_WARNING"),
                              QObject::tr("HOM_SELECT_FILE_3") );
    }
    // Sinon : rien
    _codret = 1 ;
  }
}
// ------------------------------------------------------------------------
void MonEditFile::PushOnPrint()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de MonEditFile::PushOnPrint")
  QMessageBox::warning( 0, QObject::tr("HOM_WARNING"),
                            QObject::tr("HOM_INACTIVE_BUTTON") );
  return;
}


