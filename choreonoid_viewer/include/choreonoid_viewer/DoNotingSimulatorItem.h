#ifndef CHOREONOID_VIEWER_DONOTINGSIMULATORITEM_H
#define CHOREONOID_VIEWER_DONOTINGSIMULATORITEM_H

#include <cnoid/Item>
#include <thread>
#include <cnoid/SimulatorItem>

namespace choreonoid_viewer {

  class DoNotingSimulatorItem : public cnoid::SimulatorItem {
  public:
    static void initializeClass(cnoid::ExtensionManager* ext);
    DoNotingSimulatorItem();
  protected:
    virtual bool initializeSimulation(const std::vector<cnoid::SimulationBody*>& simBodies) override;
    virtual bool stepSimulation(const std::vector<cnoid::SimulationBody*>& activeSimBodies) override;
    virtual cnoid::SimulationBody* createSimulationBody(cnoid::Body* orgBody, cnoid::CloneMap& cloneMap) override;
  };
  typedef cnoid::ref_ptr<DoNotingSimulatorItem> DoNotingSimulatorItemPtr;
}

#endif
