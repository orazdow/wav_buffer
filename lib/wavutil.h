#ifndef WAVUTIL
#define WAVUTIL

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>

struct WavHeader{
    uint32_t numsamples; // srate*(unsigned long)(1000/lengthms);
     
    uint32_t ChunkID; // "RIFF"
    uint32_t ChunkSize; // 36+a.Subchunk2Size
    uint32_t Format; // "WAVE" 
    uint32_t Subchunk1ID; //"fmt"
    uint32_t Subchunk1Size; // usually 16
    uint16_t AudioFormat; // 1 = pcm
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate; // samplerate*numchannels*bitdepth)/8
    uint16_t BlockAlign; // (numchannels*bitdepth)/8    
    uint16_t BitsPerSample;
    uint32_t Subchunk2ID; // "data"
    uint32_t Subchunk2Size; // (numsamples*numchannels*bitdepth)/8
    
    uint32_t header[11];  
    uint8_t* data;    
};

// assuming machine is little-endian, byteswap text id fields back to big-endian 
uint32_t swap4(uint32_t num);
//
uint16_t swap2(uint16_t num);
// concatenate two words and swap to little-endian 
uint32_t concat_flip(uint16_t a, uint16_t b);

// print functtions
void print32(uint32_t a, const char* fmt);
void printHeader(struct WavHeader wh);

// load wavdata into heavdr struct
int getWav(const char* fpath, struct WavHeader* wh);

// get relevent fields
void getHeaderFields(struct WavHeader* h);

// create wavheader with params
struct WavHeader setParams(int numchannels, int bitdepth, int srate, unsigned int lengthms);

// write callback prototype
typedef void(*writecb)(uint16_t* data, unsigned long num_bytes, unsigned int frame_size);

// 
void writeWav(const char* fstring, writecb cb, int numchannels, int bitdepth, int srate, unsigned int lengthms);

#ifdef __cplusplus
}
#endif

#endif