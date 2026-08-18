#pragma once
class FloatBuffer;

class Lighting
{
private:
	static FloatBuffer *lb;

public:
	static void turnOff();
    static void turnOn();
	static void turnOnGui();
	static void setAmbient(float a);
private:
	static FloatBuffer *getBuffer(double a, double b, double c, double d);
    static FloatBuffer *getBuffer(float a, float b, float c, float d);
};
