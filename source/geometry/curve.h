#pragma once

#include "resources/resource.h"

class CCurve : public IResource
{
public:
	enum class EType { UNKNOWN = 0, CATMULL_ROM };

	CCurve() = default;
    ~CCurve();
	bool load(const std::string& name);
	void clear();
	void addKnot(const VEC3& point);
	void addKnotAtIndex(const VEC3& point, int index) const;
	void removeKnotAtIndex(int index) const;
	VEC3 evaluate(float ratio) const;
	VEC3 evaluateAsCatmull(float ratio) const;

private:
	std::vector<VEC3> * _knots = new std::vector<VEC3>();
	EType _type = EType::UNKNOWN;
};
