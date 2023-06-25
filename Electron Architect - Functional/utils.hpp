#pragma once

double EaseInOut(double t);
double ClampDouble(double x, double min, double max);
double LerpDouble(double a, double b, double amount);

double AnimatedFade(double timeSinceHover, double fadePeriod, double startValue, double endValue);
