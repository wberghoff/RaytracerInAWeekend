#include "../inc/PPMFile.h"

int main(int argc, char** argv)
{
	const int nx = 200, ny = 100;
	PPMFile Test(nx, ny);
	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			Test.AddNewPixel(static_cast<float>(i) / nx, static_cast<float>(j) / ny, 0.2f);
		}
	}
	Test.Write("Test");
	return 0;
}