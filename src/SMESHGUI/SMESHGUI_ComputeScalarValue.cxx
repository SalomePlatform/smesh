using namespace std;
//  File      : SMESHGUI_ComputeScalarValue.cxx
//  Created   : Mon Jun 24 14:06:00 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#include "SMESHGUI_ComputeScalarValue.h"
#include "utilities.h"
#include <math.h>




//=============================================================================
/*!
 *
 */
//=============================================================================
static double ComputeLength(float* p1, float* p2) {
  float a1,a2,a3,b1,b2,b3;
  a1 =  p1[0];
  a2 =  p1[1];
  a3 =  p1[2];
  b1 =  p2[0];
  b2 =  p2[1];
  b3 =  p2[2];
  // MESSAGE( a1 << " "<< a2 << " "<< a3 << " " << b1 << " "<< b2 << " "<< b3 );
  float X1,Y1,Z1,X2,Z2,Y2;
  X1 = b1 - a1;
  Y1 = b2 - a2;
  Z1 = b3 - a3;
  // MESSAGE( X1 << " "<< Y1 << " "<< Z1 );
  float e1;
  e1 = sqrt( X1*X1 + Y1*Y1 + Z1*Z1 ) ;
  // MESSAGE( "Length = " << e1 );
  return e1;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
double SMESHGUI_ComputeScalarValue::LengthEdges(vtkCell* theCell) {
  int num_points = theCell->GetNumberOfPoints ();
  vtkPoints* points = theCell->GetPoints();
  if (num_points != 2 ) return 0;
  float* pnt1 = points->GetPoint(0);
  float* pnt2 = points->GetPoint(1);
  return ComputeLength(pnt1,pnt2);
};

//=============================================================================
/*!
 *
 */
//=============================================================================
static double ComputeAreaOfTriangle(float* p1, float* p2, float* p3) {
  double a1,a2,a3,b1,b2,b3,c1,c2,c3;
  a1 =  p1[0];
  a2 =  p1[1];
  a3 =  p1[2];
  b1 =  p2[0];
  b2 =  p2[1];
  b3 =  p2[2];
  c1 =  p3[0];
  c2 =  p3[1];
  c3 =  p3[2];
  
  float e1, e2, e3;
  e1 = sqrt( (a1-b1)*(a1-b1) + (a2-b2)*(a2-b2) + (a3-b3)*(a3-b3) ) ;
  e2 = sqrt( (b1-c1)*(b1-c1) + (b2-c2)*(b2-c2) + (b3-c3)*(b3-c3) ) ;
  e3 = sqrt( (c1-a1)*(c1-a1) + (c2-a2)*(c2-a2) + (c3-a3)*(c3-a3) ) ;
  
  // MESSAGE( "e = " << e1 << " " << e2 <<"  " << e3 );
  float s = (e1+e2+e3)/2;
  double area = sqrt(s*(s-e1)*(s-e2)*(s-e3));
  // MESSAGE( "area = " << area );
  return area;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
double SMESHGUI_ComputeScalarValue::AreaElements(vtkCell* theCell){
  // MESSAGE ( " SMESHGUI_ComputeScalarValue::AreaElements " )
  int num_points = theCell->GetNumberOfPoints ();
  vtkPoints* points = theCell->GetPoints();
  // MESSAGE( "num_points =  "<< num_points );
  for (int j = 0; j < theCell->GetNumberOfPoints (); j++) {
    float* pnt = points->GetPoint(j);
    // MESSAGE( pnt[0] << " " << pnt[1] << " " <<  pnt[2] );
  }
  if (num_points < 3 ) return 0;
  if (num_points == 3) {
    float* p1 = points->GetPoint(0);
    float* p2 = points->GetPoint(1);
    float* p3 = points->GetPoint(2);
    double area = ComputeAreaOfTriangle(p1,p2,p3);
    return area;
  } else if (num_points == 4) {
    float* p1 = points->GetPoint(0);
    float* p2 = points->GetPoint(1);
    float* p3 = points->GetPoint(2);
    float* p4 = points->GetPoint(3);
    double area1 = ComputeAreaOfTriangle(p1,p2,p3);
    double area2 = ComputeAreaOfTriangle(p3,p4,p1);
    return area1+area2;
  }
};

//=============================================================================
/*!
 *
 */
//=============================================================================
double  SMESHGUI_ComputeScalarValue::Taper(vtkCell* theCell){
  int num_points = theCell->GetNumberOfPoints ();
  vtkPoints* points = theCell->GetPoints();
  if (num_points != 4 ) return 0;
  float* p1 = points->GetPoint(0);
  float* p2 = points->GetPoint(1);
  float* p3 = points->GetPoint(2);
  float* p4 = points->GetPoint(3);
  double A1 = ComputeAreaOfTriangle(p4,p1,p2);
  double A2 = ComputeAreaOfTriangle(p3,p1,p2);
  double A3 = ComputeAreaOfTriangle(p2,p3,p4);
  double A4 = ComputeAreaOfTriangle(p3,p4,p1);
  double JA = 0.25 * (A1 + A2 + A3 + A4);
  double taper = fabs(A1/(JA-1));
  if (fabs(A2/(JA-1)) > taper) taper = fabs(A2/(JA-1));
  if (fabs(A3/(JA-1)) > taper) taper = fabs(A3/(JA-1));
  if (fabs(A4/(JA-1)) > taper) taper = fabs(A4/(JA-1));
  // MESSAGE( "Taper = " << taper);
  return taper;
};

//=============================================================================
/*!
 *
 */
//=============================================================================
double  SMESHGUI_ComputeScalarValue::AspectRatio(vtkCell* theCell) {
  int num_points = theCell->GetNumberOfPoints ();
  vtkPoints* points = theCell->GetPoints();
  if (num_points < 3 ) return 0;
  if (num_points == 3) {
    float a1,a2,a3,b1,b2,b3,c1,c2,c3;
    float* pnt = points->GetPoint(0);
    a1 =  pnt[0];
    a2 =  pnt[1];
    a3 =  pnt[2];
    pnt = points->GetPoint(1);
    b1 =  pnt[0];
    b2 =  pnt[1];
    b3 =  pnt[2];
    pnt = points->GetPoint(2);
    c1 =  pnt[0];
    c2 =  pnt[1];
    c3 =  pnt[2];  
    
    float e1, e2, e3;    
    e1 = sqrt( (a1-b1)*(a1-b1) + (a2-b2)*(a2-b2) + (a3-b3)*(a3-b3) ) ;
    e2 = sqrt( (b1-c1)*(b1-c1) + (b2-c2)*(b2-c2) + (b3-c3)*(b3-c3) ) ;
    e3 = sqrt( (c1-a1)*(c1-a1) + (c2-a2)*(c2-a2) + (c3-a3)*(c3-a3) ) ;
    
    float amax,p,s;

    amax=e1;
    if (e2>amax) amax=e2;
    if (e3>amax) amax=e3;
    
    p=(e1+e2+e3)/2;
    s=AreaElements(theCell);
  
    double aspectRatio=amax*p*sqrt(double(3))/(s*6);
    // MESSAGE( "aspectRatio = " << aspectRatio );
    return(aspectRatio);
  }
  else if (num_points == 4) {
    float a1,a2,a3,b1,b2,b3,c1,c2,c3,d1,d2,d3;
    float* pnt = points->GetPoint(0);
    a1 =  pnt[0];
    a2 =  pnt[1];
    a3 =  pnt[2];
    pnt = points->GetPoint(1);
    b1 =  pnt[0];
    b2 =  pnt[1];
    b3 =  pnt[2];
    pnt = points->GetPoint(2);
    c1 =  pnt[0];
    c2 =  pnt[1];
    c3 =  pnt[2];
    pnt = points->GetPoint(3);
    d1 =  pnt[0];
    d2 =  pnt[1];
    d3 =  pnt[2];
    
    float e1, e2, e3, e4;
    float len_min, len_max;
    e1 = sqrt( (a1-b1)*(a1-b1) + (a2-b2)*(a2-b2) + (a3-b3)*(a3-b3) ) ;
    e2 = sqrt( (b1-c1)*(b1-c1) + (b2-c2)*(b2-c2) + (b3-c3)*(b3-c3) ) ;
    e3 = sqrt( (c1-d1)*(c1-d1) + (c2-d2)*(c2-d2) + (c3-d3)*(c3-d3) ) ;
    e4 = sqrt( (d1-a1)*(d1-a1) + (d2-a2)*(d2-a2) + (d3-a3)*(d3-a3) ) ;

    len_min = e1; len_max = e1;

    if (e2 >len_max ) len_max = e2;
    if (e3 >len_max ) len_max = e3;
    if (e4 >len_max ) len_max = e4;
    if (e2 <len_min ) len_min = e2;
    if (e3 <len_min ) len_min = e3;
    if (e4 <len_min ) len_min = e4;
    
    return (len_max/len_min);
  }
};

//=============================================================================
/*!
 *
 */
//=============================================================================
static double ComputeAngle(float* p1, float* p2, float* p3) {
  const double pi=4*atan(double(1));
  float a1,a2,a3,b1,b2,b3,c1,c2,c3;
  a1 =  p1[0];
  a2 =  p1[1];
  a3 =  p1[2];
  b1 =  p2[0];
  b2 =  p2[1];
  b3 =  p2[2];
  c1 =  p3[0];
  c2 =  p3[1];
  c3 =  p3[2];
  float X1,Y1,Z1,X2,Z2,Y2;
  X1 = b1 - a1;
  X2 = c1 - b1;
  Y1 = b2 - a2;
  Y2 = c2 - b2;
  Z1 = b3 - a3;
  Z2 = c3 - b3;
  

  float e1, e2, e3;
  e1 = sqrt( X1*X1 + Y1*Y1 + Z1*Z1 ) ;
  e2 = sqrt( X2*X2 + Y2*Y2 + Z2*Z2 ) ;
  double dot=(X1*(X2)+Y1*(Y2)+Z1*(Z2));
  //  MESSAGE( pi );
  //  MESSAGE( dot/(e1*e2) );
  double cosinus = dot/(e1*e2);
  cosinus = fabs(cosinus);
  return 180*acos (cosinus)/pi;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
double SMESHGUI_ComputeScalarValue::MinimumAngle(vtkCell* theCell) {
  int num_points = theCell->GetNumberOfPoints ();
  vtkPoints* points = theCell->GetPoints();
  if (num_points < 3 ) return 0;
  float* pnt1 = points->GetPoint(0);
  float* pnt2 = points->GetPoint(1);
  float* pnt3 = points->GetPoint(2);
  if (num_points == 3) {
    double a1,a2,a3,amin;
    a1=fabs(ComputeAngle(pnt1,pnt2,pnt3));
    amin=a1;
    a2=fabs(ComputeAngle(pnt2,pnt3,pnt1));
    if (a2<amin) amin=a2;
    a3=fabs(ComputeAngle(pnt3,pnt1,pnt2));
    if (a3<amin) amin=a3;
    // MESSAGE( "Minimal angle " << amin );
    return amin;
  }
  else if (num_points == 4) {
    float* pnt4 = points->GetPoint(3);
    double a1,a2,a3,a4,amin;
    a1=fabs(ComputeAngle(pnt1,pnt2,pnt3));
    amin=a1;
    a2=fabs(ComputeAngle(pnt2,pnt3,pnt4));
    if (a2<amin) amin=a2;
    a3=fabs(ComputeAngle(pnt3,pnt4,pnt1));
    if (a3<amin) amin=a3;
    a4=fabs(ComputeAngle(pnt4,pnt1,pnt2));
    if (a4<amin) amin=a4;
    
    // MESSAGE( "Minimal angle " << amin );
    return amin;
  }
};

//=============================================================================
/*!
 *
 */
//=============================================================================
double  SMESHGUI_ComputeScalarValue::Skew(vtkCell* theCell) {
  int num_points = theCell->GetNumberOfPoints ();
  vtkPoints* points = theCell->GetPoints();
  if (num_points < 3 ) return 0;
  //triangle case
  if (num_points == 3) {
    float* pnt1 = points->GetPoint(0);
    float* pnt2 = points->GetPoint(1);
    float* pnt3 = points->GetPoint(2);
    double a1,a2,a3,amax;
    a1=fabs(60 - fabs(ComputeAngle(pnt1,pnt2,pnt3)));
    amax=a1;
    a2=fabs(60 - fabs(ComputeAngle(pnt2,pnt3,pnt1)));
    if (a2>amax) amax=a2;
    a3=fabs(60 - fabs(ComputeAngle(pnt3,pnt1,pnt2)));
    if (a3>amax) amax=a3;
    // MESSAGE( "Skew = " << amax );
    return amax;
  } 
  //quadrangle case
  else if (num_points == 4) {
    float* pnt1 = points->GetPoint(0);
    float* pnt2 = points->GetPoint(1);
    float* pnt3 = points->GetPoint(2);
    float* pnt4 = points->GetPoint(3);
    
    double a1,a2,a3,a4,amax;
    a1=fabs(90 - fabs(ComputeAngle(pnt1,pnt2,pnt3)));
    amax=a1;
    a2=fabs(90 - fabs(ComputeAngle(pnt2,pnt3,pnt4)));
    if (a2>amax) amax=a2;
    a3=fabs(90 - fabs(ComputeAngle(pnt3,pnt4,pnt1)));
    if (a3>amax) amax=a3;
    a4=fabs(90 - fabs(ComputeAngle(pnt4,pnt1,pnt2)));
    if (a4>amax) amax=a4;
    // MESSAGE( "Skew = " << amax );
    return amax;
  }
};

//=============================================================================
/*!
 *
 */
//=============================================================================
static double ComputeA(float* p1, float* p2, float* p3, float* G) {
  double e1 = sqrt(pow(p2[0]-p1[0], 2)+pow(p2[1]-p1[1], 2)+pow(p2[2]-p1[2], 2));
  double e2 = sqrt(pow(p3[0]-p2[0], 2)+pow(p3[1]-p2[1], 2)+pow(p3[2]-p2[2], 2));
  double l;
  if (e1 < e2) l = 0.5*e1;
  else l = 0.5*e2;
  float GI[3], GJ[3], N[3];;
  GI[0] = (p2[0]-p1[0])/2-G[0];
  GI[1] = (p2[1]-p1[1])/2-G[1];
  GI[2] = (p2[2]-p1[2])/2-G[2];
  
  GJ[0] = (p3[0]-p2[0])/2-G[0];
  GJ[1] = (p3[1]-p2[1])/2-G[1];
  GJ[2] = (p3[2]-p2[2])/2-G[2];
  
  N[0] = GI[1]*GJ[2] - GI[2]*GJ[1];
  N[1] = GI[2]*GJ[0] - GI[0]*GJ[2];
  N[2] = GI[0]*GJ[1] - GI[1]*GJ[0];
  
  double H;
  float T[3];
  T[0] = (p1[0]-G[0])*N[0];
  T[1] = (p1[1]-G[1])*N[1];
  T[2] = (p1[2]-G[2])*N[2];
  
  H = sqrt(pow(T[0],2)+pow(T[1],2)+pow(T[2],2))/sqrt(pow(N[0],2)+pow(N[1],2)+pow(N[2],2));
  double A = H/l;
  return A;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
double  SMESHGUI_ComputeScalarValue::Warp(vtkCell* theCell) {
  int num_points = theCell->GetNumberOfPoints ();
  vtkPoints* points = theCell->GetPoints();
  if (num_points != 4 ) return 0;
  float* p1 = points->GetPoint(0);
  float* p2 = points->GetPoint(1);
  float* p3 = points->GetPoint(2);
  float* p4 = points->GetPoint(3);
  double G1, G2, G3;
  float G[3];
  G[0] = (p1[0]+p2[0]+p3[0]+p4[0])/4;
  G[1] = (p1[1]+p2[1]+p3[1]+p4[1])/4;
  G[2] = (p1[2]+p2[2]+p3[2]+p4[2])/4;
  double amax = ComputeA(p1, p2, p3, G);
  double nextA = ComputeA(p2, p3, p4, G);
  if (nextA > amax) amax = nextA;
  nextA = ComputeA(p3, p4, p1, G);
  if (nextA > amax) amax = nextA;
  nextA = ComputeA(p4, p1, p2, G);
  if (nextA > amax) amax = nextA;
  // MESSAGE( "Warp = " << amax );
  return amax;
}
