#pragma once

#include "state.h"
#include "components/player_controller/comp_player_tempcontroller.h"

namespace FSM
{
	TargetCamera * getTargetCamera(const json& jData);
	
	Noise * getNoise(const json& jData);

	class IdleState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _radius = 0.3f;
		float _speed = 1.f;
		std::string _animationName;
	};

	class FallState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _force = 1.f;
		float _speed = 1.f;
		float _size = 1.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class WalkState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		float _rotation_speed = 1.f;
		std::string _animationName;
	};

  class WalkSlowState : public IState
  {
    virtual void onStart(CContext& ctx) const override;
    virtual void onFinish(CContext& ctx) const override;
    virtual bool load(const json& jData);

  private:
    TargetCamera * _target = nullptr;
    Noise * _noise = nullptr;
    float _size = 1.f;
    float _speed = 1.f;
    float _radius = 0.3f;
    float _rotation_speed = 1.f;
    std::string _animationName;
  };

	class RunState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		float _rotation_speed = 1.f;
		std::string _animationName;
	};

	class CrouchState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		float _rotation_speed = 1.f;
		std::string _animationName;
	};

    class SonarState : public IState
    {
        virtual void onStart(CContext& ctx) const override;
        virtual void onFinish(CContext& ctx) const override;
        virtual bool load(const json& jData);

    private:
        TargetCamera * _target = nullptr;
        Noise * _noise = nullptr;
        float _size = 1.f;
        float _speed = 1.f;
        float _radius = 0.3f;
        float _rotation_speed = 1.f;
        std::string _animationName;
    };

	class CrouchWalkState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		float _rotation_speed = 1.f;
		std::string _animationName;
	};

  class CrouchWalkSlowState : public IState
  {
    virtual void onStart(CContext& ctx) const override;
    virtual void onFinish(CContext& ctx) const override;
    virtual bool load(const json& jData);

  private:
    TargetCamera * _target = nullptr;
    Noise * _noise = nullptr;
    float _size = 1.f;
    float _speed = 1.f;
    float _radius = 0.3f;
    float _rotation_speed = 1.f;
    std::string _animationName;
  };

	class EnterMergeState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class MergeState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class ExitMergeState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

  class ExitMergeCrouchedState : public IState
  {
    virtual void onStart(CContext& ctx) const override;
    virtual void onFinish(CContext& ctx) const override;
    virtual bool load(const json& jData);

  private:
    TargetCamera * _target = nullptr;
    Noise * _noise = nullptr;
    float _size = 1.f;
    float _speed = 1.f;
    float _radius = 0.3f;
    std::string _animationName;
  };

	class SoftLandState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class HardLandState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class LandMergeState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);

	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 1.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class AttackState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);
	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 2.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class InhibitorRemovedState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);
	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 2.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class InhibitorTryToRemoveState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);
	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 2.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class DieState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);
	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 2.f;
		float _radius = 0.3f;
		std::string _animationName;
	};

	class DeadState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);
	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 2.f;
		float _radius = 0.3f;
		std::string _animationName;
	};
	class GrabEnemyState : public IState
	{
		virtual void onStart(CContext& ctx) const override;
		virtual void onFinish(CContext& ctx) const override;
		virtual bool load(const json& jData);
	private:
		TargetCamera * _target = nullptr;
		Noise * _noise = nullptr;
		float _size = 1.f;
		float _speed = 2.f;
		float _radius = 0.3f;
		std::string _animationName;
	};
  class MovingObjectState : public IState
  {
    virtual void onStart(CContext& ctx) const override;
    virtual void onFinish(CContext& ctx) const override;
    virtual bool load(const json& jData);
  private:
    TargetCamera * _target = nullptr;
    Noise * _noise = nullptr;
    float _size = 1.f;
    float _speed = 2.f;
    float _radius = 0.3f;
    std::string _animationName;
  };
  class PressingButtonState : public IState
  {
	  virtual void onStart(CContext& ctx) const override;
	  virtual void onFinish(CContext& ctx) const override;
	  virtual bool load(const json& jData);
  private:
	  TargetCamera * _target = nullptr;
	  Noise * _noise = nullptr;
	  float _size = 1.f;
	  float _speed = 2.f;
	  float _radius = 0.3f;
	  std::string _animationName;
  };
}
