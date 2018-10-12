#include "mcv_platform.h"
#include "modules/system/module_lerp.h"


void CModuleLerp::update(float delta) {
	
	if (_elements_to_lerp.size() > 0) {
		std::list<LerpingElement>::iterator it = _elements_to_lerp.begin();
		while (it != _elements_to_lerp.end()) {

			if ((*it).current_time >= (*it).time_to_start_lerping) {
				float diff = (*it).value_to_lerp - (*it).max_element_to_lerp;
				float percentage = Clamp( ((((*it).current_time - (*it).time_to_start_lerping)) / (*it).time_to_end_lerp) , 0.0f, 1.0f );
				*(*it).element_to_lerp = (*it).max_element_to_lerp + (diff * percentage);
			}
			(*it).current_time += delta;
			if( ((*it).current_time - (*it).time_to_start_lerping) >= (*it).time_to_end_lerp ) {
				*(*it).element_to_lerp = (*it).value_to_lerp;
				_elements_to_lerp.erase(it);
			}

			it++;
		}
	}
}

void CModuleLerp::render() {

}

void CModuleLerp::lerpElement(float *element_to_lerp, float value_to_lerp, float time_to_lerp, float time_to_start) {

	LerpingElement new_lerp_element;
	new_lerp_element.element_to_lerp = element_to_lerp;
	new_lerp_element.max_element_to_lerp = *element_to_lerp;
	new_lerp_element.value_to_lerp = value_to_lerp;
	new_lerp_element.current_time = 0.0f;
	new_lerp_element.time_to_end_lerp = time_to_lerp;
	new_lerp_element.time_to_start_lerping = time_to_start;
	_elements_to_lerp.push_back(new_lerp_element);
}
