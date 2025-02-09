#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"
#include "components/player_controller/comp_player_animator.h"
#include "gui/gui_widget.h"

#define SM_THRESHOLD_MIN 0.025f
#define SM_THRESHOLD_MAX 0.2f
 
class TCompTempPlayerController;

typedef void (TCompTempPlayerController::*actionfinish)();
typedef void (TCompTempPlayerController::*actionhandler)(float);

struct TargetCamera {

    std::string name;
    float blendIn;
    float blendOut;
    float fov;
};

struct Noise {
    bool isOnlyOnce;
    float noiseRadius;
    float timeToRepeat;
    bool isNoise;
    bool isArtificial;
};

struct TMsgStateStart {

    actionhandler action_start;
    float speed;
    float size;
    float radius;

    TargetCamera * target_camera = nullptr;

    Noise * noise = nullptr;

    DECL_MSG_ID();
};

struct TMsgStateFinish {

    actionfinish action_finish;
    std::string meshname;
    float speed;

    DECL_MSG_ID();
};

class TCompTempPlayerController : public TCompBase
{
    struct StateColors {
        VEC4 colorIdle;
        VEC4 colorMerge;
        VEC4 colorDead;
        VEC4 colorInhib;
    } playerColor;

    physx::PxFilterData * pxPlayerFilterData;
    physx::PxFilterData * pxShadowFilterData;
    physx::PxQueryFilterData PxPlayerDiscardQuery;

    actionhandler state;
    CHandle target_camera;

    CHandle movableObject = CHandle();
    VEC3 directionMovableObject;
    float movingObjectSpeed;
    VEC3 movingObjectOffset;

    VEC3 temp_invert;
    //MAT44 temp_invert;
    float temp_deg;
    float *staminaBarValue;

    float mergeAngle;
    float currentSpeed;
    float rotationSpeed;
    float fallingTime;
    float maxFallingTime;
    float hardFallingTime;
    float fallingDistance;
    float startFallingDistance;
    float maxFallingDistance;
    float maxAttackDistance;
    std::string auxCamera;

    /* Stamina private variables */
    float stamina;
    float minStamina;
    float maxStamina;
    float incrStamina;
    float decrStaticStamina;
    float decrStaminaHorizontal;
    float decrStaminaVertical;
    float minStaminaChange;

    float maxLife = 100.f;
    float life = 100.f;
    float lifeIncr = 5.f;
    float timerSinceLastDamage = 0.f;
    float timeToStartRecoverFromDamage = 5.f;

    int timesRemoveInhibitorKeyPressed = 0;
    float timeToPressAgain = 0.7f;
    float timeInhib = 0.0f;


    CHandle weaponLeft;
    CHandle weaponRight;
    bool weaponsActive = false;
    float attackTimer = 0.f;
    float timeToDeployWeapons = 0.5f;

    void onCreate(const TMsgEntityCreated& msg);
    void onGroupCreated(const TMsgEntitiesGroupCreated& msg);
    void onSceneCreated(const TMsgSceneCreated& msg);
    void onStateStart(const TMsgStateStart& msg);
    void onStateFinish(const TMsgStateFinish& msg);

    void onPlayerHit(const TMsgPlayerHit& msg);
    void onPlayerStunned(const TMsgPlayerStunned& msg);
    void onPlayerKilled(const TMsgPlayerDead& msg);
    void onPlayerInhibited(const TMsgInhibitorShot& msg);
    void onPlayerExposed(const TMsgPlayerIlluminated& msg);
    void onPlayerPaused(const TMsgScenePaused& msg);
    void onConsoleChanged(const TMsgConsoleOn& msg);
    void onShadowChange(const TMsgShadowChange& msg);
    void onInfiniteStamina(const TMsgInfiniteStamina& msg);
    void onPlayerImmortal(const TMsgPlayerImmortal& msg);
    void onSpeedBoost(const TMsgSpeedBoost& msg);
    void onPlayerInvisible(const TMsgPlayerInvisible& msg);
    void onMsgNoClipToggle(const TMsgNoClipToggle& msg);
    void onMsgBulletHit(const TMsgBulletHit& msg);


    DECL_SIBLING_ACCESS();

public:

    ~TCompTempPlayerController();

    /* Debug and console variables */
    bool infiniteStamina;
    bool isImmortal;
    bool isInvisible;
    bool isInNoClipMode = false;
    float speedBoost = 1.0f;
    std::string dbCameraState;

    float stepTimer = 0.0f;
    bool stepRight = true;

    bool isMerged;
    bool isGrounded;
    bool canMergeFall;
    bool isMergeFalling;
    bool pressedMergeFallInTime;
    bool isInhibited;
    bool canAttack;
    bool canRemoveInhibitor;

    unsigned int hitPoints;
    unsigned int initialTimesToPressInhibitorRemoveKey;

    void debugInMenu();
    void renderDebug();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    /* State functions */
    void walkState(float dt);
    void fallState(float dt);
    void mergeFallState(float dt);
    void idleState(float dt);
    void mergeState(float dt);
    void resetState(float dt);
    void exitMergeState(float dt);
    void removingInhibitorState(float dt);
    void movingObjectState(float dt);
    void stunnedState(float dt);
    void resetRemoveInhibitor();
    void markObjectAsMoving(bool isBeingMoved, VEC3 newDirection = VEC3::Zero, float speed = 0);
    void resetMergeFall();

    /* Player condition tests */
    const bool concaveTest(void);
    const bool convexTest(void);
    const bool onMergeTest(float dt);
    const bool groundTest(float dt);
    const bool canMergeFallTest(float dt);
    const bool canAttackTest(float dt);
    const bool canSonarPunch();

    /* Auxiliar functions */
    void updateStamina(float dt);
    void updateShader(float dt);
    void updateLife(float dt);
    void updateWeapons(float dt);
    void mergeEnemy();
    void resetMerge();
    bool isDead();
    void upButtonReselased();
    void invertAxis(VEC3 old_up, bool type);
    void getDamage(float dmg);
    void die();
    void activateCanLandSM(bool activate);
    void pauseEnemy();
    void stunEnemy();
    const bool isStaminaFull() { return stamina / maxStamina != 1.f; };
    CHandle getLeftWeapon() { return weaponLeft; };
    CHandle getRightWeapon() { return weaponRight; };
    bool isMergedInMergeableZone();

    VEC3 getMotionDir(const VEC3 & front, const VEC3 & left, bool default = true);

    /* Lua functions */
    void playPlayerStep(bool left);
    void playLandParticles(bool left); 
    void playSMSpirals();
    void resetSMFilters();

    static void registerMsgs();
};

