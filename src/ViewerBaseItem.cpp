#include <choreonoid_viewer/ViewerBaseItem.h>

#include <iostream>
#include <cnoid/LazyCaller>
#include <cnoid/ItemManager>

namespace cnoid {
  void ViewerBaseItem::initializeClass(ExtensionManager* ext)
  {
    ext->itemManager()
      .registerClass<ViewerBaseItem>("ViewerBaseItem");
    //.addCreationPanel<ViewerSampleItem>() プラグインをロードしただけで勝手にこのItemのコンストラクタが呼ばれてしまう
  }

  ViewerBaseItem::ViewerBaseItem() {
    cnoid::callLater([&](){this->main_common();});
  }

  void ViewerBaseItem::main_common()
  {
    this->thread_ = std::thread([&](){this->main();});
  }

  void ViewerBaseItem::main()
  {
  }

}

