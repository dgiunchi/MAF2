/*=========================================================================

 Program: MAF2
 Module: mafOpFilterVolume
 Authors: Paolo Quadrani
 
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

#include "mafOpFilterVolume.h"

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafGUI.h"

#include "mafVMEVolumeGray.h"

#include "vtkMAFSmartPointer.h"
#include "vtkImageData.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkImageMedian3D.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(mafOpFilterVolume);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafOpFilterVolume::mafOpFilterVolume(const wxString &label) 
:mafOp(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_OP;
	m_Canundo	= true;

	m_InputPreserving = false;

  m_PreviewResultFlag	= false;
	m_ClearInterfaceFlag= false;
  
	m_ResultImageData	  = NULL;
	m_OriginalImageData = NULL;
  m_InputData         = NULL;

  m_Dimensionality  = 3;
  m_SmoothRadius[0] = m_SmoothRadius[1] = m_SmoothRadius[2] = 1.5;
  m_StandardDeviation[0] = m_StandardDeviation[1] = m_StandardDeviation[2] = 2.0;

  m_KernelSize[0] = m_KernelSize[1] = m_KernelSize[2] = 1;

  m_ApplyDirectlyOnInput = 0;
}
//----------------------------------------------------------------------------
mafOpFilterVolume::~mafOpFilterVolume()
//----------------------------------------------------------------------------
{
	vtkDEL(m_ResultImageData);
	vtkDEL(m_OriginalImageData);
}
//----------------------------------------------------------------------------
bool mafOpFilterVolume::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node && node->IsMAFType(mafVMEVolumeGray));
}
//----------------------------------------------------------------------------
mafOp *mafOpFilterVolume::Copy()
//----------------------------------------------------------------------------
{
  return (new mafOpFilterVolume(m_Label));
}
//----------------------------------------------------------------------------
// Constants:
//----------------------------------------------------------------------------
enum FILTER_SURFACE_ID
{
	ID_SMOOTH = MINID,
  ID_STANDARD_DEVIATION,
  ID_RADIUS_FACTOR,
  ID_MEDIAN,
  ID_KERNEL_SIZE,
	ID_PREVIEW,
	ID_CLEAR,
  ID_APPLY_ON_INPUT,
};
//----------------------------------------------------------------------------
void mafOpFilterVolume::OpRun()   
//----------------------------------------------------------------------------
{ 
  m_InputData = (vtkImageData*)((mafVME *)m_Input)->GetOutput()->GetVTKData();
  if (!m_ApplyDirectlyOnInput)
  {
	  vtkNEW(m_ResultImageData);
		m_ResultImageData->DeepCopy(m_InputData);
		
		vtkNEW(m_OriginalImageData);
		m_OriginalImageData->DeepCopy(m_InputData);
  }
	
	if (!m_TestMode)
	{
		CreateGui();
	  ShowGui();
	}
}
//----------------------------------------------------------------------------
void mafOpFilterVolume::CreateGui()
//----------------------------------------------------------------------------
{
  // interface:
  m_Gui = new mafGUI(this);

  m_Gui->Bool(ID_APPLY_ON_INPUT,_("apply on input"),&m_ApplyDirectlyOnInput,0,_("Check this flag for big volumes to save memory"));
  m_Gui->Label("");
  m_Gui->Label(_("smooth"),true);
  m_Gui->Vector(ID_STANDARD_DEVIATION,_("sd: "),m_StandardDeviation,0.1,100,2,_("standard deviation for smooth filter"));
  m_Gui->Vector(ID_RADIUS_FACTOR,_("radius: "),m_SmoothRadius,1,10,2,_("radius for smooth filter"));
  m_Gui->Button(ID_SMOOTH,_("apply smooth"));

  m_Gui->Label("");
  m_Gui->Label(_("median"),true);
  m_Gui->Vector(ID_KERNEL_SIZE,_("kernel: "),m_KernelSize,1,10,_("size of kernel"));
  m_Gui->Button(ID_MEDIAN,_("apply median"));

  m_Gui->Divider(2);
  m_Gui->Button(ID_PREVIEW,_("preview"));
  m_Gui->Button(ID_CLEAR,_("clear"));
  m_Gui->OkCancel();
  m_Gui->Enable(wxOK,false);

  m_Gui->Enable(ID_PREVIEW,false);
  m_Gui->Enable(ID_CLEAR,false);

  m_Gui->Divider();
}
//----------------------------------------------------------------------------
void mafOpFilterVolume::OpDo()
//----------------------------------------------------------------------------
{
  if (m_ResultImageData)
  {
    ((mafVMEVolumeGray *)m_Input)->SetData(m_ResultImageData,((mafVME *)m_Input)->GetTimeStamp());
    mafEventMacro(mafEvent(this, CAMERA_UPDATE));
  }
}
//----------------------------------------------------------------------------
void mafOpFilterVolume::OpUndo()
//----------------------------------------------------------------------------
{
  if (m_OriginalImageData)
  {
    ((mafVMEVolumeGray *)m_Input)->SetData(m_OriginalImageData,((mafVME *)m_Input)->GetTimeStamp());
    mafEventMacro(mafEvent(this, CAMERA_UPDATE));
  }
}
//----------------------------------------------------------------------------
void mafOpFilterVolume::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {	
      case ID_APPLY_ON_INPUT:
        if (m_ApplyDirectlyOnInput)
        {
          wxMessageBox(_("Filters are applied directly to the input volume. No undo can be done to retrieve previous volume."),_("Warning"));
          vtkDEL(m_ResultImageData);
          vtkDEL(m_OriginalImageData);
        }
        else
        {
          m_InputData = (vtkImageData*)((mafVME *)m_Input)->GetOutput()->GetVTKData();
          vtkNEW(m_ResultImageData);
          m_ResultImageData->DeepCopy(m_InputData);

          vtkNEW(m_OriginalImageData);
          m_OriginalImageData->DeepCopy(m_InputData);
        }
      break;
      case ID_SMOOTH:
        OnSmooth();
      break;
      case ID_MEDIAN:
        OnMedian();
      break;
      case ID_PREVIEW:
        OnPreview(); 
      break;
      case ID_CLEAR:
        OnClear(); 
      break;
      case wxOK:
        if(m_PreviewResultFlag)
          OnPreview();
        OpStop(OP_RUN_OK);        
      break;
      case wxCANCEL:
        if(m_ClearInterfaceFlag)
          OnClear();
        OpStop(OP_RUN_CANCEL);        
      break;
    }
  }
}
//----------------------------------------------------------------------------
void mafOpFilterVolume::OpStop(int result)
//----------------------------------------------------------------------------
{
	HideGui();
	mafEventMacro(mafEvent(this,result));
}
//----------------------------------------------------------------------------
void mafOpFilterVolume::OnSmooth()
//----------------------------------------------------------------------------
{
	if (!m_TestMode)
	{
		wxBusyCursor wait;
		m_Gui->Enable(ID_SMOOTH,false);
	  m_Gui->Enable(ID_RADIUS_FACTOR,false);
	  m_Gui->Enable(ID_RADIUS_FACTOR,false);
		m_Gui->Update();
	}

  vtkMAFSmartPointer<vtkImageGaussianSmooth> smoothFilter;
	if (m_ApplyDirectlyOnInput)
    smoothFilter->SetInput(m_InputData);
  else
    smoothFilter->SetInput(m_ResultImageData);
  smoothFilter->SetDimensionality(m_Dimensionality);
  smoothFilter->SetRadiusFactors(m_SmoothRadius);
  smoothFilter->SetStandardDeviations(m_StandardDeviation);
	smoothFilter->Update();

  if (m_ApplyDirectlyOnInput)
  {
    ((mafVMEVolumeGray *)m_Input)->SetData(smoothFilter->GetOutput(),((mafVME *)m_Input)->GetTimeStamp());
    mafEventMacro(mafEvent(this, CAMERA_UPDATE));
  }
  else
    m_ResultImageData->DeepCopy(smoothFilter->GetOutput());

  m_PreviewResultFlag = !m_ApplyDirectlyOnInput;

  if (!m_TestMode)
  {
	  m_Gui->Enable(ID_SMOOTH,true);
		m_Gui->Enable(ID_RADIUS_FACTOR,true);
	  m_Gui->Enable(ID_RADIUS_FACTOR,true);
	
		m_Gui->Enable(ID_PREVIEW,m_PreviewResultFlag);
		m_Gui->Enable(ID_CLEAR,m_PreviewResultFlag);
		m_Gui->Enable(wxOK,true);
	  m_Gui->Enable(wxCANCEL,m_PreviewResultFlag);
  }
}
//----------------------------------------------------------------------------
void mafOpFilterVolume::OnMedian()
//----------------------------------------------------------------------------
{
  if (!m_TestMode)
  {
	  wxBusyCursor wait;
	  m_Gui->Enable(ID_MEDIAN,false);
	  m_Gui->Enable(ID_KERNEL_SIZE,false);
	  m_Gui->Update();
  }

  vtkMAFSmartPointer<vtkImageMedian3D> medianFilter;
  if (m_ApplyDirectlyOnInput)
    medianFilter->SetInput(m_InputData);
  else
    medianFilter->SetInput(m_ResultImageData);
  medianFilter->SetKernelSize(m_KernelSize[0],m_KernelSize[1],m_KernelSize[2]);
  medianFilter->Update();

  if (m_ApplyDirectlyOnInput)
  {
    ((mafVMEVolumeGray *)m_Input)->SetData(medianFilter->GetOutput(),((mafVME *)m_Input)->GetTimeStamp());
    mafEventMacro(mafEvent(this, CAMERA_UPDATE));
  }
  else
    m_ResultImageData->DeepCopy(medianFilter->GetOutput());

  m_PreviewResultFlag = !m_ApplyDirectlyOnInput;

  if (!m_TestMode)
  {
	  m_Gui->Enable(ID_MEDIAN,true);
	  m_Gui->Enable(ID_KERNEL_SIZE,true);
	
	  m_Gui->Enable(ID_PREVIEW,m_PreviewResultFlag);
	  m_Gui->Enable(ID_CLEAR,m_PreviewResultFlag);
	  m_Gui->Enable(wxOK,true);
	  m_Gui->Enable(wxCANCEL,m_PreviewResultFlag);
  }
}
//----------------------------------------------------------------------------
void mafOpFilterVolume::OnPreview()
//----------------------------------------------------------------------------
{
	wxBusyCursor wait;
	
  ((mafVMEVolumeGray *)m_Input)->SetData(m_ResultImageData,((mafVME *)m_Input)->GetTimeStamp());

	m_Gui->Enable(ID_PREVIEW,false);
	m_Gui->Enable(ID_CLEAR,true);
	m_Gui->Enable(wxOK,true);

	m_PreviewResultFlag   = false;
	m_ClearInterfaceFlag	= true;

	mafEventMacro(mafEvent(this, CAMERA_UPDATE));
}
//----------------------------------------------------------------------------
void mafOpFilterVolume::OnClear()
//----------------------------------------------------------------------------
{
	if (!m_TestMode)
	{
		wxBusyCursor wait;
	}

  ((mafVMEVolumeGray *)m_Input)->SetData(m_OriginalImageData,((mafVME *)m_Input)->GetTimeStamp());
	
  if (!m_ApplyDirectlyOnInput)
  {
    m_ResultImageData->DeepCopy(m_OriginalImageData);
  }

	if (!m_TestMode)
	{
		m_Gui->Enable(ID_SMOOTH,true);
		m_Gui->Enable(ID_RADIUS_FACTOR,true);
	  m_Gui->Enable(ID_STANDARD_DEVIATION,true);
	
		m_Gui->Enable(ID_PREVIEW,false);
		m_Gui->Enable(ID_CLEAR,false);
		m_Gui->Enable(wxOK,false);
		m_Gui->Update();
	}

	m_PreviewResultFlag = false;
	m_ClearInterfaceFlag= false;

	mafEventMacro(mafEvent(this, CAMERA_UPDATE));
}
