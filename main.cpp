#include <stdio.h>
#include "wavutil.h"
#include <math.h>
#include "pa.h"
#include "string.h"
#include "pa_ringbuffer.h"
#include "ringbuffer.h"
#include <mutex>
#include <thread>
#include <atomic> 
#include <condition_variable>

#define LOCKING
// #define LOCK_FREE
#define EXTRA_WORK

#define AUDIO_BUFF_LEN 128
#define RING_BUFF_LEN 2048
#define MIN(a,b) (a)<(b) ? (a):(b)

const double PI = acos(-1);
const double srate = 44100;
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

RingBuffer rb;
WavHeader wh;
wavbuff wb;

PaUtilRingBuffer aBuffer; 
void* aBuffData;
short temp[4096];
int run = 1;

void produce(){
  while(run){
    size_t num = 0;

  #ifdef LOCKING
    std::unique_lock<std::mutex> lk(wav_mutex);

    while(buffRequest == 0){  
      cv.wait(lk);
    }
  #endif

  #ifdef LOCK_FREE  
    num = PaUtil_WriteRingBuffer(&aBuffer, wb.data+wb.index, buffRequest);
  #else
    num = rb_push(&rb, wb.data+wb.index, buffRequest);
  #endif

    if(num > 256)
    buffRequest = 0;

  #ifdef LOCKING
    ccv.notify_one();
    lk.unlock(); ;
  #endif  
  }
}

void pafunc(const float* in, float* out, unsigned long frames, void* data){

  #ifdef LOCKING
       std::unique_lock<std::mutex> lk(wav_mutex); 
  #endif

       wavbuff* w = (wavbuff*)data;
       size_t num = 0;

  #ifdef LOCKING
      #ifndef LOCK_FREE
         while(rb_popAvail(&rb) < frames*2){
      #else
         while(PaUtil_GetRingBufferReadAvailable(&aBuffer) < frames*2){
      #endif
        ccv.wait(lk);
      } 
  #endif

      #ifndef LOCK_FREE
        num = rb_pop(&rb, temp, frames);
      #else
        num = PaUtil_ReadRingBuffer(&aBuffer, temp, frames);
      #endif

          for(unsigned long i = 0; i < num; i++ ){
            *out++ = (float)temp[i]*downcvt;
            w->index = (w->index+1) % w->length;
         }

      #ifndef LOCK_FREE
        if(rb_popAvail(&rb) < frames*2){
      #else
        if(PaUtil_GetRingBufferReadAvailable(&aBuffer) < frames*2){  
      #endif

          buffRequest =  MIN(w->length-w->index, frames*16);   
        }else{
          buffRequest = 0;
        }

    #ifdef LOCKING
        cv.notify_one();
        lk.unlock();
    #endif
}

void randomWork(unsigned long long len){
    while(len--){}
}

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

char line[2048];
char filename[2048];

int main(int argc, char** argv) {   

    buffRequest = 2048;  
     
  #ifndef LOCK_FREE
    rb_init(&rb, NULL, RING_BUFF_LEN);
  #else
    aBuffData = (short*)malloc(sizeof(short)*RING_BUFF_LEN);
    PaUtil_InitializeRingBuffer(&aBuffer, sizeof(short), RING_BUFF_LEN, aBuffData);
  #endif  

    if(loadWav("worm.wav", &wh, &wb) < 0)
        return -1;

    std::thread t(produce);


    Pa a(pafunc, 0, 1, srate, AUDIO_BUFF_LEN, &wb);
    a.start();

    while(1){
        fgets(line, 2048, stdin);

        if(line[0] && loadcmd(line, filename)){
            line[0] = 0;
            loadWav(filename, &wh, &wb);
        }

        if(strncmp(line, "quit", 4) == 0)
            break;

    }

    run = 0;
    t.join();

  #ifndef LOCK_FREE
    rb_destroy(&rb);
  #else
    free(aBuffData);
  #endif
    return 0;
}