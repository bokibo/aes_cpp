// AES_final.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include <string>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <chrono>
#include "const.h"
using namespace std;


//multiplication for Inverse MixColumns
#define xtime(x)   ((x<<1) ^ (((x>>7) & 1) * 0x1b))
#define Multiply(x,y) (((y & 1) * x) ^ ((y>>1 & 1) * xtime(x)) ^ ((y>>2 & 1) * xtime(xtime(x))) ^ ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^ ((y>>4 & 1) * xtime(xtime(xtime(xtime(x))))))


//-------------------- STRUCT BLOCK --------------------
struct Block {
	char item[4][4];
};


//-------------------- LENGTH OF TEXT FILE --------------------
long fileLength(const char* filename) {
	FILE * f = fopen(filename, "r");
	long length;
	if (f)
	{
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fclose(f);
		return length;
	}
	else
		return 0;
}

//-------------------- PADDING --------------------
char padding(Block* plaintext, int num_of_blocks, int plaintext_length) {
	int x = plaintext_length % 16;
	int ii = x % 4;
	int jj = x / 4;
	fprintf(stderr, "\nChar for padding? ");
	unsigned char c = getchar();
	fprintf(stderr, "\n");
	bool padding = false;
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			if (ii == j & jj == i)
				padding = true;
			if (padding)
				plaintext[num_of_blocks - 1].item[j][i] = c;
		}
	}
	return c;
}


//-------------------- KEY SCHEDULING ALGORITHM --------------------
void key_scheduling(Block * keys) {
	//initial key
	char key[4][4] = {
		{ 0x54, 0x73, 0x20, 0x67 },
		{ 0x68, 0x20, 0x4b, 0x20 },
		{ 0x61, 0x6d, 0x75, 0x46 },
		{ 0x74, 0x79, 0x6e, 0x75 } };

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			keys[0].item[i][j] = key[i][j];
		}
	}

	// key scheduling algorithm
	for (int k = 1; k <= 10; k++) {
		Block tempNew;
		Block tempOld = keys[k - 1];
		char temp[4] = { tempOld.item[0][3], tempOld.item[1][3], tempOld.item[2][3], tempOld.item[3][3] }; //last column of first key
																										   //ROTWORD
		char t = temp[0];
		temp[0] = temp[1];
		temp[1] = temp[2];
		temp[2] = temp[3];
		temp[3] = t;

		//SUBBYTES
		//cout << endl << "3." << endl;
		for (int i = 0; i < 4; i++) {
			int x = (temp[i] >> 4) & 0xf;
			int y = temp[i] & 0xf;
			temp[i] = Sbox[x][y];

		}
		char temp2[4] = { tempOld.item[0][0], tempOld.item[1][0], tempOld.item[2][0], tempOld.item[3][0] }; //first column of first key
																											//xor second column and temp and Rcon 1st round
		for (int i = 0; i < 4; i++) {
			temp2[i] = temp[i] ^ tempOld.item[i][0];
			temp2[i] = temp2[i] ^ Rcon[i][k - 1];
		}

		for (int i = 0; i < 4; i++)  //first column of 2nd key
			tempNew.item[i][0] = temp2[i];

		for (int j = 1; j < 4; j++) {
			for (int i = 0; i < 4; i++)
			{
				tempNew.item[i][j] = (tempNew.item[i][j - 1] ^ tempOld.item[i][j]);
			}
		}
		keys[k] = tempNew;
	} //end of key scheduling
}


//-------------------- ENCRYPTION --------------------
void encryption(Block* keys, Block * plaintext, Block* ciphertext, int num_of_blocks)
{
	for (int t = 0; t < num_of_blocks; t++) 
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				//ciphertext[t].item[i][j] = plaintext[t].item[i][j];
				ciphertext[t].item[i][j] = plaintext[t].item[i][j] ^ keys[0].item[i][j];
			}
		}
	}
	//start of 9 rounds
	int num_of_rounds = 9;
	for (int r = 0; r < num_of_rounds; r++) 
	{
		for (int t = 0; t < num_of_blocks; t++) 
		{ // loop for ciphertext[t]

		//------------------ SubBytes -----------------------
			for (int i = 0; i < 4; ++i) 
			{
				for (int j = 0; j < 4; ++j) 
				{
					int x = (ciphertext[t].item[i][j] & 0xf0) >> 4;
					int y = (ciphertext[t].item[i][j] & 0xf);
					ciphertext[t].item[i][j] = Sbox[x][y];
				}
			}

			//-------------------------- Shift Rows --------------------------
			char temp;
			// Rotate first row 1 columns to left
			temp = ciphertext[t].item[1][0];
			ciphertext[t].item[1][0] = ciphertext[t].item[1][1];
			ciphertext[t].item[1][1] = ciphertext[t].item[1][2];
			ciphertext[t].item[1][2] = ciphertext[t].item[1][3];
			ciphertext[t].item[1][3] = temp;

			// Rotate second row 2 columns to left
			temp = ciphertext[t].item[2][0];
			ciphertext[t].item[2][0] = ciphertext[t].item[2][2];
			ciphertext[t].item[2][2] = temp;
			temp = ciphertext[t].item[2][1];
			ciphertext[t].item[2][1] = ciphertext[t].item[2][3];
			ciphertext[t].item[2][3] = temp;

			// Rotate third row 3 columns to left
			temp = ciphertext[t].item[3][0];
			ciphertext[t].item[3][0] = ciphertext[t].item[3][3];
			ciphertext[t].item[3][3] = ciphertext[t].item[3][2];
			ciphertext[t].item[3][2] = ciphertext[t].item[3][1];
			ciphertext[t].item[3][1] = temp;

			//------------------- end of shift rows --------------------------

			//------------------- mix columns --------------------------------
			char nn[4][4];
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					char tt[4];
					for (int k = 0; k < 4; ++k)
					{
						char mix_temp;
						if ((char)0x01 == MixCol[i][k])
						{
							mix_temp = ciphertext[t].item[k][j];
						}
						else if ((char)0x02 == MixCol[i][k])
						{
							mix_temp = ciphertext[t].item[k][j] << 1;
							int msb = ((ciphertext[t].item[k][j] & 0x80) >> 7) & 0x01;
							if (msb == 1)
							{
								mix_temp ^= 0x1b;
							}
						}
						else if ((char)0x03 == MixCol[i][k])
						{
							mix_temp = ciphertext[t].item[k][j] << 1;
							int msb = ((ciphertext[t].item[k][j] & 0x80) >> 7) & 0x01;
							if (msb == 1)
							{
								mix_temp ^= 0x1b;
							}
							mix_temp ^= ciphertext[t].item[k][j];
						}
						tt[k] = mix_temp;
					}
					char tempc = tt[0] ^ tt[1] ^ tt[2] ^ tt[3];
					nn[i][j] = tempc;
				}
			}
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					ciphertext[t].item[i][j] = nn[i][j];
				}

			} //------------------ end of mix columns --------------------------

			  //------------------------- add round key --------------------------
			for (size_t i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					ciphertext[t].item[i][j] ^= keys[r + 1].item[i][j];
				}
			}
			//------------------ end of round key ---------------------------
		}
	} //end of 9 rounds

	  //------------------ SubBytes -----------------------
	for (int t = 0; t < num_of_blocks; t++) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				int x = (ciphertext[t].item[i][j] & 0xf0) >> 4;
				int y = (ciphertext[t].item[i][j] & 0xf);
				ciphertext[t].item[i][j] = Sbox[x][y];
			}
		}

		//-------------------------- Shift Rows --------------------------
		char temp;
		// Rotate first row 1 columns to left
		temp = ciphertext[t].item[1][0];
		ciphertext[t].item[1][0] = ciphertext[t].item[1][1];
		ciphertext[t].item[1][1] = ciphertext[t].item[1][2];
		ciphertext[t].item[1][2] = ciphertext[t].item[1][3];
		ciphertext[t].item[1][3] = temp;

		// Rotate second row 2 columns to left
		temp = ciphertext[t].item[2][0];
		ciphertext[t].item[2][0] = ciphertext[t].item[2][2];
		ciphertext[t].item[2][2] = temp;
		temp = ciphertext[t].item[2][1];
		ciphertext[t].item[2][1] = ciphertext[t].item[2][3];
		ciphertext[t].item[2][3] = temp;

		// Rotate third row 3 columns to left
		temp = ciphertext[t].item[3][0];
		ciphertext[t].item[3][0] = ciphertext[t].item[3][3];
		ciphertext[t].item[3][3] = ciphertext[t].item[3][2];
		ciphertext[t].item[3][2] = ciphertext[t].item[3][1];
		ciphertext[t].item[3][1] = temp;

		//------------------- end of shift rows ----------------------------------
		//------------------- add round key -----------------------------
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				ciphertext[t].item[i][j] ^= keys[10].item[i][j];
			}
		}
	} //end of encription 
}


//-------------------- DECRYPTION --------------------
void decryption(Block* keys, Block* plaintext2, Block* ciphertext, int num_of_blocks) {
	// inverse add round
	for (int t = 0; t < num_of_blocks; t++) {
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				plaintext2[t].item[i][j] = ciphertext[t].item[i][j] ^ keys[10].item[i][j];
			}
		}
	}

	// inverse 9 rounds
	for (int k = 9; k >= 1; k--)
	{
		for (int t = 0; t < num_of_blocks; t++) {
			// inverse shift rows
			char temp = plaintext2[t].item[1][3];
			plaintext2[t].item[1][3] = plaintext2[t].item[1][2];
			plaintext2[t].item[1][2] = plaintext2[t].item[1][1];
			plaintext2[t].item[1][1] = plaintext2[t].item[1][0];
			plaintext2[t].item[1][0] = temp;

			temp = plaintext2[t].item[2][2];
			plaintext2[t].item[2][2] = plaintext2[t].item[2][0];
			plaintext2[t].item[2][0] = temp;
			temp = plaintext2[t].item[2][3];
			plaintext2[t].item[2][3] = plaintext2[t].item[2][1];
			plaintext2[t].item[2][1] = temp;

			temp = plaintext2[t].item[3][0];
			plaintext2[t].item[3][0] = plaintext2[t].item[3][1];
			plaintext2[t].item[3][1] = plaintext2[t].item[3][2];
			plaintext2[t].item[3][2] = plaintext2[t].item[3][3];
			plaintext2[t].item[3][3] = temp;
			//end of inv shift rows

			//inverse subbytes
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					int x = (plaintext2[t].item[i][j] & 0xf0) >> 4;
					int y = (plaintext2[t].item[i][j] & 0xf);
					plaintext2[t].item[i][j] = InvSbox[x][y];

				}

			} //end subbytes

			  //inverse add round key
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					plaintext2[t].item[i][j] ^= keys[k].item[i][j];

				}

			} //end add round

			Block tempb;
			//inverse mix columns
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					char tt[4];
					for (int k = 0; k < 4; ++k)
					{
						tt[k] = Multiply(plaintext2[t].item[k][i], InvMixCol[j][k]);
					}
					char dd = tt[0] ^ tt[1] ^ tt[2] ^ tt[3];

					tempb.item[j][i] = dd;
				}
			} //end of inv mix columns

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					plaintext2[t].item[i][j] = tempb.item[i][j];
				}
			}
		}
	} // end of 9 inverse rounds

	for (int t = 0; t < num_of_blocks; t++) {
		// inverse shift rows
		char temp = plaintext2[t].item[1][3];
		plaintext2[t].item[1][3] = plaintext2[t].item[1][2];
		plaintext2[t].item[1][2] = plaintext2[t].item[1][1];
		plaintext2[t].item[1][1] = plaintext2[t].item[1][0];
		plaintext2[t].item[1][0] = temp;

		temp = plaintext2[t].item[2][2];
		plaintext2[t].item[2][2] = plaintext2[t].item[2][0];
		plaintext2[t].item[2][0] = temp;
		temp = plaintext2[t].item[2][3];
		plaintext2[t].item[2][3] = plaintext2[t].item[2][1];
		plaintext2[t].item[2][1] = temp;

		temp = plaintext2[t].item[3][0];
		plaintext2[t].item[3][0] = plaintext2[t].item[3][1];
		plaintext2[t].item[3][1] = plaintext2[t].item[3][2];
		plaintext2[t].item[3][2] = plaintext2[t].item[3][3];
		plaintext2[t].item[3][3] = temp;

		//inverse subbytes
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				int x = (plaintext2[t].item[i][j] & 0xf0) >> 4;
				int y = (plaintext2[t].item[i][j] & 0xf);
				plaintext2[t].item[i][j] = InvSbox[x][y];
			}
		}

		//inverse add round key
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				plaintext2[t].item[i][j] ^= keys[0].item[i][j];
			}
		}
	}
}

void printBlock(Block* block) {

	for (size_t ii = 0; ii < 11; ii++)
	{
		for (size_t jj = 0; jj < 4; jj++) {
			for (size_t kk = 0; kk < 4; kk++)
			{
				cout << " " << hex << (int)(block[ii].item[jj][kk] & 0xff);
			}
			cout << endl;
		}
		cout << "--------------" << endl;
	}
}

int main()
{

	//-------------------- SYSTEM INFO --------------------
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	printf("   Hardware information: \n");
	printf("     OEM ID: %u\n", siSysInfo.dwOemId);
	printf("     Number of logical processors: %u\n",
		siSysInfo.dwNumberOfProcessors);
	printf("     Page size: %u\n", siSysInfo.dwPageSize);
	printf("     Processor type: ");
	switch (siSysInfo.dwProcessorType)
	{
	case 386:
		printf("PROCESSOR INTEL 386\n");
		break;
	case 486:
		printf("PROCESSOR INTEL 486\n");
		break;
	case 586:
		printf("PROCESSOR INTEL PENTIUM\n");
		break;
	case 2200:
		printf("PROCESSOR INTEL IA64\n");
		break;
	case 8664:
		printf("PROCESSOR AMD X8664\n");
		break;
	default:
		break;
	}
	printf("     Minimum application address: %lx\n",
		siSysInfo.lpMinimumApplicationAddress);
	printf("     Maximum application address: %lx\n",
		siSysInfo.lpMaximumApplicationAddress);
	printf("     Active processor mask: %u\n",
		siSysInfo.dwActiveProcessorMask);
	cout << endl;

	//-------------------- APPLICATION INFORMATION --------------------
	cout << "   Application information" << endl;
	Block *keys = new Block[11];
	key_scheduling(keys);

	cout << "   Enter text file name: ";
	char name[20];
	scanf_s("%s", name);
	long plaintext_length = fileLength(name);
	int num_of_blocks = (plaintext_length % 16 == 0) ? plaintext_length / 16 : plaintext_length / 16 + 1;
	Block * temp_plaintext = new Block[num_of_blocks];
	Block * plaintext = new Block[num_of_blocks];
	ifstream ifs(name);
	int k = 0;
	while (ifs) {
		ifs.read((char*)temp_plaintext[k].item, 16);
		k++;
	}
	int num_of_zeros = 0;
	for (int t = 0; t < num_of_blocks; t++) {
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (temp_plaintext[t].item[i][j] == '\n')
					num_of_zeros++;
				plaintext[t].item[j][i] = temp_plaintext[t].item[i][j];
			}
		}
	}
	//-------------------- TEXT INFO --------------------
	cout << "     Number of characters in file: " << plaintext_length << endl;
	cout << "     Number of blocks with zeros: " << num_of_blocks << endl;
	cout << "     Number of zeros: " << num_of_zeros << endl;
	int real_num_of_blocks = ((plaintext_length - num_of_zeros) % 16 == 0) ? (plaintext_length - num_of_zeros) / 16 : (plaintext_length - num_of_zeros) / 16 + 1;
	cout << "     Number of blocks without zeros: " << real_num_of_blocks << endl;

	//-------------------- ROW AND COLUMN SWAP --------------------
	Block* real_plaintext = new Block[real_num_of_blocks];
	for (int t = 0; t < real_num_of_blocks; t++) {
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				real_plaintext[t].item[i][j] = plaintext[t].item[i][j];
			}
		}
	}
	free(plaintext);
	free(temp_plaintext);
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<float> fsec;

	//-------------------- ENCRYPTION --------------------
	cout << "     Encryption..." << endl;
	Block * ciphertext = new Block[real_num_of_blocks];
	auto enc_start = Time::now();
	encryption(keys, real_plaintext, ciphertext, real_num_of_blocks);
	auto enc_end = Time::now();
	fsec enc_ms = enc_end - enc_start;
	cout << "     Encryption finished in " << enc_ms.count() * 1000 << " miliseconds" << endl;

	//-------------------- DECRYPTION --------------------
	cout << "     Decryption..." << endl;
	Block * plaintext2 = new Block[real_num_of_blocks];
	auto dec_start = Time::now();
	decryption(keys, plaintext2, ciphertext, real_num_of_blocks);
	auto dec_end = Time::now();
	fsec dec_ms = dec_end - dec_start;
	cout << "     Decryption finished in " << dec_ms.count()*1000 << " miliseconds" << endl;

	//-------------------- WRITING TO TEXT FILES --------------------
	cout << "     Writing to txt files..." << endl;
	ofstream off, off2, off3;
	int index = 0;
	for (size_t i = 0; i < 20; i++)
	{
		if (name[i] == '.')
		{
			index = i;
			break;
		}
	}
	name[index] = '_';
	name[index + 1] = 'i';
	name[index + 2] = 'n';
	name[index + 3] = 'f';
	name[index + 4] = 'o';
	name[index + 5] = '.';
	name[index + 6] = 't';
	name[index + 7] = 'x';
	name[index + 8] = 't';
	name[index + 9] = '\0';
	off3.open(name, ofstream::trunc);
	off3 << "Number of characters in file: " << plaintext_length << endl;
	off3 << "Number of blocks with zeros: " << num_of_blocks << endl;
	off3 << "Number of zeros: " << num_of_zeros << endl;
	off3 << "Number of blocks without zeros: " << real_num_of_blocks << endl;
	off3 << "Encryption finished in " << enc_ms.count() * 1000 << " miliseconds" << endl;
	off3 << "Decryption finished in " << dec_ms.count() * 1000 << " miliseconds" << endl;
	off3.close();

	off.open("encrypted_text.txt", ofstream::trunc);
	off2.open("decrypted_text.txt", ofstream::trunc);
	int counter = 0;
	for (size_t t = 0; t < real_num_of_blocks; t++)
	{
		for (size_t i = 0; i < 4; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				if (counter == plaintext_length - num_of_zeros)
				{
					break;
				}

				else {
					off << ciphertext[t].item[j][i];
					//if(plaintext2[t].item[j][i] != c)
					off2 << plaintext2[t].item[j][i];
					counter++;
				}
			}
		}
	}
	off.close();
	off2.close();
	system("pause");
	return 0;
}

