/* Ollie Razdow CS575 - Final Project - Audio Synchronization */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pa.h"
#include "wavutil.h"
#include "ringbuffer.h"
#include "pa_ringbuffer.h"
#include <mutex>
#include <thread>
#include <atomic> 
#include <condition_variable>

// -----------------------
// SET TEST DEFINITONS HERE
// #define LOCKING
// #define LOCK_FREE
// #define EXTRA_WORK
// -----------------------

#ifdef LOCK_FREE
  #define PUSH(buffer, data, size) PaUtil_WriteRingBuffer((buffer), (data), size) 
  #define POP(buffer, data, size) PaUtil_ReadRingBuffer((buffer), (data), size)
  #define POP_AVAIL(buffer) PaUtil_GetRingBufferReadAvailable((buffer))
#else
  #define PUSH(buffer, data, size) rb_push((buffer), (data), size)
  #define POP(buffer, data, size) rb_pop((buffer), (data), size) 
  #define POP_AVAIL(buffer) rb_popAvail((buffer))  
#endif  
#define MIN(a,b) (a)<(b) ? (a):(b)

// playback buffer
#define AUDIO_BUFF_LEN 128
// ring buffer
#define RING_BUFF_LEN 2048
// sample rate
#define SAMPLE_RATE 44100

const float downcvt = (float)1/32768.0;

std::mutex wav_mutex;
std::condition_variable cv;
std::condition_variable ccv;
std::atomic_size_t buffRequest;

typedef struct{
  short* data;
  size_t length;
  size_t index;

}wavbuff;

WavHeader wh;
wavbuff wb;

#ifndef LOCK_FREE
  RingBuffer rb;
#else
  PaUtilRingBuffer rb; 
  void* aBuffData;
#endif

short temp[4096];
int run = 1;
char line[2048];
char filename[2048];

// producer thread
void produce(){
  while(run){

  #ifdef LOCKING
    std::unique_lock<std::mutex> lk(wav_mutex);

    while(buffRequest == 0)  
      cv.wait(lk);
  #endif

    // write data to buffer
    size_t num = PUSH(&rb, wb.data+wb.index, buffRequest);

    // update buffer request
    if(num > 256)
      buffRequest = 0;

  #ifdef LOCKING
    ccv.notify_one();
    lk.unlock(); ;
  #endif  
  }
}

// audio thread
void pafunc(const float* in, float* out, unsigned long frames, void* data){

  #ifdef LOCKING
      std::unique_lock<std::mutex> lk(wav_mutex); 
      while(POP_AVAIL(&rb) < frames*2)
        ccv.wait(lk);
  #endif

        wavbuff* w = (wavbuff*)data;

        // read data from buffer
        size_t num = POP(&rb, temp, frames);

        // write samples
        for(unsigned long i = 0; i < num; i++ ){
          *out++ = (float)temp[i]*downcvt;
          w->index = (w->index+1) % w->length;
        }

        // update buffer request
        if(POP_AVAIL(&rb) < frames*2){
          buffRequest =  MIN(w->length-w->index, frames*16);   
        }else{
          buffRequest = 0;
        }

    #ifdef LOCKING
        cv.notify_one();
        lk.unlock();
    #endif
}

// simulate extra loading time
void randomWork(unsigned long long len){
    while(len--){}
}

// load wav file
int loadWav(const char* path, WavHeader* wh, wavbuff* wb){
    if(getWav(path, wh) < 0)
        return -1;

    printf("loading: %s\n", path);  

    wav_mutex.lock();  
  #ifdef EXTRA_WORK
    randomWork(999999999);
  #endif
    wb->data = (short*)wh->data;
    wb->length = wh->numsamples;
    wb->index = 0;
    wav_mutex.unlock();

    return 0;
}

// load command
int loadcmd(const char* in, char* buff){
  char* ldptr = NULL;
  char* spc = NULL;
  char* ext = NULL;
    
  ldptr = (char*)strstr(in,"load");  
  if(ldptr)
  spc = (char*)strstr(ldptr, " "); 
  if(spc)  
  ext = (char*)strstr(spc,".wav");
  
  if(ext){
    *(ext+4) = 0;
    char* filepath = spc+1;
    strcpy(buff, filepath);
    return 1;
  }
    return 0;
}

// main
int main(int argc, char** argv) {   

    // initial buffer request
    buffRequest = 2048;  
  
  // initialize ring buffer   
  #ifndef LOCK_FREE
    rb_init(&rb, NULL, RING_BUFF_LEN);
  #else
    aBuffData = (short*)malloc(sizeof(short)*RING_BUFF_LEN);
    PaUtil_InitializeRingBuffer(&rb, sizeof(short), RING_BUFF_LEN, aBuffData);
  #endif  

    // load wav file
    if(loadWav("worm.wav", &wh, &wb) < 0)
        return -1;

    // start producer thread
    std::thread t(produce);

    // start audio thread
    Pa a(pafunc, 0, 1, SAMPLE_RATE, AUDIO_BUFF_LEN, &wb);
    a.start();

    // poll for load/quit commands
    while(1){
        fgets(line, 2048, stdin);

        if(line[0] && loadcmd(line, filename)){
            line[0] = 0;
            loadWav(filename, &wh, &wb);
        }

        if(strncmp(line, "quit", 4) == 0)
            break;
    }

    // exit program
    run = 0;
    t.join();

  #ifndef LOCK_FREE
    rb_destroy(&rb);
  #else
    free(aBuffData);
  #endif

    return 0;
}