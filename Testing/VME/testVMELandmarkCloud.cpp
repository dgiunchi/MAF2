// Test mafVMEGeneric class

#include "mafVMERoot.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"
#include "mafVMEOutputPointSet.h"
#include "mafTransform.h"

#include "vtkMAFSmartPointer.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkOutlineSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkCamera.h"
#include "vtkProperty.h"
#include "vtkTexture.h"
#include "vtkGlyph3D.h"
#include "vtkSphereSource.h"

#include <iostream>

//-------------------------------------------------------------------------
int main()
//-------------------------------------------------------------------------
{
  // create a small tree with a root, a volume and a slicer 
  mafSmartPointer<mafVMERoot> root;
  root->SetName("ROOT");

  mafSmartPointer<mafVMELandmarkCloud> cloud;
  root->AddChild(cloud);
  int num_lm = cloud->GetNumberOfLandmarks();
  MAF_TEST(num_lm == 0);
  cloud->Open();
  MAF_TEST(cloud->IsOpen());
  cloud->SetRadius(1.0);
  cloud->AppendLandmark(0,0,0,"lm0");
  cloud->AppendLandmark(5,5,5,"lm1");
  cloud->AppendLandmark(-1,3,-1,"lm2");
  num_lm = cloud->GetNumberOfLandmarks();
  MAF_TEST(num_lm == 3);

  double xyz[3];

  // test landmark position 
  cloud->GetLandmark(0,xyz, 0);
  MAF_TEST(mafEquals(xyz[0],0));
  MAF_TEST(mafEquals(xyz[1],0));
  MAF_TEST(mafEquals(xyz[2],0));

  cloud->GetLandmark(1,xyz, 0);
  MAF_TEST(mafEquals(xyz[0],5));
  MAF_TEST(mafEquals(xyz[1],5));
  MAF_TEST(mafEquals(xyz[2],5));

  cloud->GetLandmark(2,xyz, 0);
  MAF_TEST(mafEquals(xyz[0],-1));
  MAF_TEST(mafEquals(xyz[1],3));
  MAF_TEST(mafEquals(xyz[2],-1));

  // create windows
  vtkMAFSmartPointer<vtkRenderer> renderer;
  vtkMAFSmartPointer<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer);

  renWin->SetSize(800,800);
  renderer->SetBackground( 0.1, 0.2, 0.4 );

  mafOBB cloudBounds;
  cloud->GetOutput()->GetVMELocalBounds(cloudBounds);
  MAF_TEST(!cloudBounds.IsValid()); // should be invalid: cloud has no data when open
  cloud->GetOutput()->GetBounds(cloudBounds);
  MAF_TEST(cloudBounds.IsValid());
  cloudBounds.Print(std::cerr);

  cloud->Close();
  cloud->GetOutput()->GetVMELocalBounds(cloudBounds);
  MAF_TEST(cloudBounds.IsValid()); // now cloud is closed => local bounds should be valid
  cloud->SetRadius(0.5);
  double rad = cloud->GetRadius();
  MAF_TEST(mafEquals(rad,0.5));

  cloud->GetLandmark(1,xyz, 0);
  MAF_TEST(mafEquals(xyz[0],5));
  MAF_TEST(mafEquals(xyz[1],5));
  MAF_TEST(mafEquals(xyz[2],5));

  // try to add a landmark when the cloud is closed!
  cloud->AppendLandmark(2,-2,2,"lm3");
  num_lm = cloud->GetNumberOfLandmarks();
  MAF_TEST(num_lm == 4);

  // create actor to display the slicer plane
  vtkMAFSmartPointer<vtkSphereSource> sphere;
  sphere->SetRadius(rad);

  vtkMAFSmartPointer<vtkGlyph3D> glyph_mapper;
  glyph_mapper->SetInput(cloud->GetOutput()->GetVTKData());
  glyph_mapper->SetSource(sphere->GetOutput());

  vtkMAFSmartPointer<vtkPolyDataMapper> cloud_mapper;
  cloud_mapper->SetInput(glyph_mapper->GetOutput());
  
  vtkMAFSmartPointer<vtkActor> cloud_actor;
  cloud_actor->SetMapper(cloud_mapper);

  renderer->AddActor(cloud_actor);
  renderer->ResetCamera();
  renWin->Render();

  int steps = 50;
  for (int t = 0; t < steps; t++) 
  {
    renderer->GetActiveCamera()->Azimuth(-(180.0/steps));
    renWin->Render();
    mafSleep(50);
  }

  mafTimeStamp ct = cloud->GetTimeStamp();
  cloud->Open();
  // add a landmark to a different time stamp
  cloud->SetLandmark(0,1,1,1,1.0);
  num_lm = cloud->GetNumberOfLandmarks();
  MAF_TEST(num_lm == 4);
  // set the time stamp
  cloud->SetTimeStamp(1.0);
  // the number of lm should remain the same
  num_lm = cloud->GetNumberOfLandmarks();
  MAF_TEST(num_lm == 4);
  
  // check the lm's position to the new timestamp
  
  // lm1's position should be changed
  cloud->GetLandmark(0,xyz,1.0);
  MAF_TEST(mafEquals(xyz[0],1));
  MAF_TEST(mafEquals(xyz[1],1));
  MAF_TEST(mafEquals(xyz[2],1));
  // others lm's positions should be the same
  cloud->GetLandmark(1,xyz,1.0);
  MAF_TEST(mafEquals(xyz[0],5));
  MAF_TEST(mafEquals(xyz[1],5));
  MAF_TEST(mafEquals(xyz[2],5));
  cloud->GetLandmark(2,xyz,1.0);
  MAF_TEST(mafEquals(xyz[0],-1));
  MAF_TEST(mafEquals(xyz[1],3));
  MAF_TEST(mafEquals(xyz[2],-1));
  cloud->GetLandmark(3,xyz,1.0);
  MAF_TEST(mafEquals(xyz[0],2));
  MAF_TEST(mafEquals(xyz[1],-2));
  MAF_TEST(mafEquals(xyz[2],2));

  // reset the timestamp to 0
  cloud->SetTimeStamp(0.0);
  // set the visibility of lm1 in different timestamp
  cloud->SetLandmarkVisibility("lm1",false,1.0);
  // check the visibility at timestamp 0 that should be true (default visibility)
  MAF_TEST(cloud->GetLandmarkVisibility("lm1"));
  // check the visibility at timestamp 1.0 just set
  MAF_TEST(cloud->GetLandmarkVisibility("lm1",1.0) == false);

  mafSleep(3000);

  std::cerr << "Test completed successfully!" << std::endl;
  return MAF_OK;
}