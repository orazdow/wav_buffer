#include "wavutil.h"

int getWav(const char* fpath, struct WavHeader* wh){

    FILE* f;
    f = fopen(fpath, "rb" );  
    if(f == NULL)
        goto READ_ERROR;

    if(fseek (f , 0 , SEEK_END) != 0) 
        goto READ_ERROR;

    if(!(ftell(f) >= 44))
        goto READ_ERROR;

    fseek(f, 0, SEEK_SET);

    fread(&wh->header, 4, 11, f);

    getHeaderFields(wh);

    wh->data = (unsigned char*)malloc(wh->Subchunk2Size);

    fread(wh->data, 1, wh->Subchunk2Size, f);

    fclose(f);

    return 0;

    READ_ERROR:
    fprintf(stderr, "File read error\n");
    return -1;
}

void getHeaderFields(struct WavHeader* h){

    h->SampleRate = h->header[6];
    h->AudioFormat = h->header[5];
    h->NumChannels = (h->header[5] >> 16) & 0xff;
    h->BitsPerSample = (h->header[8] >> 16) & 0xff;
    h->Subchunk2Size = h->header[10];
    h->numsamples = 8 * h->Subchunk2Size / h->BitsPerSample;
}

// set header 
struct WavHeader setParams(int numchannels, int bitdepth, int srate, unsigned int lengthms){
    struct WavHeader a;
    
    a.numsamples = (unsigned long)(srate*(lengthms/1000.0));   
    // set fields
    a.Subchunk2Size = (a.numsamples*numchannels*bitdepth)/8; ; //adjust if dynamic length
    a.Subchunk2ID = swap4(0x64617461); //"data"
    a.BitsPerSample = bitdepth;
    a.BlockAlign = (numchannels*bitdepth)/8;
    a.ByteRate = (srate*numchannels*bitdepth)/8;
    a.SampleRate = srate;
    a.NumChannels = numchannels;
    a.AudioFormat = 1;
    a.Subchunk1Size = 16;
    a.Subchunk1ID = swap4(0x666d7420); //"fmt"
    a.Format = swap4(0x57415645); //"WAVE"
    a.ChunkSize = 36 + a.Subchunk2Size; //adjust if dynamic length
    a.ChunkID = swap4(0x52494646); //"RIFF"
    // create header     
    a.header[0] = a.ChunkID;
    a.header[1] = a.ChunkSize;
    a.header[2] = a.Format; 
    a.header[3] = a.Subchunk1ID;
    a.header[4] = a.Subchunk1Size;
    a.header[5] = concat_flip(a.AudioFormat, a.NumChannels);
    a.header[6] = a.SampleRate;
    a.header[7] = a.ByteRate;
    a.header[8] = concat_flip(a.BlockAlign, a.BitsPerSample);  
    a.header[9] = a.Subchunk2ID;
    a.header[10] = a.Subchunk2Size;
    
    return a;   
}

// write callback prototype
typedef void(*writecb)(uint16_t* data, unsigned long num_bytes, unsigned int frame_size);

void writeWav(const char* fstring, writecb cb, int numchannels, int bitdepth, int srate, unsigned int lengthms){
   
   struct WavHeader a = setParams(numchannels, bitdepth, srate, lengthms);  
   uint16_t* data = (uint16_t*)malloc(a.Subchunk2Size);
   // uint16_t data[a.Subchunk2Size/2]; 
   
   FILE *f;
   f = fopen(fstring, "wb" );
   // write header
   fwrite(a.header, 4 , 22 , f );
   // fill data
   cb(data, a.Subchunk2Size/a.NumChannels, a.BitsPerSample/8);
   // write data
   fwrite(data, 1, a.Subchunk2Size, f);
   fclose(f);
   free(data);
       
}

// assuming machine is little-endian, byteswap text id fields back to big-endian 
uint32_t swap4(uint32_t num){
    return ((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000);
}
//
uint16_t swap2(uint16_t num){
    return (num>>8) | (num<<8);
}
// concatenate two words and swap to little-endian 
uint32_t concat_flip(uint16_t a, uint16_t b){
    return b<<16 | a;
}

void print32(uint32_t a, const char* fmt){
	for (int i = 0; i < 4; ++i)
	{
		printf(fmt, a>>i*8);
	}
	printf("\n");
}

void printHeader(struct WavHeader wh){
  // default params..
	// int start = (idx > -1) ? idx : 0;
	// int end = (idx > -1) ? idx+1 : 11;
    for(int i = 0; i < 11; i++)
      if(i == 0 || i == 2 || i == 3 || i == 9){
          print32(wh.header[i], "%c");
      }else{
          print32(wh.header[i], "%u");
      }
}