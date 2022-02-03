#include <choreonoid_viewer/choreonoid_viewer.h>

#include <QCoreApplication>
#include <cnoid/RootItem>
#include <cnoid/SceneProvider>
#include <cnoid/SceneView>
#include <cnoid/LazyCaller>
#include <cnoid/ItemTreeView>
#include <iostream>

namespace choreonoid_viewer {
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

  void Viewer::drawObjects(bool flush){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    std::lock_guard<std::mutex> guard(this->mutex_);

    cnoid::callSynchronously([&](){this->notify();});

    if(flush) this->flush();
  }

  void Viewer::flush(){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;

    //std::lock_guard<std::mutex> guard(this->mutex_);

    QCoreApplication::processEvents(QEventLoop::AllEvents);
  }

  class LinkState{
  public:
    double q;
    double u;
    double dq;
    double ddq;
    double q_target;
    double dq_target;
    cnoid::Vector3 v;
    cnoid::Vector3 w;
    cnoid::Vector3 dv;
    cnoid::Vector3 dw;
    cnoid::Vector6 F_ext;
  };

  void Viewer::notify(){
    // 消滅したobjectsを削除
    for(std::unordered_map<cnoid::BodyPtr, cnoid::BodyItemPtr>::iterator it=this->currentObjects_.begin(); it != this->currentObjects_.end();){
      if(this->nextObjects_.find(it->first) != this->nextObjects_.end()) it++;
      else{
        it->second->detachFromParentItem();
        it = this->currentObjects_.erase(it);
      }
    }

    // 増加したobjectsを反映
    for(std::unordered_set<cnoid::BodyPtr>::iterator it=this->nextObjects_.begin(); it != this->nextObjects_.end(); it++){
      if(this->currentObjects_.find(*it) != this->currentObjects_.end()) continue;

      cnoid::BodyItemPtr bodyItem = new cnoid::BodyItem();
      {
        // 今の状態を保存
        cnoid::Position rootT = (*it)->rootLink()->T();
        std::vector<LinkState> linkStates;
        for(int i=0;i<(*it)->numLinks();i++){
          LinkState linkState;
          linkState.q = (*it)->link(i)->q();
          linkState.u = (*it)->link(i)->u();
          linkState.dq = (*it)->link(i)->dq();
          linkState.ddq = (*it)->link(i)->ddq();
          linkState.q_target = (*it)->link(i)->q_target();
          linkState.dq_target = (*it)->link(i)->dq_target();
          linkState.v = (*it)->link(i)->v();
          linkState.w = (*it)->link(i)->w();
          linkState.dv = (*it)->link(i)->dv();
          linkState.dw = (*it)->link(i)->dw();
          linkState.F_ext = (*it)->link(i)->F_ext();
          linkStates.push_back(linkState);
        }
        bodyItem->setBody(*it); // Body::InitializePositionが呼ばれてしまうので、戻す必要がある
        // 今の状態に戻す
        (*it)->rootLink()->T() = rootT;
        for(int i=0;i<(*it)->numLinks();i++){
          (*it)->link(i)->q() = linkStates[i].q;
          (*it)->link(i)->u() = linkStates[i].u;
          (*it)->link(i)->dq() = linkStates[i].dq;
          (*it)->link(i)->ddq() = linkStates[i].ddq;
          (*it)->link(i)->q_target() = linkStates[i].q_target;
          (*it)->link(i)->dq_target() = linkStates[i].dq_target;
          (*it)->link(i)->v() = linkStates[i].v;
          (*it)->link(i)->w() = linkStates[i].w;
          (*it)->link(i)->dv() = linkStates[i].dv;
          (*it)->link(i)->dw() = linkStates[i].dw;
          (*it)->link(i)->F_ext() = linkStates[i].F_ext;
        }
        (*it)->calcForwardKinematics(true, true);
        //initializeDeviceStatesの対応もした方がいい? TODO
      }
      cnoid::RootItem::instance()->addChildItem(bodyItem);
      cnoid::ItemTreeView::instance()->checkItem(bodyItem, true);
      this->currentObjects_[*it] = bodyItem;
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

    for(std::unordered_map<cnoid::BodyPtr, cnoid::BodyItemPtr>::iterator it = this->currentObjects_.begin(); it != this->currentObjects_.end(); it++){
      it->second->notifyKinematicStateChange();
    }
    for(std::unordered_set<cnoid::SgNodePtr>::iterator it= this->currentDrawOn_.begin(); it != this->currentDrawOn_.end(); it++){
      (*it)->notifyUpdate();
    }
  }
}

