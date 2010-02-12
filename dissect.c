/**
	@file	dissect.c
	@brief	packet dissection routine
	@author	Wes Fournier

	|
*/

#include "fast.h"

#define STOP_BIT 0x80

/** Decodes a stop-bit encoded block of data into a regular string of bits

	This is, obviously, a pretty inefficient way to solve this, and we
	should come up with a better way, but this will suffice for testing

	@param in input data
	@param max_in_bytes length of input data
	@param out output buffer
	@param max_out_bytes maximum size of output buffer
	@return number of bytes used in output buffer, or -1 if the
		input data was malformed, or -2 if the output buffer was too small
*/
int decode_stop_bits(
	const char* in,
	int max_in_bytes,
	char* out,
	int max_out_bytes)
{
	int i = 0;

	// first find the number of bytes in data
	int nbytes=0;
	for(i=0;i<max_in_bytes;i++)
	{
		if(in[i] & STOP_BIT)
		{
			nbytes=i+1;
			break;
		}
	}
	if(!nbytes)
	{
		// missing stop bit
		return -1;
	}

	// compute needed actual storage space
	int nbits=nbytes*7;
	int outbytes = nbits/8 + (nbits%8>0 ? 1 : 0);
	if(max_out_bytes<outbytes)
	{
		// not enough output space
		return -2;
	}

	int b=0;

	// put all our output bits into a temporary string,
	// skipping the stop bits
	char* bitstr = g_malloc(nbits);

	for(i=0;i<nbytes && b<nbits;i++)
	{
		bitstr[b] = in[i]&0x40;
		bitstr[b+1]=in[i]&0x20;
		bitstr[b+2]=in[i]&0x10;
		bitstr[b+3]=in[i]&0x08;
		bitstr[b+4]=in[i]&0x04;
		bitstr[b+5]=in[i]&0x02;
		bitstr[b+6]=in[i]&0x01;
		b+=7;
	}

	// copy the bit string back out
	for(i=0,b=0;i<outbytes && b<nbits;i++)
	{
		out[i] =
			(bitstr[ b ]<<7) | (bitstr[b+1]<<6) | (bitstr[b+2]<<5) |
			(bitstr[b+3]<<4) | (bitstr[b+4]<<3) | (bitstr[b+5]<<2) |
			(bitstr[b+6]<<1) | (bitstr[b+7]);
		b+=8;
	}

	g_free(bitstr);

	return outbytes;

	/* This was my first attempt at a decoding algorithm; eventually
		the decoding should be done a bit-level but for now we can just
		use a less efficient bit-string solution
	*//*
	// transfer the bits starting from the end, leaving off the stop bit
	for(i=nbytes-1;i>=0;i--)
	{
		out[i]=0;

		// copy over the current byte
		out[i]= in[i] >> (nbytes-i+1);

		// copy over the part of the preceeding byte we need
		if(i>0) out[i] |= in[i-1] << (8-(nbytes-i));
	}

	return nbytes;*/
}

void FAST_dissect(int id, tvbuff_t* tvb, int n, packet_info* pinfo,
	proto_tree* tree)
{
	/**/
}
