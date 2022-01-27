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
    this->nextObjects_ = objs;
  }
  void Viewer::objects(const std::vector<cnoid::BodyPtr>& objs){
    this->nextObjects_ = std::unordered_set<cnoid::BodyPtr>(objs.begin(),objs.end());
  }
  void Viewer::objects(cnoid::BodyPtr obj){
    this->nextObjects_.insert(obj);
  }

  void Viewer::drawOn(const std::unordered_set<cnoid::SgNodePtr>& objs){
    this->nextDrawOn_ = objs;
  }
  void Viewer::drawOn(const std::vector<cnoid::SgNodePtr>& objs){
    this->nextDrawOn_ = std::unordered_set<cnoid::SgNodePtr>(objs.begin(),objs.end());
  }
  void Viewer::drawOn(cnoid::SgNodePtr obj){
    this->nextDrawOn_.insert(obj);
  }

  void Viewer::drawObjects(bool flush){
    // choreonoidが起動していないときは何もしない
    if(!QCoreApplication::instance()) return;
    cnoid::callSynchronously([&](){this->notify();});

    if(flush) this->flush();
  }

  void Viewer::flush(){
    QCoreApplication::processEvents(QEventLoop::AllEvents);
  }

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
      bodyItem->setBody(*it);
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

