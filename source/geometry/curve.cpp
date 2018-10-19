#include "mcv_platform.h"
#include "curve.h"
#include "resources/json_resource.h"

// ----------------------------------------------
class CCurveResourceClass : public CResourceClass {
public:
	CCurveResourceClass() {
		class_name = "Curves";
		extensions = { ".curve" };
	}
	IResource* create(const std::string& name) const override {
		dbg("Creating curve %s\n", name.c_str());
		CCurve* res = new CCurve();
		bool is_ok = res->load(name);
		assert(is_ok);
        EngineFiles.addPendingResourceFile(name, false);
		return res;
	}
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CCurve>() {
	static CCurveResourceClass the_resource_class;
	return &the_resource_class;
}
// ----------------------------------------------

bool CCurve::load(const std::string& name)
{
    auto jData = Resources.get(name)->as<CJsonResource>()->getJson();

	std::string typeName = jData["type"];
	if (typeName == "catmull-rom")
	{
		_type = EType::CATMULL_ROM;
	}

	auto& jKnots = jData["knots"];
	for (auto& jKnot : jKnots)
	{
		VEC3 knot = loadVEC3(jKnot);
		addKnot(knot);
	}

	return true;
}

void CCurve::clear()
{
	_knots->clear();
}

void CCurve::addKnot(const VEC3& point)
{
	_knots->push_back(point);
}

void CCurve::addKnotAtIndex(const VEC3& point, int index) const
{
	_knots->insert(_knots->begin() + index, point);
}

void CCurve::removeKnotAtIndex(int index) const
{
	_knots->erase(_knots->begin() + index);
}

VEC3 CCurve::evaluate(float ratio) const
{
	if (_type == EType::CATMULL_ROM)
	{
		return evaluateAsCatmull(ratio);
	}
	return VEC3::Zero;
}

VEC3 CCurve::evaluateAsCatmull(float ratio) const
{
	ratio = Clamp(ratio, 0.f, 0.99999f);
	int nsegments = (int)_knots->size() - 3;
	float ratioPerSegment = 1.f / (float)nsegments;
	int currentSegment = (int)(ratio / ratioPerSegment);
	float segmentRatio = fmodf(ratio, ratioPerSegment) / ratioPerSegment;

	int idx = currentSegment + 1;

	VEC3 p1 = (*_knots)[idx - 1];
	VEC3 p2 = (*_knots)[idx];
	VEC3 p3 = (*_knots)[idx + 1];
	VEC3 p4 = (*_knots)[idx + 2];

	return VEC3::CatmullRom(p1, p2, p3, p4, segmentRatio);
}