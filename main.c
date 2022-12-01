#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include "MpegDecoder.h"
#include "IDCT.c"
#include "VLC.c"

#define picture_start_code 0x00000100
#define slice_start_codes 0x00000101
#define user_data_start_code 0x000001b2
#define sequence_header_code 0x000001b3
#define sequence_error_code 0x000001b4
#define extension_start_code 0x000001b5
#define sequence_end_code 0x000001b7
#define group_start_code 0x000001b8
#define system_start_codes 0x000001b9
#define NOTHING -9527

#define MAX_N_FRAMES 200

#define clip(l, v, h) ((v) < (l) ? (l) : ((h) < (v) ? (h) : (v)))
#define sign(v) (v > 0 ? 1 : (v < 0 ? -1 : 0))


double pixel_ar_table[16] = {0.0, 1.0, 0.6735, 0.7031, 0.7615, 0.8055, 0.8437, 0.8935, 0.9157, 0.9815, 1.0255, 1.0695, 1.095, 1.1575, 1.2051};
double fps_table[16] = {0.0, 23.976, 24.0, 25.0, 29.97, 30.0, 50.0, 59.94, 60.0};
int block_comp[6] = { 0, 0, 0, 0, 1, 2 };
int scan[64] = { 0, 1, 5, 6, 14, 15, 27, 28, 2, 4, 7, 13, 16, 26, 29, 42, 3, 8, 12, 17, 25, 30, 41, 43, 9, 11, 18, 24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63 };


//initial video structure
void init_video_struct(video_struct* video)
{
	video->pos_ind = 0;
	video->bits = 0;
	video->b_len = 0;
	for(int i=1;i<33;i++)
	{
		video->And_table[i] = (1 << (63-i)) + video->And_table[i-1];
	}

	video->frame = (Pixel**)malloc(MAX_N_FRAMES * sizeof(Pixel*));
	int alen = video->width * video->height;
	Pixel* allocate_mem = (Pixel*)malloc(MAX_N_FRAMES * alen *sizeof(Pixel));
	for(int i=0 ; i<MAX_N_FRAMES ; i++, allocate_mem+=alen)
	{
		video->frame[i] = allocate_mem;		
	}
	video->nframes = 0;
}

//extract the next n bits
uint32_t nextbits(int n, video_struct* video, int popout)
{
	uint64_t changed_bits = video->bits;
	int changed_len = video->b_len;
	long changed_pos_ind = video->pos_ind;
	uint32_t nbits;
	//if the number of bits needed is less than that in buffer
	if(n <= changed_len)
	{
		nbits = (changed_bits | And_table[n]) >> (64-n);
		changed_bits <<= n;
		changed_len -= n;
	}
	//if the number of bits needed is more than that in buffer
	else if(n > changed_len)
	{
		//get one or more bytes from stream until enough
		while(n > changed_len)
		{
			changed_bits = changed_bits + (video->stream[pos_ind] << (56-changed_len));
			changed_len += 8;
			changed_pos_ind++;
		}
		
		nbits = (changed_bits | And_table[n]) >> (64-n);
		changed_bits <<= n;
		changed_len -= n;
	}
	//whether pop bits out
	if(popout)
	{
		video->bits = changed_bits;
		video->b_len = changed_len;
		video->pos_ind = changed_pos_ind;
	}
	return nbits;
}

//discard remaining bits, align to byte
void align_to_byte(video_struct* video)
{
	//remaining  bits should be less 8 bits
	if(video->b_len < 8)
	{
		video->bits = 0;
		video->b_len = 0;
	}
	//debug message
	else if(video->b_len > 8)
	{
		printf("Syntax Error: b_len > 8\n");
	}
}

uint32_t decodeVLC(int** table, video_struct* video)
{
	int len = 0;
	uint32_t code_value = 0;
	while(1)
	{
		code_value <<= 1;
		code_value += nextbits(1, video, true);
		len++;
		if(table[len][code_value] != NOTHING)
		{
			return table[len][code_value];
		}
	}
}

void decodeBlock(video_struct* video, int id)
{
    int block_zz[64];
    memset(block_zz, 0, 64);
    int run_level, run, level, pos = 0;
    if (video->cur_mb.intra) {
        int size, diff = 0;
        if (id < 4) size = decodeVLC(video->dct_dc_luma_VLCtable, video);
        else size = decodeVLC(video->dct_dc_chroma_VLCtable, video);
        if (size) {
            diff = nextbits(size, video, true);
            if (diff < 1 << (size - 1)) diff -= (1 << size) - 1;
        }
        block_zz[0] = video->cur_slice.dc_predictor[block_comp[id]] + 8 * diff;
        video->cur_slice.dc_predictor[block_comp[id]] = block_zz[0];
    }
    else{
        run_level = decodeVLC(video->dct_coeff_first_VLCtable, video);
        if (run_level == -2) {
            run = nextbits(6, video, true);
            level = (char)nextbits(8, video, true);
            if (level == 0 || level == 0xffffff80) level = level << 1 | nextbits(8, video, true);
        }
        else{
            run = run_level >> 8;
            level = (char)run_level;
        }
        pos = run;
        block_zz[pos] = level;
    }
    while ((run_level = decodeVLC(video->dct_coeff_next_VLCtable, video)) != -1) { // EOB
        if (run_level == -2) {
            run = nextbits(6, video, true);
            level = (char)nextbits(8, video, true);
            if (level == 0 || level == 0xffffff80) level = level << 1 | nextbits(8, video, true);
        }
        else{
            run = run_level >> 8;
            level = (char)run_level;
        }
        pos += run + 1;
        block_zz[pos] = level;
    }
    if (video->cur_mb.intra) {
        for (int i = 1; i < 64; i++) {
            block_zz[i] = (2 * block_zz[i] * video->cur_mb.q_scale * video->intra_q_matrix[i]) / 16;
            if (!block_zz & 1) block_zz[i] -= sign(block_zz[i]);
            block_zz[i] = clip(-2048, block_zz[i], 2047);
        }
    }
    else {
        for (int i = 0; i < 64; i++) {
            block_zz[i] = (2 * block_zz[i] + sign(block_zz[i])) * video->cur_mb.q_scale * non_intra_q_matrix[i] / 16;
            if (!(block_zz[i] & 1)) block_zz[i] -= sign(block_zz[i]);
            block_zz[i] = clip(-2048, block_zz[i], 2047);
         }
    }
    for (int i = 0; i < 64; i++) video->cur_mb.block[id][i] = block_zz[scan[i]];
    idct(video->cur_mb.block[id]);
}

inline void buildBlockFromMV(video_struct* video)
{
    int base_x = (video->cur_mb.addr / video->width_mb) * 16;
    int base_y = (video->cur_mb.addr % video->width_mb) * 16;
    int right_for = video->cur_mb.recon_right_for >> 1;
    int down_for = video->cur_mb.recon_down_for >> 1;
    int right_half_for = video->cur_mb.recon_right_for - (right_for << 1);
    int down_half_for = video->cur_mb.recon_down_for - (down_for << 1);
    int right_back = video->cur_mb.recon_right_back >> 1;
    int down_back = video->cur_mb.recon_down_back >> 1;
    int right_half_back = video->cur_mb.recon_right_back - (right_back << 1);
    int down_half_back = video->cur_mb.recon_down_back - (down_back << 1);
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int x = video->height - (base_x + i) - 1,
                y = base_y + j;
            int R_for = 0, G_for = 0, B_for = 0, cnt_for = 0;
            int R_back = 0, G_back = 0, B_back = 0, cnt_back = 0;
            if (video->cur_mb.motion_forward) {
                int forx = x - down_for,
                    fory = y + right_for;
                R_for = video->forward_ref[forx * video->width + fory].r;
                G_for = video->forward_ref[forx * video->width + fory].g;
                B_for = video->forward_ref[forx * video->width + fory].b;
                cnt_for++;
                if (down_half_for) {
                    R_for += video->forward_ref[(forx - 1) * video->width + fory].r;
                    G_for += video->forward_ref[(forx - 1) * video->width + fory].g;
                    B_for += video->forward_ref[(forx - 1) * video->width + fory].b;
                    cnt_for++;
                }
                if (right_half_for) {
                    R_for += video->forward_ref[forx * video->width + fory + 1].r;
                    G_for += video->forward_ref[forx * video->width + fory + 1].g;
                    B_for += video->forward_ref[forx * video->width + fory + 1].b;
                    cnt_for++;
                }
                if (down_half_for && right_half_for) {
                    R_for += video->forward_ref[(forx - 1) * video->width + fory + 1].r;
                    G_for += video->forward_ref[(forx - 1) * video->width + fory + 1].g;
                    B_for += video->forward_ref[(forx - 1) * video->width + fory + 1].b;
                    cnt_for++;
                }
            }
            if (video->cur_mb.motion_backward) {
                int backx = x - down_back,
                    backy = y + right_back;
                R_back = video->backward_ref[backx * video->width + backy].r;
                G_back = video->backward_ref[backx * video->width + backy].g;
                B_back = video->backward_ref[backx * video->width + backy].b;
                cnt_back++;
                if (down_half_back) {
                    R_back += video->backward_ref[(backx - 1) * video->width + backy].r;
                    G_back += video->backward_ref[(backx - 1) * video->width + backy].g;
                    B_back += video->backward_ref[(backx - 1) * video->width + backy].b;
                    cnt_back++;
                }
                if (right_half_back) {
                    R_back += video->backward_ref[backx * video->width + backy + 1].r;
                    G_back += video->backward_ref[backx * video->width + backy + 1].g;
                    B_back += video->backward_ref[backx * video->width + backy + 1].b;
                    cnt_back++;
                }
                if (down_half_back && right_half_back) {
                    R_back += video->backward_ref[(backx - 1) * video->width + backy + 1].r;
                    G_back += video->backward_ref[(backx - 1) * video->width + backy + 1].g;
                    B_back += video->backward_ref[(backx - 1) * video->width + backy + 1].b;
                    cnt_back++;
                }
            }
            int R, G, B;
            if (cnt_back == 0) {
                R = R_for / cnt_for; G = G_for / cnt_for; B = B_for / cnt_for;
            }
            else if (cnt_for == 0) {
                R = R_back / cnt_back; G = G_back / cnt_back; B = B_back / cnt_back;
            }
            else {
                R = (R_for * cnt_back + R_back * cnt_for) / (cnt_for * cnt_back * 2);
                G = (G_for * cnt_back + G_back * cnt_for) / (cnt_for * cnt_back * 2);
                B = (B_for * cnt_back + B_back * cnt_for) / (cnt_for * cnt_back * 2);
            }
            video->cur_picture.pixel[x * video->width + y] = {B, G, R};
        }
    }
}

void decodeMacroblock(video_struct* video)
{
	int tmp;
    int escape_count = 0;
    //Handle stuffing codes               0000 0001 111
    while(nextbits(11, video, false) == 0x0F) 
    {
        nextbits(11, video, true);
    }
    // Handle escape codes                0000 0001 000
    while(nextbits(11, video, false) == 0x08) 
    {
        escape_count++;
        nextbits(11, video, true);
    }

    video->cur_mb.adde_incre = decodeVLC(video->macro_addrinc_VLCtable, video);

	video->cur_mb.addr = video->cur_slice.last_mb_addr;
    for (video->cur_mb.adde_incre += escape_count * 33; tmp > 1; tmp--) 
    {
        video->cur_mb.addr++;
        if (video->cur_picture.type == 'P') 
        {
            video->cur_mb.recon_right_for = video->cur_mb.recon_down_for
                = video->cur_slice.recon_right_for_prev = video->cur_slice.recon_down_for_prev = 0;
            buildBlockFromMV(video);
        }
        else if (video->cur_picture.type == 'B') 
        {
            buildBlockFromMV(video);
        }
    }
    video->cur_mb.addr++;
    printf("Macroblock pos=(%d, %d)\n", video->cur_mb.addr / video->width_mb, video->cur_mb.addr % video->width_mb);
    switch (video->cur_picture.type) 
    {
    	case 'I': tmp = decodeVLC(video->macro_I_VLCtable, video); break;
    	case 'P': tmp = decodeVLC(video->macro_P_VLCtable, video); break;
    	case 'B': tmp = decodeVLC(video->macro_B_VLCtable, video); break;
    	default: throw "picture type not supported";
    }
    video->cur_mb.quant = tmp & 0x10;
    video->cur_mb.motion_forward = tmp & 0x08;
    video->cur_mb.motion_backward = tmp & 0x04;
    video->cur_mb.pattern = tmp & 0x02;
    video->cur_mb.intra = tmp & 1;
    video->cur_mb.q_scale = video->cur_mb.quant ? nextbits(5, video, true) : video->cur_slice.q_scale;
    video->cur_slice.q_scale = video->cur_mb.q_scale;
    if (!video->cur_mb.intra || video->cur_mb.addr - video->cur_slice.last_intra_addr > 1) 
    {
        video->cur_slice.dc_predictor[0] = video->cur_slice.dc_predictor[1] = video->cur_slice.dc_predictor[2] = 1024;
    }
    if (video->cur_mb.intra) video->cur_slice.last_intra_addr = video->cur_mb.addr;
    // motion vectors
    if (video->cur_picture.type == 'B' && video->cur_mb.intra) 
    {
        video->cur_slice.recon_right_for_prev = video->cur_slice.recon_down_for_prev
            = video->cur_slice.recon_right_back_prev = video->cur_slice.recon_down_back_prev = 0;
    }

    int base_x = (video->cur_mb.addr / video->width_mb) * 16,
        base_y = (video->cur_mb.addr % video->width_mb) * 16;
    int mmax, mmin;
    int right_little, right_big, down_little, down_big;

    // forward vector
    int recon_right_for = 0, recon_down_for = 0;
    if (video->cur_mb.motion_forward) 
    {
        int forward_h_code, comp_forward_h_r = 0,
            forward_v_code, comp_forward_v_r = 0;
        forward_h_code = decodeVLC(video->mv_VLCtable, video);
        if (video->cur_picture.forward_f != 1 && forward_h_code != 0) 
        {
            tmp = nextbits(video->cur_picture.forward_r_size, video, true); // motion_horizontal_forward_r
            comp_forward_h_r = video->cur_picture.forward_f - 1 - tmp;
        }
        forward_v_code = decodeVLC(video->mv_VLCtable, video);
        if (video->cur_picture.forward_f != 1 && forward_v_code != 0) 
        {
            tmp = nextbits(video->cur_picture.forward_r_size, video, true); // motion_vertical_forward_r
            comp_forward_v_r = video->cur_picture.forward_f - 1 - tmp;
        }
        // start video 2.4.4.2
        right_little = forward_h_code * video->cur_picture.forward_f;
        if (right_little > 0) 
        {
            right_little -= comp_forward_h_r;
            right_big = right_little - 32 * video->cur_picture.forward_f;
        }
        else if (right_little < 0) 
        {
            right_little += comp_forward_h_r;
            right_big = right_little + 32 * video->cur_picture.forward_f;
        }
        else right_big = 0;
        down_little = forward_v_code * video->cur_picture.forward_f;
        if (down_little > 0) 
        {
            down_little -= comp_forward_v_r;
            down_big = down_little - 32 * video->cur_picture.forward_f;
        }
        else if (down_little < 0) 
        {
            down_little += comp_forward_v_r;
            down_big = down_little + 32 * video->cur_picture.forward_f;
        }
        else down_big = 0;
        mmax = (video->cur_picture.forward_f << 4) - 1;
        mmin = -(video->cur_picture.forward_f << 4);
        tmp = video->cur_slice.recon_right_for_prev + right_little;
        if (mmin <= tmp && tmp <= mmax) recon_right_for = tmp;
        else recon_right_for = tmp - right_little + right_big;
        video->cur_slice.recon_right_for_prev = recon_right_for;
        if (video->cur_picture.full_pel_forward) recon_right_for <<= 1;
        tmp = video->cur_slice.recon_down_for_prev + down_little;
        if (mmin <= tmp && tmp <= mmax) recon_down_for = tmp;
        else recon_down_for = tmp - down_little + down_big;
        video->cur_slice.recon_down_for_prev = recon_down_for;
        if (video->cur_picture.full_pel_forward) recon_down_for <<= 1;
    }
    else if (video->cur_picture.type == 'P') 
    {
        recon_right_for = recon_down_for
            = video->cur_slice.recon_right_for_prev = video->cur_slice.recon_down_for_prev = 0;
        video->cur_mb.motion_forward = true;
    }
    video->cur_mb.recon_right_for = recon_right_for;
    video->cur_mb.recon_down_for = recon_down_for;

    // backward vector
    int recon_right_back = 0, recon_down_back = 0;
    if (video->cur_mb.motion_backward) 
    {
        int backward_h_code, comp_backward_h_r = 0,
            backward_v_code, comp_backward_v_r = 0;
        backward_h_code = decodeVLC(video->mv_VLCtable, video);
        if (video->cur_picture.backward_f != 1 && backward_h_code != 0) 
        {
            tmp = nextbits(video->cur_picture.backward_r_size, video, true); // motion_horizontal_backward_r
            comp_backward_h_r = video->cur_picture.backward_f - 1 - tmp;
        }
        backward_v_code = decodeVLC(video->mv_VLCtable, video);
        if (video->cur_picture.backward_f != 1 && backward_v_code != 0) 
        {
            tmp = nextbits(video->cur_picture.backward_r_size, video, true); // motion_vertical_backward_r
            comp_backward_v_r = video->cur_picture.backward_f - 1 - tmp;
        }
        // start video 2.4.4.2
        right_little = backward_h_code * video->cur_picture.backward_f;
        if (right_little > 0) 
        {
            right_little -= comp_backward_h_r;
            right_big = right_little - 32 * video->cur_picture.backward_f;
        }
        else if (right_little < 0) 
        {
            right_little += comp_backward_h_r;
            right_big = right_little + 32 * video->cur_picture.backward_f;
        }
        else right_big = 0;
        down_little = backward_v_code * video->cur_picture.backward_f;
        if (down_little > 0) 
        {
            down_little -= comp_backward_v_r;
            down_big = down_little - 32 * video->cur_picture.backward_f;
        }
        else if (down_little < 0) 
        {
            down_little += comp_backward_v_r;
            down_big = down_little + 32 * video->cur_picture.backward_f;
        }
        else down_big = 0;
        mmax = (video->cur_picture.backward_f << 4) - 1;
        mmin = -(video->cur_picture.backward_f << 4);
        tmp = video->cur_slice.recon_right_back_prev + right_little;
        if (mmin <= tmp && tmp <= mmax) recon_right_back = tmp;
        else recon_right_back = tmp - right_little + right_big;
        video->cur_slice.recon_right_back_prev = recon_right_back;
        if (video->cur_picture.full_pel_backward) recon_right_back <<= 1;
        tmp = video->cur_slice.recon_down_back_prev + down_little;
        if (mmin <= tmp && tmp <= mmax) recon_down_back = tmp;
        else recon_down_back = tmp - down_little + down_big;
        video->cur_slice.recon_down_back_prev = recon_down_back;
        if (video->cur_picture.full_pel_backward) recon_down_back <<= 1;
    }
    video->cur_mb.recon_right_back = recon_right_back;
    video->cur_mb.recon_down_back = recon_down_back;

    if (!video->cur_mb.intra) 
    {
        buildBlockFromMV(video);
    }

    // coded block pattern
    int pattern = video->cur_mb.intra ? 0x3f : 0;
    if (video->cur_mb.pattern) pattern = decodeVLC(video->cbp_VLCtable, video);
    for (int i = 0; i < 6; i++)
        if (pattern & 1 << (5 - i)) decodeBlock(video, i);
    for (int i = 0; i < 16; i++) 
    {
        for (int j = 0; j < 16; j++) 
        {
            int Y_id = 0;
            if (i >= 8 && j >= 8) Y_id = 3;
            else if (i >= 8) Y_id = 2;
            else if (j >= 8) Y_id = 1;
            int Y = video->cur_mb.block[Y_id][(i % 8) * 8 + j % 8],
                Cb = video->cur_mb.block[4][(i / 2) * 8 + (j / 2)],
                Cr = video->cur_mb.block[5][(i / 2) * 8 + (j / 2)];
            int x = video->height - (base_x + i) - 1,
                y = base_y + j;
            if (video->cur_mb.intra) 
            {
                int R = clip(0, Y + 1.402 * (Cr - 128.0), 255);
                int G = clip(0, Y - 0.34414 * (Cb - 128.0) - 0.71414 * (Cr - 128.0), 255);
                int B = clip(0, Y + 1.772 * (Cb - 128.0), 255);
                video->cur_picture.pixel[x * video->width + y] = {B, G, R};
            }
            else 
            {
                if (!(pattern & 1 << (5 - Y_id))) Y = 0;
                if (!(pattern & 1 << 1)) Cb = 0;
                if (!(pattern & 1 << 0)) Cr = 0;
                int B, G, R = video->cur_picture.pixel[x * video->width + y];
                video->cur_picture.pixel[x * video->width + y] = {clip(B + Y + 1.772 * Cb), 
                												  clip(G + Y - 0.34414 * Cb - 0.71414 * Cr),
                												  clip(R + Y + 1.402 * Cr)};
            }
        }
    }
    video->cur_slice.last_mb_addr = video->cur_mb.addr;
}

void Parse_Slice_Header(video_struct* video)
{
    video->cur_slice.vert_pos = video->next_start_code & 0xff;
    video->cur_slice.q_scale = nextbits(5, video, true);
    video->cur_slice.last_mb_addr = (video->cur_slice.vert_pos - 1) * video->width_mb - 1;
    video->cur_slice.last_intra_addr = -2;
    video->cur_slice.recon_right_for_prev = video->cur_slice.recon_down_for_prev
        = video->cur_slice.recon_right_back_prev = video->cur_slice.recon_down_back_prev = 0;
    while (nextbits(1, video, true)) {
        // extra info ignored
        nextbits(8, video, true);
    }
    while (nextbits(23, video, false) != 0) {
        decodeMacroblock(video);
    }
    align_to_byte(video);
    video->next_start_code = nextbits(32, video, true);
}

void Parse_Picture_Header(video_struct* video)
{
    int temp_ref = nextbits(10, video, true);
    int pic_type = nextbits(3, video, true);
    video->cur_picture.temporal_reference = temp_ref;
    video->cur_picture.picture_type = pic_type;
    video->cur_picture.vbv_delay = nextbits(16, video, true); 
    printf("Picture #%d: %c\n", temp_ref, pic_type); 
     //P type is 2, B type is 3
    if (pic_type == 2 || pic_type == 3) {
        video->cur_picture.full_pel_forward_vector = nextbits(1, video, true);
        video->cur_picture.forward_f_code = nextbits(3, video, true);
        video->cur_picture.forward_r_size = video->cur_picture.forward_f_code - 1;
        video->cur_picture.forward_f_code = 1 << video->cur_picture.forward_r_size;
        //B type
        if (pic_type == 3) {
            video->cur_picture.full_pel_backward = nextbits(1, video, true);
            video->cur_picture.backward_f_code = nextbits(3, video, true);
            video->cur_picture.backward_r_size = video->cur_picture.backward_f_code - 1;
            video->cur_picture.backward_f = 1 << video->cur_picture.backward_r_size;
        }
    }
    //not B type
    if (pic_type != 3) {
        if (video->cur_picture.forward_ref != NULL) free(video->cur_picture.forward_ref);
        video->cur_picture.forward_ref = video->cur_picture.backward_ref;
        video->cur_picture.backward_ref = NULL;
    }
    video->cur_picture.pixel = (Pixel*)malloc(video->width * video->height * sizeof(Pixel));
    align_to_byte(video);
    video->next_start_code = nextbits(32, video, true);
    if(video->next_start_code != picture_start_code) printf("not next_start_code\n");
    while (video->next_start_code >= 0x00000101 && video->next_start_code <= 0x000001af) {
        decodeSlice(video);
    }

    //B type
     if (frame_type == 'B') 
    {
        memcpy(video->frame[video->nframes], video->cur_picture.pixel, video->width * video->height * sizeof(Pixel));
        free(video->cur_picture.pixel);
        video->nframes++;
    }
    //I type or P type
    else 
    {
    	free(video->forward_ref);
        video->forward_ref = video->backward_ref;
        video->backward_ref = video->cur_picture.pixel;
        if (video->forward_ref != NULL) 
        {
            memcpy(video->frame[video->nframes], video->forward_ref, video->width * video->height * sizeof(Pixel));
        }
    }
    video->nframes++;
}

void Parse_GOP_Header(video_struct* video)
{
	int8_t drop_frame_flag = nextbits(1, video, true);
    int8_t hours = nextbits(5, video, true);
    int8_t minutes = nextbits(6, video, true);
    int8_t marker_bit = nextbits(1, video, true);
    int8_t seconds = nextbits(6, video, true);
    int8_t pictures = nextbits(6, video, true);

    int8_t closed_gop = nextbits(1, video, true);
    int8_t broken_link = nextbits(1, video, true);
    printf("GOP timecode-> hours:%d, minutes:%d, seconds:%d, pictures:%d\n", hours, minutes, seconds, picture);
    align_to_byte(video);
    video->next_start_code = nextbits(32, video, true);
    if(video->next_start_code != picture_start_code) printf("not picture_start_code\n");
    while (video->next_start_code == picture_start_code)
    {
        Parse_Picture_Header(video);
    }
}

//parse sequence header
void Parse_Sequence_Header(video_struct* video)
{
	int width_o, height_o;
	width_o = nextbits(12, video, true);
    height_o = nextbits(12, video, true);
    video->width_mb = (width_o - 1) / 16 + 1;
    video->height_mb = (height_o - 1) / 16 + 1;
    video->width = video->width_mb * 16;
    video->height = video->height_mb * 16;
    printf("size: %dx%d (%dx%d)\n", video->width, video->height, width_o, height_o);
	video->pel_aspect_ratio = nextbits(4, video, true);
    video->picture_rate = fps_table[nextbits(4, video, true)];
	video->bit_rate = nextbits(18, video, true);
	video->marker_bit = nextbits(1, video, true);
	video->vbv_buffer_size = nextbits(10, video, true);
    video->constrained_parameters_flag = nextbits(1, video, true);

    if (nextbits(1, video))
    {
        printf("Load intra Q matrix\n");
        for (int i=0; i<64; i++)
        {
        		video->intra_q_matrix[i] = nextbits(8, video, true);
    	}
    }
        
    if (nextbits(1, video, true)) {
        printf("Load non-intra Q matrix\n");
        for (int i=0; i<64; i++)
        {
        		video->non_intra_q_matrix[i] = nextbits(8, video, true);
    	} 
    }
}


//Syntax hierarchy
void Video_Sequence(video_struct* video)
{
	while(nextbits(32, video, true) == sequence_header_code)
	{
		Parse_Sequence_Header(video);
		video->next_start_code = nextbits(32, video, true);
		if(video->next_start_code != group_start_code) printf("not group_start_code\n");
		while(video->next_start_code == group_start_code)
		{
			Parse_GOP_Header(video);
		}
	}
	if(video->next_start_code == seqeunce_end_code)
	{
		printf("Sequence end\n");
	}
}

int main(int argc, char* argv[])
{
	video_truct *video;
	video = (video_struct*)malloc(sizeof(video_struct));
	init_video_struct(video);

	FILE* rfp;
	if(rfp = fopen(argv[1], "rb"))
	{
		fseek(rfp, 0, SEEK_END);
		long int fsize=ftell(rfp);
		fseek(rfp, 0, SEEK_SET);
		video->stream = (uint8_t*)malloc(fsize*sizeof(uint8_t));
		if(fsize != fread(video->stream, sizeof(char), fsize, rfp))
		{
			printf("Error : file %s reading failed\n", argv[1]);
			free(image->data);
			exit(1);
		}
		fclose(rfp);
	}
	else
	{
		printf("Error : file %s opening failed\n", argv[1]);
		exit(1);
	}
	Video_Sequence(video);
	namedWindow("Display window", WINDOW_AUTOSIZE);
	for(int i=0; i < video->nframes;i++)
	{
		imshow("DISPLAY window", video->frame[i]);
		waitKey(1000/video->picture_rate);
	}
	return 0;
}