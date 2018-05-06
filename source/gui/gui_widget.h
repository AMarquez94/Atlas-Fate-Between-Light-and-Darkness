#pragma once

#include "gui_params.h"
#include "gui_effect.h"

namespace GUI
{
  class CWidget;
  using VWidgets = std::vector<CWidget*>;

  class CWidget
  {
  public:
    CWidget();

    void addChild(CWidget* wdgt);
    void removeChild(CWidget* wdgt);
    CWidget* getChild(const std::string& name, bool recursive = false) const;
    const std::string& getName() const;
    virtual TImageParams* getImageParams() { return nullptr; }
    virtual TTextParams* getTextParams() { return nullptr; }

    void addEffect(CEffect* fx);

    void computeLocal();
    void computeAbsolute();

    void updateAll(float delta);
    void renderAll();

    virtual void update(float delta);
    virtual void render();

  protected:
    std::string _name;
    VWidgets _children;
    VEffects _effects;
    CWidget* _parent = nullptr;
    TParams _params;
    MAT44 _local;
    MAT44 _absolute;

    friend class CParser;
  };
}