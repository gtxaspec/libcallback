/*
 * sample-common.h
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

#include </home/turismo/sdk/ISVP-T31-1.1.2-20201021/software/sdk/Ingenic-SDK-T31-1.1.2-20201021/sdk/4.7.2/include/imp/imp_common.h>
#include </home/turismo/sdk/ISVP-T31-1.1.2-20201021/software/sdk/Ingenic-SDK-T31-1.1.2-20201021/sdk/4.7.2/include/imp/imp_osd.h>
#include </home/turismo/sdk/ISVP-T31-1.1.2-20201021/software/sdk/Ingenic-SDK-T31-1.1.2-20201021/sdk/4.7.2/include/imp/imp_framesource.h>
#include </home/turismo/sdk/ISVP-T31-1.1.2-20201021/software/sdk/Ingenic-SDK-T31-1.1.2-20201021/sdk/4.7.2/include/imp/imp_isp.h>
#include </home/turismo/sdk/ISVP-T31-1.1.2-20201021/software/sdk/Ingenic-SDK-T31-1.1.2-20201021/sdk/4.7.2/include/imp/imp_encoder.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#define SENSOR_FRAME_RATE_NUM		30
#define SENSOR_FRAME_RATE_DEN		1

#define SENSOR_GC2053

#define SENSOR_NAME				"gc2053"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x37
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 1
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN			0


#define SENSOR_WIDTH_SECOND		640
#define SENSOR_HEIGHT_SECOND	360

#define SENSOR_WIDTH_THIRD		1280
#define SENSOR_HEIGHT_THIRD		720

#define BITRATE_720P_Kbs        1000

#define NR_FRAMES_TO_SAVE		200
#define STREAM_BUFFER_SIZE		(1 * 1024 * 1024)

#define ENC_VIDEO_CHANNEL		0
#define ENC_JPEG_CHANNEL		1

#define STREAM_FILE_PATH_PREFIX		"/tmp"
#define SNAP_FILE_PATH_PREFIX		"/tmp"

#define OSD_REGION_WIDTH		16
#define OSD_REGION_HEIGHT		34
#define OSD_REGION_WIDTH_SEC		8
#define OSD_REGION_HEIGHT_SEC   	18


#define SLEEP_TIME			1

#define FS_CHN_NUM			4  //MIN 1,MAX 3
#define IVS_CHN_ID          3

#define CH0_INDEX  0
#define CH1_INDEX  1
#define CH2_INDEX  2
#define CH3_INDEX  3
#define CHN_ENABLE 1
#define CHN_DISABLE 0

/*#define SUPPORT_RGB555LE*/

struct chn_conf{
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
  IMPEncoderProfile payloadType;
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
};

#define  CHN_NUM  ARRAY_SIZE(chn)

int sample_system_init();
int sample_system_exit();

int sample_framesource_streamon();
int sample_framesource_streamoff();

int sample_framesource_ext_hsv_streamon();
int sample_framesource_ext_hsv_streamoff();

int sample_framesource_ext_rbga_streamon();
int sample_framesource_ext_rbga_streamoff();

int sample_framesource_init();
int sample_framesource_exit();

int sample_framesource_ext_hsv_init();
int sample_framesource_ext_hsv_exit();

int sample_framesource_ext_rbga_init();
int sample_framesource_ext_rbga_exit();

int sample_encoder_init();
int sample_jpeg_init();
int sample_encoder_exit(void);
int sample_jpeg_exit();
IMPRgnHandle *sample_osd_init(int grpNum);
int sample_osd_exit(IMPRgnHandle *prHandle,int grpNum);

int sample_get_frame();
int sample_get_video_stream();
int sample_get_video_stream_byfd();
int sample_get_jpeg_snap();

int sample_SetIRCUT(int enable);
void *sample_soft_photosensitive_ctrl(void *p);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_COMMON_H__ */
