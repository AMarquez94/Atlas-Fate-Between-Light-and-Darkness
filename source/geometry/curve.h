#pragma once

class CCurve
{
public:
	CCurve() = default;
	void clear();
	void addKnot(const VEC3& point);
	VEC3 evaluateAsCatmull(float ratio) const;

private:
	std::vector<VEC3> _knots;
};
