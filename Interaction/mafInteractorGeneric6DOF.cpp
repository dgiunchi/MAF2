/*=========================================================================

 Program: MAF2
 Module: mafInteractorGeneric6DOF
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "mafEvent.h"

#include "mafInteractorGeneric6DOF.h"
#include "mafDeviceButtonsPadTracker.h"
#include "mafAvatar3D.h"
#include "mafRefSys.h"

#include "mafEventInteraction.h"
#include "mafTransform.h"
#include "mafCameraTransform.h"
#include "mafVME.h"

#include <assert.h>

//------------------------------------------------------------------------------
mafCxxTypeMacro(mafInteractorGeneric6DOF)
//------------------------------------------------------------------------------
mafInteractorGeneric6DOF::mafInteractorGeneric6DOF()
{
  m_IgnoreTriggerEvents = 0;
  m_DifferentialMoving  = 1;
  mafNEW(m_OutputTransform);
}

//------------------------------------------------------------------------------
mafInteractorGeneric6DOF::~mafInteractorGeneric6DOF()
{
  mafDEL(m_OutputTransform);
}

//------------------------------------------------------------------------------
void mafInteractorGeneric6DOF::OnMove(mafEventInteraction *e)
//------------------------------------------------------------------------------
{
  // if the tracker does not have an avatar we don't
  // know the mapping rule and cannot proceed
  
  if (m_Avatar)
	{
    mafMatrix *tracker_pose=e->GetMatrix();
    SetTrackerPoseMatrix(tracker_pose);

    Update();
    
    if (GetResultTransform())
    {
      m_ResultTransform->SetMatrix(*m_VME->GetOutput()->GetAbsMatrix());
      GetResultTransform()->Concatenate(m_OutputTransform,POST_MULTIPLY);
    }

    // forward the event with the pose matrix through default channel
    mafEventMacro(mafEventInteraction(this,ID_TRANSFORM,m_OutputTransform->GetMatrixPointer()));
  }
}

//------------------------------------------------------------------------------
void mafInteractorGeneric6DOF::OnEvent(mafEventBase *event)
//------------------------------------------------------------------------------
{
  assert(event);
  assert(event->GetSender());
  
  mafID id=event->GetId();
  mafID channel=event->GetChannel();

  if (channel==MCH_INPUT && GetTracker())
  {
    // if the event comes from tracker which started the interaction continue...
    if (id==mafDeviceButtonsPadTracker::GetTracker3DMoveId() && event->GetSender()==GetTracker())
    {
      mafEventInteraction *e=mafEventInteraction::SafeDownCast(event);
      assert(e);        
      OnMove(e);
    }
  }
  Superclass::OnEvent(event);
}

//------------------------------------------------------------------------------
void mafInteractorGeneric6DOF::Update()
//------------------------------------------------------------------------------
{
  if (m_DifferentialMoving == 1)
  {
    // not yet implemented should not be here
    UpdateDeltaTransform();
    
    // remove the scaling factor...
    m_DeltaTransform->GetOrientation(m_OutputOrientation);
    m_DeltaTransform->GetPosition(m_OutputPosition);

    // this simply remove anything but orientation and translation...
    // Can I use polar decomposition for this? Is it more precise?
    // How does it performs?
    m_OutputTransform->SetOrientation(m_OutputOrientation);
    m_OutputTransform->SetPosition(m_OutputPosition);
    
    double pivot_position[3];
    mafTransform::GetPosition(*GetPivotRefSys()->GetMatrix(),pivot_position);

    // Rotate around the rotational constraint ref sys center
    m_OutputTransform->Translate(-pivot_position[0],-pivot_position[1],-pivot_position[2],PRE_MULTIPLY);
    m_OutputTransform->Translate(pivot_position,POST_MULTIPLY);

    // Store current pose matrix
    TrackerSnapshot(m_TrackerPoseMatrix);
  }
	else
	{
    if (m_Avatar)
    {  
      mafMatrix outmatrix;
      m_Avatar->TrackerToWorld(*m_TrackerPoseMatrix,outmatrix,false);
      m_OutputTransform->SetMatrix(outmatrix);
    }
  }
}

