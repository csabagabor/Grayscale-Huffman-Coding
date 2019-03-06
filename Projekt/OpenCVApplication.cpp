// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <vector>
#include <string>
#include <bitset>

#define MAX_SIZE 1000000
#define MAX_SIZE_CODE 1000

struct code_struct
{
	int probab;//probability in whole numbers(without dividing it with the total number)
	std::string cod = "";
};


std::vector<code_struct> calculateCodes(int probabilities[], int index);
void swap(int *xp, int *yp);
void selectionSort(int arr[], int n);
void saveToBinary(Mat_<uchar> img, std::string encoded[]);


void testOpenImage()
{

	int histo[256] = { 0 };
	Mat_<uchar>img;
	img = imread("Images/cell.bmp", CV_LOAD_IMAGE_GRAYSCALE);	// Read the image

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			histo[img(i,j)]++;
		}
	}

	int probabilities[256], index = 0;
	for (int i = 0; i < 256; i++) {
		if (histo[i] > 0) {
			probabilities[index] = histo[i];
			index++;
		}
	}

	std::vector<code_struct> res = calculateCodes(probabilities, index);

	std::string encoded[256];

	for (int i = 0; i < 256; i++) {
		if (histo[i] > 0) {
			std::vector<code_struct>::iterator it = res.begin();
			for (; it != res.end(); ) {
				if (histo[i] == it->probab) {
					encoded[i] = it->cod;
					//reverse string
					std::reverse(encoded[i].begin(), encoded[i].end());
					it = res.erase(it);
					break;
				}
				else {
					++it;
				}
			}
		}
	}
	saveToBinary(img, encoded);
}

std::vector<code_struct> calculateCodes(int probabilities[], int index ) {
	std::vector<code_struct> res;
	code_struct e1, e2;
	//if only 2 elements then stop
	if (index == 2) {
		e1.probab = probabilities[0];
		e2.probab = probabilities[1];

		if (probabilities[0] < probabilities[1]) {
			e1.cod.push_back('1');
			e2.cod.push_back('0');
		}
		else {
			e1.cod.push_back('0');
			e2.cod.push_back('1');
		}
	}
	else {
		selectionSort(probabilities, index);
		int min1 = probabilities[index - 1];
		int min2 = probabilities[index - 2];

		probabilities[index - 2] = min1 + min2;
		index--;

		//call itself
		res = calculateCodes(probabilities, index);

		//find probability of sum
		for (int i = 0; i < res.size(); i++) {
			if (res.at(i).probab == min1 + min2) {
				e1.probab = min1;
				e2.probab = min2;
				std::string code_sum = res.at(i).cod;
				//min1 < min2
				e1.cod = code_sum;
				e1.cod.push_back('1');

				e2.cod = code_sum;
				e2.cod.push_back('0');

				//delete the sum element
				res.erase(res.begin() + i);
				break;
			}
		}
	}
	res.push_back(e1);
	res.push_back(e2);
	return res;
}

void swap(int *xp, int *yp)
{
	int temp = *xp;
	*xp = *yp;
	*yp = temp;
}

void selectionSort(int arr[], int n)
{
	int i, j, min_idx;
	for (i = 0; i < n - 1; i++)
	{
		min_idx = i;
		for (j = i + 1; j < n; j++)
			if (arr[j] > arr[min_idx])
				min_idx = j;

		swap(&arr[min_idx], &arr[i]);
	}
}


void saveToBinary(Mat_<uchar> img, std::string encoded[])
{
	std::string res ="";
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			res.append(encoded[img(i,j)]);
		}
	}
	if (res.size() > MAX_SIZE) {
		printf("ERROR: PICTURE IS TO BIG!\n");
		return;
	}
	std::bitset<MAX_SIZE> bits(res);
	FILE* pFile;
	pFile = fopen("output.dat", "wb");

	//write out table
	for (int i = 0; i < 256; i++) {
		unsigned char size = encoded[i].size();//max 256
		//if (size > 0) {
			//fwrite(&i, 1, 1, pFile);
			fwrite(&size, 1, 1, pFile);
			//write code (multiple of 8=1 byte)
			std::bitset<MAX_SIZE_CODE> code_bits(encoded[i]);
			//fwrite(&code_bits, 1, size / 8 + 1, pFile);
		//}
	}

	if (pFile != NULL) {
		//fwrite(&bits, 1, res.size() / 8, pFile);
		fclose(pFile);
	}
}

void decodeFromBinary() {
	std::string encoded[256] = { "" };
	FILE* pFile;
	pFile = fopen("output.dat", "rb");
	for (int i = 0; i < 256; i++) {
		unsigned char size;
		fread(&size, 1, 1, pFile);

		//char buffer[MAX_SIZE_CODE];
		//fread(buffer, 1, size / 8 + 1, pFile);
		char c;
		std::string input = "";
		int pc = 0;
		for (int j = 0; j < size / 8 + 1; j++)
		{
			fread(&c, 1, 1, pFile);
			for (int i = 7; i >= 0; i--) { // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes
				pc++;
				if (pc > size) break;
				int bit = ((c >> i) & 1);
				if(bit==1)
					input.append("1");
				else input.append("0");
			}
		}
		//std::string input(buffer, size / 8 + 1); // Convert char array into string
		//std::bitset<MAX_SIZE_CODE>  codes_bits("1111");  // Convert string into bitset
		encoded[i] = input;
	}
	fclose(pFile);
}

int main()
{
	int op;

	do
	{
		//system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Open image\n");
		printf(" 2 - Decode image\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d",&op);
		switch (op)
		{
			case 1:
				testOpenImage();
				break;
			case 2:
				decodeFromBinary();
				break;
		}
	}
	while (op!=0);
	return 0;
}