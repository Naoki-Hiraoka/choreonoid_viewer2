#ifndef CHOREONOID_VIEWER_VIEWER_BASE_ITEM_H
#define CHOREONOID_VIEWER_VIEWER_BASE_ITEM_H

#include <cnoid/Item>
#include <thread>

namespace cnoid {

  class ViewerBaseItem : public Item
  {
  public:
    static void initializeClass(ExtensionManager* ext);

    ViewerBaseItem();

  protected:

    virtual void main();

  private:
    void main_common();

    std::thread thread_;
  };

  typedef ref_ptr<ViewerBaseItem> ViewerBaseItemPtr;
}

#endif
