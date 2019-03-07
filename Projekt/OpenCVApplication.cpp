// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <vector>
#include <string>
#include <bitset>
#include <map>

#define MAX_SIZE 1000000
#define MAX_SIZE_CODE 1000


int current_bit = 0;
unsigned char bit_buffer;

void WriteBit(int bit, FILE* f)
{
	if (bit)
		bit_buffer |= (1 << current_bit);

	current_bit++;
	if (current_bit == 8)
	{
		fwrite(&bit_buffer, 1, 1, f);
		current_bit = 0;
		bit_buffer = 0;
	}
}

void Flush_Bits(FILE* f)
{
	while (current_bit)
		WriteBit(0, f);
}

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
	FILE* pFile;
	pFile = fopen("output.dat", "wb");

	if (pFile != NULL) {
		//first save image height, width
		//printf("%d", sizeof(img.rows));
		fwrite(&img.rows, 4, 1, pFile);
		fwrite(&img.cols, 4, 1, pFile);

		//write out table
		for (int i = 0; i < 256; i++) {
			unsigned char size = encoded[i].size();//max 256
			fwrite(&size, 1, 1, pFile);
			for (int j = 0; j < size; j++) {
				if (encoded[i][j] == '0')
					WriteBit(0, pFile);
				else WriteBit(1, pFile);
			}
			Flush_Bits(pFile);
		}

		long res_size = res.size();
		//write size of res
		fwrite(&res_size, 4, 1, pFile);
		//write actual image data
		for (int i = 0; i < res.size(); i++) {
			if (res[i] == '0')
				WriteBit(0, pFile);
			else WriteBit(1, pFile);
		}
		Flush_Bits(pFile);
		fclose(pFile);
	}
}

void decodeFromBinary() {
	
	std::map<std::string, uchar> encoded;
	FILE* pFile;
	char c;
	pFile = fopen("output.dat", "rb");

	long width = 0, height = 0;

	//read iamge dimensions
	fread(&height, 4, 1, pFile);
	fread(&width, 4, 1, pFile);

	Mat img(height, width, CV_8UC1);//store result in this
	for (int i = 0; i < 256; i++) {
		unsigned char size;
		fread(&size, 1, 1, pFile);
	
		std::string input = "";
		int pc = 0;
		for (int j = 0; j < (size-1) / 8 + 1 && size>0; j++)
		{
			fread(&c, 1, 1, pFile);
			for (int i = 0; i < 8; i++) { // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes
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
		encoded[input] = i;
	}

	//read size of encoded strings
	long res_size = 0;
	//write size of res
	fread(&res_size, 4, 1, pFile);
	//read actual compressed image into a big buffer
	std::string image = "";
	int pc = 0;
	while (fread(&c, 1, 1, pFile)) {
		for (int i = 0; i < 8; i++) {
			pc++;
			if (pc > res_size) break;
			int bit = ((c >> i) & 1);
			if (bit == 1)
				image.append("1");
			else image.append("0");
		}
	}

	pc = 0;
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			//decode string and build image
			//res_size = image.size()
			std::string good_code = "";
			for (; pc < image.size(); pc++) {
				good_code += image[pc];
				if (encoded.find(good_code) != encoded.end()) {//code is valid
					uchar value = encoded[good_code];
					img.at<uchar>(i, j) = value;
					break;
				}
			}
		}
	}
	imshow("image", img);
	waitKey();
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