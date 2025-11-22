#ifndef CHOREONOID_VIEWER_H
#define CHOREONOID_VIEWER_H

#include <mutex>
#include <condition_variable>
#include <cnoid/SceneDrawables>
#include <cnoid/BodyItem>
#include <unordered_set>
#include <unordered_map>
#include <cnoid/WorldItem>
#include <cnoid/GLVisionSimulatorItem>

#include <choreonoid_viewer/DoNotingSimulatorItem.h>
#include <choreonoid_viewer/ViewerBaseItem.h>

namespace choreonoid_viewer {

  // 各メンバ関数はthread safeにする
  class Viewer {
  public:
    Viewer();
    void objects(const std::unordered_set<cnoid::BodyPtr>& objs); // objectsをすべて消して新しくする
    void objects(const std::vector<cnoid::BodyPtr>& objs); // objectsをすべて消して新しくする
    void objects(cnoid::BodyPtr obj); // objectsに追加する

    void drawOn(const std::unordered_set<cnoid::SgNodePtr>& objs); // drawOnをすべて消して新しくする
    void drawOn(const std::vector<cnoid::SgNodePtr>& objs); // drawOnをすべて消して新しくする
    void drawOn(cnoid::SgNodePtr obj); // drawOnを追加する

    void cameras(const std::unordered_set<cnoid::DevicePtr>& cameras); // camerasをすべて消して新しくする
    void cameras(const std::vector<cnoid::DevicePtr>& cameras); // camerasをすべて消して新しくする
    void cameras(cnoid::DevicePtr camera); // camerasに追加する
    double timeStep = 0.001;

    void drawObjects(bool flush=true); // 描画対象を更新する. calcForwardKinematicsは自分で呼ぶこと

    void flush(); // 今,描画を更新する. main thread以外で実行している場合は、flushを呼ばなくても勝手に更新されるはずだが、robotなどの姿勢がすぐに変化する場合には、変化前に描画を更新する必要があるので、flushを呼んだ方がいい.

  protected:
    void notify(bool flush);

    std::unordered_set<cnoid::BodyPtr> nextObjects_;
    std::unordered_set<cnoid::SgNodePtr> nextDrawOn_;
    std::unordered_set<cnoid::DevicePtr> nextCameras_;

    std::unordered_map<cnoid::BodyPtr, cnoid::BodyItemPtr> currentObjects_;
    std::unordered_set<cnoid::SgNodePtr> currentDrawOn_;
    std::unordered_map<cnoid::DevicePtr, cnoid::GLVisionSimulatorItemPtr> currentCameras_;
    cnoid::SgGroupPtr markerGroup_;

    std::mutex mutex_;

    DoNotingSimulatorItemPtr simulatorItem_;
    cnoid::ref_ptr<cnoid::WorldItem> worldItem_;

    std::mutex pauseMtx_;
    std::condition_variable pauseCv_;

  };
}

#endif
