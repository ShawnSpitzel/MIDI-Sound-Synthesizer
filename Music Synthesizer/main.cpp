#include <iostream>
#include <cmath>
using namespace std;

#include "soundMaker.h"
atomic<double> frequency = 0.0;
double MakeSineNoise(double dTime)
{
	double dOutput = sin(frequency * 2.0 * 3.14159 * dTime);
	return dOutput * 0.5; // Master Volume
}
double MakeSquareNoise(double dTime)
{
	double amplitude = 1.0;
	double squareness = 3.0;
	double dOutput = ((amplitude) / (pow(abs(squareness), (sin((squareness * frequency * 3.14159 * dTime) - 0.0)))) + 1.0) - (amplitude / 2.0);
	return dOutput * 0.5; // Master Volume
}

int main() {
	vector<wstring> devices = NoiseMaker<short>::Enumerate();
	for (auto d : devices) wcout << "Found Output Device: " << d << endl;
	wcout << "Using Device: " << devices[0] << endl;
	NoiseMaker<short> sound(devices[0]);
	frequency = 440.0;
	sound.SetUserFunction(MakeSineNoise);
	while (1) {

	}
}