#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

/* Function to save a frame as a PPM image (color) */
void save_frame_as_ppm(AVFrame *frame, int frame_num, int width, int height) {
    char filename[1024];
    snprintf(filename, sizeof(filename), "input.ppm", frame_num);

    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, " Could not open %s\n", filename);
        return;
    }

    fprintf(file, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++) {
        fwrite(frame->data[0] + y * frame->linesize[0], 1, width * 3, file);
    }

    fclose(file);
    printf(" Saved PPM frame %d -> %s\n", frame_num, filename);
}

/* Function to save a PGM image converted from PPM using custom weights */
void save_frame_as_pgm_from_ppm(AVFrame *rgb_frame, int frame_num, int width, int height, float x_cof, float y_cof, float z_cof) {
    char filename[1024];
    snprintf(filename, sizeof(filename), "input.pgm", frame_num);

    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, " Could not open %s\n", filename);
        return;
    }

    fprintf(file, "P5\n%d %d\n255\n", width, height);

    uint8_t *grayscale_buffer = (uint8_t *)malloc(width * height);
    if (!grayscale_buffer) {
        fprintf(stderr, " Memory allocation failed\n");
        fclose(file);
        return;
    }

    for (int y = 0; y < height; y++) {
        uint8_t *rgb_row = rgb_frame->data[0] + y * rgb_frame->linesize[0];
        uint8_t *gray_row = grayscale_buffer + y * width;

        for (int x_index = 0; x_index < width; x_index++) {
            uint8_t r = rgb_row[x_index * 3];
            uint8_t g = rgb_row[x_index * 3 + 1];
            uint8_t b = rgb_row[x_index * 3 + 2];
            gray_row[x_index] = (uint8_t)(x_cof * r + y_cof * g + z_cof * b);
        }
    }

    fwrite(grayscale_buffer, 1, width * height, file);

    free(grayscale_buffer);
    fclose(file);
    printf(" Saved PGM frame %d -> %s\n", frame_num, filename);
}

/* Function to start dppgm */
static void start_dppgm(GtkWidget *widget, gpointer data) {
    g_spawn_async(NULL, (char *[]){"./dppgm", NULL}, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
}

/* Function to start dpppm */
static void start_dpppm(GtkWidget *widget, gpointer data) {
    g_spawn_async(NULL, (char *[]){"./dpppm", NULL}, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
}

int main(int argc, char **argv) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <input_video> <frame_number> <x_cof> <y_cof> <z_cof>\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    int target_frame = atoi(argv[2]);
    float x_cof = atof(argv[3]);
    float y_cof = atof(argv[4]);
    float z_cof = atof(argv[5]);

    if (target_frame < 0) {
        fprintf(stderr, "Error: Frame number cannot be negative.\n");
        return 1;
    }

    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *codec_ctx = NULL;
    const AVCodec *codec = NULL;
    AVFrame *frame = NULL;
    AVPacket *pkt = NULL;
    struct SwsContext *sws_ctx = NULL;
    int video_stream_index = -1;
    int frame_num = 0;

    avformat_network_init();

    if (avformat_open_input(&fmt_ctx, input_file, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open input file %s\n", input_file);
        return 1;
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info\n");
        return 1;
    }

    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        fprintf(stderr, "No video stream found\n");
        return 1;
    }

    AVCodecParameters *codec_params = fmt_ctx->streams[video_stream_index]->codecpar;
    codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        fprintf(stderr, "Unsupported codec\n");
        return 1;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Could not allocate codec context\n");
        return 1;
    }

    if (avcodec_parameters_to_context(codec_ctx, codec_params) < 0) {
        fprintf(stderr, "Could not copy codec parameters\n");
        return 1;
    }

    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return 1;
    }

    frame = av_frame_alloc();
    pkt = av_packet_alloc();
    if (!frame || !pkt) {
        fprintf(stderr, "Could not allocate memory\n");
        return 1;
    }

    sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                             codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
                             SWS_BILINEAR, NULL, NULL, NULL);

    if (!sws_ctx) {
        fprintf(stderr, "Could not initialize the conversion context\n");
        return 1;
    }

    AVFrame *rgb_frame = av_frame_alloc();
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);
    uint8_t *buffer = (uint8_t *)av_malloc(num_bytes);
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer,
                         AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_stream_index) {
            if (avcodec_send_packet(codec_ctx, pkt) < 0) {
                break;
            }

            while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                if (frame_num == target_frame) {
                    printf("Target frame %d reached!\n", frame_num);

                    sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0,
                              codec_ctx->height, rgb_frame->data, rgb_frame->linesize);

                    save_frame_as_ppm(rgb_frame, frame_num, codec_ctx->width, codec_ctx->height);
                    save_frame_as_pgm_from_ppm(rgb_frame, frame_num, codec_ctx->width, codec_ctx->height, x_cof, y_cof, z_cof);

                    // Start dppgm and dpppm
                    start_dppgm(NULL, NULL);
                    start_dpppm(NULL, NULL);

                    av_free(buffer);
                    av_frame_free(&rgb_frame);
                    sws_freeContext(sws_ctx);
                    exit(0);
                }
                frame_num++;
            }
        }
        av_packet_unref(pkt);
    }

    fprintf(stderr, "Error: Frame number %d was not found in the video.\n", target_frame);

    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);

    return 1;
}
