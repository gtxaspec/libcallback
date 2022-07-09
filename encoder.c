#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include </home/turismo/git_repos/libcallback/sample-common.h>

#define TAG "Sample-Common"


static const IMPEncoderRcMode S_RC_METHOD = IMP_ENC_RC_MODE_CAPPED_QUALITY;

struct chn_conf chn[FS_CHN_NUM] = {
	{
		.index = CH0_INDEX,
		.enable = CHN0_EN,
    .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = CROP_EN,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 0,

			.picWidth = SENSOR_WIDTH,
			.picHeight = SENSOR_HEIGHT,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH0_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH0_INDEX, 0},
	},
	{
		.index = CH1_INDEX,
		.enable = CHN1_EN,
    .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_SECOND,
			.scaler.outheight = SENSOR_HEIGHT_SECOND,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH1_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH1_INDEX, 0},
	},
	{
		.index = CH2_INDEX,
		.enable = CHN2_EN,
    .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_SECOND,
			.scaler.outheight = SENSOR_HEIGHT_SECOND,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH2_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH2_INDEX, 0},
	},
	{
		.index = CH3_INDEX,
		.enable = CHN3_EN,
    .payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_EXT_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_SECOND,
			.scaler.outheight = SENSOR_HEIGHT_SECOND,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH3_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH3_INDEX, 0},
	},
};

int sample_encoder_init(int param_1) {
	printf("LOG: sample_encoder_init run\n");
	fprintf(stderr, "LOG: PARAM_1 is: %d\n",param_1);
	set_video_max_fps(30);
        int i, ret, chnNum = 0;
        IMPFSChnAttr *imp_chn_attr_tmp;
        IMPEncoderChnAttr channel_attr;

    for (i = 0; i <  FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            imp_chn_attr_tmp = &chn[i].fs_chn_attr;
            chnNum = chn[i].index;

            memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
/*
                        float ratio = 1;
                        if (((uint64_t)imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) > (1280 * 720)) {
                                ratio = log10f(((uint64_t)imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720.0)) + 1;
                        } else {
                                ratio = 1.0 / (log10f((1280 * 720.0) / ((uint64_t)imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight)) + 1);
                        }
                        ratio = ratio > 0.1 ? ratio : 0.1;
                        unsigned int uTargetBitRate = BITRATE_720P_Kbs * ratio;
*/
                        unsigned int uTargetBitRate = BITRATE_720P_Kbs;

            ret = IMP_Encoder_SetDefaultParam(&channel_attr, chn[i].payloadType, S_RC_METHOD,
                    imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
                    imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen,
                    imp_chn_attr_tmp->outFrmRateNum * 2 / imp_chn_attr_tmp->outFrmRateDen, 2,
                    (S_RC_METHOD == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
                    uTargetBitRate);
            if (ret < 0) {
                //IMP_LOG_ERR(TAG, "IMP_Encoder_SetDefaultParam(%d) error !\n", chnNum);
                printf("LOG:IMP_Encoder_SetDefaultParam error\n");
		return -1;
            }
#ifdef LOW_BITSTREAM
                        IMPEncoderRcAttr *rcAttr = &channel_attr.rcAttr;
                        uTargetBitRate /= 2;

                        switch (rcAttr->attrRcMode.rcMode) {
                                case IMP_ENC_RC_MODE_FIXQP:
                                        rcAttr->attrRcMode.attrFixQp.iInitialQP = 38;
                                        break;
                                case IMP_ENC_RC_MODE_CBR:
                                        rcAttr->attrRcMode.attrCbr.uTargetBitRate = uTargetBitRate;
                                        rcAttr->attrRcMode.attrCbr.iInitialQP = -1;
                                        rcAttr->attrRcMode.attrCbr.iMinQP = 34;
                                        rcAttr->attrRcMode.attrCbr.iMaxQP = 51;
                                        rcAttr->attrRcMode.attrCbr.iIPDelta = -1;
                                        rcAttr->attrRcMode.attrCbr.iPBDelta = -1;
                                        rcAttr->attrRcMode.attrCbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
                                        rcAttr->attrRcMode.attrCbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
                                        break;
                                case IMP_ENC_RC_MODE_VBR:
                                        rcAttr->attrRcMode.attrVbr.uTargetBitRate = uTargetBitRate;
                                        rcAttr->attrRcMode.attrVbr.uMaxBitRate = uTargetBitRate * 4 / 3;
                                        rcAttr->attrRcMode.attrVbr.iInitialQP = -1;
                                        rcAttr->attrRcMode.attrVbr.iMinQP = 34;
                                        rcAttr->attrRcMode.attrVbr.iMaxQP = 51;
                                        rcAttr->attrRcMode.attrVbr.iIPDelta = -1;
                                        rcAttr->attrRcMode.attrVbr.iPBDelta = -1;
                                        rcAttr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
                                        rcAttr->attrRcMode.attrVbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
                                        break;
                                case IMP_ENC_RC_MODE_CAPPED_VBR:
                                        rcAttr->attrRcMode.attrCappedVbr.uTargetBitRate = uTargetBitRate;
                                        rcAttr->attrRcMode.attrCappedVbr.uMaxBitRate = uTargetBitRate * 4 / 3;
                                        rcAttr->attrRcMode.attrCappedVbr.iInitialQP = -1;
                                        rcAttr->attrRcMode.attrCappedVbr.iMinQP = 34;
                                        rcAttr->attrRcMode.attrCappedVbr.iMaxQP = 51;
                                        rcAttr->attrRcMode.attrCappedVbr.iIPDelta = -1;
                                        rcAttr->attrRcMode.attrCappedVbr.iPBDelta = -1;
                                        rcAttr->attrRcMode.attrCappedVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
                                        rcAttr->attrRcMode.attrCappedVbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
                                        rcAttr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
                                        break;
                                case IMP_ENC_RC_MODE_CAPPED_QUALITY:
                                        rcAttr->attrRcMode.attrCappedQuality.uTargetBitRate = uTargetBitRate;
                                        rcAttr->attrRcMode.attrCappedQuality.uMaxBitRate = uTargetBitRate * 4 / 3;
                                        rcAttr->attrRcMode.attrCappedQuality.iInitialQP = -1;
                                        rcAttr->attrRcMode.attrCappedQuality.iMinQP = 34;
                                        rcAttr->attrRcMode.attrCappedQuality.iMaxQP = 51;
                                        rcAttr->attrRcMode.attrCappedQuality.iIPDelta = -1;
                                        rcAttr->attrRcMode.attrCappedQuality.iPBDelta = -1;
                                        rcAttr->attrRcMode.attrCappedQuality.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
                                        rcAttr->attrRcMode.attrCappedQuality.uMaxPictureSize = uTargetBitRate * 4 / 3;
                                        rcAttr->attrRcMode.attrCappedQuality.uMaxPSNR = 42;
                                        break;
                                case IMP_ENC_RC_MODE_INVALID:
                                        //IMP_LOG_ERR(TAG, "unsupported rcmode:%d, we only support fixqp, cbr vbr and capped vbr\n", rcAttr->attrRcMode.rcMode);
                                        printf("LOG: unsupported rcmode\n");
					return -1;
                        }
#endif

            ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
            if (ret < 0) {
               // IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", chnNum);
		fprintf(stderr, "LOG: IMP_Encoder_CreateChnerror %d\n", chnNum);
                return 0;
            }

                        ret = IMP_Encoder_RegisterChn(chn[i].index, chnNum);
                        if (ret < 0) {
                                //IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", chn[i].index, chnNum, ret);
				printf("IMP_Encoder_RegisterChnerror\n");
                                return -1;
                        }
                }
        }

        return 0;

}

/*
static void __attribute ((constructor)) sample_encoder_init_master(void) {
  real_sample_encoder_init = dlsym(dlopen("/system/lib/liblocalsdk.so", RTLD_LAZY), "sample_encoder_init");
}
*/
