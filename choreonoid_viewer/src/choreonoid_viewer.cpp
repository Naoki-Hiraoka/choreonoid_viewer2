#include <choreonoid_viewer/choreonoid_viewer.h>

#include <QCoreApplication>
#include <cnoid/RootItem>
#include <cnoid/SceneProvider>
#include <cnoid/SceneView>
#include <cnoid/SceneWidget>
#include <cnoid/LazyCaller>
#include <cnoid/ItemTreeView>
#include <cnoid/Camera>
#include <cnoid/RangeCamera>
#include <cnoid/RangeSensor>
#include <iostream>

namespace choreonoid_viewer {

  Viewer::Viewer(){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    // Item treeの操作はmain threadで行う
    cnoid::callSynchronously([&](){
      this->worldItem_ = new cnoid::WorldItem();
      cnoid::RootItem::instance()->addChildItem(this->worldItem_);
      this->simulatorItem_ = new DoNotingSimulatorItem();

      // pauseSimulation()が呼ばれた後、実際にpauseされた後にthis->pauseCv_が起床
      this->simulatorItem_->sigSimulationPaused().connect([&]{
                                                            this->pauseCv_.notify_all();
                                                          });

      this->simulatorItem_->sigSimulationStarted().connect([&]{

        // run()が1loopで止まるように. 1loop経過後にpauseSimulation()が呼ばれる.
        // startSimulation()したときにPostDynamicsFunctionはclearされてしまうので
        this->simulatorItem_->addPostDynamicsFunction([&]{
                                                      cnoid::callSynchronously([&](){this->simulatorItem_->pauseSimulation();});
                                                    });

        // startSimulationする前はsimulationBodyが無いので
        for(std::unordered_map<cnoid::DevicePtr, cnoid::GLVisionSimulatorItemPtr>::iterator it = this->currentCameras_.begin(); it!=this->currentCameras_.end();it++){
          cnoid::DevicePtr device = it->first;
          cnoid::BodyItemPtr bodyItem = this->currentObjects_[device->body()];
          if(!bodyItem) {
            std::cerr << "!bodyItem" << std::endl;
            continue;
          }
          cnoid::SimulationBodyPtr simBody = this->simulatorItem_->findSimulationBody(bodyItem);
          if(!simBody) {
            std::cerr << "!simBody" << std::endl;
            continue;
          }
          cnoid::DevicePtr simDevice = simBody->body()->findDevice(device->name());
          simDevice->sigStateChanged().connect([device, bodyItem, simBody, simDevice]{
                                                 cnoid::CameraPtr camera = cnoid::dynamic_pointer_cast<cnoid::Camera>(device);
                                                 cnoid::RangeCameraPtr rangeCamera = cnoid::dynamic_pointer_cast<cnoid::RangeCamera>(device);
                                                 cnoid::RangeSensorPtr rangeSensor = cnoid::dynamic_pointer_cast<cnoid::RangeSensor>(device);
                                                 if(camera){
                                                   cnoid::CameraPtr cameraSim = cnoid::dynamic_pointer_cast<cnoid::Camera>(simDevice);
                                                   std::shared_ptr<cnoid::Image> image = std::make_shared<cnoid::Image>(cameraSim->constImage());
                                                   camera->setImage(image);
                                                   if(rangeCamera){
                                                     cnoid::RangeCameraPtr rangeCameraSim = cnoid::dynamic_pointer_cast<cnoid::RangeCamera>(simDevice);
                                                     std::shared_ptr<std::vector<cnoid::Vector3f> > points = std::make_shared<std::vector<cnoid::Vector3f> >(rangeCameraSim->constPoints());
                                                     rangeCamera->setPoints(points);
                                                     rangeCamera->setDense(rangeCameraSim->isDense());
                                                   }
                                                 }else if(rangeSensor){
                                                   cnoid::RangeSensorPtr rangeSensorSim = cnoid::dynamic_pointer_cast<cnoid::RangeSensor>(simDevice);
                                                   std::shared_ptr<cnoid::RangeSensor::RangeData> rangeData = std::make_shared<cnoid::RangeSensor::RangeData>(rangeSensorSim->rangeData());
                                                   rangeSensor->setRangeData(rangeData);
                                                 }
                                               });
        }
                                                           });
      // sleepしない
      this->simulatorItem_->setRealtimeSyncMode(cnoid::SimulatorItem::NonRealtimeSync);

      this->worldItem_->addChildItem(this->simulatorItem_);
      ;});
  }

  Viewer::~Viewer(){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    cnoid::callSynchronously([&](){
      for(std::unordered_map<cnoid::BodyPtr, cnoid::BodyItemPtr>::iterator it = this->currentObjects_.begin(); it != this->currentObjects_.end(); it++){
        it->second->removeFromParentItem();
      }
      for(std::unordered_set<cnoid::SgNodePtr>::iterator it= this->currentDrawOn_.begin(); it != this->currentDrawOn_.end(); it++){
        this->markerGroup_->removeChild(*it);
      }
      for(std::unordered_map<cnoid::DevicePtr, cnoid::GLVisionSimulatorItemPtr>::iterator it=this->currentCameras_.begin(); it != this->currentCameras_.end();it++){
        it->second->removeFromParentItem();
      }
      cnoid::SceneView::instance()->sceneWidget()->sceneRoot()->removeChild(this->markerGroup_);
      this->simulatorItem_->removeFromParentItem();
      this->worldItem_->removeFromParentItem();
                             });
  }

  void Viewer::objects(const std::unordered_set<cnoid::BodyPtr>& objs){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->nextObjects_ = objs;
  }
  void Viewer::objects(const std::vector<cnoid::BodyPtr>& objs){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->nextObjects_ = std::unordered_set<cnoid::BodyPtr>(objs.begin(),objs.end());
  }
  void Viewer::objects(cnoid::BodyPtr obj){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->nextObjects_.insert(obj);
  }

  void Viewer::drawOn(const std::unordered_set<cnoid::SgNodePtr>& objs){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->nextDrawOn_ = objs;
  }
  void Viewer::drawOn(const std::vector<cnoid::SgNodePtr>& objs){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->nextDrawOn_ = std::unordered_set<cnoid::SgNodePtr>(objs.begin(),objs.end());
  }
  void Viewer::drawOn(cnoid::SgNodePtr obj){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->nextDrawOn_.insert(obj);
  }

  void Viewer::cameras(const std::unordered_set<cnoid::DevicePtr>& cameras){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->nextCameras_ = cameras;
  }
  void Viewer::cameras(const std::vector<cnoid::DevicePtr>& cameras){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->nextCameras_ = std::unordered_set<cnoid::DevicePtr>(cameras.begin(),cameras.end());
  }
  void Viewer::cameras(cnoid::DevicePtr camera){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->nextCameras_.insert(camera);
  }

  void Viewer::drawObjects(bool flush){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    this->notify(flush);
  }

  void Viewer::flush(){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    //std::lock_guard<std::mutex> guard(this->mutex_);

    if(this->currentCameras_.size() > 0){
      std::unique_lock<std::mutex> lock(this->pauseMtx_);
      this->pauseCv_.wait(lock, [&] {return this->simulatorItem_->isPausing(); }); // choreonoid::SimulatorItemはpause中はsleep(50ms)のloopがまわるので、50ms以上の頻度でdrawObjects, flushを呼ぶことはできない.
    }

    cnoid::callSynchronously([&](){
      QCoreApplication::processEvents(QEventLoop::AllEvents); // viewerへの描画 & カメラ書き込み
    });

  }

  class LinkState{
  public:
    double q;
    double u;
    double dq;
    double ddq;
    double q_target;
    double dq_target;
    cnoid::Isometry3 T;
    cnoid::Vector3 v;
    cnoid::Vector3 w;
    cnoid::Vector3 dv;
    cnoid::Vector3 dw;
    cnoid::Vector6 F_ext;
  };

  void Viewer::notify(bool flush){

    cnoid::callSynchronously([&](){

      bool stopSimulationRequired = false;

      // objectsで指定されたbodyに加えて、camerasで指定されたcameraが属するbodyを描画する.
      std::unordered_set<cnoid::BodyPtr> nextObjects;
      nextObjects.insert(this->nextObjects_.begin(),this->nextObjects_.end());
      for(std::unordered_set<cnoid::DevicePtr>::iterator it = this->nextCameras_.begin(); it!=this->nextCameras_.end();it++){
        nextObjects.insert((*it)->body());
      }

      // 消滅したobjectsを削除
      for(std::unordered_map<cnoid::BodyPtr, cnoid::BodyItemPtr>::iterator it=this->currentObjects_.begin(); it != this->currentObjects_.end();){
        if(nextObjects.find(it->first) != nextObjects.end()) it++;
        else{
          it->second->removeFromParentItem();
          it = this->currentObjects_.erase(it);
          stopSimulationRequired = true;
        }
      }

      // 増加したobjectsを反映
      for(std::unordered_set<cnoid::BodyPtr>::iterator it=nextObjects.begin(); it != nextObjects.end(); it++){
        if(this->currentObjects_.find(*it) != this->currentObjects_.end()) continue;

        cnoid::BodyItemPtr bodyItem = new cnoid::BodyItem();
        {
          // 今の状態を保存
          std::vector<LinkState> linkStates;
          for(int i=0;i<(*it)->numLinks();i++){
            LinkState linkState;
            linkState.q = (*it)->link(i)->q();
            linkState.u = (*it)->link(i)->u();
            linkState.dq = (*it)->link(i)->dq();
            linkState.ddq = (*it)->link(i)->ddq();
            linkState.q_target = (*it)->link(i)->q_target();
            linkState.dq_target = (*it)->link(i)->dq_target();
            linkState.T = (*it)->link(i)->T();
            linkState.v = (*it)->link(i)->v();
            linkState.w = (*it)->link(i)->w();
            linkState.dv = (*it)->link(i)->dv();
            linkState.dw = (*it)->link(i)->dw();
            linkState.F_ext = (*it)->link(i)->F_ext();
            linkStates.push_back(linkState);
          }
          bodyItem->setBody(*it); // Body::InitializePositionが呼ばれてしまうので、戻す必要がある
          // 今の状態に戻す
          for(int i=0;i<(*it)->numLinks();i++){
            (*it)->link(i)->q() = linkStates[i].q;
            (*it)->link(i)->u() = linkStates[i].u;
            (*it)->link(i)->dq() = linkStates[i].dq;
            (*it)->link(i)->ddq() = linkStates[i].ddq;
            (*it)->link(i)->q_target() = linkStates[i].q_target;
            (*it)->link(i)->dq_target() = linkStates[i].dq_target;
            (*it)->link(i)->T() = linkStates[i].T;
            (*it)->link(i)->v() = linkStates[i].v;
            (*it)->link(i)->w() = linkStates[i].w;
            (*it)->link(i)->dv() = linkStates[i].dv;
            (*it)->link(i)->dw() = linkStates[i].dw;
            (*it)->link(i)->F_ext() = linkStates[i].F_ext;
          }
          //initializeDeviceStatesの対応もした方がいい? TODO
        }
        this->worldItem_->addChildItem(bodyItem);
        bodyItem->setChecked(true);
        this->currentObjects_[*it] = bodyItem;
        stopSimulationRequired = true;
      }

      // 消滅したDrawOnを削除
      for(std::unordered_set<cnoid::SgNodePtr>::iterator it=this->currentDrawOn_.begin(); it != this->currentDrawOn_.end();){
        if(this->nextDrawOn_.find(*it) != this->nextDrawOn_.end()) it++;
        else{
          if(this->markerGroup_) this->markerGroup_->removeChild(*it);
          it = this->currentDrawOn_.erase(it);
        }
      }

      // 増加したDrawOnを反映
      for(std::unordered_set<cnoid::SgNodePtr>::iterator it=this->nextDrawOn_.begin(); it != this->nextDrawOn_.end(); it++){
        if(this->currentDrawOn_.find(*it) != this->currentDrawOn_.end()) continue;

        if(!this->markerGroup_) {
          this->markerGroup_ = new cnoid::SgGroup;
          this->markerGroup_->setName("Marker");
          cnoid::SceneView::instance()->sceneWidget()->sceneRoot()->addChild(this->markerGroup_);
        }

        this->markerGroup_->addChild(*it);
        this->currentDrawOn_.insert(*it);
      }

      // 消滅したcamerasを削除
      for(std::unordered_map<cnoid::DevicePtr, cnoid::GLVisionSimulatorItemPtr>::iterator it=this->currentCameras_.begin(); it != this->currentCameras_.end();){
        if(this->nextCameras_.find(it->first) != this->nextCameras_.end()) it++;
        else{
          it->second->removeFromParentItem();
          it = this->currentCameras_.erase(it);
          stopSimulationRequired = true;
        }
      }

      // 増加したcamerasを反映
      for(std::unordered_set<cnoid::DevicePtr>::iterator it=this->nextCameras_.begin(); it != nextCameras_.end(); it++){
        if(this->currentCameras_.find(*it) != this->currentCameras_.end()) continue;

        cnoid::GLVisionSimulatorItemPtr gLVisionSimulatorItem = new cnoid::GLVisionSimulatorItem();
        gLVisionSimulatorItem->setTargetBodies((*it)->body()->name());
        gLVisionSimulatorItem->setTargetSensors((*it)->name());
        gLVisionSimulatorItem->setMaxLatency(0.0); // レンダリングはマルチスレッドで非同期で行われ、レンダリング完了後のpostDynamics時に更新される. BestEffortMode=falseの時、レンダリング開始からmin(1/frameRate,maxLatency) step経過しても完了していない場合、レンダリング完了を待つ同期が行われる. flush()実行後にレンダリング結果が受け取れていることを保証したいので、maxlatency=0とする.
        gLVisionSimulatorItem->setBestEffortMode(false);
        this->simulatorItem_->addChildItem(gLVisionSimulatorItem);
        this->currentCameras_[*it] = gLVisionSimulatorItem;
        stopSimulationRequired = true;
      }

      for(std::unordered_map<cnoid::BodyPtr, cnoid::BodyItemPtr>::iterator it = this->currentObjects_.begin(); it != this->currentObjects_.end(); it++){
        it->second->notifyKinematicStateChange();
      }
      for(std::unordered_set<cnoid::SgNodePtr>::iterator it= this->currentDrawOn_.begin(); it != this->currentDrawOn_.end(); it++){
        (*it)->notifyUpdate();
      }

      if(this->timeStep != this->simulatorItem_->worldTimeStep()) stopSimulationRequired = true;  // setTimeStepで設定した値はstartSimulationを実行するまでは反映されないので.
      if(stopSimulationRequired){
        this->simulatorItem_->stopSimulation();
      }
      if(this->currentCameras_.size() > 0){
        if(this->simulatorItem_->isRunning()){

          const std::vector<cnoid::SimulationBody*>& simulationBodies = this->simulatorItem_->simulationBodies();
          for(int i=0;i<simulationBodies.size();i++){
            cnoid::BodyPtr body = simulationBodies[i]->bodyItem()->body();
            cnoid::BodyPtr simBody = simulationBodies[i]->body();

            simBody->rootLink()->T() = body->rootLink()->T();
            for(int j=0;j<body->numLinks();j++){
              simBody->link(j)->q() = body->link(j)->q();
              simBody->link(j)->u() = body->link(j)->u();
              simBody->link(j)->dq() = body->link(j)->dq();
              simBody->link(j)->ddq() = body->link(j)->ddq();
              simBody->link(j)->q_target() = body->link(j)->q_target();
              simBody->link(j)->dq_target() = body->link(j)->dq_target();
              simBody->link(j)->T() = body->link(j)->T();
              simBody->link(j)->v() = body->link(j)->v();
              simBody->link(j)->w() = body->link(j)->w();
              simBody->link(j)->dv() = body->link(j)->dv();
              simBody->link(j)->dw() = body->link(j)->dw();
              simBody->link(j)->F_ext() = body->link(j)->F_ext();
            }
          }

          this->simulatorItem_->restartSimulation(); // ここでは現在のBodyItemの状態がdyBodyにコピーされないので、上の処理で手動でコピーする. GLVisionSimulatorItemがレンダリングするのはdyBody
        }else{
          this->simulatorItem_->setTimeStep(this->timeStep);
          this->simulatorItem_->startSimulation(false/*doReset*/); // ここで現在のBodyItemの状態がdyBodyにコピーされる. GLVisionSimulatorItemがレンダリングするのはdyBody
        }
      }

    }); // callSync

    if(flush) this->flush();
  }


}

