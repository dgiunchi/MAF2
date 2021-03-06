/*=========================================================================

 Program: MAF2
 Module: mafPipePointSetTest
 Authors: Daniele Giunchi
 
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

#include <cppunit/config/SourcePrefix.h>
#include "mafPipePointSetTest.h"
#include "mafPipePointSet.h"

#include "mafSceneNode.h"
#include "mafVMEPointSet.h"
#include "mmaMaterial.h"
#include "vtkMAFAssembly.h"

#include "vtkMapper.h"
#include "vtkJPEGWriter.h"
#include "vtkJPEGReader.h"
#include "vtkWindowToImageFilter.h"
#include "vtkImageMathematics.h"
#include "vtkImageData.h"
#include "vtkPointData.h"

#include "vtkCamera.h"
#include "vtkProp3DCollection.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkDelaunay2D.h"


// render window stuff
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

#include <iostream>
#include <fstream>
//----------------------------------------------------------------------------
void mafPipePointSetTest::TestFixture()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
void mafPipePointSetTest::setUp()
//----------------------------------------------------------------------------
{
  vtkNEW(m_Renderer);
  vtkNEW(m_RenderWindow);
  vtkNEW(m_RenderWindowInteractor);
}
//----------------------------------------------------------------------------
void mafPipePointSetTest::tearDown()
//----------------------------------------------------------------------------
{
  vtkDEL(m_Renderer);
  vtkDEL(m_RenderWindow);
  vtkDEL(m_RenderWindowInteractor);
}
//----------------------------------------------------------------------------
void mafPipePointSetTest::TestPipeExecution()
//----------------------------------------------------------------------------
{

  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(0.,0.,0.);
  points->InsertNextPoint(1.,0.,0.);
  points->InsertNextPoint(0.,1.,0.);
  points->InsertNextPoint(1.,1.,0.);
  points->InsertNextPoint(0.,0.,1.);
  points->InsertNextPoint(1.,0.,1.);
  points->InsertNextPoint(0.,1.,1.);
  points->InsertNextPoint(1.,1.,1.);

  /*vtkCellArray *cellArray = vtkCellArray::New();
  int index[2];
  index[0] = 0;index[1] = 1;
  cellArray->InsertNextCell(2,index);
  index[0] = 1;index[1] = 2;
  cellArray->InsertNextCell(2,index);
  index[0] = 2;index[1] = 3;
  cellArray->InsertNextCell(2,index);
  index[0] = 3;index[1] = 4;
  cellArray->InsertNextCell(2,index);
  index[0] = 4;index[1] = 5;
  cellArray->InsertNextCell(2,index);
  index[0] = 5;index[1] = 6;
  cellArray->InsertNextCell(2,index);
  index[0] = 6;index[1] = 7;
  cellArray->InsertNextCell(2,index);*/

  vtkPolyData *polyData = vtkPolyData::New();
  polyData->SetPoints(points);
  //polyData->SetLines(cellArray);
  polyData->Update();

  vtkDelaunay2D *delaunay = vtkDelaunay2D::New();
  delaunay->SetInput(polyData);
  delaunay->SetAlpha(5);
  delaunay->SetTolerance(2);
  delaunay->Update();
  
  mafVMEPointSet *pointsetInput;
  mafNEW(pointsetInput);
  pointsetInput->SetData(delaunay->GetOutput(),0);
  pointsetInput->GetOutput()->GetVTKData()->Update();
  pointsetInput->GetOutput()->Update();
  pointsetInput->Update();

  ///////////////// render stuff /////////////////////////

  m_Renderer->SetBackground(0.1, 0.1, 0.1);

  m_RenderWindow->AddRenderer(m_Renderer);
  m_RenderWindow->SetSize(640, 480);
  m_RenderWindow->SetPosition(200,0);

  m_RenderWindowInteractor->SetRenderWindow(m_RenderWindow);

  ///////////// end render stuff /////////////////////////

  mmaMaterial *material;
  material = pointsetInput->GetMaterial();

  //Assembly will be create when instancing mafSceneNode
  mafSceneNode *sceneNode;
  sceneNode = new mafSceneNode(NULL,NULL,pointsetInput, NULL);

  /////////// Pipe Instance and Creation ///////////
  mafPipePointSet *pipePointSet = new mafPipePointSet;
  pipePointSet->Create(sceneNode);
  pipePointSet->m_RenFront = m_Renderer;
  
  ////////// ACTORS List ///////////////
  vtkProp3DCollection *actorList = pipePointSet->GetAssemblyFront()->GetParts();

  actorList->InitTraversal();
  vtkProp *actor = actorList->GetNextProp();
  while(actor)
  {   
    ((vtkActor *)actor)->GetProperty()->SetPointSize(5.);
    ((vtkActor *)actor)->GetProperty()->Modified();
    ((vtkActor *)actor)->Modified();
    m_Renderer->AddActor(actor);
    m_RenderWindow->Render();

    actor = actorList->GetNextProp();
  }

  m_RenderWindow->Render();

	int j=0;
  for(int i=0; i<=30 ; i = i+3, j = j++)
	{
		if(i > 15)
		{
			pipePointSet->Select(true);
		}

    m_Renderer->GetActiveCamera()->Azimuth(i); 
		m_Renderer->GetActiveCamera()->Elevation(j);
		m_RenderWindow->Render();
		printf("\n Visualization: azimut->%d elev->%d  - %s \n", i, j, i>15?"selected":"NOT selected");
		CompareImages(i);
		mafSleep(600);
	}
  //m_RenderWindowInteractor->Start();
  
  

  m_Renderer->RemoveAllProps();

  delete pipePointSet;
  delete sceneNode;
  mafDEL(pointsetInput);
  vtkDEL(delaunay);
  vtkDEL(polyData);
  //vtkDEL(cellArray);
  vtkDEL(points);

  delete wxLog::SetActiveTarget(NULL);
}
//----------------------------------------------------------------------------
void mafPipePointSetTest::CompareImages(int imageIndex)
//----------------------------------------------------------------------------
{
  char *file = __FILE__;
  std::string name(file);
  int slashIndex =  name.find_last_of('\\');

  name = name.substr(slashIndex+1);

  int pointIndex =  name.find_last_of('.');
  name = name.substr(0, pointIndex);



  mafString controlOriginFile=MAF_DATA_ROOT;
  controlOriginFile<<"/Test_PipePointSet/";
  controlOriginFile<<name.c_str();
  controlOriginFile<<"_";
  controlOriginFile<<"image";
  controlOriginFile<<imageIndex;
  controlOriginFile<<".jpg";

  fstream controlStream;
  controlStream.open(controlOriginFile.GetCStr()); 

  // visualization control
  m_RenderWindow->OffScreenRenderingOn();
  vtkWindowToImageFilter *w2i;
  vtkNEW(w2i);
  w2i->SetInput(m_RenderWindow);
  //w2i->SetMagnification(magnification);
  w2i->Update();
  m_RenderWindow->OffScreenRenderingOff();

  //write comparing image
  vtkJPEGWriter *w;
  vtkNEW(w);
  w->SetInput(w2i->GetOutput());
  mafString imageFile=MAF_DATA_ROOT;

  if(!controlStream)
  {
    imageFile<<"/Test_PipePointSet/";
    imageFile<<name.c_str();
    imageFile<<"_";
    imageFile<<"image";
  }
  else
  {
    imageFile<<"/Test_PipePointSet/";
    imageFile<<name.c_str();
    imageFile<<"_";
    imageFile<<"comp";
  }

  imageFile<<imageIndex;
  imageFile<<".jpg";
  w->SetFileName(imageFile.GetCStr());
  w->Write();

  if(!controlStream)
  {
    vtkDEL(w);
    vtkDEL(w2i);

    controlStream.close();
    return;
  }
  controlStream.close();

  //read original Image
  vtkJPEGReader *rO;
  vtkNEW(rO);
  mafString imageFileOrig=MAF_DATA_ROOT;
  imageFileOrig<<"/Test_PipePointSet/";
  imageFileOrig<<name.c_str();
  imageFileOrig<<"_";
  imageFileOrig<<"image";
  imageFileOrig<<imageIndex;
  imageFileOrig<<".jpg";
  rO->SetFileName(imageFileOrig.GetCStr());
  rO->Update();

  vtkImageData *imDataOrig = rO->GetOutput();

  //read compared image
  vtkJPEGReader *rC;
  vtkNEW(rC);
  rC->SetFileName(imageFile.GetCStr());
  rC->Update();

  vtkImageData *imDataComp = rC->GetOutput();


  vtkImageMathematics *imageMath;
  vtkNEW(imageMath);
  imageMath->SetInput1(imDataOrig);
  imageMath->SetInput2(imDataComp);
  imageMath->SetOperationToSubtract();
  imageMath->Update();

  double srR[2] = {-1,1};
  imageMath->GetOutput()->GetPointData()->GetScalars()->GetRange(srR);

  CPPUNIT_ASSERT(srR[0] == 0.0 && srR[1] == 0.0);

  // end visualization control
  vtkDEL(imageMath);
  vtkDEL(rC);
  vtkDEL(rO);

  vtkDEL(w);
  vtkDEL(w2i);
}