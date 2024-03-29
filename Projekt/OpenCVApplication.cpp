// 
/* File Format
*head:*
"rows"(4Bytes) "cols"(4Bytes)

*table:*
256 number of times: "Size"(1Byte) "code"("Size" number bits supplemented with some more bits to make up full bytes)

*actual image data:*
"size of image"(4Bytes) - indicates the number of bits in the full huffman encoding of the image
"bits of the huffman encoding"("size of image" number of bits supplemented with some more bits to make up full bytes)
  -bits are not stored consecutively, ex. 0b(01100100 00111110) stores the code 00100110 011111..
*/
#include "stdafx.h"
#include "common.h"
#include <vector>
#include <string>
#include <map>

long totalHeaderSize = 0;//used for analytics purposes only
long totalBodySize = 0;//used for analytics purposes only
bool write_header = false;



int bit_number = 0;
unsigned char bit_buffer;

void writeBit(char bit, FILE* f)
{
	if (bit == '1')
		bit_buffer |= (1 << bit_number);

	bit_number++;
	if (bit_number == 8)//buffer full, needs to write 1 byte
	{
		//analytics only
		if (write_header) totalHeaderSize++;
		else totalBodySize++;

		//
		fwrite(&bit_buffer, 1, 1, f);
		bit_buffer = 0;
		bit_number = 0;
	}
}

void flushBits(FILE* f)
{
	while (bit_number > 0)//if buffer is not empty write it into file
		writeBit(0, f);
}

struct code_struct
{
	int probab;//probability in whole numbers(without dividing it with the total number)
	std::string cod = "";
};

std::vector<code_struct> calculateCodes(int probabilities[], int index);
void swap(int *xp, int *yp);
void selectionSort(int arr[], int n);
void saveToBinary(Mat_<uchar> img, std::string encoded[], std::string name);

void encodeImage()
{
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		int histo[256] = { 0 };
		Mat_<uchar>img;

		std::string path = fname;
		img = imread(path, CV_LOAD_IMAGE_GRAYSCALE);	// Read the image

		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				histo[img(i, j)]++;
			}
		}

		int probabilities[256] = { 0 }, index = 0;
		for (int i = 0; i < 256; i++) {
			if (histo[i] > 0) {
				probabilities[index] = histo[i];
				index++;
			}
		}

		std::vector<code_struct> res = calculateCodes(probabilities, index);

		std::string encoded[256] = { "" };

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

		//save output in same folder with extension .dat
		saveToBinary(img, encoded, std::string(fname)+".dat");
	}
}

std::vector<code_struct> calculateCodes(int probabilities[], int index ) {
	std::vector<code_struct> res;
	code_struct e1, e2;
	
	//if less than 2 elements then stop
	if (index == 1) {//only one color
		e1.probab = probabilities[0];
		e1.cod.push_back('0');
		res.push_back(e1);
		return res;
	}
	else if (index == 2) {
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

void saveToBinary(Mat_<uchar> img, std::string encoded[], std::string fileName)
{
	std::string res ="";
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			std::string corresponding_code = encoded[img(i, j)];
			res.append(corresponding_code);
		}
	}
	FILE* pFile;
	pFile = fopen(fileName.c_str(), "wb");

	totalHeaderSize = 0;//used for analytics purposes only
	totalBodySize = 0;//used for analytics purposes only
	write_header = true;//used for analytics purposes only

	if (pFile != NULL) {
		//first save image height, width
		fwrite(&img.rows, 4, 1, pFile);
		fwrite(&img.cols, 4, 1, pFile);

		//write out table
		for (int i = 0; i < 256; i++) {
			unsigned char size = encoded[i].size();//max 256
			fwrite(&size, 1, 1, pFile);
			for (int j = 0; j < size; j++) {
				writeBit(encoded[i][j], pFile);
			}
			flushBits(pFile);
		}

		write_header = false;//used for analytics purposes only

		long res_size = res.size();
		//write size of res
		fwrite(&res_size, 4, 1, pFile);
		//write actual image data
		for (int i = 0; i < res.size(); i++) {
			writeBit(res[i], pFile);
		}
		flushBits(pFile);
		fclose(pFile);

		printf("Successful compression. Compression Ratio: %lf\n", (float)(img.cols*img.rows*8)/ res.size());
		printf("Header/Body Ratio: %lf\n", (float)(totalHeaderSize) / totalBodySize);
	}
}

void decodeFromBinary() {
	
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {

		std::map<std::string, uchar> encoded;
		FILE* pFile;
		char c;
		pFile = fopen(fname, "rb");

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
			for (int j = 0; j < (size - 1) / 8 + 1 && size>0; j++)
			{
				fread(&c, 1, 1, pFile);
				for (int i = 0; i < 8; i++) { // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes
					pc++;
					if (pc > size) break;
					int bit = ((c >> i) & 1);
					if (bit == 1)
						input.append("1");
					else input.append("0");
				}
			}
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
				while (pc < image.size()) {
					good_code += image[pc];
					pc++;//increment here because of 'break'
					if (encoded.find(good_code) != encoded.end()) {//code is valid
						uchar value = encoded[good_code];
						img.at<uchar>(i, j) = value;
						break;
					}
				}
			}
		}

		//save image
		std::string outFile(fname);
		if (outFile.size() > 8 && strstr(outFile.c_str(), ".bmp")) {//check is original file contained BMP extension
		 outFile = outFile.substr(0, outFile.size() - 8) + ".out.bmp";
		}
		else{
			outFile = outFile.substr(0, outFile.size() - 4) + ".out.bmp";
		}
		imwrite(outFile, img);
		//show image
		imshow("image", img);
		waitKey();
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
		printf(" 1 - Encode Image\n");
		printf(" 2 - Decode image\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d",&op);
		switch (op)
		{
			case 1:
				encodeImage();
				break;
			case 2:
				decodeFromBinary();
				break;
		}
	}
	while (op!=0);
	return 0;
}