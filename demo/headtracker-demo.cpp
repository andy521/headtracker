#define HT_PI 3.14159265f
#include "ht-api.h"
#include <opencv2/opencv.hpp>
//#include <QtNetwork/QUdpSocket>
#include <stdio.h>

using namespace std;
using namespace cv;

static volatile bool ht_quitp = false;

#ifdef __unix
#include <signal.h>

static void ht_quit_handler(int foo) {
    ht_quitp = true;
}

#endif

#pragma pack(push, 2)
struct THeadPoseData {
        double x, y, z, yaw, pitch, roll;
        long frame_number;
};
#pragma pack(pop)

int main(int argc, char** argv)
{
    bool start = false;
#ifdef __unix
    (void) signal(SIGTERM, ht_quit_handler);
    (void) signal(SIGHUP, ht_quit_handler);
    (void) signal(SIGINT, ht_quit_handler);
#endif
    ht_config_t config;

    config.classification_delay = 500;
    config.field_of_view = 56;
    config.pyrlk_pyramids = 0;
    config.pyrlk_win_size_w = config.pyrlk_win_size_h = 21;
    config.max_keypoints = 150;
    config.keypoint_distance = 3.5;
    config.force_width = 640;
    config.force_height = 480;
    config.force_fps = 30;
    config.camera_index = 0;
    config.ransac_num_iters = 100;
    config.ransac_max_reprojection_error = 10;
    config.ransac_max_inlier_error = 10;
    config.ransac_max_mean_error = 8;
    config.ransac_abs_max_mean_error = 14;
    config.debug = 1;
    config.ransac_min_features = 0.8;
    config.flandmark_delay = 200;
    for (int i = 0; i < 5; i++)
    	config.dist_coeffs[i] = 0;

    headtracker_t* ctx = ht_make_context(&config, argc > 1 ? argv[1] : NULL);
    ht_result_t result;

#if 0
    QUdpSocket sock;
    QHostAddress addr("127.0.0.1");
    sock.bind(addr, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
#endif
    THeadPoseData pose;
    int frameno = 0;

    namedWindow("capture");

    while (!ht_quitp && ht_cycle(ctx, &result)) {
        if (result.filled) {
            start = true;
            pose.frame_number = frameno;
            pose.x = result.tx;
            pose.y = result.ty;
            pose.z = result.tz;
            pose.yaw = result.rotx;
            pose.pitch = result.roty;
            pose.roll = result.rotz;
            //sock.writeDatagram((const char*) &pose, sizeof(THeadPoseData), addr, 5550);
#if 0
            printf("POSE %.2f %.2f %.2f | %.2f %.2f %.2f\n",
                   pose.yaw, pose.pitch, pose.roll,
                   pose.x, pose.y, pose.z);
#endif
        } else if (start && argc > 1) {
            abort();
            break;
        }
        frameno++;
        const cv::Mat& frame = ht_get_bgr_frame(ctx);
        cv::Mat channels[3];
        cv::split(frame, channels);
        imshow("capture", channels[2]);
        waitKey(1);
    }

    ht_free_context(ctx);
	return 0;
}

