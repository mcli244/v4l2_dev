#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/version.h>

#include "my_v4l2.h"
#include "my_v4l2_ioctl.h"

#define VID_MODULE_NAME "my_v4l2_vid"


static struct my_v4l2_video_ctx my_v4l2_ctx;


static void my_v4l2_video_device_release(struct v4l2_device *v4l2_dev)
{

}

static int my_v4l2_open(struct file *file) {
    pr_info("my_v4l2: Device opened\n");
    return 0;
}

static int my_v4l2_release(struct file *file) {
    pr_info("my_v4l2: Device closed\n");
    return 0;
}

static long my_v4l2_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    pr_info("my_v4l2: IOCTL command received: 0x%x\n", cmd);
    return 0;
}

static const struct v4l2_file_operations my_v4l2_fops = {
    .owner = THIS_MODULE,
    .open = my_v4l2_open,
    .release = my_v4l2_release,
    // ioctl -> unlocked_ioctl -> video_ioctl2 -> my_v4l2_ioctl_ops(函数集) 相当于通过video_ioctl2中转了一次
	.unlocked_ioctl = video_ioctl2,	
    // .unlocked_ioctl = my_v4l2_ioctl,			
};

void _my_v4l2_cap_init(struct v4l2_capability *cap)
{
    if(cap == NULL)
    {
        pr_err("cap is NULL\n");
        return;
    }
    // capabilities信息
	strcpy(cap->driver, "my_v4l2_driver"); // 驱动名称
	strcpy(cap->card, "my_v4l2_device");   // 设备名称
	cap->version = KERNEL_VERSION(0, 0, 1);             // 版本号
	cap->capabilities =	V4L2_CAP_VIDEO_CAPTURE |        // 说明这是一个“视频采集设备”，这里是摄像头；
                        V4L2_CAP_STREAMING |            // 说明这个设备支持“流式传输”；
                        V4L2_CAP_DEVICE_CAPS;           // 说明这个设备支持“设备能力”查询；
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING; // 实际开放给应用层的能力，是上面能力的子集
}


static int my_v4l2_video_pdrv_probe(struct platform_device *pdev)
{
    int erron;
    int ret;
    struct video_device *vfd;

    dev_info(&pdev->dev, "my_v4l2_video_pdrv_probe\n");

	memset(&my_v4l2_ctx, 0, sizeof(my_v4l2_ctx));
	my_v4l2_ctx.dev = &pdev->dev;

    /* register v4l2_device */
    snprintf(my_v4l2_ctx.v4l2_dev.name, sizeof(my_v4l2_ctx.v4l2_dev.name), "%s-%03d", VID_MODULE_NAME, 0);
	ret = v4l2_device_register(&pdev->dev, &my_v4l2_ctx.v4l2_dev);
	if (ret < 0) {
        dev_err(&pdev->dev, "v4l2_device_register failed ret:%d ", ret);
		return -ENOMEM;
	}

    _my_v4l2_cap_init(&my_v4l2_ctx.cap);

    // 视频设备操作
    vfd             = &my_v4l2_ctx.vid_cap_dev;  
    vfd->fops       = &my_v4l2_fops;
    vfd->v4l2_dev   = &my_v4l2_ctx.v4l2_dev;
    vfd->device_caps = my_v4l2_ctx.cap.device_caps;
    vfd->v4l2_dev->release = my_v4l2_video_device_release;
    vfd->release 	= video_device_release_empty;
    vfd->ioctl_ops  = &my_v4l2_ioctl_ops;
	snprintf(vfd->name, sizeof(vfd->name),  "my-v4l2-%03d-vid-cap", 0);
	video_set_drvdata(vfd, &my_v4l2_ctx);
    erron = video_register_device(vfd, VFL_TYPE_VIDEO, -1);
    if(erron)
    {
        dev_err(&pdev->dev, "video_register_device erron:%d ", erron);
        goto unreg_dev;
    }

    platform_set_drvdata(pdev, &my_v4l2_ctx);

    return 0;

unreg_dev:

    v4l2_device_put(&my_v4l2_ctx.v4l2_dev);

    return -ENOMEM;
}
static int my_v4l2_video_pdrv_remove(struct platform_device *pdev)
{
    struct my_v4l2_video_ctx *ctx;

    dev_info(&pdev->dev, "my_v4l2_video_pdrv_remove\n");

    ctx = (struct my_v4l2_video_ctx *)platform_get_drvdata(pdev);

    video_unregister_device(&ctx->vid_cap_dev);

    v4l2_device_put(&ctx->v4l2_dev);

    return 0;
}

static const struct of_device_id my_v4l2_of_match[] = {
    { .compatible = "my-v4l2,video" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_v4l2_of_match);

static struct platform_driver my_v4l2_driver = {
    .probe = my_v4l2_video_pdrv_probe,
    .remove = my_v4l2_video_pdrv_remove,
    .driver = {
        .name = "my_v4l2_video",
        .of_match_table = my_v4l2_of_match,
    },
};  

module_platform_driver(my_v4l2_driver);

MODULE_AUTHOR("CoreyLee");
MODULE_DESCRIPTION("My V4l2 Video Driver");
MODULE_LICENSE("GPL");


