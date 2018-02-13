#ifndef _AIC_PATROL
#define _AIC_PATROL

#include "ai_controller.h"
#include "entity/common_msgs.h"


struct Waypoint {
	VEC3 position;
	VEC3 lookAt;				//TODO: guess how to make the waypoint orientated to something
};

class CAIPatrol : public IAIController
{

	/* Atributes */
	std::vector<Waypoint> _waypoints;
	int currentWaypoint;


	//float speed;
	//float rotationSpeed;
	//float fov;
	//float distToIdleWar;
	//float distToBack;
	//float distToChase;
	//std::string entityToChase;
	//int life;

	/* Timers */
	//float idleWarTimer = 0.f;
	//float idleWarTimerMax = 0.f;
	//float idleWarTimerBase;
	//int idleWarTimerExtra;
	//float currentOrbitRotation = 0.f;
	//float orbitRotationMax = 0.f;
	//float orbitRotationBase;
	//int orbitRotationExtra;

	DECL_SIBLING_ACCESS();

	void onMsgDamage(const TMsgDamage& msg);

public:
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();

	void IdleState(float);				//Does nothing
	void SeekWptState(float);				//Moves to currentWpt
	void WaitInWptState(float);			//Rotates to the desired position and wait for x time
	void NextWptState(float);				//Calculates next currentWpt
	void ClosestWptState(float);			//Calculates the closest waypoint
	void SuspectState(float);				//The enemy thinks it may have seen the player. More or less time depending different factors
	void ShootInhibitorState(float);		//The enemy shoots the inhibitor to the player
	void ChaseState(float);				//Goes towards the player's position				
	void AttackState(float);				//Attack the player
	void IdleWarState(float);				//Waiting before attacks
	void BeginAlertState(float);			//Turn on lintern. Enter in alert state (easy to see the player)
	void GoToNoiseState(float);			//The enemy goes to the location where he heard something
	void GoToPatrolState(float);			//The enemy goes to the location of a stunned enemy in order to repair it
	void FixOtherPatrolState(float);		//The enemy fixes another enemy
	void GoPlayerLastPosState(float);		//The enemy goes to the last position he has seen the player
	void SeekPlayerState(float);			//The enemy keeps searching the player a bit more
	void StunnedState(float);				//The enemy has been stunned by the player
	void FixedState(float);				//The enemy has been fixed by another enemy
	void ShadowMergedState(float);		//The enemy has been shadowmerged by the player
  
	void ChooseOrbitSideState(float);		
	void OrbitLeftState(float);
	void OrbitRightState(float);
	void BackState(float);
	void HitState(float);
	void DeadState(float);

  void Init();

  void addWaypoint(Waypoint wpt) { _waypoints.push_back(wpt); };
  Waypoint getWaypoint() { return _waypoints[currentWaypoint]; }

  static void registerMsgs();
};

#endif