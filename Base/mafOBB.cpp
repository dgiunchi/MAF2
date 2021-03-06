/*=========================================================================

 Program: MAF2
 Module: mafOBB
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "mafOBB.h"
#include "mafMatrix.h"
#include "mafIndent.h"
#include "mafTransform.h"
#include "math.h"
//-------------------------------------------------------------------------
mafOBB::mafOBB()
//-------------------------------------------------------------------------
{
  m_Bounds[0]=0;
  m_Bounds[1]=-1;
  m_Bounds[2]=0;
  m_Bounds[3]=-1;
  m_Bounds[4]=0;
  m_Bounds[5]=-1;
}

//-------------------------------------------------------------------------
mafOBB::mafOBB(double source[6])
//-------------------------------------------------------------------------
{

  for (int i=0;i<6;i++)
  {
    m_Bounds[i]=source[i];
  }
}

//-------------------------------------------------------------------------
mafOBB::~mafOBB()
//-------------------------------------------------------------------------
{
}

//-------------------------------------------------------------------------
void mafOBB::Reset()
//-------------------------------------------------------------------------
{
  m_Bounds[0]=0;
  m_Bounds[1]=-1;
  m_Bounds[2]=0;
  m_Bounds[3]=-1;
  m_Bounds[4]=0;
  m_Bounds[5]=-1;
  Modified();
}

//-------------------------------------------------------------------------
bool mafOBB::Equals(mafOBB &bounds) const
//-------------------------------------------------------------------------
{
  for (int i=0;i<6;i++)
  {
    if (!mafEquals(m_Bounds[i],bounds.m_Bounds[i])) // consider only 15 digits to avoid dirty bits
    {
      return false;
    }
  }
  
  return  m_Matrix==bounds.m_Matrix;
}

//-------------------------------------------------------------------------
bool mafOBB::Equals(double bounds[6]) const
//-------------------------------------------------------------------------
{
  for (int i=0;i<6;i++)
  {
    if (!mafEquals(m_Bounds[i],bounds[i])) // consider only 15 digits to avoid dirty bits
    {
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------------
bool mafOBB::Equals(float bounds[6]) const
//-------------------------------------------------------------------------
{
  float myBounds[6];
  CopyTo(myBounds);

  for (int i=0;i<6;i++)
  {
    if (!mafFloatEquals(myBounds[i],bounds[i]))
    {
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------------
void mafOBB::DeepCopy(float bounds[6])
//-------------------------------------------------------------------------
{
  for (int i=0;i<6;i++)
  {
    m_Bounds[i]=bounds[i];
  }

  Modified();
}


//-------------------------------------------------------------------------
void mafOBB::DeepCopy(double bounds[6])
//-------------------------------------------------------------------------
{
  for (int i=0;i<6;i++)
  {
    m_Bounds[i]=bounds[i];
  }

  Modified();
}

//-------------------------------------------------------------------------
void mafOBB::DeepCopy(mafOBB *source)
//-------------------------------------------------------------------------
{
  for (int i=0;i<6;i++)
  {
    m_Bounds[i]=source->m_Bounds[i];
  }

  m_Matrix=source->m_Matrix; 
  Modified();
}

//-------------------------------------------------------------------------
void mafOBB::CopyTo(float target[6]) const
//-------------------------------------------------------------------------
{
  for (int i=0;i<6;i++)
  {
    target[i]=(float)m_Bounds[i];
  }
}

//-------------------------------------------------------------------------
void mafOBB::CopyTo(double target[6]) const
//-------------------------------------------------------------------------
{
  for (int i=0;i<6;i++)
  {
    target[i]=m_Bounds[i];
  }
}

//-------------------------------------------------------------------------
void mafOBB::ApplyTransform(const mafMatrix &mat, mafOBB &newbounds)
//-------------------------------------------------------------------------
{
  double newpoints[4*8];

  int i=0;

  if (newbounds.IsValid())
  {

    for (int x=0;x<2;x++)
    {
      double X=newbounds.m_Bounds[x];
      for (int y=0;y<2;y++)
      {
        double Y=newbounds.m_Bounds[y+2];
        for (int z=0;z<2;z++)
        {
          double Z=newbounds.m_Bounds[z+4];
        
          newpoints[i*4]=X;
          newpoints[i*4+1]=Y;
          newpoints[i*4+2]=Z;
          newpoints[i*4+3]=1;

          mat.MultiplyPoint(&(newpoints[4*i]),&(newpoints[4*i]));
        
          i++;
        }
      }
    }
  
    // Initialize the new bounds coincident with a single point
    newbounds.m_Bounds[0]=newpoints[0];
    newbounds.m_Bounds[1]=newpoints[0];
    newbounds.m_Bounds[2]=newpoints[1];
    newbounds.m_Bounds[3]=newpoints[1];
    newbounds.m_Bounds[4]=newpoints[2];
    newbounds.m_Bounds[5]=newpoints[2];

    // find the right bounds
    for (i=0;i<8;i++)
    {
      // is this point outside current X bounds
      if (newpoints[i*4]<newbounds.m_Bounds[0])
        newbounds.m_Bounds[0]=newpoints[i*4];
      if (newpoints[i*4]>newbounds.m_Bounds[1])
        newbounds.m_Bounds[1]=newpoints[i*4];

      // is this point outside current Y bounds
      if (newpoints[i*4+1]<newbounds.m_Bounds[2])
        newbounds.m_Bounds[2]=newpoints[i*4+1];
      if (newpoints[i*4+1]>newbounds.m_Bounds[3])
        newbounds.m_Bounds[3]=newpoints[i*4+1];

      // is this point outside current Z bounds
      if (newpoints[i*4+2]<newbounds.m_Bounds[4])
        newbounds.m_Bounds[4]=newpoints[i*4+2];
      if (newpoints[i*4+2]>newbounds.m_Bounds[5])
        newbounds.m_Bounds[5]=newpoints[i*4+2];
    }
    
    newbounds.m_Matrix.Identity(); // reset matrix to identiy
    newbounds.Modified();
  }
  else
  {
    newbounds.Reset();
  }

}

//-------------------------------------------------------------------------
void mafOBB::MergeBounds(mafOBB &bounds)
//-------------------------------------------------------------------------
{
  if (IsValid())
  {
    if (bounds.IsValid())
    {
      MergeBounds(m_Bounds,bounds.m_Bounds);
      Modified();
    }
  }
  else
  {
    DeepCopy(bounds);
  }
}

//-------------------------------------------------------------------------
void mafOBB::MergeBounds(double b1[6], double b2[6])
//-------------------------------------------------------------------------
{
  b1[0] = (b1[0]<b2[0]) ?	b1[0] : b2[0];    
	b1[2] = (b1[2]<b2[2]) ?	b1[2] : b2[2];    
	b1[4] = (b1[4]<b2[4]) ?	b1[4] : b2[4];    
	b1[1] = (b1[1]>b2[1]) ?	b1[1] : b2[1];    
	b1[3] = (b1[3]>b2[3]) ?	b1[3] : b2[3];    
	b1[5] = (b1[5]>b2[5]) ?	b1[5] : b2[5];
}

//-------------------------------------------------------------------------
void mafOBB::GetDimensions(float dims[3]) const
//-------------------------------------------------------------------------
{
  dims[0]=(float)(m_Bounds[1]-m_Bounds[0]);
  dims[1]=(float)(m_Bounds[3]-m_Bounds[2]);
  dims[2]=(float)(m_Bounds[5]-m_Bounds[4]);
}

//-------------------------------------------------------------------------
void mafOBB::GetDimensions(double dims[3]) const
//-------------------------------------------------------------------------
{
  dims[0]=m_Bounds[1]-m_Bounds[0];
  dims[1]=m_Bounds[3]-m_Bounds[2];
  dims[2]=m_Bounds[5]-m_Bounds[4];
}

//-------------------------------------------------------------------------
void mafOBB::GetCenter(float center[3]) const
//-------------------------------------------------------------------------
{
  center[0]=(float)(m_Bounds[0]+m_Bounds[1])/2;
  center[1]=(float)(m_Bounds[2]+m_Bounds[3])/2;
  center[2]=(float)(m_Bounds[4]+m_Bounds[5])/2;
}

//-------------------------------------------------------------------------
void mafOBB::GetCenter(double center[3]) const
//-------------------------------------------------------------------------
{
  center[0]=(m_Bounds[0]+m_Bounds[1])/2;
  center[1]=(m_Bounds[2]+m_Bounds[3])/2;
  center[2]=(m_Bounds[4]+m_Bounds[5])/2;
}

//-------------------------------------------------------------------------
void mafOBB::SetCenter(double center[3])
//-------------------------------------------------------------------------
{
  double dims[3];
  GetDimensions(dims);
  m_Bounds[0]=center[0]-dims[0]/2;
  m_Bounds[1]=center[0]+dims[0]/2;
  m_Bounds[2]=center[1]-dims[1]/2;
  m_Bounds[3]=center[1]+dims[1]/2;
  m_Bounds[4]=center[2]-dims[2]/2;
  m_Bounds[5]=center[2]+dims[2]/2;
  Modified();
}

//-------------------------------------------------------------------------
void mafOBB::SetCenter(float center[3])
//-------------------------------------------------------------------------
{
  double dims[3];
  GetDimensions(dims);
  m_Bounds[0]=center[0]-dims[0]/2;
  m_Bounds[1]=center[0]+dims[0]/2;
  m_Bounds[2]=center[1]-dims[1]/2;
  m_Bounds[3]=center[1]+dims[1]/2;
  m_Bounds[4]=center[2]-dims[2]/2;
  m_Bounds[5]=center[2]+dims[2]/2;
  Modified();
}

//-------------------------------------------------------------------------
void mafOBB::SetDimensions(double dims[3])
//-------------------------------------------------------------------------
{
  double center[3];
  GetCenter(center);
  m_Bounds[0]=center[0]-dims[0]/2;
  m_Bounds[1]=center[0]+dims[0]/2;
  m_Bounds[2]=center[1]-dims[1]/2;
  m_Bounds[3]=center[1]+dims[1]/2;
  m_Bounds[4]=center[2]-dims[2]/2;
  m_Bounds[5]=center[2]+dims[2]/2;
  Modified();
}

//-------------------------------------------------------------------------
void mafOBB::SetDimensions(float dims[3])
//-------------------------------------------------------------------------
{
  float center[3];
  GetCenter(center);
  m_Bounds[0]=center[0]-dims[0]/2;
  m_Bounds[1]=center[0]+dims[0]/2;
  m_Bounds[2]=center[1]-dims[1]/2;
  m_Bounds[3]=center[1]+dims[1]/2;
  m_Bounds[4]=center[2]-dims[2]/2;
  m_Bounds[5]=center[2]+dims[2]/2;
  Modified();
}

//-------------------------------------------------------------------------
double mafOBB::GetWidth() const
//-------------------------------------------------------------------------
{
  double dims[3];
  GetDimensions(dims);
  return dims[0];
}

//-------------------------------------------------------------------------
double mafOBB::GetHeight() const
//-------------------------------------------------------------------------
{
  double dims[3];
  GetDimensions(dims);
  return dims[1];
}
//-------------------------------------------------------------------------
double mafOBB::GetDepth() const
//-------------------------------------------------------------------------
{
  double dims[3];
  GetDimensions(dims);
  return dims[2];
}

//-------------------------------------------------------------------------
void mafOBB::SetOrientation(double rx, double ry, double rz)
//-------------------------------------------------------------------------
{
  mafTransform::SetOrientation(m_Matrix,rx,ry,rz);
  Modified();
}

//-------------------------------------------------------------------------
void mafOBB::GetOrientation(double *rxyz)
//-------------------------------------------------------------------------
{
  mafTransform::GetOrientation(m_Matrix,rxyz);
  Modified();
}
//-------------------------------------------------------------------------
void mafOBB::Print(std::ostream& os, const int tabs) const
//-------------------------------------------------------------------------
{
  mafIndent indent(tabs);
  os << indent << "Bounds: ["<<m_Bounds[0]<<","<<m_Bounds[1]<<","<<m_Bounds[2]<<"," \
    <<m_Bounds[3]<<","<<m_Bounds[4]<<","<<m_Bounds[5]<<"]\n";
  os << indent << "Matrix: \n";
  m_Matrix.Print(os,indent.GetNextIndent()); 
}
