#include <choreonoid_viewer/choreonoid_viewer.h>
#include <cnoid/Body>
#include <cnoid/BodyLoader>
#include <thread>
#include <iostream>
#include "choreonoid_viewer_sample_lib.h"

int mymain(){
  cnoid::BodyLoader bodyLoader;
  cnoid::BodyPtr robot = bodyLoader.load("/opt/ros/melodic/share/openhrp3/share/OpenHRP-3.1/sample/model/sample1.wrl");

  choreonoid_viewer::Viewer viewer;
  viewer.objects(robot);

  for(int i=0;i<100;i++){
    robot->joint(0)->q() = 0.01 * i;
    robot->calcForwardKinematics();
    viewer.drawObjects();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}
