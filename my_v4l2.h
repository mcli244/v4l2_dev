#ifndef __MY_V4L2_H__
#define __MY_V4L2_H__

#include <linux/videodev2.h>
#include <media/v4l2-device.h>

struct my_v4l2_video_ctx
{
	struct device			*dev;				// 设备指针
	struct v4l2_device		v4l2_dev;			// v4l2设备
	struct video_device		vid_cap_dev;		// 视频设备
	struct v4l2_capability 	cap;				// 表示设备能力
};


#endif /*__MY_V4L2_H__*/