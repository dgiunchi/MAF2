/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafOpFilterVolume.h,v $
  Language:  C++
  Date:      $Date: 2008-03-06 11:55:06 $
  Version:   $Revision: 1.1 $
  Authors:   Paolo Quadrani
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __mafOpFilterVolume_H__
#define __mafOpFilterVolume_H__

#include "mafOp.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class vtkImageData;
class mafEvent;

//----------------------------------------------------------------------------
// mafOpFilterVolume :
//----------------------------------------------------------------------------
/** */
class mafOpFilterVolume: public mafOp
{
public:
	mafOpFilterVolume(const wxString &label = "FilterVolume");
	~mafOpFilterVolume(); 
	virtual void OnEvent(mafEventBase *maf_event);
	
  mafTypeMacro(mafOpFilterVolume, mafOp);

  mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

	/** Builds operation's interface. */
	void OpRun();

	/** Execute the operation. */
	void OpDo();

	/** Makes the undo for the operation. */
	void OpUndo();

  void ApplyFiltersToInputData(bool apply) {m_ApplyDirectlyOnInput = apply;};

  void ApplyFiltersToInputDataOn() {ApplyFiltersToInputData(true);};

  void ApplyFiltersToInputDataOff() {ApplyFiltersToInputData(false);};

protected:
	/** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	void OpStop(int result);
	
	bool m_ClearInterfaceFlag;
	bool m_PreviewResultFlag;

	vtkImageData	*m_ResultImageData;
	vtkImageData	*m_OriginalImageData;
  vtkImageData  *m_InputData;

	int    m_Dimensionality;
  double m_SmoothRadius[3];
  double m_StandardDeviation[3];

  int m_KernelSize[3];

  int m_ApplyDirectlyOnInput; ///< For big volume data, torn on this flag to save memory and apply filters directly to the input data.

  /** Smooth the surface. */
	void OnSmooth();

  /** Execute median filter.*/
  void OnMedian();

	/** Make the preview of the surface filtering. */
	void OnPreview();  

	/** Clear all the surface applied filtering. */
	void OnClear();
};
#endif