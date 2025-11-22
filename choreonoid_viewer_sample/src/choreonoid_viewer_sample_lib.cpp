#include <choreonoid_viewer/choreonoid_viewer.h>
#include <cnoid/Body>
#include <cnoid/BodyLoader>
#include <cnoid/SceneMarkers>
#include <cnoid/Camera>
#include <thread>
#include <iostream>
#include "choreonoid_viewer_sample_lib.h"

namespace choreonoid_viewer_sample{
  int mymain(){
    {

      cnoid::BodyLoader bodyLoader;
      cnoid::BodyPtr robot = bodyLoader.load(std::string("/opt/ros/")+getenv("ROS_DISTRO")+"/share/openhrp3/share/OpenHRP-3.1/sample/model/sample1.wrl");
      cnoid::SphereMarkerPtr marker = new cnoid::SphereMarker(0.3, cnoid::Vector3f(1.0,0.0,0.0));
      cnoid::CameraPtr camera = robot->findDevice<cnoid::Camera>("VISION_SENSOR1");

      choreonoid_viewer::Viewer viewer;
      viewer.objects(robot);
      viewer.drawOn(marker);
      viewer.cameras(camera);

      for(int i=0;i<100;i++){
        robot->link("RARM_SHOULDER_P")->q() = -1.0 -0.01 * i;
        robot->calcForwardKinematics();
        marker->translation()[0] = 0.01 * i;
        viewer.drawObjects(false);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if(i >= 1000/30 && i % 40 == 0){ // 30fpsなので
          std::string fileName = "/tmp/camera" + std::to_string(i) + ".png";
          std::cout << "write to " << fileName << std::endl;
          camera->image().save(fileName);  // markerはカメラに描画されない.
        }
      }


      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    return 0;
  }
}
