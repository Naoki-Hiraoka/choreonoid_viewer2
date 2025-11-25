#include <cnoid/Plugin>
#include <cnoid/ItemManager>

#include <choreonoid_viewer/choreonoid_viewer.h>

namespace choreonoid_viewer{

  class ViewerPlugin : public cnoid::Plugin
  {
  public:

    ViewerPlugin() : Plugin("Viewer")
    {
      require("Body");
    }
    virtual bool initialize() override
    {
      DoNotingSimulatorItem::initializeClass(this);
      ViewerBaseItem::initializeClass(this);
      return true;
    }
  };


}

CNOID_IMPLEMENT_PLUGIN_ENTRY(choreonoid_viewer::ViewerPlugin)
