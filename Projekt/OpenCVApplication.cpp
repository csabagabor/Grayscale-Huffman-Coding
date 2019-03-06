// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <vector>
#include <string>
#include <bitset>

#define MAX_SIZE 1000000

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
	char fname[MAX_PATH];
	if (openFileDlg(fname))
	{
		int histo[256] = { 0 };
		Mat_<uchar>img;
		img = imread(fname);

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
	std::bitset<MAX_SIZE> bits(res);
	FILE* pFile;
	pFile = fopen("output.dat", "w+");
	if (pFile != NULL) {
		//fwrite(&bits, 1, res.size() / 8, pFile);
		fprintf(pFile, "\nThis is a sample text file\n");
		fclose(pFile);
	}
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
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d",&op);
		switch (op)
		{
			case 1:
				testOpenImage();
				break;
		}
	}
	while (op!=0);
	return 0;
}