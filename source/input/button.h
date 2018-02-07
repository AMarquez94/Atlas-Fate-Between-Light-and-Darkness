#pragma once

namespace Input
{
	struct TButton
	{
		float value = 0.f;
		float prevValue = 0.f;
		float time_pressed = 0.f;
		float time_released = 0.f;

		bool isPressed() const
		{
			return value != 0.f;
		}

		bool wasPressed() const
		{
			return prevValue != 0.f;
		}

		bool getsPressed() const
		{
			return !wasPressed() && isPressed();
		}

		bool getsReleased() const
		{
			return wasPressed() && !isPressed();
		}

		void update(float delta, float currentValue)
		{
			time_pressed += delta;
			time_released += delta;

			prevValue = value;
			value = currentValue;

			if (wasPressed())
			{
				time_released = 0.f;
			}
			else
			{
				time_pressed = 0.f;
			}
		}
	};
}