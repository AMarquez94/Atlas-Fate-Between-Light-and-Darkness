#include "mcv_platform.h"
#include "curve.h"
#include "utils/utils.h"

void CCurve::clear()
{
	_knots.clear();
}

void CCurve::addKnot(const VEC3& point)
{
	_knots.push_back(point);
}

VEC3 CCurve::evaluateAsCatmull(float ratio) const
{
	ratio = Clamp(ratio, 0.f, 0.99999f);
	int nsegments = _knots.size() - 3;
	float ratioPerSegment = 1.f / (float)nsegments;
	int currentSegment = (int)(ratio / ratioPerSegment);
	float segmentRatio = fmodf(ratio, ratioPerSegment) / ratioPerSegment;

	int idx = currentSegment + 1;

	VEC3 p1 = _knots[idx - 1];
	VEC3 p2 = _knots[idx];
	VEC3 p3 = _knots[idx + 1];
	VEC3 p4 = _knots[idx + 2];

	return VEC3::CatmullRom(p1, p2, p3, p4, segmentRatio);
}