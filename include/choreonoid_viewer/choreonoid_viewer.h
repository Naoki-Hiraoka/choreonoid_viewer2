#ifndef CHOREONOID_VIEWER_H
#define CHOREONOID_VIEWER_H

#include <mutex>
#include <cnoid/SceneDrawables>
#include <cnoid/BodyItem>
#include <unordered_set>
#include <unordered_map>
#include <choreonoid_viewer/ViewerBaseItem.h>

namespace choreonoid_viewer {

  // 各メンバ関数はthread safeにする
  class Viewer {
  public:
    void objects(const std::unordered_set<cnoid::BodyPtr>& objs); // objectsをすべて消して新しくする
    void objects(const std::vector<cnoid::BodyPtr>& objs); // objectsをすべて消して新しくする
    void objects(cnoid::BodyPtr obj); // objectsに追加する

    void drawOn(const std::unordered_set<cnoid::SgNodePtr>& objs); // drawOnをすべて消して新しくする
    void drawOn(const std::vector<cnoid::SgNodePtr>& objs); // drawOnをすべて消して新しくする
    void drawOn(cnoid::SgNodePtr obj); // drawOnを追加する

    void drawObjects(bool flush=false); // 描画対象を更新する. calcForwardKinematicsは自分で呼ぶこと

    void flush(); // 今,描画を更新する. main threadで実行する必要がある. 別threadで実行している場合は、flushを呼ばなくても勝手に更新されるはず

  protected:
    void notify(bool flush);

    std::unordered_set<cnoid::BodyPtr> nextObjects_;
    std::unordered_set<cnoid::SgNodePtr> nextDrawOn_;

    std::unordered_map<cnoid::BodyPtr, cnoid::BodyItemPtr> currentObjects_;
    std::unordered_set<cnoid::SgNodePtr> currentDrawOn_;
    cnoid::SgGroupPtr markerGroup_;

    std::mutex mutex_;
  };
}

#endif
