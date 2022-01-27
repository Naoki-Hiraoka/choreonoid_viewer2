#include <cnoid/Plugin>
#include <cnoid/ItemManager>

#include <choreonoid_viewer/choreonoid_viewer.h>
#include "choreonoid_viewer_sample_lib.h"

namespace cnoid{

    class ViewerSampleItem : public ViewerBaseItem
  {
  public:
    static void initializeClass(ExtensionManager* ext){
      ext->itemManager().registerClass<ViewerSampleItem>("ViewerSampleItem");
    }
  protected:
    virtual void main() override{
      mymain();
    }
  };
  typedef ref_ptr<ViewerSampleItem> ViewerSampleItemPtr;

  class ViewerSamplePlugin : public Plugin
  {
  public:

    ViewerSamplePlugin() : Plugin("ViewerSample")
    {
      require("Body");
    }
    virtual bool initialize() override
    {
      ViewerSampleItem::initializeClass(this);
      return true;
    }
  };


}

CNOID_IMPLEMENT_PLUGIN_ENTRY(cnoid::ViewerSamplePlugin)
