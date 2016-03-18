// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// ---
// File    : GeomSelectionTools.cxx
// Authors : Nicolas GEIMER (OCC)
// ---

#include "GeomSelectionTools.h"

#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Application.h>
#include <SUIT_Session.h>

#include <GEOMImpl_Types.hxx>
#include <GEOM_Client.hxx>
#include <GEOM_wrap.hxx>
#include <SALOME_ListIO.hxx>
#include <SMESHGUI_Utils.h>

#include <BRepAdaptor_Surface.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>

#include "utilities.h"

#include "SALOME_LifeCycleCORBA.hxx"
#include <sstream>

/*!
 * Constructor
 * @param aStudy pointer to the Study
 *
 */
GeomSelectionTools::GeomSelectionTools(_PTR(Study) aStudy)
{
  myStudy = aStudy;
}

/*!
 * Accessor to the Study used by this GeomSelectionTools object
 * @return The study used by the GeomSelectionTools class
 */
_PTR(Study) GeomSelectionTools::getMyStudy()
{
    return myStudy;
}

/*!
 * Allows to get the Salome Application
 * @return A LightApp_SelectionMgr Pointer or 0 if it can't get it.
 */
SalomeApp_Application*  GeomSelectionTools::GetSalomeApplication()
{
  SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
  if (anApp)
    return anApp;
  else
    return 0;
}

/*!
 * Allows to get the selection manager from LightApp
 * @return A LightApp_SelectionMgr Pointer or 0 if it can't get it.
 */
LightApp_SelectionMgr*  GeomSelectionTools::selectionMgr()
{
   SalomeApp_Application* anApp = GetSalomeApplication();
   if (anApp)
     return dynamic_cast<LightApp_SelectionMgr*>( anApp->selectionMgr() );
   else
     return 0;
}

/*!
 * Return the list of the selected Salome Interactive Object (SALOME_ListIO*)
 * @return the list of the selected Salome Interactive Object
 */
SALOME_ListIO* GeomSelectionTools::getSelectedSalomeObjects()
{
  SALOME_ListIO* selected = new SALOME_ListIO;
  LightApp_SelectionMgr* aSel = selectionMgr();
  aSel->selectedObjects( *selected, NULL, false );
  return selected;
}

/*!
 * Return the first selected Salome Interactive Object (Handle(Salome_InteractiveObject))
 * @return the first selected Salome Interactive Object
 */
Handle(SALOME_InteractiveObject) GeomSelectionTools::getFirstSelectedSalomeObject()
{
  SALOME_ListIO selected;
  LightApp_SelectionMgr* aSel = selectionMgr();
  aSel->selectedObjects( selected, NULL, false );
  if (!selected.IsEmpty()){
    SALOME_ListIteratorOfListIO anIt(selected);
    Handle(SALOME_InteractiveObject) anIO;
    anIO = anIt.Value();
    return anIO;
  }
  return NULL;
}

/*!
 * Return the entry of the first selected Object
 * @return the entry of the first selected Object
 */
std::string GeomSelectionTools::getFirstSelectedEntry()
{
  Handle(SALOME_InteractiveObject) anIO;
  std::string entry="";
  anIO=GeomSelectionTools::getFirstSelectedSalomeObject();
  return GeomSelectionTools::getEntryOfObject(anIO);
}

/*!
 * Return the entry of a Salome Interactive Object
 * @param anIO the Handle of the Salome Interactive Object
 * @return the entry of the Salome Interactive Object
 */
std::string GeomSelectionTools::getEntryOfObject(Handle(SALOME_InteractiveObject) anIO){
  std::string entry="";
  _PTR(SObject) aSO = myStudy->FindObjectID(anIO->getEntry());
  if (aSO){
    _PTR(SObject) aRefSObj;
    // If selected object is a reference
    if ( aSO->ReferencedObject( aRefSObj ))
      entry = aRefSObj->GetID();
    // If selected object is a reference is not a reference
    else
      entry= anIO->getEntry();
  }
  return entry;
}

/*!
 * Retrieve the name from the entry of the object
 * @param entry the entry of the object
 * @return the name of the object
 */
std::string GeomSelectionTools::getNameFromEntry(std::string entry){
  std::string name = "";
  _PTR(SObject) aSO = myStudy->FindObjectID(entry);
  if (aSO){
    _PTR(SObject) aRefSObj;
    // If selected object is a reference
    if ( aSO->ReferencedObject( aRefSObj ))
      name = aRefSObj->GetName();
    // If selected object is a reference is not a reference
    else
      name = aSO->GetName();
   }
  return name;
}


/*!
 * Retrieve the component type of the first selected object, it manages successfully references.
 * @return the component type of the first selected object
 */
std::string GeomSelectionTools::getFirstSelectedComponentDataType()
{
  Handle(SALOME_InteractiveObject) anIO;
  std::string DataType="";
  anIO=GeomSelectionTools::getFirstSelectedSalomeObject();
  _PTR(SObject) aSO = myStudy->FindObjectID(anIO->getEntry());
  if (aSO){
    _PTR(SObject) aRefSObj;
    // If selected object is a reference
    if ( aSO->ReferencedObject( aRefSObj ))
      DataType= aRefSObj->GetFatherComponent()->ComponentDataType();
    // If selected object is a reference is not a reference
    else
      DataType=anIO->getComponentDataType();
 }
 return DataType;
}

/*!
 * Retrieve the shape type from the entry
 * @return the shape type from the entry, return TopAbs_SHAPE if the object does not define a shape or a group.
 */
TopAbs_ShapeEnum GeomSelectionTools::entryToShapeType(std::string entry){
//   MESSAGE("GeomSelectionTools::entryToShapeType"<<entry );
  TopoDS_Shape S = TopoDS_Shape();
  TopAbs_ShapeEnum ShapeType = TopAbs_SHAPE;
   _PTR(SObject) aSO = myStudy->FindObjectID(entry);
  if (aSO){
    _PTR(SObject) aRefSObj;
    GEOM::GEOM_Object_var aShape;
    // MESSAGE("Got a SO");
    // If selected object is a reference
    if ( aSO->ReferencedObject( aRefSObj ))
      aSO = aRefSObj;
    // MESSAGE("aSO->GetFatherComponent()->ComponentDataType(): " << aSO->GetFatherComponent()->ComponentDataType());
    if (  strcmp(aSO->GetFatherComponent()->ComponentDataType().c_str(),"GEOM") == 0)
      aShape = SMESH::SObjectToInterface<GEOM::GEOM_Object>(aSO);
    if ( !aShape->_is_nil() ){
      // MESSAGE("Got the Geom Object ");
      // MESSAGE("Geom Object Type "<< aShape->GetType());
      SalomeApp_Application* anApp = GetSalomeApplication();
      if (anApp) {
//         MESSAGE("Got Application");
        Engines::EngineComponent_var component = anApp->lcc()->FindOrLoad_Component( "FactoryServer","GEOM" );
        GEOM::GEOM_Gen_var _geomEngine = GEOM::GEOM_Gen::_narrow(component);
//         MESSAGE("Got GEOM engine");
        // if the Geom Object is a group
        if (aShape->GetType() == GEOM_GROUP){
//           MESSAGE("It's a group");
          GEOM::GEOM_IGroupOperations_wrap aGroupOp =
            _geomEngine->GetIGroupOperations(myStudy->StudyId());
          ShapeType= (TopAbs_ShapeEnum)aGroupOp->GetType(aShape);
        }
        // if not
        else {
          GEOM_Client* aClient = new GEOM_Client();
          if ( aClient && !_geomEngine->_is_nil() ) {
//             MESSAGE("GEOM client is OK and GEOM engine is not null");
            S = aClient->GetShape( _geomEngine, aShape );
            ShapeType=S.ShapeType();
            if ( ShapeType == TopAbs_COMPOUND )
            {
              TopoDS_Iterator it( S );
              if ( it.More() )
                ShapeType = it.Value().ShapeType();
            }
          }
        }
      }
    }
  }
//   MESSAGE("ShapeType returned is " << ShapeType);
  return ShapeType;
}

/*!
 * Gives the ShapeType of the first Selected Object, return TopAbs_SHAPE if the first selected object does not define a shape.
 * @return the ShapeType of the first Selected Object, return TopAbs_SHAPE if the first selected object does not define a shape.
 */
TopAbs_ShapeEnum GeomSelectionTools:: getFirstSelectedShapeType()
{
 Handle(SALOME_InteractiveObject) anIO;
 anIO=GeomSelectionTools::getFirstSelectedSalomeObject();
 return entryToShapeType(anIO->getEntry());
}

/*!
 *  Print information to std output of the face
 *  and return the OCC type of face: Plane, Cylinder,Cone, Sphere, Torus, BezierSurface,BSplineSurface, SurfaceOfRevolution,SurfaceOfExtrusion, OtherSurface
 *  @param TopoDS_Shape S Face we want information about.
 *  @return the OCC type of face: Plane, Cylinder,Cone, Sphere, Torus, BezierSurface,BSplineSurface, SurfaceOfRevolution,SurfaceOfExtrusion, OtherSurface
 *  return Other_Surface if the selected face is not a face.
 *  Information printed is :
 *  U and V degrees
 *  U and V number of poles
 *  U and V number of knots
 *  U or V is Rational ?
 *
 */
GeomAbs_SurfaceType GeomSelectionTools::getFaceInformation(TopoDS_Shape S)
{
  GeomAbs_SurfaceType surf_type=GeomAbs_OtherSurface ;
  if (!S.IsNull() &&  S.ShapeType()==TopAbs_FACE){
    TopoDS_Face f=TopoDS::Face(S);
    BRepAdaptor_Surface surf_adap(f);

    /* Global Information */
    std::cout << "GLOBAL INFORMATION" << std::endl;
    std::cout << "******************" << std::endl;
    std::stringstream buffer;
    buffer << "Degre U : " <<  surf_adap.UDegree();
   //conversion nécessaire pour affichage
    std::cout << buffer.str() << std::endl;
    std::cout <<  " Degre V : " <<  surf_adap.VDegree() << std::endl;
    std::cout <<  " Nb Poles U : " <<  surf_adap.NbUPoles() << std::endl;
    std::cout <<  " Nb Poles V : " <<  surf_adap.NbVPoles() << std::endl;
    std::cout <<  " Nb Noeuds U : " <<  surf_adap.NbUKnots() << std::endl;
    std::cout <<  " Nb Noeuds V : " <<  surf_adap.NbVKnots() << std::endl;
    std::cout <<  " U Rationnel ? " <<  surf_adap.IsURational() << std::endl;
    std::cout <<  " V Rationnel ? " <<  surf_adap.IsVRational() << std::endl;

    surf_type=surf_adap.GetType();
  }
  return surf_type;
}


//////////////////////////////////////////
// Utility functions
//////////////////////////////////////////
#include <QLocale>
#include <QRegExp>

QString PluginUtils::PrintDoubleValue( double theValue, int thePrecision )
{
  const double prec = 1e-12;

  if ( qAbs(theValue) < prec )
    return "0";

  QString aRes = QLocale().toString( theValue, thePrecision >= 0 ? 'f' : 'g', qAbs( thePrecision ) );

  if ( prec > 0 ) {
    int p = 0;
    while ( p < thePrecision ) {
      QString aRes = QLocale().toString( theValue, thePrecision >= 0 ? 'f' : 'g', qAbs( p++ ) );
      double v = aRes.toDouble();
      double err = qAbs( theValue - v );
      if ( err > 0 && err <= prec )
        break;
    }
  }

  // remove trailing zeroes

  QRegExp expre( QString( "(%1|%2)[+-]?[0-9]+$" ).arg( QLocale().exponential().toLower(),
                               QLocale().exponential().toUpper() ) );

  int idx = aRes.indexOf( expre );
  QString aResExp = "";
  if ( idx >= 0 ) {
    aResExp = aRes.mid( idx );
    aRes = aRes.left( idx );
  }

  if ( aRes.contains( QLocale().decimalPoint() ) )
    aRes.remove( QRegExp( QString( "(\\%1|0)0*$" ).arg( QLocale().decimalPoint() ) ) );

  return aRes == "-0" ? QString( "0" ) : aRes + aResExp;
}
