#ifndef _INTERPOLATORS_H_
#define _INTERPOLATORS_H_

#include <math.h>
#include "geometry/angular.h"

namespace Interpolator {

	// ------------------------------
	// linear
	// ------------------------------
	inline float linear(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		return end * ratio + start * (1 - ratio);
	}

	// ------------------------------
	// quadratic
	// ------------------------------
	inline float quadIn(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return c * ratio*ratio + start;
	}
	inline float quadOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return -c * ratio * (ratio - 2.f) + start;
	}
	inline float quadInOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		float t = ratio * 2.f;
		if (t < 1.f) return ((c*0.5f)*t*t) + start;
		const float tm = t - 1;
		return -(c * 0.5f) * (((tm - 2.f)*(tm)) - 1.f) + start;
	}

	// ------------------------------
	// cubic
	// ------------------------------
	inline float cubicIn(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return c * ratio*ratio*ratio + start;
	}
	inline float cubicOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		const float ratio2 = ratio - 1;
		return c * (ratio2*ratio2*ratio2 + 1) + start;
	}
	inline float cubicInOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		float ratio2 = 2 * ratio;
		if (ratio2 < 1) return c / 2 * ratio2*ratio2*ratio2 + start;
		ratio2 -= 2;
		return c / 2 * (ratio2*ratio2*ratio2 + 2) + start;
	}

	// ------------------------------
	// quart
	// ------------------------------
	inline float quartIn(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return c * ratio*ratio*ratio*ratio + start;
	}
	inline float quartOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		const float t = ratio - 1;
		return -c * (t*t*t*t - 1.f) + start;
	}
	inline float quartInOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		float t = ratio * 2.f;
		if (t < 1.f) return c * 0.5f *t*t*t*t + start;
		t -= 2.f;
		return -c * 0.5f * ((t)*t*t*t - 2.f) + start;
	}

	// ------------------------------
	// quint
	// ------------------------------
	inline float quintIn(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return c * ratio*ratio*ratio*ratio*ratio + start;
	}
	inline float quintOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		float ratio2 = ratio - 1.f;
		return c * ((ratio2)*ratio2*ratio2*ratio2*ratio2 + 1.f) + start;
	}
	inline float quintInOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		float t = ratio * 2.f;
		if (t < 1.f) return c * 0.5f *t*t*t*t*t + start;
		t -= 2.f;
		return c * 0.5f*((t)*t*t*t*t + 2.f) + start;
	}

	// ------------------------------
	// back
	// ------------------------------
	inline float backIn(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		float s = 1.70158f;
		return c * (ratio)*ratio*((s + 1)*ratio - s) + start;
	}
	inline float backOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		const float s = 1.70158f;
		const float ratio2 = ratio - 1;
		return c * (ratio2*ratio2*((s + 1)*ratio2 + s) + 1) + start;
	}
	inline float backInOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		float s = 1.70158f;
		s *= (1.525f);
		float ratio2 = ratio;
		if ((ratio2 *= 2) < 1) return c / 2 * (ratio2*ratio2*(((s)+1)*ratio2 - s)) + start;
		const float postFix = ratio2 -= 2;
		return c / 2 * ((postFix)*ratio2*(((s)+1)*ratio2 + s) + 2) + start;
	}

	// ------------------------------
	// elastic
	// ------------------------------
	inline float elasticIn(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		const float p = 0.3f;
		const float a = c;
		const float s = p * 0.25f;
		float ratio2 = ratio;
		const float postFix = a * powf(2.f, 10.f*(ratio2 -= 1)); // this is a fix, again, with post-increment operators
		return -(postFix * sinf((float)((ratio2 - s)*(2 * M_PI) / p))) + start;
	}
	inline float elasticOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		const float p = 0.3f;
		const float a = c;
		const float s = p * 0.25f;
		return (a*powf(2.f, -10 * ratio) * sinf((float)((ratio - s)*(2 * M_PI) / p)) + c + start);
	}
	inline float elasticInOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		float ratio2 = 2 * ratio;
		const float c = end - start;
		const float p = (.3f*1.5f);
		const float a = c;
		const float s = p * 0.25f;

		if (ratio2 < 1) {
			const float postFix = a * powf(2.f, 10.f*(ratio2 -= 1)); // postIncrement is evil
			return -.5f*(postFix* sinf((float)((ratio2 - s)*(2 * M_PI) / p))) + start;
		}
		const float postFix = a * powf(2.f, -10.f*(ratio2 -= 1)); // postIncrement is evil
		return postFix * sinf((float)((ratio2 - s)*(2 * M_PI) / p))*.5f + c + start;
	}

	// ------------------------------
	// bounce
	// ------------------------------
	inline float bounceOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		if (ratio < (1.f / 2.75f)) return c * (7.5625f * ratio * ratio) + start;
		if (ratio < (2 / 2.75f)) {
			const float t = ratio - (1.5f / 2.75f);
			return c * (7.5625f*t*t + .75f) + start;
		}
		if (ratio < (2.5 / 2.75)) {
			const float t = ratio - (2.25f / 2.75f);
			return c * (7.5625f*t*t + .9375f) + start;
		}

		const float t = ratio - (2.625f / 2.75f);
		return c * (7.5625f*t*t + .984375f) + start;
	}
	inline float bounceIn(const float& start, const float& end, const float ratio) {
		return bounceOut(end, start, 1.0f - ratio);
	}
	inline float bounceInOut(const float& start, const float& end, const float ratio) {
		const float m = (start + end) * 0.5f;
		if (ratio < 0.5f) return bounceIn(start, m, ratio*2.f);
		return bounceOut(m, end, ratio*2.f - 1.f);
	}

	// ------------------------------
	// circular
	// ------------------------------
	inline float circularIn(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return -c * (std::sqrt(1.f - ratio * ratio) - 1.f) + start;
	}
	inline float circularOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		const float t = ratio - 1.f;
		return c * std::sqrt(1.f - t * t) + start;
	}
	inline float circularInOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		float t = ratio * 2.f;
		if (t < 1.f) return -c * 0.5f * (sqrtf(1.f - t * t) - 1.f) + start;
		const float t2 = t - 2.f;
		return c * 0.5f * (sqrtf(1.f - t2 * (t2)) + 1.f) + start;
	}

	// ------------------------------
	// expo
	// ------------------------------
	inline float expoIn(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return c * powf(2.f, 10 * (ratio - 1.f)) + start;
	}
	inline float expoOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return c * (-powf(2.f, -10 * ratio) + 1.f) + start;
	}
	inline float expoInOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		float t = ratio * 2.f;
		if (t < 1.f) return c * 0.5f * powf(2.f, 10 * (t - 1.f)) + start;
		return c * 0.5f * (-powf(2.f, -10.f * --t) + 2) + start;
	}

	// ------------------------------
	// sine
	// ------------------------------
	inline float sineIn(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return -c * cosf((float)(ratio * M_PI * 0.5f)) + c + start;
	}
	inline float sineOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return c * sinf((float)(ratio * M_PI * 0.5f)) + start;
	}
	inline float sineInOut(const float& start, const float& end, const float ratio) {
		if (ratio <= 0.0f) return start;
		if (ratio >= 1.0f) return end;
		const float c = end - start;
		return -c * 0.5f * (cosf((float)(M_PI * ratio)) - 1.f) + start;
	}

	// ------------------------------
	// classes (for ease of use)
	// ------------------------------
	class IInterpolator {
	public:
		virtual float blend(const float& start, const float& end, const float ratio) = 0;
	};

	class TLinearInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return linear(start, end, ratio); }
	};
	class TQuadInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return quadIn(start, end, ratio); }
	};
	class TQuadOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return quadOut(start, end, ratio); }
	};
	class TQuadInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return quadInOut(start, end, ratio); }
	};
	class TCubicInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return cubicIn(start, end, ratio); }
	};
	class TCubicOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return cubicOut(start, end, ratio); }
	};
	class TCubicInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return cubicInOut(start, end, ratio); }
	};
	class TQuartInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return quartIn(start, end, ratio); }
	};
	class TQuartOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return quartOut(start, end, ratio); }
	};
	class TQuartInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return quartInOut(start, end, ratio); }
	};
	class TQuintInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return quintIn(start, end, ratio); }
	};
	class TQuintOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return quintOut(start, end, ratio); }
	};
	class TQuintInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return quintInOut(start, end, ratio); }
	};
	class TBackInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return backIn(start, end, ratio); }
	};
	class TBackOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return backOut(start, end, ratio); }
	};
	class TBackInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return backInOut(start, end, ratio); }
	};
	class TElasticInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return elasticIn(start, end, ratio); }
	};
	class TElasticOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return elasticOut(start, end, ratio); }
	};
	class TElasticInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return elasticInOut(start, end, ratio); }
	};
	class TBounceInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return bounceIn(start, end, ratio); }
	};
	class TBounceOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return bounceOut(start, end, ratio); }
	};
	class TBounceInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return bounceInOut(start, end, ratio); }
	};
	class TCircularInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return circularIn(start, end, ratio); }
	};
	class TCircularOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return circularOut(start, end, ratio); }
	};
	class TCircularInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return circularInOut(start, end, ratio); }
	};
	class TExpoInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return expoIn(start, end, ratio); }
	};
	class TExpoOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return expoOut(start, end, ratio); }
	};
	class TExpoInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return expoInOut(start, end, ratio); }
	};
	class TSineInInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return sineIn(start, end, ratio); }
	};
	class TSineOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return sineOut(start, end, ratio); }
	};
	class TSineInOutInterpolator : public IInterpolator {
		float blend(const float& start, const float& end, const float ratio) override { return sineInOut(start, end, ratio); }
	};


};

#endif
