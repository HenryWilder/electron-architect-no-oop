#include "utils.hpp"

double EaseInOut(double t)
{
	double tt = t * t;
	double divisor = 2.0 * (tt - t) + 1.0;
	double result = tt / divisor;
	return result;
}

double ClampDouble(double x, double min, double max)
{
	x = (x < min) ? min : x;
	x = (x > max) ? max : x;
	return x;
}

double LerpDouble(double a, double b, double amount)
{
	double result = a + amount * (b - a);
	return result;
}

double GetAnimatedFadeDuration(double fadeSpeed, double startValue, double endValue)
{
	double minValue, maxValue;
	if (startValue < endValue)
	{
		minValue = startValue;
		maxValue = endValue;
	}
	else
	{
		minValue = endValue;
		maxValue = startValue;
	}

	double fadeWidth = maxValue - minValue;
	double totalFadeTimeNeeded = fadeWidth / fadeSpeed;
	return totalFadeTimeNeeded;
}

double AnimatedFade(double timeSinceHover, double fadePeriod, double startValue, double endValue)
{
	double fadePercent = timeSinceHover / fadePeriod;
	double fadePercentSaturated = ClampDouble(fadePercent, 0.0, 1.0);
	double easedFadePercent = EaseInOut(fadePercentSaturated);
	double result = LerpDouble(startValue, endValue, easedFadePercent);
	return result;
}
