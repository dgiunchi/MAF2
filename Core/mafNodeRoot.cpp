/*=========================================================================

 Program: MAF2
 Module: mafNodeRoot
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "mafNodeRoot.h"
#include "mafAttribute.h"
#include "mafIndent.h"
#include "mafStorage.h"
#include "mafEventIO.h"

//-------------------------------------------------------------------------
mafCxxTypeMacro(mafNodeRoot)
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
void mafNodeRoot::OnEvent(mafEventBase *e)
//-------------------------------------------------------------------------
{
  if (e->GetChannel()==MCH_UP)
  {
    mafNodeRoot::OnRootEvent(e);
  }
  else
  {
    Superclass::OnEvent(e);
  }
}

//-------------------------------------------------------------------------
void mafNodeRoot::Print(std::ostream& os, const int tabs)// const
//-------------------------------------------------------------------------
{
  mafNode::Print(os,tabs);
  mafRoot::Print(os,tabs);
}

//-------------------------------------------------------------------------
char** mafNodeRoot::GetIcon() 
//-------------------------------------------------------------------------
{
  #include "mafNodeRoot.xpm"
  return mafNodeRoot_xpm;
}
