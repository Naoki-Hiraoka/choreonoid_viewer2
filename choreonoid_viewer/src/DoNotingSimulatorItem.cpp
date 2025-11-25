#include <choreonoid_viewer/DoNotingSimulatorItem.h>

#include <iostream>
#include <cnoid/LazyCaller>
#include <cnoid/ItemManager>
#include <cnoid/Body>

namespace choreonoid_viewer {
  void DoNotingSimulatorItem::initializeClass(cnoid::ExtensionManager* ext)
  {
    ext->itemManager().registerClass<DoNotingSimulatorItem, cnoid::SimulatorItem>("DoNotingSimulatorItem");
    //ext->itemManager().addCreationPanel<DoNotingSimulatorItem>();
  }
  DoNotingSimulatorItem::DoNotingSimulatorItem()
    : SimulatorItem("DoNotingSimulatorItem")
  {
  }
  bool DoNotingSimulatorItem::initializeSimulation(const std::vector<cnoid::SimulationBody*>& simBodies) {
    return true;
  }
  bool DoNotingSimulatorItem::stepSimulation(const std::vector<cnoid::SimulationBody*>& activeSimBodies) {
    return true;
  }
  cnoid::SimulationBody* DoNotingSimulatorItem::createSimulationBody(cnoid::Body* orgBody, cnoid::CloneMap& cloneMap)
  {
    return new cnoid::SimulationBody(orgBody->clone(cloneMap));
  }
}

