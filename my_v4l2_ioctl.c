#include "my_v4l2_ioctl.h"
#include "my_v4l2.h"
#include <media/videobuf2-vmalloc.h>

typedef struct
{
	char description[32];
	uint32_t pixelformat;
	int bytes_per_pixel;
	int width_max;
	int height_max;
}my_v4l2_support_fmt;

my_v4l2_support_fmt my_support_fmt_list[]=
{
	{
		.description = "RGB 8:8:8",
		.pixelformat = V4L2_PIX_FMT_RGB24,
		.bytes_per_pixel = 3,
		.width_max = 640,
		.height_max = 480,
	},
	{
		.description = "RGB 5:6:5",
		.pixelformat = V4L2_PIX_FMT_RGB565,
		.bytes_per_pixel = 2,
		.width_max = 640,
		.height_max = 480,
	},
	{
		.description = "GREY 8-bit",
		.pixelformat = V4L2_PIX_FMT_GREY,
		.bytes_per_pixel = 1,
		.width_max = 416,
		.height_max = 480,
	},
	{
		.description = "GREY 4-bit",
		.pixelformat = V4L2_PIX_FMT_Y4,
		.bytes_per_pixel = 1,
		.width_max = 416,
		.height_max = 480,
	}
};

static struct v4l2_format my_v4l2_cur_fmt = {
	.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
	.fmt.pix.width = 640,
	.fmt.pix.height = 480,
	.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24,
	.fmt.pix.field = V4L2_FIELD_INTERLACED,
	.fmt.pix.bytesperline = 640 * 3,
	.fmt.pix.sizeimage = 640 * 480 * 3,
};

/* 查询当前设备的能力 应用层：VIDIOC_QUERYCAP */
static int my_querycap(struct file *file, void *fh, struct v4l2_capability *cap)
{
	struct my_v4l2_video_ctx *ctx = video_drvdata(file);

	dev_info(ctx->dev, "my_querycap");

	memcpy(cap, &ctx->cap, sizeof(struct v4l2_capability));	

	return 0;
}

/* 列举支持哪种格式 应用层：VIDIOC_ENUM_FMT */
static int my_enum_fmt_vid_cap(struct file *file, void *fh,struct v4l2_fmtdesc *f)
{
	struct my_v4l2_video_ctx *ctx = video_drvdata(file);

	dev_info(ctx->dev, "my_enum_fmt_vid_cap");

	if (f->index >= ARRAY_SIZE(my_support_fmt_list))	
		return -EINVAL;

	strcpy(f->description, my_support_fmt_list[f->index].description);
	f->pixelformat = my_support_fmt_list[f->index].pixelformat;


	return 0;
}

/* 获取当前使用的格式 应用层：VIDIOC_G_FMT */
static int my_g_fmt_vid_cap(struct file *file, void *fh,struct v4l2_format *f)
{
	struct my_v4l2_video_ctx *ctx = video_drvdata(file);
	dev_info(ctx->dev, "my_g_fmt_vid_cap");

	memcpy(f, &my_v4l2_cur_fmt, sizeof(struct v4l2_format));
	return 0;
}


/* 尝试是否支持某种格式 */
static int my_try_fmt_vid_cap(struct file *file, void *fh,struct v4l2_format *f)
{
    enum v4l2_field field;
	int index=0;

	struct my_v4l2_video_ctx *ctx = video_drvdata(file);
	dev_info(ctx->dev, "my_try_fmt_vid_cap");

	for(index=0; index<ARRAY_SIZE(my_support_fmt_list); index++)
	{
		if(f->fmt.pix.pixelformat == my_support_fmt_list[index].pixelformat)
			break;
	}
	if(index >= ARRAY_SIZE(my_support_fmt_list))
		return -EINVAL;
	
	
	field = f->fmt.pix.field;
	if (field == V4L2_FIELD_ANY) {
		field = V4L2_FIELD_INTERLACED;
	} else if (V4L2_FIELD_INTERLACED != field) {
		return -EINVAL;
	}

	v4l_bound_align_image(&f->fmt.pix.width, 48, my_support_fmt_list[index].width_max, 2,	&f->fmt.pix.height, 32, my_support_fmt_list[index].height_max, 0, 0);
	f->fmt.pix.bytesperline =	f->fmt.pix.width * my_support_fmt_list[index].bytes_per_pixel;
	f->fmt.pix.sizeimage 	=	f->fmt.pix.height * f->fmt.pix.bytesperline;

	return 0;
}

/* 获取当前使用的格式 应用层：VIDIOC_S_FMT */
static int my_s_fmt_vid_cap(struct file *file, void *fh,struct v4l2_format *f)
{
	int ret;

	struct my_v4l2_video_ctx *ctx = video_drvdata(file);
	dev_info(ctx->dev, "my_s_fmt_vid_cap");

	ret = my_try_fmt_vid_cap(file, NULL, f);
	if (ret < 0)
		return ret;

    memcpy(&my_v4l2_cur_fmt, f, sizeof(struct v4l2_format));
	
	return 0;
}

struct v4l2_ioctl_ops my_v4l2_ioctl_ops =
{
    .vidioc_querycap          = my_querycap,            // 查询设备能力	VIDIOC_QUERYCAP
	.vidioc_g_fmt_vid_cap     = my_g_fmt_vid_cap,       // 获取当前格式	VIDIOC_G_FMT 
    .vidioc_enum_fmt_vid_cap  = my_enum_fmt_vid_cap,    // 列举支持的格式 VIDIOC_ENUM_FMT 
    .vidioc_try_fmt_vid_cap   = my_try_fmt_vid_cap,     // 尝试设置格式	
    .vidioc_s_fmt_vid_cap     = my_s_fmt_vid_cap,       // 设置格式	VIDIOC_S_FMT 
};
