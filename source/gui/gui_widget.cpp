#include "mcv_platform.h"
#include "gui/gui_widget.h"

using namespace GUI;

CWidget::CWidget()
{
    enabled = true;
}

void CWidget::addChild(CWidget* wdgt)
{
    if (wdgt && wdgt->_parent != this)
    {
        if (wdgt->_parent)
            wdgt->_parent->removeChild(wdgt);
        _children.push_back(wdgt);
        wdgt->_parent = this;
    }
}

void CWidget::removeChild(CWidget* wdgt)
{
    auto it = std::find(_children.begin(), _children.end(), wdgt);
    if (it != _children.end())
    {
        _children.erase(it);
        wdgt->_parent = nullptr;
    }
}

CWidget* CWidget::getChild(const std::string& name, bool recursive) const
{
    for (auto& child : _children)
    {
        if (child->_name == name)
        {
            return child;
        }
    }

    if (recursive)
    {
        for (auto& child : _children)
        {
            CWidget* wdgt = child->getChild(name, true);
            if (wdgt)
            {
                return wdgt;
            }
        }
    }

    return nullptr;
}

const std::string& CWidget::getName() const
{
    return _name;
}

void CWidget::addEffect(CEffect* fx)
{
    _effects.push_back(fx);
    fx->setWidget(this);
}

void CWidget::computeLocal()
{
    MAT44 tr = MAT44::CreateTranslation(_params._position.x, _params._position.y, 0.f);
    MAT44 rot = MAT44::CreateFromYawPitchRoll(0.f, 0.f, _params._rotation);
    MAT44 sc = MAT44::CreateScale(_params._scale.x, _params._scale.y, 1.f);
    _local = rot * sc * tr;
}

void CWidget::computeAbsolute()
{
    computeLocal();
    if (_parent)
    {
        _absolute = _local * _parent->_absolute;
    }
    else
    {
        _absolute = _local;
    }
    // update my children absolutes
    for (auto& child : _children)
        child->computeAbsolute();
}

void CWidget::updateAll(float delta)
{
    if (enabled) {
        update(delta);
        for (auto& fx : _effects)
        {
            fx->update(delta);
        }
        for (auto& child : _children)
        {
            child->updateAll(delta);
        }
    }
}

void CWidget::renderAll()
{
    if (enabled) {
        render();
        for (auto& child : _children)
        {
            child->renderAll();
        }
    }
}

void CWidget::update(float)
{
    // ...
}

void CWidget::render()
{
    // ...
}

void CWidget::enable(bool status) 
{
    enabled = status;
}