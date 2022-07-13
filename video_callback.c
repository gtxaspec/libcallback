#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>
#include "platform.h"

// contrast of image effect.
extern int IMP_ISP_Tuning_SetContrast(unsigned char contrast);
extern int IMP_ISP_Tuning_GetContrast(unsigned char *pcontrast);

// brightness of image effect.
extern int IMP_ISP_Tuning_SetBrightness(unsigned char bright);
extern int IMP_ISP_Tuning_GetBrightness(unsigned char *pbright);

// saturation of image effect.
extern int IMP_ISP_Tuning_SetSaturation(unsigned char sat);
extern int IMP_ISP_Tuning_GetSaturation(unsigned char *psat);

// sharpness of image effect.
extern int IMP_ISP_Tuning_SetSharpness(unsigned char sharpness);
extern int IMP_ISP_Tuning_GetSharpness(unsigned char *psharpness);

// AE compensation.
// AE compensation parameters can adjust the target of the image AE.
extern int IMP_ISP_Tuning_SetAeComp(int comp);
extern int IMP_ISP_Tuning_GetAeComp(int *comp);

// AE Max parameters.
extern int IMP_ISP_Tuning_SetAe_IT_MAX(unsigned int it_max);
extern int IMP_ISP_Tuning_GetAE_IT_MAX(unsigned int *it_max);

// DPC Strength.
extern int IMP_ISP_Tuning_SetDPC_Strength(unsigned int ratio);
extern int IMP_ISP_Tuning_GetDPC_Strength(unsigned int *ratio);

// DRC Strength.
extern int IMP_ISP_Tuning_SetDRC_Strength(unsigned int ratio);
extern int IMP_ISP_Tuning_GetDRC_Strength(unsigned int *ratio);

// highlight intensity controls.
extern int IMP_ISP_Tuning_SetHiLightDepress(uint32_t strength);
extern int IMP_ISP_Tuning_GetHiLightDepress(uint32_t *strength);

// Set 3D noise reduction intensity.
extern int IMP_ISP_Tuning_SetTemperStrength(uint32_t ratio);

// Set 2D noise reduction intensity.
extern int IMP_ISP_Tuning_SetSinterStrength(uint32_t ratio);

// Max value of sensor analog gain.
extern int IMP_ISP_Tuning_SetMaxAgain(uint32_t gain);
extern int IMP_ISP_Tuning_GetMaxAgain(uint32_t *gain);

// Max value of sensor Digital gain.
extern int IMP_ISP_Tuning_SetMaxDgain(uint32_t gain);
extern int IMP_ISP_Tuning_GetMaxDgain(uint32_t *gain);

// ISP image mirror(horizontal) effect function (enable/disable)
extern int IMP_ISP_Tuning_SetISPHflip(int mode);
extern int IMP_ISP_Tuning_GetISPHflip(int *pmode);

// ISP image mirror(vertical) effect function (enable/disable)
extern int IMP_ISP_Tuning_SetISPVflip(int mode);
extern int IMP_ISP_Tuning_GetISPVflip(int *pmode);

struct frames_st {
  unsigned char *buf;
  size_t length;
};
typedef int (* framecb)(struct frames_st *);
static int (*real_local_sdk_video_set_encode_frame_callback)(int ch, void *callback);
static int video0_encode_capture(struct frames_st *frames);
static int video1_encode_capture(struct frames_st *frames);
static int video6_encode_capture(struct frames_st *frames);
static int video7_encode_capture(struct frames_st *frames);

struct video_capture_st {
  framecb capture;
  int width;
  int height;
  const char *device;
  unsigned int format;

  framecb callback;
  int enable;
  int initialized;
  int fd;
};

static struct video_capture_st video_capture[] = {
  {
    .capture = video0_encode_capture,
    .width = 1920,
    .height = 1080,
    .device = "/dev/video1",
    .format = V4L2_PIX_FMT_H264,

    .callback = NULL,
    .enable = 0,
    .initialized = 0,
    .fd = -1,
  },
  {
    .capture = video1_encode_capture,
    .width = 640,
    .height = 360,
    .device = "/dev/video2",
    .format = V4L2_PIX_FMT_H264,

    .callback = NULL,
    .enable = 0,
    .initialized = 0,
    .fd = -1,
  },
};

static char videoResBuf[256];

static char *Flip(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    int vflip, hflip;
    IMP_ISP_Tuning_GetISPHflip(&hflip);
    IMP_ISP_Tuning_GetISPVflip(&vflip);
    if(!hflip && !vflip) return "normal";
    if(hflip && !vflip) return "flip";
    if(!hflip && vflip) return "mirror";
    return "flip_mirror";
  }

  if(!strcmp(p, "normal")) {
    IMP_ISP_Tuning_SetISPVflip(0);
    IMP_ISP_Tuning_SetISPHflip(0);
  } else if(!strcmp(p, "flip")) {
    IMP_ISP_Tuning_SetISPVflip(1);
    IMP_ISP_Tuning_SetISPHflip(0);
  } else if(!strcmp(p, "mirror")) {
    IMP_ISP_Tuning_SetISPVflip(0);
    IMP_ISP_Tuning_SetISPHflip(1);
  } else if(!strcmp(p, "flip_mirror")) {
    IMP_ISP_Tuning_SetISPVflip(1);
    IMP_ISP_Tuning_SetISPHflip(1);
  } else {
    return "error";
  }
  return "ok";
}

static char *Contrast(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned char cont;
    IMP_ISP_Tuning_GetContrast(&cont);
    sprintf(videoResBuf, "%d\n", cont);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetContrast(atoi(p));
  return res ? "error": "ok";
}

static char *Brightness(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned char bri;
    IMP_ISP_Tuning_GetBrightness(&bri);
    sprintf(videoResBuf, "%d\n", bri);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetBrightness(atoi(p));
  return res ? "error": "ok";
}

static char *Saturation(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned char sat;
    IMP_ISP_Tuning_GetSaturation(&sat);
    sprintf(videoResBuf, "%d\n", sat);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetSaturation(atoi(p));
  return res ? "error": "ok";
}

static char *Sharpness(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned char sharpness;
    IMP_ISP_Tuning_GetSharpness(&sharpness);
    sprintf(videoResBuf, "%d\n", sharpness);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetSharpness(atoi(p));
  return res ? "error": "ok";
}

static char *AEComp(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    int comp;
    IMP_ISP_Tuning_GetAeComp(&comp);
    sprintf(videoResBuf, "%d\n", comp);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetAeComp(atoi(p));
  return res ? "error": "ok";
}

static char *AEItMax(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int itMax;
    IMP_ISP_Tuning_GetAE_IT_MAX(&itMax);
    sprintf(videoResBuf, "%d\n", itMax);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetAe_IT_MAX(atoi(p));
  return res ? "error": "ok";
}

static char *Sinter(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  int res = IMP_ISP_Tuning_SetSinterStrength(atoi(p));
  return res ? "error": "ok";
}

static char *Temper(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  int res = IMP_ISP_Tuning_SetTemperStrength(atoi(p));
  return res ? "error": "ok";
}

static char *DPC(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int dpc;
    IMP_ISP_Tuning_GetDPC_Strength(&dpc);
    sprintf(videoResBuf, "%d\n", dpc);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetDPC_Strength(atoi(p));
  return res ? "error": "ok";
}

static char *DRC(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int drc;
    IMP_ISP_Tuning_GetDRC_Strength(&drc);
    sprintf(videoResBuf, "%d\n", drc);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetDRC_Strength(atoi(p));
  return res ? "error": "ok";
}

static char *HiLight(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int strength;
    IMP_ISP_Tuning_GetHiLightDepress(&strength);
    sprintf(videoResBuf, "%d\n", strength);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetHiLightDepress(atoi(p));
  return res ? "error": "ok";
}

static char *AGain(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int gain;
    IMP_ISP_Tuning_GetMaxAgain(&gain);
    sprintf(videoResBuf, "%d\n", gain);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetMaxAgain(atoi(p));
  return res ? "error": "ok";
}

static char *DGain(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int gain;
    IMP_ISP_Tuning_GetMaxDgain(&gain);
    sprintf(videoResBuf, "%d\n", gain);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetMaxDgain(atoi(p));
  return res ? "error": "ok";
}

struct CommandTableSt {
  const char *cmd;
  char * (*func)(char *);
};

static struct CommandTableSt VideoCommandTable[] = {
  { "flip",      &Flip }, // flip [normal/flip/mirror/flip_mirror]
  { "cont",      &Contrast }, // cont 0 - 255(center:128)
  { "bri",       &Brightness }, // bri 0 - 255(center:128)
  { "sat",       &Saturation }, // sat 0 - 255(center:128)
  { "sharp",     &Sharpness }, // sharp 0 - 255(center:128)
  { "sinter",    &Sinter }, // sinter 0 - 255(center:128)
  { "temper",    &Temper }, // temper 0 - 255(center:128)
  { "aecomp",    &AEComp }, // aecomp 0 - 255
  { "aeitmax",   &AEItMax }, // aeitmax 0-
  { "dpc",       &DPC }, // dpc 0 - 255
  { "drc",       &DRC }, // drc 0 - 255
  { "hilight",   &HiLight }, // hilight 0 - 10
  { "again",     &AGain }, // again 0 -
  { "dgain",     &DGain }, // dgain 0 -
};

char *VideoCapture(int fd, char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(p) {
    for(int i = 0; i < sizeof(VideoCommandTable) / sizeof(struct CommandTableSt); i++) {
      if(!strcmp(p, VideoCommandTable[i].cmd)) return (*VideoCommandTable[i].func)(tokenPtr);
    }
  }

  int ch = 0;
  if(p && (!strcmp(p, "0") || !strcmp(p, "1"))) {
    ch = atoi(p);
    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  }
  if(!p) return video_capture[ch].enable ? "on" : "off";
  if(!strcmp(p, "on")) {
    video_capture[ch].enable = 1;
    printf("[command] [video_callback.c] [CH%d] video capture on\n", ch);
    return "ok";
  }
  if(!strcmp(p, "off")) {
    video_capture[ch].enable = 0;
    printf("[command] [video_callback.c] [CH%d] video capture off\n", ch);
    return "ok";
  }
  return "error";
}

static int video_encode_capture(int ch, struct frames_st *frames) {

const char *product_T31="/opt/wz_mini/tmp/.T31";
const char *product_T20="/opt/wz_mini/tmp/.T20";
const char *product_DB3="/opt/wz_mini/tmp/.WYZEDB3";

  if(!video_capture[ch].initialized) {
    video_capture[ch].initialized = 1;
    int err;

    //Check for T20 & DOORBELL

    if( access( product_T20, F_OK ) != -1 ) {
	//T20
	if(ch == 0) {
		video_capture[ch].device = "/dev/video6";
        	fprintf(stderr, "[command] [video_callback.c] [CH0] v4l2_device_path = %s\n", video_capture[ch].device);
	} else if(ch == 1) {
		video_capture[ch].device = "/dev/video7";
        	fprintf(stderr, "[command] [video_callback.c] [CH1] v4l2_device_path = %s\n", video_capture[ch].device);
	}
    } else {
	//T31
	if(ch == 0) {
		video_capture[ch].device = "/dev/video1";
		fprintf(stderr, "[command] [video_callback.c] [CH0] v4l2_device_path = %s\n", video_capture[ch].device);

		if( access( product_DB3, F_OK ) == 0 ) {
        	        /* doorbell resolution */
                	printf("[command] [video_callback.c] [CH0] video resolution 1728x1296\n");
	                video_capture[ch].width = 1728;
        	        video_capture[ch].height = 1296;
		} else {
        	        /* v3 and panv2 res */
	                printf("[command] [video_callback.c] [CH0] video resolution 1920x1080\n");
        	        video_capture[ch].width = 1920;
                	video_capture[ch].height = 1080;
		}
	} else if(ch == 1) {
		video_capture[ch].device = "/dev/video2";
        	fprintf(stderr, "[command] [video_callback.c] [CH1] v4l2_device_path = %s\n", video_capture[ch].device);

		if( access( product_DB3, F_OK ) == 0 ) {
	       	        /* doorbell resolution */
                	printf("[command] [video_callback.c] [CH1] video resolution 640x480\n");
	                video_capture[ch].width = 640;
        	        video_capture[ch].height = 480;
		} else {
        	        /* v3 and panv2 res */
	                printf("[command] [video_callback.c] [CH1] video resolution 640x360\n");
        	        video_capture[ch].width = 640;
                	video_capture[ch].height = 360;
		}
	}

    }

    video_capture[ch].fd = open(video_capture[ch].device, O_WRONLY, 0777);
    if(video_capture[ch].fd < 0) fprintf(stderr, "[command] [video_callback.c] Failed to open V4L2 device: %s\n", video_capture[ch].device);
    struct v4l2_format vid_format;
    memset(&vid_format, 0, sizeof(vid_format));
    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vid_format.fmt.pix.width = video_capture[ch].width;
    vid_format.fmt.pix.height = video_capture[ch].height;
    vid_format.fmt.pix.pixelformat = video_capture[ch].format;
    vid_format.fmt.pix.sizeimage = 0;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.bytesperline = 0;
    vid_format.fmt.pix.colorspace = V4L2_PIX_FMT_YUV420;
    err = ioctl(video_capture[ch].fd, VIDIOC_S_FMT, &vid_format);
    if(err < 0) fprintf(stderr, "[command] [video_callback.c] Unable to set V4L2 %s format: %d\n", video_capture[ch].device, err);
    err = ioctl(video_capture[ch].fd, VIDIOC_STREAMON, &vid_format);
    if(err < 0) fprintf(stderr, "[command] [video_callback.c] Unable to perform VIDIOC_STREAMON %s: %d\n", video_capture[ch].device, err);
  }

  if((video_capture[ch].fd >= 0) && video_capture[ch].enable) {
    int size = write(video_capture[ch].fd, frames->buf, frames->length);
    if(size != frames->length) fprintf(stderr,"[command] [video_callback.c] Stream write error %s: %s\n", video_capture[ch].device, size);
  }
  return (video_capture[ch].callback)(frames);
}

static int video0_encode_capture(struct frames_st *frames) {
  return video_encode_capture(0, frames);
}

static int video1_encode_capture(struct frames_st *frames) {
  return video_encode_capture(1, frames);
}

int local_sdk_video_set_encode_frame_callback(int ch, void *callback) {

  fprintf(stderr, "[command] [video_callback.c] local_sdk_video_set_encode_frame_callback streamChId=%d, callback=0x%x\n", ch, callback);

  static int ch_count = 0;

/* two callbacks for video stream 0 are typically detected, unknown what the difference is between them, but if they are both hooked, the app breaks. grab just one of them. */
  //stream 0
  if( (ch == 0) && ch_count == 2) {
    video_capture[ch].callback = callback;
    fprintf(stderr,"[command] [video_callback.c] [CH0] encoder function injection hook video_encode_cb=0x%x\n", video_capture[ch].callback);
    callback = video_capture[ch].capture;
  }
    fprintf(stderr,"[command] [video_callback.c] channel counter is at %x\n", ch_count);

  //stream 1
  if( (ch == 1) && ch_count == 1) {
    video_capture[ch].callback = callback;
    fprintf(stderr,"[command] [video_callback.c] [CH1] encoder function injection hook video_encode_cb=0x%x\n", video_capture[ch].callback);
    callback = video_capture[ch].capture;
  }

    ch_count=ch_count+1;

  return real_local_sdk_video_set_encode_frame_callback(ch, callback);
}


static void __attribute ((constructor)) video_callback_init(void) {

  real_local_sdk_video_set_encode_frame_callback = dlsym(dlopen("/system/lib/liblocalsdk.so", RTLD_LAZY), "local_sdk_video_set_encode_frame_callback");
}

