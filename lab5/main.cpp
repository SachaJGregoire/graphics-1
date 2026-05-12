#define _CRT_SECURE_NO_WARNINGS 1
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <random>
static std::default_random_engine engine;
static std::uniform_real_distribution<double> uniform(0, 1);
#ifndef M_PI
#define M_PI 3.14159265358979323856
#endif
#include <algorithm>
#include <iostream>

int main() {

	int W, H, C;
	
	//stbi_set_flip_vertically_on_load(true);
	unsigned char *imageA  = stbi_load("imgA.jpg",
                                 &W,
                                 &H,
                                 &C,
                                 STBI_rgb);
	std::vector<double> imageA_double(W*H*3);
	for (int i=0; i<W*H*3; i++) imageA_double[i] = imageA[i];

	unsigned char *imageB = stbi_load("redim.jpg",
                                 &W,
                                 &H,
                                 &C,
                                 STBI_rgb);
	std::vector<double> imageB_double(W*H*3);
	for (int i=0; i<W*H*3; i++) imageB_double[i] = imageB[i];

	

	int nbiter = 1e3;
	std::vector<std::pair<double, int>> projA(W*H);
	std::vector<std::pair<double, int>> projB(W*H);
	for (int iter = 0; iter < nbiter; iter++) {
		double r1 = uniform(engine);
		double r2 = uniform(engine);
		double x = cos(2 * M_PI * r1) * sqrt(r2 * (1 - r2));
		double y = sin(2 * M_PI * r1) * sqrt(r2 * (1 - r2));
		double z = 1 - 2 * r2;

		for (int i = 0; i < W*H; i++) {
			double A0 = imageA_double[3*i+0];
			double A1 = imageA_double[3*i+1];
			double A2 = imageA_double[3*i+2];
			projA[i] = {x * A0 + y * A1 + z * A2, 3*i};
			double B0 = imageB_double[3*i+0];
			double B1 = imageB_double[3*i+1];
			double B2 = imageB_double[3*i+2];
			projB[i] = {x * B0 + y * B1 + z * B2, 3*i};
		}

		std::sort(projA.begin(), projA.end());
		std::sort(projB.begin(), projB.end());

		for (int i = 0; i < W*H; i++) {
			imageA_double[projA[i].second+0] += (projB[i].first - projA[i].first) * x;
			imageA_double[projA[i].second+1] += (projB[i].first - projA[i].first) * y;
			imageA_double[projA[i].second+2] += (projB[i].first - projA[i].first) * z;
		}
	}
	for (int i = 0; i < W*H; i++) {
		for (int j = 0; j < 3; j++) {
			int ind = 3*i+j;
			if (imageA_double[ind] < 0) imageA_double[ind] = 0;
			else if (imageA_double[ind] > 255) imageA_double[ind] = 0;
		}
	}
	


	std::vector<unsigned char> image_result(W*H * 3, 0);
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			image_result[(i*W + j) * 3 + 0] = imageA_double[(i*W+j)*3+0]/**0.5*/;
			image_result[(i*W + j) * 3 + 1] = imageA_double[(i*W+j)*3+1]/**0.3*/;
			image_result[(i*W + j) * 3 + 2] = imageA_double[(i*W+j)*3+2]/**0.2*/;
		}
	}
	stbi_write_png("image.png", W, H, 3, &image_result[0], 0);

	return 0;
}