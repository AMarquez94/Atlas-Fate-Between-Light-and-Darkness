#ifndef _AIC_PATROL
#define _AIC_PATROL

#include "ai_controller.h"

class CAIPatrol: public CAIController
{
private:
	static const float VISIONANGLE;
	static const float VISIONDISTANCE;
public:
	void IdleState();
	void SeekWptState();
	void NextWptState();
	void Init();

	void TurnState(void);
	void FollowState(void);
};

#endif