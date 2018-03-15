#pragma once

#include "state.h"

namespace FSM
{
  class AnimationState : public IState
  {
    virtual void onStart(CContext& ctx) const override;
    virtual bool load(const json& jData);
  };

  /*class ChangeMeshState : public IState
  {
    virtual void onStart(CContext& ctx) const override 
    {
      TCompRender* render = ...;
      CMesh* mesh = Resources.getMesh(meshName);
      render->setMesh(mesh);

      TCompPhysics* c_physics;
      c_physics->addImpulse(VEC3(0, 1, 0) * _force);
    }

  private:
    std::string& meshName;
  };*/


  /*class JumpState : public IState
  {
    virtual void onStart(CContext& ctx) const override
    {
      TCompPhysics* c_physics;
      c_physics->addImpulse(VEC3(0, 1, 0) * _jumpForce);
    }

  private:
    float _jumpForce;
  };*/
}
