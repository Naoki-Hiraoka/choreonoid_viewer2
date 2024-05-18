#include <cnoid/Plugin>
#include <cnoid/ItemManager>

#include <choreonoid_viewer/choreonoid_viewer.h>
#include "choreonoid_viewer_sample_lib.h"

namespace choreonoid_viewer{

  class ViewerSampleItem : public choreonoid_viewer::ViewerBaseItem
  {
  public:
    static void initializeClass(cnoid::ExtensionManager* ext){
      ext->itemManager().registerClass<ViewerSampleItem>("ViewerSampleItem");
    }
  protected:
    virtual void main() override{
      mymain();
    }
  };
  typedef cnoid::ref_ptr<ViewerSampleItem> ViewerSampleItemPtr;

  class ViewerSamplePlugin : public cnoid::Plugin
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

CNOID_IMPLEMENT_PLUGIN_ENTRY(choreonoid_viewer::ViewerSamplePlugin)
