#ifndef INC_COMMON_ENTITY_MSGS_
#define INC_COMMON_ENTITY_MSGS_

#include "msgs.h"
#include "geometry/transform.h"

struct TMsgEntityCreated {
	DECL_MSG_ID();
};

struct TMsgSceneCreated {
	DECL_MSG_ID();
};

struct TMsgEnteredCapsuleShadow {
	DECL_MSG_ID();
};
struct TMsgExitedCapsuleShadow {
	DECL_MSG_ID();
};

struct TMsgEntityDestroyed {
	CHandle h_other_entity;
	DECL_MSG_ID();
};

struct TMsgGlitchController {
    bool revert;
    DECL_MSG_ID();
};

struct TMsgAssignBulletOwner {
	CHandle h_owner;
	DECL_MSG_ID();
};

struct TMsgDamage {
	CHandle h_sender;
	CHandle h_bullet;
	DECL_MSG_ID();
};

// Sent to all entities from a parsed file once all the entities
// in that file has been created. Used to link entities between them
struct TEntityParseContext;

struct TMsgEntitiesGroupCreated {
	const TEntityParseContext& ctx;
	DECL_MSG_ID();
};

struct TMsgHierarchyGroupCreated {
  const TEntityParseContext& ctx;
  DECL_MSG_ID();
};

struct TMsgPlayerDead {
	CHandle h_sender;
	DECL_MSG_ID();
};

struct TMsgEnemyStunned {
	CHandle h_sender;
	DECL_MSG_ID();
};

struct TMsgEnemyStunnedCheckpoint {
    CHandle h_sender;
    DECL_MSG_ID();
};

struct TMsgPlayerHit {
	CHandle h_sender;
	DECL_MSG_ID();
};

struct TMsgPlayerStunned {
    CHandle h_sender;
    DECL_MSG_ID();
};

struct TMsgBulletHit {
    CHandle h_sender;
    float damage;
    DECL_MSG_ID();
};

struct TMsgInhibitorShot {
	CHandle h_sender;
	DECL_MSG_ID();
};

struct TMsgPatrolShadowMerged {
	CHandle h_sender;
	CHandle h_objective;
	DECL_MSG_ID();
};

struct TMsgPatrolFixed {
	CHandle h_sender;
	DECL_MSG_ID();
};

struct TMsgPlayerIlluminated {
	CHandle h_sender;
  bool isIlluminated;
	DECL_MSG_ID();
};

struct TMsgSonarActive {
    float value;

    DECL_MSG_ID();
};

struct TMsgFadeBody {
    bool is_exit;
	float fade_time = -1.0f;
	VEC4 fade_color = VEC4(-1,0,0,0);
    DECL_MSG_ID();
};

struct TMsgVideoStatus {
    bool status;
    DECL_MSG_ID();
};

struct TMsgTriggerEnter {
	CHandle h_other_entity;
	DECL_MSG_ID();
};

struct TMsgTriggerExit {
	CHandle h_other_entity;
	DECL_MSG_ID();
};

struct TMsgPhysxContact {
	CHandle other_entity;
	DECL_MSG_ID();
};

struct TMsgPhysxContactLost {
    CHandle other_entity;
    DECL_MSG_ID();
};

struct TMsgSetCameraActive {
	std::string previousCamera;
	std::string actualCamera;
	VEC3 directionToLookAt;
	DECL_MSG_ID();
};

struct TMsgSetCameraCancelled {
	DECL_MSG_ID();
};

struct TMsgCameraActivated {
    CHandle previousCamera;
	DECL_MSG_ID();
};

struct TMsgCameraDeprecated {
	DECL_MSG_ID();
};

struct TMsgCameraFullyActivated {
	DECL_MSG_ID();
};

struct TMsgCameraReset {
  bool both_angles;
  bool only_y;
  DECL_MSG_ID();
};

struct TMsgCameraResetTargetPos {
    DECL_MSG_ID();
};

struct TMsgAnimationCallback {
	std::string function_to_call;
	DECL_MSG_ID();
};

struct TMsgAnimationPlaced {
	DECL_MSG_ID();
};

struct TMsgSkeletonPlaceAnimation {
	DECL_MSG_ID();
};

struct TMsgAnimationCompleted {
	std::string animation_name;
	DECL_MSG_ID();
};

struct TMsgAnimationAudioCallback {
    std::string audioName;
    bool isRelativeToPlayer;
    DECL_MSG_ID();
};

struct TMsgTimeOut {
	DECL_MSG_ID();
};

struct TMsgSpawn {
	DECL_MSG_ID();
};

struct TMsgDefineLocalAABB {
	AABB* aabb;
	DECL_MSG_ID();
};

struct TMsgSpawnAt {
	CTransform where;
	DECL_MSG_ID();
};

struct TMsgScenePaused {
	bool isPaused;
	DECL_MSG_ID();
};

struct TMsgCameraShake {
	float time_to_stop;
	float amount;
	float speed;
	DECL_MSG_ID();
};

struct TMsgAIPaused {
    bool isPaused;
    DECL_MSG_ID();
};

struct TMsgConsoleOn {
	bool isConsoleOn;
	DECL_MSG_ID();
};

struct TMsgSystemNoClipToggle {
    DECL_MSG_ID();
};

struct TMsgNoClipToggle {
    DECL_MSG_ID();
};

struct TMsgNoiseMade {
	CHandle hNoiseSource;
	VEC3 noiseOrigin;
	bool isArtificialNoise;
	DECL_MSG_ID();
};

struct TMsgOrderReceived {
	CHandle hOrderSource;
	VEC3 position;
	DECL_MSG_ID();
};

struct TMsgShadowChange {
	bool is_shadowed;
	DECL_MSG_ID();
};

struct TMsgSceneLoaded {
    std::string scene_name;
    DECL_MSG_ID();
};

struct TMsgSetVisible {
    bool visible;
    DECL_MSG_ID();
};

struct TMsgObjectBeingMoved {
  bool isBeingMoved;
  CHandle hMovingObject;
  VEC3 direction;
  float speed;
  DECL_MSG_ID();
};

struct TMsgOmniFadeOut {
	
	DECL_MSG_ID();
};

struct TMsgInfiniteStamina {
    DECL_MSG_ID();
};

struct TMsgSpeedBoost {
    float speedBoost;
    DECL_MSG_ID();
};

struct TMsgPlayerInvisible {
    DECL_MSG_ID();
};

struct TMsgPlayerImmortal {
	bool state = true;
    DECL_MSG_ID();
};

struct TMsgPlayerInShadows {
    DECL_MSG_ID();
};

struct TMsgLanternsDisable {
    bool disable;
    DECL_MSG_ID();
};

struct TMsgButtonActivated {
    DECL_MSG_ID();
};

struct TMsgWeaponsActivated {
    bool activate;
    DECL_MSG_ID();
};

struct TMsgStopAudioComponent {
    DECL_MSG_ID();
};

struct TMsgEntityCanLandSM {
    bool canSM;
    DECL_MSG_ID();
};

struct TMsgPlayerAIEnabled {
    std::string state;
    bool enableAI;
    DECL_MSG_ID();
};

struct TMsgEmisiveCapsuleState {
	bool enable;
	bool random;
	float time_to_lerp;
	DECL_MSG_ID();
};

struct TMsgRotatorAccelerate {
	float speed;
	float time_to_lerp;
	float time_to_start;
	DECL_MSG_ID();
};

struct TMsgCinematicState {
    std::string state;
    bool enableCinematic;
    DECL_MSG_ID();
};

struct TMsgWarnEnemy {
    VEC3 playerPosition;
    DECL_MSG_ID();
};

struct TMsgResetPatrolLights {
    DECL_MSG_ID();
};

struct TMsgCameraFov {
    float new_fov;
    float blend_time;
    DECL_MSG_ID();
};

struct TMsgCircularControllerTarget {
    CHandle new_target;
    DECL_MSG_ID();
};

struct TMsgEnemySuspecting {
    CHandle enemy_suspecting;
    bool is_suspecting;
    DECL_MSG_ID();
};

struct TMsgEnemyNothingHere {
    CHandle enemy;
    VEC3 position;
    DECL_MSG_ID();
};

//struct TMsgSpotlightsToggle {
//    DECL_MSG_ID();
//};

#endif

