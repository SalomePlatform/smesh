// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

// SMESH SMESHGUI : reading of xml file with list of available hypotheses and algorithms
// File   : SMESHGUI_XmlHandler.cxx
// Author : Julia DOROVSKIKH, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_XmlHandler.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Hypotheses.h"

// SALOME GUI includes
#include "SUIT_ResourceMgr.h"

// SALOME KERNEL includes
#include <utilities.h>

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
  return true;
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
/* It's Need to tranlate lib name for WIN32 or X platform
 * (only client lib, because server lib translates in SMESH_Gen_i::createHypothesis
 *  for normal work of *.py files )
 */
      if( !myClientLib.isEmpty() )
      {
#ifdef WNT
      //myServerLib += ".dll";
        myClientLib += ".dll";
#else
      //myServerLib = "lib" + myServerLib + ".so";
        myClientLib = "lib" + myClientLib + ".so";
#endif
      }


      QString aResName = atts.value("resources");
      if (aResName != "")
      {
        MESSAGE("Loading Resources " << aResName.toLatin1().data());
        SUIT_ResourceMgr* resMgr = SMESHGUI::resourceMgr();
        QString lang = resMgr->stringValue( resMgr->langSection(), "language", "en" );
        resMgr->loadTranslator( "resources", QString( "%1_msg_%2.qm" ).arg( aResName, lang ) );
        resMgr->loadTranslator( "resources", QString( "%1_images.qm" ).arg( aResName, lang ) );
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
      bool isNeedGeom = true, isSupportSubmeshes = false;
      QString aNeedGeom = atts.value("need-geom");
      if ( !aNeedGeom.isEmpty() )
        isNeedGeom = (aNeedGeom == "true");
      QString suppSub = atts.value("support-submeshes");
      if ( !suppSub.isEmpty() )
        isSupportSubmeshes = (suppSub == "true");

      QString aDimStr = atts.value("dim");
      aDimStr = aDimStr.remove( ' ' );
      QStringList aDimList = aDimStr.split( ',', QString::SkipEmptyParts );
      QStringList::iterator anIter;
      bool isOk;
      QList<int> aDim;
      for ( anIter = aDimList.begin(); anIter != aDimList.end(); ++anIter )
      {
        int aVal = (*anIter).toInt( &isOk );
        if ( isOk )
          aDim.append( aVal );
      }

      // for algo
      enum { HYPOS = 0, OPT_HYPOS, INPUT, OUTPUT, NB_ATTRIBUTES };
      const char* name [NB_ATTRIBUTES] = { "hypos", "opt-hypos", "input", "output" };
      QStringList attr [NB_ATTRIBUTES];
      for ( int i = 0; i < NB_ATTRIBUTES; ++i ) {
        QString aStr = atts.value( name[i] );
        if ( !aStr.isEmpty() ) {
          aStr.remove( ' ' );
          attr[ i ] = aStr.split( ',', QString::SkipEmptyParts );
        }
      }
      
      HypothesisData* aHypData =
        new HypothesisData (aHypAlType, myPluginName, myServerLib, myClientLib,
                            aLabel, anIcon, aDim, isAux,
                            attr[ HYPOS ], attr[ OPT_HYPOS ], attr[ INPUT ], attr[ OUTPUT ],
                            isNeedGeom, isSupportSubmeshes );

      if (qName == "algorithm")
      {
        myAlgorithmsMap.insert(aHypAlType,aHypData);
      }
      else
      {
        myHypothesesMap.insert(aHypAlType,aHypData);
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
      myListOfHypothesesSets.append( aHypoSet );

      for ( int isHypo = 0; isHypo < 2; ++isHypo )
      {
        QString aHypos = isHypo ? atts.value("hypos") : atts.value("algos");
        aHypos = aHypos.remove( ' ' );
        aHypoSet->set( !isHypo, aHypos.split( ',', QString::SkipEmptyParts ) );
      }
    }
  }
  else if ( qName == "python-wrap" ||
            qName == "algo"        ||
            qName == "hypo"         )
  {
    // elements used in SMESH_2smeshpy
    return true;
  }
  else
  {
    // error
    return false;
  }
  return true;
}


/*!
  Reimplemented from QXmlDefaultHandler.
*/
bool SMESHGUI_XmlHandler::endElement (const QString&, const QString&, const QString&)
{
  return true;
}


/*!
  Reimplemented from QXmlDefaultHandler.
*/
bool SMESHGUI_XmlHandler::characters (const QString& ch)
{
  // we are not interested in whitespaces
  QString ch_simplified = ch.simplified();
  if ( ch_simplified.isEmpty() )
    return true;
  return true;
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
