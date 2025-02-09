#include "mcv_platform.h"
#include "comp_player_input.h"
#include "components/comp_fsm.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/comp_render.h"
#include "render/render_objects.h"
#include "render/render_utils.h"
#include "comp_player_attack_cast.h"
#include "components/lighting/comp_fade_controller.h"
#include "components/ia/comp_bt_player.h"

DECL_OBJ_MANAGER("player_input", TCompPlayerInput);

void TCompPlayerInput::debugInMenu() {
}

void TCompPlayerInput::load(const json& j, TEntityParseContext& ctx) {
  float _timeOffsetToRemoveInhibitor = 0.2f;
}

void TCompPlayerInput::update(float dt)
{
    if (!CHandle(this).getOwner().isValid())
        return;

    if (!paused && !isConsoleOn && !isInNoClipMode && !_playerAIEnabled) {
        CEntity* e = CHandle(this).getOwner();
        _time += dt;

        /* Movement messages*/
        {
            if (EngineInput["btUp"].hasChanged()
                || EngineInput["btDown"].hasChanged()
                || EngineInput["btLeft"].hasChanged()
                || EngineInput["btRight"].hasChanged()) {

                TMsgSetFSMVariable walkMsg;
                walkMsg.variant.setName("speed");

                movementValue.x = EngineInput["btRight"].value - EngineInput["btLeft"].value;
                movementValue.y = EngineInput["btUp"].value - EngineInput["btDown"].value;

                float total_value = movementValue.Length();

                walkMsg.variant.setFloat(total_value);
                e->sendMsg(walkMsg);
            }

            /* Speed boost messages */
            TMsgSetFSMVariable boostMsg;

            if (EngineInput["btRun"].hasChanged()) {

                boostMsg.variant.setName("boost_speed");
                crouchButton = false;
                TMsgSetFSMVariable crouch;
                crouch.variant.setName("crouch");
                crouch.variant.setBool(false);
                e->sendMsg(crouch);

                boostMsg.variant.setFloat(EngineInput["btRun"].value);
                e->sendMsg(boostMsg);
            }

            if (EngineInput["btSlow"].hasChanged()) {
                boostMsg.variant.setName("slow_speed");
                boostMsg.variant.setFloat(EngineInput["btSlow"].value);
                e->sendMsg(boostMsg);
            }
        }

        /* Shadow merge messages*/
        {
            if (EngineInput["btShadowMerging"].hasChanged())
            {
                //TMsgSetFSMVariable shadowMerge;
                //shadowMerge.variant.setName("onmerge");
                //shadowMerge.variant.setBool(EngineInput["btShadowMerging"].isPressed());
                //e->sendMsg(shadowMerge);
            }
        }

        /* Player/User interaction messages */
        {

            if (EngineInput["btAttack"].getsPressed())
            {
                TCompTempPlayerController * c_my_player = get<TCompTempPlayerController>();
                if (c_my_player->canAttack) {
                    TMsgSetFSMVariable attack;
                    attack.variant.setName("attack");
                    attack.variant.setBool(true);
                    e->sendMsg(attack);
                    attackButtonJustPressed = true;
                }
            }
            else if (attackButtonJustPressed) {
                TMsgSetFSMVariable attack;
                attack.variant.setName("attack");
                attack.variant.setBool(false);
                e->sendMsg(attack);

                attackButtonJustPressed = false;
            }

            if (EngineInput["btSonar"].getsPressed()) {
                TCompTempPlayerController* c_my_player = get<TCompTempPlayerController>();
                if (c_my_player->canSonarPunch()) {
                    TMsgSetFSMVariable sonar;
                    sonar.variant.setName("sonar");
                    sonar.variant.setBool(true);
                    e->sendMsg(sonar);
                    sonarButtonJustPressed = true;
                }
            }
            else if (sonarButtonJustPressed) {
                TMsgSetFSMVariable sonar;
                sonar.variant.setName("sonar");
                sonar.variant.setBool(false);
                e->sendMsg(sonar);

                sonarButtonJustPressed = false;
            }

            if (EngineInput["btCrouch"].getsPressed() && !EngineInput["btRun"].isPressed())
            {
                crouchButton = !crouchButton;
                TMsgSetFSMVariable crouch;
                crouch.variant.setName("crouch");
                crouch.variant.setBool(crouchButton);
                e->sendMsg(crouch);
            }

            if (EngineInput["btAction"].hasChanged())
            {
                TCompPlayerAttackCast* playerCast = e->get<TCompPlayerAttackCast>();
                CHandle button = playerCast->getClosestButton();
                //GrabEnemy messages
                if (EngineInput["btAction"].getsPressed() && playerCast->getClosestEnemyToGrab().isValid()) {
                    _enemyStunned = true;
                    TMsgSetFSMVariable grabEnemy;
                    grabEnemy.variant.setName("grabEnemy");
                    grabEnemy.variant.setBool(true);
                    e->sendMsg(grabEnemy);
                }
                else if (EngineInput["btAction"].getsPressed() && playerCast->getClosestMovableObjectInRange().isValid()) {
                    _moveObject = true;
                    TMsgSetFSMVariable moveObject;
                    moveObject.variant.setName("movingObject");
                    moveObject.variant.setBool(true);
                    e->sendMsg(moveObject);
                }
                else if (EngineInput["btAction"].getsPressed() && button.isValid()) {
                    _buttonPressed = true;
                    //Entering into ButtonPressedState
                    TMsgSetFSMVariable activatingButton;
                    activatingButton.variant.setName("buttonPressed");
                    activatingButton.variant.setBool(true);
                    e->sendMsg(activatingButton);

                    TMsgButtonActivated msg;
                    button.sendMsg(msg);
                }

                if (EngineInput["btAction"].getsReleased()) {
                    _enemyStunned = false;
                    TMsgSetFSMVariable buttonReleased;
                    buttonReleased.variant.setName("grabEnemy");
                    buttonReleased.variant.setBool(false);
                    e->sendMsg(buttonReleased);

                    _moveObject = false;
                    buttonReleased.variant.setName("movingObject");
                    buttonReleased.variant.setBool(false);
                    e->sendMsg(buttonReleased);

                    if (_buttonPressed) {
                        TMsgSetFSMVariable activatingButton;
                        activatingButton.variant.setName("buttonPressed");
                        activatingButton.variant.setBool(false);
                        e->sendMsg(activatingButton);
                    }
                }

                if (_enemyStunned && !playerCast->getClosestEnemyToGrab().isValid()) {
                    _enemyStunned = false;
                    TMsgSetFSMVariable grabEnemy;
                    grabEnemy.variant.setName("grabEnemy");
                    grabEnemy.variant.setBool(false);
                    e->sendMsg(grabEnemy);
                }

                if (_moveObject && !playerCast->getClosestMovableObjectInRange().isValid()) {
                    _moveObject = false;
                    TMsgSetFSMVariable moveObject;
                    moveObject.variant.setName("movingObject");
                    moveObject.variant.setBool(false);
                    e->sendMsg(moveObject);
                }
            }

            if (EngineInput["btSecAction"].getsPressed())
            {
                /* Move this from here.. */
                TCompTempPlayerController * c_my_player = get<TCompTempPlayerController>();

                if (c_my_player->isInhibited && c_my_player->canRemoveInhibitor && _time >= _timerRemoveInhibitor + _timeOffsetToRemoveInhibitor) {
                    _timerRemoveInhibitor = _time;
                    TMsgSetFSMVariable keyPressed;
                    keyPressed.variant.setName("inhibitorTryToRemove");
                    keyPressed.variant.setBool(true);
                    e->sendMsg(keyPressed);
                }
            }

            if (EngineInput["btDebugShadows"].getsPressed()) {
                TCompTempPlayerController * c_my_player = get<TCompTempPlayerController>();
                c_my_player->infiniteStamina = !c_my_player->infiniteStamina;
            }

            if (EngineInput["btUp"].getsReleased() || EngineInput["btDown"].getsReleased() ||
                EngineInput["btRight"].getsReleased() || EngineInput["btLeft"].getsReleased()) {
                TCompTempPlayerController * c_my_player = get<TCompTempPlayerController>();
                c_my_player->upButtonReselased();
            }
        }
    }
}

void TCompPlayerInput::registerMsgs()
{
    DECL_MSG(TCompPlayerInput, TMsgNoClipToggle, onMsgNoClipToggle);
    DECL_MSG(TCompPlayerInput, TMsgPlayerAIEnabled, onMsgPlayerAIEnabled);
}

void TCompPlayerInput::onMsgNoClipToggle(const TMsgNoClipToggle & msg)
{
    isInNoClipMode = !isInNoClipMode;
}

void TCompPlayerInput::onMsgPlayerAIEnabled(const TMsgPlayerAIEnabled & msg)
{
    _playerAIEnabled = msg.enableAI;

    if (_playerAIEnabled) {
        CEntity* e = CHandle(this).getOwner();
        TMsgSetFSMVariable walkMsg;
        walkMsg.variant.setName("speed");
        walkMsg.variant.setFloat(0);
        e->sendMsg(walkMsg);

        TCompTempPlayerController * c_my_player = get<TCompTempPlayerController>();
        c_my_player->upButtonReselased();
    }
}
