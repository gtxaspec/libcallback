#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <tinyalsa/pcm.h>
#include "platform.h"

static const int AudioDecviceID = 1;
static const int AudioChID = 0;

typedef struct {
  int samplerate; // Audio sampling rate. 8000 Hz
  int bitwidth; // Audio sampling precision. 16 bit
  int soundmode; // Audio channel mode. 1(mono)
  int frmNum;  // Number of cached frames, range: [2, MAX_AUDIO_FRAME_NUM].
  int numPerFrm; // Number of sample points per frame.
  int chnCnt;  // Number of channels supported.
} IMPAudioIOAttr;

// attribute of the audio input device.
extern int IMP_AI_SetPubAttr(int audioDevId, IMPAudioIOAttr *attr);
extern int IMP_AI_GetPubAttr(int audioDevId, IMPAudioIOAttr *attr);

// AI high pass filtering function.
extern int IMP_AI_DisableHpf();
extern int IMP_AI_EnableHpf(IMPAudioIOAttr *attr);

// AI automatic gain feature.
extern int IMP_AI_DisableAgc();
extern int IMP_AI_EnableAgc(IMPAudioIOAttr *attr, short TargetLevelDbfs, short CompressionGaindB);

// Noise suppression.
extern int IMP_AI_DisableNs();
extern int IMP_AI_EnableNs(IMPAudioIOAttr *attr, int mode);

// Enable audio echo cancellation feature of the specified audio input and audio output.
extern int IMP_AI_DisableAec(int aiDevId, int aiCh);
extern int IMP_AI_EnableAec(int aiDevId, int aiChn, int aoDevId, int aoChn);

// audio input volume. -30 - 120, default: 60
extern int IMP_AI_SetVol(int audioDevId, int aiChn, int aiVol);
extern int IMP_AI_GetVol(int audioDevId, int aiChn, int *vol);

// audio input gain. 0 - 31
extern int IMP_AI_SetGain(int audioDevId, int aiChn, int aiGain);
extern int IMP_AI_GetGain(int audioDevId, int aiChn, int *aiGain);

//alc gain value. 0 - 7
extern int IMP_AI_SetAlcGain(int audioDevId, int aiChn, int aiPgaGain);
extern int IMP_AI_GetAlcGain(int audioDevId, int aiChn, int *aiPgaGain);

struct frames_st {
  void *buf;
  size_t length;
};
typedef int (* framecb)(struct frames_st *);

static uint32_t (*real_local_sdk_audio_set_pcm_frame_callback)(int ch, void *callback);
//static void *audio_pcm_cb = NULL;

static int audio0_encode_capture(struct frames_st *frames);
static int audio1_encode_capture(struct frames_st *frames);

struct audio_capture_st {
  framecb capture;
  int card;
  struct pcm *pcm;

  framecb callback;
  int enable;
  int initialized;
};
struct audio_capture_st audio_capture[] = {
  {
    .capture = audio0_encode_capture,
    .card = 0,
    .pcm = NULL,

    .callback = NULL,
    .enable = 0,
    .initialized = 0,
  },
  {
    .capture = audio1_encode_capture,
    .card = 2,
    .pcm = NULL,

    .callback = NULL,
    .enable = 0,
    .initialized = 0,
  },
};

static char audioResBuf[256];

static char *HighPassFilter(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  int ret = -1;
  if(p && !strcmp(p, "off")) {
    ret = IMP_AI_DisableHpf();
  }
  if(p && !strcmp(p, "on")) {
    IMPAudioIOAttr attr;
    ret = IMP_AI_GetPubAttr(AudioDecviceID, &attr);
    if(!ret) ret = IMP_AI_EnableHpf(&attr);
  }
  return ret ? "error" : "ok";
}

static char *AutoGainControl(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";

  int ret = -1;
  if(!strcmp(p, "off")) {
     ret = IMP_AI_DisableAgc(); // Exception
  } else {
    int targetLevelDbfs = atoi(p);
    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
    if(p) {
      int compressionGaindB = atoi(p);
      IMPAudioIOAttr attr;
      ret = IMP_AI_GetPubAttr(AudioDecviceID, &attr);
      if(!ret) ret = IMP_AI_EnableAgc(&attr, targetLevelDbfs, compressionGaindB);
    }
  }
  return ret ? "error" : "ok";
}

static char *NoiseSuppression(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";

  int ret = -1;
  if(!strcmp(p, "off")) {
    ret = IMP_AI_DisableNs();
  } else {
    int level = atoi(p);
    IMPAudioIOAttr attr;
    ret = IMP_AI_GetPubAttr(AudioDecviceID, &attr);
    if(!ret) ret = IMP_AI_EnableNs(&attr, level);
  }
  return ret ? "error" : "ok";
}

static char *EchoCancellation(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  int ret = -1;
  if(p && !strcmp(p, "off")) {
    ret = IMP_AI_DisableAec(AudioDecviceID, AudioChID);
  }
  if(p && !strcmp(p, "on")) {
    ret = IMP_AI_EnableAec(AudioDecviceID, AudioChID, AudioDecviceID, AudioChID);
  }
  return ret ? "error" : "ok";
}

static char *Volume(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    int vol;
    int ret = IMP_AI_GetVol(AudioDecviceID, AudioChID, &vol);
    if(ret) return "error";
    sprintf(audioResBuf, "%d\n", vol);
    return audioResBuf;
  }
  int ret = IMP_AI_SetVol(AudioDecviceID, AudioChID, atoi(p));
  return ret ? "error" : "ok";
}

static char *Gain(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    int gain;
    int ret = IMP_AI_GetGain(AudioDecviceID, AudioChID, &gain);
    if(ret) return "error";
    sprintf(audioResBuf, "%d\n", gain);
    return audioResBuf;
  }
  int ret = IMP_AI_SetGain(AudioDecviceID, AudioChID, atoi(p));
  return ret ? "error" : "ok";
}

static char *AlcGain(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    int gain;
    int ret = IMP_AI_GetAlcGain(AudioDecviceID, AudioChID, &gain);
    if(ret) return "error";
    sprintf(audioResBuf, "%d\n", gain);
    return audioResBuf;
  }
  int ret = IMP_AI_SetAlcGain(AudioDecviceID, AudioChID, atoi(p));
  return ret ? "error" : "ok";
}

struct CommandTableSt {
  const char *cmd;
  char * (*func)(char *);
};

static struct CommandTableSt AudioCommandTable[] = {
  { "hpf",      &HighPassFilter }, // hpf on/off
  { "agc",      &AutoGainControl }, // agc gainLevel:0-31(dB) maxGain:0-90(dB)
  { "ns",       &NoiseSuppression }, // ns off/0-3
  { "aec",      &EchoCancellation }, // aec on/off
  { "vol",      &Volume }, // vol -30 - 120
  { "gain",     &Gain }, // gain 0 - 31
  { "alc",      &AlcGain }, // alc 0-7
};

char *AudioCapture(int fd, char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(p) {
    for(int i = 0; i < sizeof(AudioCommandTable) / sizeof(struct CommandTableSt); i++) {
      if(!strcmp(p, AudioCommandTable[i].cmd)) return (*AudioCommandTable[i].func)(tokenPtr);
    }
  }

  int ch = 0;
  if(p && (!strcmp(p, "0") || !strcmp(p, "1"))) {
    ch = atoi(p);
    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  }
  if(!p) {
    if(!audio_capture[ch].pcm) return "disabled";
    return audio_capture[ch].enable ? "on" : "off";
  }
  if(!strcmp(p, "on")) {
    audio_capture[ch].enable = 1;
    if(audio_capture[ch].pcm) pcm_start(audio_capture[ch].pcm);
    printf("[command] [audio_callback.c] [CH%d] audio capture on\n", ch);
    return "ok";
  }
  if(!strcmp(p, "off")) {
    audio_capture[ch].enable = 0;
    if(audio_capture[ch].pcm) pcm_stop(audio_capture[ch].pcm);
    printf("[command] [audio_callback.c] [CH%d] audio capture off\n", ch);
    return "ok";
  }
  return "error";
}
////////

static uint32_t audio_encode_capture(int ch, struct frames_st *frames) {

  static int snd_rate = 16000;

  struct pcm_config config = {
    .channels = 1,
    .rate = snd_rate,
    .format = PCM_FORMAT_S16_LE,
    .period_size = 320,
    .period_count = 16,
    .start_threshold = 0,
    .silence_threshold = 0,
    .silence_size = 0,
    .stop_threshold = 0,
  };

void audio_proc(int ch) {

const char *product_T20="/opt/wz_mini/tmp/.T20";

    if(!audio_capture[ch].pcm) {
     if( access( product_T20, F_OK ) == 0 ) {
         config.rate = 8000;
	//put this in a function and call it instead of this
     }
      if( ch == 1 ) {
         config.rate = 8000;
      }
         audio_capture[ch].pcm = pcm_open(audio_capture[ch].card, 1, PCM_OUT | PCM_MMAP, &config);
      if(audio_capture[ch].pcm == NULL) {
          fprintf(stderr, "[command] [audio_callback.c] failed to allocate memory for PCM%d\n", ch);
      } else if(!pcm_is_ready(audio_capture[ch].pcm)) {
        fprintf(stderr, "[command] [audio_callback.c] failed to open PCM%d : %s\n", ch, pcm_get_error(audio_capture[ch].pcm));
        pcm_close(audio_capture[ch].pcm);
        audio_capture[ch].pcm = NULL;
      }
    }

    if(audio_capture[ch].pcm && audio_capture[ch].enable) {
      if(pcm_mmap_avail(audio_capture[ch].pcm) >= config.period_size * config.period_count / 4) {
        int err = pcm_writei(audio_capture[ch].pcm, frames->buf, pcm_bytes_to_frames(audio_capture[ch].pcm, frames->length));
        if(err < 0) fprintf(stderr, "[command] [audio_callback.c] [command] [audio_callback.c] pcm_writei ch%d err=%d\n", ch, err);
      } else {
        fprintf(stderr, "[command] [audio_callback.c] audio buffer full, dropping packet: ch%d %d\n", ch, frames->length);
        pcm_prepare(audio_capture[ch].pcm);
      }
    }
}

    if( access( product_T20, F_OK ) == 0 ) {
       for(int ch = 0; ch < 2; ch++) {
        audio_proc(ch);
       }
    } else {
       audio_proc(ch);
    }

  return ((framecb)audio_capture[ch].callback)(frames);
}

static int audio0_encode_capture(struct frames_st *frames) {
  return audio_encode_capture(0, frames);
}

static int audio1_encode_capture(struct frames_st *frames) {
  return audio_encode_capture(1, frames);
}

uint32_t local_sdk_audio_set_pcm_frame_callback(int ch, void *callback) {

  fprintf(stderr, "[command] [audio_callback.c] local_sdk_audio_set_pcm_frame_callback streamChId=%d, callback=0x%x\n", ch, callback);

  static int ch_count = 0;

  if( (ch == 0) && ch_count == 0) {
    audio_capture[ch].callback = callback;
    fprintf(stderr, "[command] [audio_callback.c] [CH0] encoder function injection hook audio_encode_cb=0x%x\n", audio_capture[ch].callback);
    callback = audio_capture[ch].capture;
}
    fprintf(stderr,"[command] [audio_callback.c] channel counter is at %x\n", ch_count);

  if( (ch == 1) && ch_count == 1) {
    audio_capture[ch].callback = callback;
    fprintf(stderr, "[command] [audio_callback.c] [CH1] encoder function injection hook audio_encode_cb=0x%x\n", audio_capture[ch].callback);
    callback = audio_capture[ch].capture;
}

//T20 only, we have to latch on to the same callback as CH0, since the T20's only have one audio callback
 if( access( product_T20, F_OK ) == 0 ) {
  if( (ch == 0) && ch_count == 1) {
    audio_capture[1].callback = callback;
    fprintf(stderr, "[command] [audio_callback.c] [CH0-1] encoder function injection second callback for V2 hook audio_pcm_cb=0x%x\n", audio_capture[1].callback);
    callback = audio_capture[1].callback;
    ch =1;
  }
}
  ch_count=ch_count+1;

  return real_local_sdk_audio_set_pcm_frame_callback(ch, callback);
}

static void __attribute ((constructor)) audio_callback_init(void) {

  real_local_sdk_audio_set_pcm_frame_callback = dlsym(dlopen("/system/lib/liblocalsdk.so", RTLD_LAZY), "local_sdk_audio_set_pcm_frame_callback");
}
