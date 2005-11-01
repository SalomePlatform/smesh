//  SMESH SMESHGUI : reading of xml file with list of available hypotheses and algorithms
//  Copyright (C) 2003  CEA
//
//  This library is free software; you can redistribute it and/or
// //  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//

//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_XmlHandler.cxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#define  INCLUDE_MENUITEM_DEF 

// QT Include
#include <qfileinfo.h>
#include <qstringlist.h>

#include "SMESHGUI.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_Desktop.h"

#include "SMESHGUI_XmlHandler.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_Utils.h"

#include "utilities.h"

using namespace std;

/*!
  Constructor
*/
SMESHGUI_XmlHandler::SMESHGUI_XmlHandler()
{
}

/*!
  Destructor
*/
SMESHGUI_XmlHandler::~SMESHGUI_XmlHandler()
{
}

/*!
  Starts parsing of document. Does some initialization

  Reimplemented from QXmlDefaultHandler.
*/
bool SMESHGUI_XmlHandler::startDocument()
{
  myErrorProt = "";
  return TRUE;
}

/*!
  Does different actions depending on the name of the tag and the
  state you are in document.

  Reimplemented from QXmlDefaultHandler.
*/
bool SMESHGUI_XmlHandler::startElement (const QString&, const QString&, 
                                        const QString& qName, 
                                        const QXmlAttributes& atts)
{
  if (qName == "meshers")
  {
    myHypothesesMap.clear();
    myAlgorithmsMap.clear();
  }
  else if (qName == "meshers-group") // group of hypotheses and algorithms
  {
//    if (atts.value("server-lib") != "")
    {
      myPluginName = atts.value("name");
      myServerLib  = atts.value("server-lib");
      myClientLib  = atts.value("gui-lib");

      QString aResName = atts.value("resources");
      if (aResName != "")
      {
        MESSAGE("Loading Resources " << aResName);
        SUIT_ResourceMgr* resMgr = SMESHGUI::resourceMgr();
        resMgr->loadTranslator("resources",aResName+"_msg_en.qm");
	resMgr->loadTranslator("resources",aResName+"_images.qm");
      }
    }
  }
  else if (qName == "hypotheses") // group of hypotheses
  {
  }
  else if (qName == "algorithms") // group of algorithms
  {
  }
  else if (qName == "hypothesis" || qName == "algorithm") // hypothesis or algorithm
  {
    if (atts.value("type") != "")
    {
      QString aHypAlType = atts.value("type");
      QString aLabel = atts.value("label-id");
      QString anIcon = atts.value("icon-id");
      bool isAux = atts.value("auxiliary") == "true";
      
      QString aDimStr = atts.value("dim");
      aDimStr = aDimStr.remove( ' ' );
      QStringList aDimList = QStringList::split( ',', aDimStr );
      QStringList::iterator anIter;
      bool isOk;
      QValueList<int> aDim;
      for ( anIter = aDimList.begin(); anIter != aDimList.end(); ++anIter )
      {
        int aVal = (*anIter).toInt( &isOk );
        if ( isOk )
          aDim.append( aVal - 1 );
      }
      
      HypothesisData* aHypLibNames =
        new HypothesisData (myPluginName, myServerLib, myClientLib,
                            aLabel, anIcon, aDim, isAux );

      if (qName == "algorithm")
      {
        myAlgorithmsMap[(char*)aHypAlType.latin1()] = aHypLibNames;
      }
      else
      {
        myHypothesesMap[(char*)aHypAlType.latin1()] = aHypLibNames;
      }
    }
  }
  else if (qName == "hypotheses-set-group") // group of sets of hypotheses
  {
  }
  else if (qName == "hypotheses-set") // a set of hypotheses
  {
    if (atts.value("name") != "")
    {
      HypothesesSet* aHypoSet = new HypothesesSet ( atts.value("name") );
      myListOfHypothesesSets.push_back( aHypoSet );

      for ( int isHypo = 0; isHypo < 2; ++isHypo )
      {
        QString aHypos = isHypo ? atts.value("hypos") : atts.value("algos");
        aHypos = aHypos.remove( ' ' );
        QStringList* aHypoList = isHypo ? & aHypoSet->HypoList : & aHypoSet->AlgoList;
        *aHypoList = QStringList::split( ',', aHypos );
      }
    }
  }
  else
  {
    // error
    return FALSE;
  }
  return TRUE;
}


/*!
  Reimplemented from QXmlDefaultHandler.
*/
bool SMESHGUI_XmlHandler::endElement (const QString&, const QString&, const QString&)
{
  return TRUE;
}


/*!
  Reimplemented from QXmlDefaultHandler.
*/
bool SMESHGUI_XmlHandler::characters (const QString& ch)
{
  // we are not interested in whitespaces
  QString ch_simplified = ch.simplifyWhiteSpace();
  if ( ch_simplified.isEmpty() )
    return TRUE;
  return TRUE;
}


/*!
  Returns the default error string.

  Reimplemented from QXmlDefaultHandler.
*/
QString SMESHGUI_XmlHandler::errorString()
{
  return "the document is not in the quote file format";
}

/*!
  Returns the error protocol if parsing failed

  Reimplemented from QXmlDefaultHandler.
*/
QString SMESHGUI_XmlHandler::errorProtocol()
{
  return myErrorProt;
}

/*!
  Returns exception

  Reimplemented from QXmlDefaultHandler.
*/
bool SMESHGUI_XmlHandler::fatalError (const QXmlParseException& exception)
{
  myErrorProt += QString("fatal parsing error: %1 in line %2, column %3\n")
    .arg(exception.message())
    .arg(exception.lineNumber())
    .arg(exception.columnNumber());
  
  return QXmlDefaultHandler::fatalError( exception );
}
