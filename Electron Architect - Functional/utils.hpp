#pragma once

double EaseInOut(double t);
double ClampDouble(double x, double min, double max);
double LerpDouble(double a, double b, double amount);

// Seconds needed to fully transition with an arbitrary speed and distance.
// Pass to AnimatedFade as fadePeriod to change animation from fixed duration to fixed speed.
double GetAnimatedFadeDuration(double fadeSpeed, double startValue, double endValue);
double AnimatedFade(double timeSinceHover, double fadePeriod, double startValue, double endValue);
