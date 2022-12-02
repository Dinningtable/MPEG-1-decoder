#include <stdint.h>

typedef struct Pixel{
	uint8_t b, g, r;
}Pixel;


//the video structure
typedef struct video_struct{
	uint8_t* stream;
	uint32_t next_start_code;
	long pos_ind;
	uint64_t bits;
	int b_len;
	uint64_t And_table[33];
	pixel* backward_ref, forward_ref;

	int width, width_mb;
    int height, height_mb;
    double pel_aspect_ratio;
    double picture_rate;
	int bit_rate;
	int marker_bit;
	int vbv_buffer_size;
    int8_t constrained_parameters_flag;

    uint8_t intra_q_matrix[8][8];
    uint8_t non_intra_q_matrix[8][8];

	Sequence_Header cur_sequence;
	GOP_Header cur_gop;
	Picture_Header cur_picture;
	Slice_Header cur_slice;
	Macroblock cur_mb;

	int** macro_addrinc_VLCtable;
    int** macro_I_VLCtable;
    int** macro_P_VLCtable;
    int** macro_B_VLCtable;
    int** cbp_VLCtable;
    int** mv_VLCtable;
    int** dct_dc_luma_VLCtable;
    int** dct_dc_chroma_VLCtable;
    int** dct_coeff_first_VLCtable;
    int** dct_coeff_next_VLCtable;

    Pixel** frame;
    int nframes;
}video_struct;

//Sequence layer
//start with sequence_header_code 0x000001B3
typedef struct Sequece_Header{
    int width, width_mb;
    int height, height_mb;
    double pel_aspect_ratio;
    double picture_rate;
	int bit_rate;
	int marker_bit;
	int vbv_buffer_size;
    int8_t constrained_parameters_flag;

    uint8_t intra_q_matrix[64];
    uint8_t non_intra_q_matrix[64];
}Sequece_Header;

//Group of pictures layer
//start with group_start_code 0x000001B8
typedef struct GOP_Header{
    int8_t drop_frame_flag;
    // Time code
    int8_t hours;
    int8_t minutes;
    int8_t marker_bit;
    int8_t seconds;
    int8_t pictures;

    int8_t closed_gop;
    int8_t broken_link;
}GOP_Header;

//Picture layer
//start with picture_start_code 0x00000100
typedef struct Picture_Header{
	int16_t temp_ref;
	int8_t type;
	int32_t vbv_delay;
	int8_t full_pel_for_vec;
	int8_t forward_f_code;
	int8_t forward_f;
	int8_t forward_r_size;
	int8_t full_pel_back_vec;
    int8_t backward_f_code;
    int8_t backward_f;
    int8_t backward_r_size;
    Pixel* pixel;
}Picture_Header;

//Slice layer
//start with slice_start_code 0x00000101
typedef struct Slice_Header{
    int vert_pos;
    int q_scale;
    int last_mb_addr;
    int last_intra_addr;
    int dc_predictor[3]; // Y, Cb, Cr
    int recon_right_for_prev, recon_down_for_prev;
    int recon_right_back_prev, recon_down_back_prev;
}Slice_Header;

//Macroblock layer
typedef struct Macroblock{
    int8_t escape;
    int addr_incre;
    int type;
    int quant;
    int motion_forward;
    int motion_backward;
    int intra;
    int q_scale;
    int block[6][64];
    int recon_right_for, recon_down_for;
    int recon_right_back, recon_down_back;
}Macroblock;