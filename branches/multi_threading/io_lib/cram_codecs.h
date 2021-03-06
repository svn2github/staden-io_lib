/*
 * Author: James Bonfield, Wellcome Trust Sanger Institute. 2013
 */

#ifndef _CRAM_ENCODINGS_H_
#define _CRAM_ENCODINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

struct cram_codec;

/*
 * Slow but simple huffman decoder to start with.
 * Read a bit at a time, keeping track of {length, value}
 * eg. 1 1 0 1 => {1,1},  {2,3}, {3,6}, {4,13}
 *
 * Keep track of this through the huffman code table.
 * For fast scanning we have an index of where the first code of length X
 * appears.
 */
typedef struct {
    int32_t symbol;
    int32_t p; // next code start value, minus index to codes[]
    int32_t code;
    int32_t len;
} cram_huffman_code;

typedef struct {
    cram_huffman_code *codes;
} cram_huffman_decoder;

#define MAX_HUFF 128
typedef struct {
    cram_huffman_code *codes;
    int nvals;
    int val2code[MAX_HUFF+1]; // value to code lookup for small values
} cram_huffman_encoder;

typedef struct {
    int32_t offset;
    int32_t nbits;
} cram_beta_decoder;

typedef struct {
    int32_t offset;
} cram_gamma_decoder;

typedef struct {
    int32_t offset;
    int32_t k;
} cram_subexp_decoder;

typedef struct {
    int32_t content_id;
    enum cram_external_type type;
} cram_external_decoder;

typedef struct {
    struct cram_codec *len_codec;
    struct cram_codec *value_codec;
} cram_byte_array_len_decoder;

typedef struct {
    unsigned char stop;
    int32_t content_id;
} cram_byte_array_stop_decoder;

typedef struct {
    uint32_t len_len;
    unsigned char *len_dat;
    uint32_t val_len;
    unsigned char *val_dat;
} cram_byte_array_len_encoder;

/*
 * A generic codec structure.
 */
typedef struct cram_codec {
    enum cram_encoding codec;
    void (*free)(struct cram_codec *codec);
    int (*decode)(cram_slice *slice, struct cram_codec *codec,
		  cram_block *in, char *out, int *out_size);
    int (*encode)(cram_slice *slice, struct cram_codec *codec,
		  cram_block *out, char *in, int in_size);
    int (*store)(struct cram_codec *codec, cram_block *b, char *prefix,
		 int version);
    union {
	cram_huffman_decoder         huffman;
	cram_external_decoder        external;
	cram_beta_decoder            beta;
	cram_gamma_decoder           gamma;
	cram_subexp_decoder          subexp;
	cram_byte_array_len_decoder  byte_array_len;
	cram_byte_array_stop_decoder byte_array_stop;

	cram_huffman_encoder         e_huffman;
	cram_external_decoder        e_external;
	cram_byte_array_stop_decoder e_byte_array_stop;
	cram_byte_array_len_encoder  e_byte_array_len;
	cram_beta_decoder            e_beta;
    };
} cram_codec;

char *cram_encoding2str(enum cram_encoding t);

cram_codec *cram_decoder_init(enum cram_encoding codec, char *data, int size,
			      enum cram_external_type option,
			      int version);
cram_codec *cram_encoder_init(enum cram_encoding codec, cram_stats *st,
			      enum cram_external_type option, void *dat,
			      int version);

//int cram_decode(void *codes, char *in, int in_size, char *out, int *out_size);
//void cram_decoder_free(void *codes);

//#define GET_BIT_MSB(b,v) (void)(v<<=1, v|=(b->data[b->byte] >> b->bit)&1, (--b->bit == -1) && (b->bit = 7, b->byte++))

#define GET_BIT_MSB(b,v) (void)(v<<=1, v|=(b->data[b->byte] >> b->bit)&1, b->byte += (b->bit==0), b->bit+=(b->bit==0)*8-1)

#ifdef __cplusplus
}
#endif

#endif /* _CRAM_ENCODINGS_H_ */
