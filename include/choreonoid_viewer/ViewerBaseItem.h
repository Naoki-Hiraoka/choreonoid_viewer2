#ifndef CHOREONOID_VIEWER_VIEWER_BASE_ITEM_H
#define CHOREONOID_VIEWER_VIEWER_BASE_ITEM_H

#include <cnoid/Item>
#include <thread>

namespace choreonoid_viewer {

  class ViewerBaseItem : public cnoid::Item
  {
  public:
    static void initializeClass(cnoid::ExtensionManager* ext);

    ViewerBaseItem();

  protected:

    virtual void main();

  private:
    void main_common();

    std::thread thread_;
  };

  typedef cnoid::ref_ptr<ViewerBaseItem> ViewerBaseItemPtr;
}

#endif
