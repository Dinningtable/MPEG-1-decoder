#include <MpegDecoder.h>
#include <string.h>

#define ADDR_CODE_MAX_LEN 12
#define ADDR_CODE_MAX_VALUE 36
#define I_CODE_MAX_LEN 3
#define I_CODE_MAX_VALUE 2
#define P_CODE_MAX_LEN 7
#define P_CODE_MAX_VALUE 4
#define B_CODE_MAX_LEN 7
#define B_CODE_MAX_VALUE 4
#define CBP_CODE_MAX_LEN 10
#define CBP_CODE_MAX_VALUE 32
#define MV_CODE_MAX_LEN 12
#define MV_CODE_MAX_VALUE 64
#define DCT_DC_LUMA_CODE_MAX_LEN 8
#define DCT_DC_LUMA_CODE_MAX_VALUE 128
#define DCT_DC_CHROMA_CODE_MAX_LEN 9
#define DCT_DC_CHROMA_CODE_MAX_VALUE 256
#define DCT_COEFF_COMMON_CODE_MAX_LEN 18
#define DCT_COEFF_COMMON_CODE_MAX_VALUE 128
#define DCT_COEFF_FIRST_CODE_MAX_LEN 3
#define DCT_COEFF_FIRST_CODE_MAX_VALUE 4
#define DCT_COEFF_NEXT_CODE_MAX_LEN 4
#define DCT_COEFF_NEXT_CODE_MAX_VALUE 8
#define NOTHING -9527

void ADDENTRY(int** table, char* code, int value)
{
		int len = strlen(code);
		int code_value = 0;
		for(int i=0;i<len;i++)
		{
			code_value <<= 1;
			if(code[i] == '1') code_value += 1;
		}
		table[len][code_value] = value;
}

void build_addrinc_VLCtable(video_struct* video)
{
	video->macro_addrinc_VLCtable = (int**)malloc(ADDR_CODE_MAX_LEN*sizeof(int*));
	int* allocate_mem = (int*)malloc(ADDR_CODE_MAX_LEN * ADDR_CODE_MAX_VALUE*sizeof(int));
	for(int i=0 ; i<ADDR_CODE_MAX_LEN ; i++, allocate_mem+=ADDR_CODE_MAX_VALUE)
	{
		video->macro_addrinc_VLCtable[i] = allocate_mem;		
	}
	memset(allocate_mem, NOTHING, ADDR_CODE_MAX_LEN * ADDR_CODE_MAX_VALUE*sizeof(int));
    //macro_addrinc_VLCtable[code_len][code] = value
    //                          code 1 
    video->macro_addrinc_VLCtable[1][1] = 1;
    //						  code 011
    video->macro_addrinc_VLCtable[3][3] = 2;
    //						  code 010
    video->macro_addrinc_VLCtable[3][2] = 3;
    //					     code 0011
    video->macro_addrinc_VLCtable[4][3] = 4;
    //					     code 0010
    video->macro_addrinc_VLCtable[4][2] = 5;
    //					    code 00011
    video->macro_addrinc_VLCtable[5][3] = 6;
    //					    code 00010
    video->macro_addrinc_VLCtable[5][2] = 7;
    //					  code 0000111
    video->macro_addrinc_VLCtable[7][7] = 8;
    //					  code 0000110
    video->macro_addrinc_VLCtable[7][6] = 9;
    //					  code 00001011
    video->macro_addrinc_VLCtable[8][11] = 10;
    //					  code 00001010
    video->macro_addrinc_VLCtable[8][10] = 11;
    //					  code 00001001
    video->macro_addrinc_VLCtable[8][9] = 12;
    //					  code 00001000
    video->macro_addrinc_VLCtable[8][8] = 13;
    //					  code 00000111
    video->macro_addrinc_VLCtable[8][7] = 14;
    //					  code 00000110
    video->macro_addrinc_VLCtable[8][6] = 15;
    //					 code 0000010111
    video->macro_addrinc_VLCtable[10][23] = 16;
    //					 code 0000010110
    video->macro_addrinc_VLCtable[10][22] = 17;
    //					 code 0000010101
    video->macro_addrinc_VLCtable[10][21] = 18;
	//					 code 0000010100
    video->macro_addrinc_VLCtable[10][20] = 19;
    //					 code 0000010011
    video->macro_addrinc_VLCtable[10][19] = 20;
    //					 code 0000010010
    video->macro_addrinc_VLCtable[10][18] = 21;
    //					code 00000100011
    video->macro_addrinc_VLCtable[11][35] = 22;
    //					code 00000100010
    video->macro_addrinc_VLCtable[11][34] = 23;
    //					code 00000100001
    video->macro_addrinc_VLCtable[11][33] = 24;
    //					code 00000100000
    video->macro_addrinc_VLCtable[11][32] = 25;
    //					code 00000011111
    video->macro_addrinc_VLCtable[11][31] = 26;
    //					code 00000011110
    video->macro_addrinc_VLCtable[11][30] = 27;
    //					code 00000011101
    video->macro_addrinc_VLCtable[11][29] = 28;
    //					code 00000011100
    video->macro_addrinc_VLCtable[11][28] = 29;
    //					code 00000011011
    video->macro_addrinc_VLCtable[11][27] = 30;
    //					code 00000011010
    video->macro_addrinc_VLCtable[11][26] = 31;
    //					code 00000011001
    video->macro_addrinc_VLCtable[11][25] = 32;
    //					code 00000011000
    video->macro_addrinc_VLCtable[11][24] = 33;
    //macroblock_stuffing code 00000001111
    video->macro_addrinc_VLCtable[11][15] = -1;
    //macroblock_escape code 00000001000
    video->macro_addrinc_VLCtable[11][8] = -2;
}

static inline void build_I_VLCtable(int** table)
{
    table = (int**)malloc(I_CODE_MAX_LEN*sizeof(int*));
	int* allocate_mem = (int*)malloc(I_CODE_MAX_LEN*I_CODE_MAX_VALUE*sizeof(int));
	for(int i=0 ; i<I_CODE_MAX_LEN ; i++, allocate_mem+=I_CODE_MAX_VALUE)
	{
		table[i] = allocate_mem;		
	}
	memset(allocate_mem, NOTHING, I_CODE_MAX_LEN*I_CODE_MAX_VALUE*sizeof(int));
    //code 1
    table[1][1] = 1;
    //code 01
    table[2][1] = 17;
}

static inline void build_P_VLCtable(int** table)
{
    table = (int**)malloc(P_CODE_MAX_LEN*sizeof(int*));
	int* allocate_mem = (int*)malloc(P_CODE_MAX_LEN * P_CODE_MAX_VALUE*sizeof(int));
	for(int i=0 ; i<P_CODE_MAX_LEN ; i++, allocate_mem+=P_CODE_MAX_VALUE)
	{
		table[i] = allocate_mem;		
	}
	memset(allocate_mem, NOTHING, P_CODE_MAX_LEN * P_CODE_MAX_VALUE*sizeof(int));
    ADDENTRY(table, "1", 0x0a);
    ADDENTRY(table, "01", 0x02);
    ADDENTRY(table, "001", 0x08);
    ADDENTRY(table, "00011", 0x01);
    ADDENTRY(table, "00010", 0x1a);
    ADDENTRY(table, "00001", 0x12);
    ADDENTRY(table, "000001", 0x11);
}

void build_B_VLCtable(int** table)
{
	table = (int**)malloc(B_CODE_MAX_LEN*sizeof(int*));
	int* allocate_mem = (int*)malloc(B_CODE_MAX_LEN * B_CODE_MAX_VALUE*sizeof(int));
	for(int i=0 ; i<B_CODE_MAX_LEN ; i++, allocate_mem+=B_CODE_MAX_VALUE)
	{
		table[i] = allocate_mem;		
	}
	memset(allocate_mem, NOTHING, B_CODE_MAX_LEN * B_CODE_MAX_VALUE*sizeof(int));
    ADDENTRY(table, "10", 0x0c);
    ADDENTRY(table, "11", 0x0e);
    ADDENTRY(table, "010", 0x04);
    ADDENTRY(table, "011", 0x06);
    ADDENTRY(table, "0010", 0x08);
    ADDENTRY(table, "0011", 0x0a);
    ADDENTRY(table, "00011", 0x01);
    ADDENTRY(table, "00010", 0x1e);
    ADDENTRY(table, "000011", 0x1a);
    ADDENTRY(table, "000010", 0x16);
    ADDENTRY(table, "000001", 0x11);
}

static inline void build_cbp(int** table)
{
	table = (int**)malloc(CBP_CODE_MAX_LEN*sizeof(int*));
	int* allocate_mem = (int*)malloc(CBP_CODE_MAX_LEN * CBP_CODE_MAX_VALUE*sizeof(int));
	for(int i=0 ; i<CBP_CODE_MAX_LEN ; i++, allocate_mem+=CBP_CODE_MAX_VALUE)
	{
		table[i] = allocate_mem;		
	}
	memset(allocate_mem, NOTHING, CBP_CODE_MAX_LEN * CBP_CODE_MAX_VALUE*sizeof(int));
    ADDENTRY(table, "111", 60);
    ADDENTRY(table, "1101", 4);
    ADDENTRY(table, "1100", 8);
    ADDENTRY(table, "1011", 16);
    ADDENTRY(table, "1010", 32);
    ADDENTRY(table, "10011", 12);
    ADDENTRY(table, "10010", 48);
    ADDENTRY(table, "10001", 20);
    ADDENTRY(table, "10000", 40);
    ADDENTRY(table, "01111", 28);
    ADDENTRY(table, "01110", 44);
    ADDENTRY(table, "01101", 52);
    ADDENTRY(table, "01100", 56);
    ADDENTRY(table, "01011", 1);
    ADDENTRY(table, "01010", 61);
    ADDENTRY(table, "01001", 2);
    ADDENTRY(table, "01000", 62);
    ADDENTRY(table, "001111", 24);
    ADDENTRY(table, "001110", 36);
    ADDENTRY(table, "001101", 3);
    ADDENTRY(table, "001100", 63);
    ADDENTRY(table, "0010111", 5);
    ADDENTRY(table, "0010110", 9);
    ADDENTRY(table, "0010101", 17);
    ADDENTRY(table, "0010100", 33);
    ADDENTRY(table, "0010011", 6);
    ADDENTRY(table, "0010010", 10);
    ADDENTRY(table, "0010001", 18);
    ADDENTRY(table, "0010000", 34);
    ADDENTRY(table, "00011111", 7);
    ADDENTRY(table, "00011110", 11);
    ADDENTRY(table, "00011101", 19);
    ADDENTRY(table, "00011100", 35);
    ADDENTRY(table, "00011011", 13);
    ADDENTRY(table, "00011010", 49);
    ADDENTRY(table, "00011001", 21);
    ADDENTRY(table, "00011000", 41);
    ADDENTRY(table, "00010111", 14);
    ADDENTRY(table, "00010110", 50);
    ADDENTRY(table, "00010101", 22);
    ADDENTRY(table, "00010100", 42);
    ADDENTRY(table, "00010011", 15);
    ADDENTRY(table, "00010010", 51);
    ADDENTRY(table, "00010001", 23);
    ADDENTRY(table, "00010000", 43);
    ADDENTRY(table, "00001111", 25);
    ADDENTRY(table, "00001110", 37);
    ADDENTRY(table, "00001101", 26);
    ADDENTRY(table, "00001100", 38);
    ADDENTRY(table, "00001011", 29);
    ADDENTRY(table, "00001010", 45);
    ADDENTRY(table, "00001001", 53);
    ADDENTRY(table, "00001000", 57);
    ADDENTRY(table, "00000111", 30);
    ADDENTRY(table, "00000110", 46);
    ADDENTRY(table, "00000101", 54);
    ADDENTRY(table, "00000100", 58);
    ADDENTRY(table, "000000111", 31);
    ADDENTRY(table, "000000110", 47);
    ADDENTRY(table, "000000101", 55);
    ADDENTRY(table, "000000100", 59);
    ADDENTRY(table, "000000011", 27);
    ADDENTRY(table, "000000010", 39);
}

void build_mv(int** table)
{
	table = (int**)malloc(MV_CODE_MAX_LEN*sizeof(int*));
	int* allocate_mem = (int*)malloc(MV_CODE_MAX_LEN * MV_CODE_MAX_VALUE*sizeof(int));
	for(int i=0 ; i<MV_CODE_MAX_LEN ; i++, allocate_mem+=MV_CODE_MAX_VALUE)
	{
		table[i] = allocate_mem;		
	}
	memset(allocate_mem, NOTHING, MV_CODE_MAX_LEN * MV_CODE_MAX_VALUE*sizeof(int));
    ADDENTRY(table, "00000011001", -16);
    ADDENTRY(table, "00000011011", -15);
    ADDENTRY(table, "00000011101", -14);
    ADDENTRY(table, "00000011111", -13);
    ADDENTRY(table, "00000100001", -12);
    ADDENTRY(table, "00000100011", -11);
    ADDENTRY(table, "0000010011", -10);
    ADDENTRY(table, "0000010101", -9);
    ADDENTRY(table, "0000010111", -8);
    ADDENTRY(table, "00000111", -7);
    ADDENTRY(table, "00001001", -6);
    ADDENTRY(table, "00001011", -5);
    ADDENTRY(table, "0000111", -4);
    ADDENTRY(table, "00011", -3);
    ADDENTRY(table, "0011", -2);
    ADDENTRY(table, "011", -1);
    ADDENTRY(table, "1", 0);
    ADDENTRY(table, "010", 1);
    ADDENTRY(table, "0010", 2);
    ADDENTRY(table, "00010", 3);
    ADDENTRY(table, "0000110", 4);
    ADDENTRY(table, "00001010", 5);
    ADDENTRY(table, "00001000", 6);
    ADDENTRY(table, "00000110", 7);
    ADDENTRY(table, "0000010110", 8);
    ADDENTRY(table, "0000010100", 9);
    ADDENTRY(table, "0000010010", 10);
    ADDENTRY(table, "00000100010", 11);
    ADDENTRY(table, "00000100000", 12);
    ADDENTRY(table, "00000011110", 13);
    ADDENTRY(table, "00000011100", 14);
    ADDENTRY(table, "00000011010", 15);
    ADDENTRY(table, "00000011000", 16);
}

void build_dct_dc_luma(int** table)
{
	table = (int**)malloc(DCT_DC_LUMA_CODE_MAX_LEN*sizeof(int*));
	int* allocate_mem = (int*)malloc(DCT_DC_LUMA_CODE_MAX_LEN * DCT_DC_LUMA_CODE_MAX_VALUE*sizeof(int));
	for(int i=0 ; i<DCT_DC_LUMA_CODE_MAX_LEN ; i++, allocate_mem+=DCT_DC_LUMA_CODE_MAX_VALUE)
	{
		table[i] = allocate_mem;		
	}
	memset(allocate_mem, NOTHING, DCT_DC_LUMA_CODE_MAX_LEN * DCT_DC_LUMA_CODE_MAX_VALUE*sizeof(int));
    ADDENTRY(table, "100", 0);
    ADDENTRY(table, "00", 1);
    ADDENTRY(table, "01", 2);
    ADDENTRY(table, "101", 3);
    ADDENTRY(table, "110", 4);
    ADDENTRY(table, "1110", 5);
    ADDENTRY(table, "11110", 6);
    ADDENTRY(table, "111110", 7);
    ADDENTRY(table, "1111110", 8);
}

void build_dct_dc_chroma(int** table)
{
	table = (int**)malloc(DCT_DC_CHROMA_CODE_MAX_LEN*sizeof(int*));
	int* allocate_mem = (int*)malloc(DCT_DC_CHROMA_CODE_MAX_LEN * DCT_DC_CHROMA_CODE_MAX_VALUE*sizeof(int));
	for(int i=0 ; i<DCT_DC_CHROMA_CODE_MAX_LEN ; i++, allocate_mem+=DCT_DC_CHROMA_CODE_MAX_VALUE)
	{
		table[i] = allocate_mem;		
	}
	memset(allocate_mem, NOTHING, DCT_DC_CHROMA_CODE_MAX_LEN * DCT_DC_CHROMA_CODE_MAX_VALUE*sizeof(int));
    ADDENTRY(table, "00", 0);
    ADDENTRY(table, "01", 1);
    ADDENTRY(table, "10", 2);
    ADDENTRY(table, "110", 3);
    ADDENTRY(table, "1110", 4);
    ADDENTRY(table, "11110", 5);
    ADDENTRY(table, "111110", 6);
    ADDENTRY(table, "1111110", 7);
    ADDENTRY(table, "11111110", 8);
}

void build_dct_coeff_common(int** table)
{
	table = (int**)malloc(DCT_COEFF_COMMON_CODE_MAX_LEN*sizeof(int*));
	int* allocate_mem = (int*)malloc(DCT_COEFF_COMMON_CODE_MAX_LEN * DCT_COEFF_COMMON_CODE_MAX_VALUE*sizeof(int));
	for(int i=0 ; i<DCT_COEFF_COMMON_CODE_MAX_LEN ; i++, allocate_mem+=DCT_COEFF_COMMON_CODE_MAX_VALUE)
	{
		table[i] = allocate_mem;		
	}
	memset(allocate_mem, NOTHING, DCT_COEFF_COMMON_CODE_MAX_LEN * DCT_COEFF_COMMON_CODE_MAX_VALUE*sizeof(int));
    ADDENTRY(table, "0110", 1 << 8 | 1);
    ADDENTRY(table, "01000", 0 << 8 | 2);
    ADDENTRY(table, "01010", 2 << 8 | 1);
    ADDENTRY(table, "001010", 0 << 8 | 3);
    ADDENTRY(table, "001110", 3 << 8 | 1);
    ADDENTRY(table, "001100", 4 << 8 | 1);
    ADDENTRY(table, "0001100", 1 << 8 | 2);
    ADDENTRY(table, "0001110", 5 << 8 | 1);
    ADDENTRY(table, "0001010", 6 << 8 | 1);
    ADDENTRY(table, "0001000", 7 << 8 | 1);
    ADDENTRY(table, "00001100", 0 << 8 | 4);
    ADDENTRY(table, "00001000", 2 << 8 | 2);
    ADDENTRY(table, "00001110", 8 << 8 | 1);
    ADDENTRY(table, "00001010", 9 << 8 | 1);
    ADDENTRY(table, "001001100", 0 << 8 | 5);
    ADDENTRY(table, "001000010", 0 << 8 | 6);
    ADDENTRY(table, "001001010", 1 << 8 | 3);
    ADDENTRY(table, "001001000", 3 << 8 | 2);
    ADDENTRY(table, "001001110", 10 << 8 | 1);
    ADDENTRY(table, "001000110", 11 << 8 | 1);
    ADDENTRY(table, "001000100", 12 << 8 | 1);
    ADDENTRY(table, "001000000", 13 << 8 | 1);
    ADDENTRY(table, "00000010100", 0 << 8 | 7);
    ADDENTRY(table, "00000011000", 1 << 8 | 4);
    ADDENTRY(table, "00000010110", 2 << 8 | 3);
    ADDENTRY(table, "00000011110", 4 << 8 | 2);
    ADDENTRY(table, "00000010010", 5 << 8 | 2);
    ADDENTRY(table, "00000011100", 14 << 8 | 1);
    ADDENTRY(table, "00000011010", 15 << 8 | 1);
    ADDENTRY(table, "00000010000", 16 << 8 | 1);
    ADDENTRY(table, "0000000111010", 0 << 8 | 8);
    ADDENTRY(table, "0000000110000", 0 << 8 | 9);
    ADDENTRY(table, "0000000100110", 0 << 8 | 10);
    ADDENTRY(table, "0000000100000", 0 << 8 | 11);
    ADDENTRY(table, "0000000110110", 1 << 8 | 5);
    ADDENTRY(table, "0000000101000", 2 << 8 | 4);
    ADDENTRY(table, "0000000111000", 3 << 8 | 3);
    ADDENTRY(table, "0000000100100", 4 << 8 | 3);
    ADDENTRY(table, "0000000111100", 6 << 8 | 2);
    ADDENTRY(table, "0000000101010", 7 << 8 | 2);
    ADDENTRY(table, "0000000100010", 8 << 8 | 2);
    ADDENTRY(table, "0000000111110", 17 << 8 | 1);
    ADDENTRY(table, "0000000110100", 18 << 8 | 1);
    ADDENTRY(table, "0000000110010", 19 << 8 | 1);
    ADDENTRY(table, "0000000101110", 20 << 8 | 1);
    ADDENTRY(table, "0000000101100", 21 << 8 | 1);
    ADDENTRY(table, "00000000110100", 0 << 8 | 12);
    ADDENTRY(table, "00000000110010", 0 << 8 | 13);
    ADDENTRY(table, "00000000110000", 0 << 8 | 14);
    ADDENTRY(table, "00000000101110", 0 << 8 | 15);
    ADDENTRY(table, "00000000101100", 1 << 8 | 6);
    ADDENTRY(table, "00000000101010", 1 << 8 | 7);
    ADDENTRY(table, "00000000101000", 2 << 8 | 5);
    ADDENTRY(table, "00000000100110", 3 << 8 | 4);
    ADDENTRY(table, "00000000100100", 5 << 8 | 3);
    ADDENTRY(table, "00000000100010", 9 << 8 | 2);
    ADDENTRY(table, "00000000100000", 10 << 8 | 2);
    ADDENTRY(table, "00000000111110", 22 << 8 | 1);
    ADDENTRY(table, "00000000111100", 23 << 8 | 1);
    ADDENTRY(table, "00000000111010", 24 << 8 | 1);
    ADDENTRY(table, "00000000111000", 25 << 8 | 1);
    ADDENTRY(table, "00000000110110", 26 << 8 | 1);
    ADDENTRY(table, "000000000111110", 0 << 8 | 16);
    ADDENTRY(table, "000000000111100", 0 << 8 | 17);
    ADDENTRY(table, "000000000111010", 0 << 8 | 18);
    ADDENTRY(table, "000000000111000", 0 << 8 | 19);
    ADDENTRY(table, "000000000110110", 0 << 8 | 20);
    ADDENTRY(table, "000000000110100", 0 << 8 | 21);
    ADDENTRY(table, "000000000110010", 0 << 8 | 22);
    ADDENTRY(table, "000000000110000", 0 << 8 | 23);
    ADDENTRY(table, "000000000101110", 0 << 8 | 24);
    ADDENTRY(table, "000000000101100", 0 << 8 | 25);
    ADDENTRY(table, "000000000101010", 0 << 8 | 26);
    ADDENTRY(table, "000000000101000", 0 << 8 | 27);
    ADDENTRY(table, "000000000100110", 0 << 8 | 28);
    ADDENTRY(table, "000000000100100", 0 << 8 | 29);
    ADDENTRY(table, "000000000100010", 0 << 8 | 30);
    ADDENTRY(table, "000000000100000", 0 << 8 | 31);
    ADDENTRY(table, "0000000000110000", 0 << 8 | 32);
    ADDENTRY(table, "0000000000101110", 0 << 8 | 33);
    ADDENTRY(table, "0000000000101100", 0 << 8 | 34);
    ADDENTRY(table, "0000000000101010", 0 << 8 | 35);
    ADDENTRY(table, "0000000000101000", 0 << 8 | 36);
    ADDENTRY(table, "0000000000100110", 0 << 8 | 37);
    ADDENTRY(table, "0000000000100100", 0 << 8 | 38);
    ADDENTRY(table, "0000000000100010", 0 << 8 | 39);
    ADDENTRY(table, "0000000000100000", 0 << 8 | 40);
    ADDENTRY(table, "0000000000111110", 1 << 8 | 8);
    ADDENTRY(table, "0000000000111100", 1 << 8 | 9);
    ADDENTRY(table, "0000000000111010", 1 << 8 | 10);
    ADDENTRY(table, "0000000000111000", 1 << 8 | 11);
    ADDENTRY(table, "0000000000110110", 1 << 8 | 12);
    ADDENTRY(table, "0000000000110100", 1 << 8 | 13);
    ADDENTRY(table, "0000000000110010", 1 << 8 | 14);
    ADDENTRY(table, "00000000000100110", 1 << 8 | 15);
    ADDENTRY(table, "00000000000100100", 1 << 8 | 16);
    ADDENTRY(table, "00000000000100010", 1 << 8 | 17);
    ADDENTRY(table, "00000000000100000", 1 << 8 | 18);
    ADDENTRY(table, "00000000000101000", 6 << 8 | 3);
    ADDENTRY(table, "00000000000110100", 11 << 8 | 2);
    ADDENTRY(table, "00000000000110010", 12 << 8 | 2);
    ADDENTRY(table, "00000000000110000", 13 << 8 | 2);
    ADDENTRY(table, "00000000000101110", 14 << 8 | 2);
    ADDENTRY(table, "00000000000101100", 15 << 8 | 2);
    ADDENTRY(table, "00000000000101010", 16 << 8 | 2);
    ADDENTRY(table, "00000000000111110", 27 << 8 | 1);
    ADDENTRY(table, "00000000000111100", 28 << 8 | 1);
    ADDENTRY(table, "00000000000111010", 29 << 8 | 1);
    ADDENTRY(table, "00000000000111000", 30 << 8 | 1);
    ADDENTRY(table, "00000000000110110", 31 << 8 | 1);

    ADDENTRY(table, "0111", (1 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "01001", (0 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "01011", (2 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001011", (0 << 8 | 3) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001111", (3 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001101", (4 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0001101", (1 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0001111", (5 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0001011", (6 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0001001", (7 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00001101", (0 << 8 | 4) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00001001", (2 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00001111", (8 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00001011", (9 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001001101", (0 << 8 | 5) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001000011", (0 << 8 | 6) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001001011", (1 << 8 | 3) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001001001", (3 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001001111", (10 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001000111", (11 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001000101", (12 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "001000001", (13 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000010101", (0 << 8 | 7) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000011001", (1 << 8 | 4) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000010111", (2 << 8 | 3) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000011111", (4 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000010011", (5 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000011101", (14 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000011011", (15 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000010001", (16 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000111011", (0 << 8 | 8) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000110001", (0 << 8 | 9) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000100111", (0 << 8 | 10) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000100001", (0 << 8 | 11) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000110111", (1 << 8 | 5) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000101001", (2 << 8 | 4) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000111001", (3 << 8 | 3) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000100101", (4 << 8 | 3) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000111101", (6 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000101011", (7 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000100011", (8 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000111111", (17 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000110101", (18 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000110011", (19 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000101111", (20 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000101101", (21 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000110101", (0 << 8 | 12) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000110011", (0 << 8 | 13) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000110001", (0 << 8 | 14) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000101111", (0 << 8 | 15) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000101101", (1 << 8 | 6) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000101011", (1 << 8 | 7) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000101001", (2 << 8 | 5) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000100111", (3 << 8 | 4) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000100101", (5 << 8 | 3) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000100011", (9 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000100001", (10 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000111111", (22 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000111101", (23 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000111011", (24 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000111001", (25 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000110111", (26 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000111111", (0 << 8 | 16) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000111101", (0 << 8 | 17) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000111011", (0 << 8 | 18) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000111001", (0 << 8 | 19) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000110111", (0 << 8 | 20) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000110101", (0 << 8 | 21) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000110011", (0 << 8 | 22) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000110001", (0 << 8 | 23) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000101111", (0 << 8 | 24) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000101101", (0 << 8 | 25) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000101011", (0 << 8 | 26) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000101001", (0 << 8 | 27) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000100111", (0 << 8 | 28) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000100101", (0 << 8 | 29) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000100011", (0 << 8 | 30) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000000000100001", (0 << 8 | 31) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000110001", (0 << 8 | 32) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000101111", (0 << 8 | 33) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000101101", (0 << 8 | 34) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000101011", (0 << 8 | 35) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000101001", (0 << 8 | 36) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000100111", (0 << 8 | 37) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000100101", (0 << 8 | 38) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000100011", (0 << 8 | 39) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000100001", (0 << 8 | 40) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000111111", (1 << 8 | 8) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000111101", (1 << 8 | 9) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000111011", (1 << 8 | 10) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000111001", (1 << 8 | 11) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000110111", (1 << 8 | 12) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000110101", (1 << 8 | 13) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "0000000000110011", (1 << 8 | 14) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000100111", (1 << 8 | 15) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000100101", (1 << 8 | 16) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000100011", (1 << 8 | 17) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000100001", (1 << 8 | 18) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000101001", (6 << 8 | 3) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000110101", (11 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000110011", (12 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000110001", (13 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000101111", (14 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000101101", (15 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000101011", (16 << 8 | 2) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000111111", (27 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000111101", (28 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000111011", (29 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000111001", (30 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "00000000000110111", (31 << 8 | 1) & 0xff00 | -(entry.value & 0xff) & 0xff);
    ADDENTRY(table, "000001", -2); // escape
}

void build_dct_coeff_first(int** table)
{
    build_dct_coeff_common(table);
    ADDENTRY(table, "10", 0x0001);
    ADDENTRY(table, "11", 0x00ff);
}

static inline void build_dct_coeff_next(int** table)
{
    build_dct_coeff_common(table);
    ADDENTRY(table, "01", -1); // EOB
    ADDENTRY(table, "110", 0x0001);
    ADDENTRY(table, "111", 0x00ff);
}