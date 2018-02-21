#ifndef INC_COMP_GROUP_H_
#define INC_COMP_GROUP_H_

#include "comp_base.h"

class CTransform;

DECL_SIBLING_ACCESS();

struct TCompGroup : public TCompBase {
	VHandles handles;
	~TCompGroup();
	void debugInMenu();
	void add(CHandle h_new_child);
	CHandle getHandleByName(std::string name);

};

#endif
