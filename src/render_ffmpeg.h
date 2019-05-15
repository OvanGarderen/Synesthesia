extern "C" {
#include <GL/gl.h>

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <string>
#include <iostream>


#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>

class Renderer 
{
private:
  AVCodecContext *c = NULL;
  AVFrame *frame;
  AVPacket pkt;
  FILE *file;
  struct SwsContext *sws_context = NULL;
  GLuint frameBuffer = 0;
  GLubyte *pixels = NULL;
  uint8_t *rgb = NULL;
  uint _w, _h;

  std::string filen;
  
  uint frames, cur_frame;

  void start(const char *filename, AVCodecID codec_id, int fps, int width, int height);
  void set_frame_yuv_from_rgb(uint8_t *rgb);
  void finish(void);

  void encode_frame(uint8_t *rgb);
  void glread_rgb(uint8_t **rgb, GLubyte **pixels, unsigned int width, unsigned int height);

public:
  Renderer(const char *filename, GLuint frameBuffer, uint frames, uint w, uint h) {
    this->frames = frames;
    this->cur_frame = 0;
    this->filen = filename;
    this->frameBuffer = frameBuffer;
    _w = w;
    _h = h;
    this->start(filen.c_str(), AV_CODEC_ID_H264, 24, _w, _h);
  }

  ~Renderer() {
    this->finish();
  }

  bool active() {return cur_frame < frames;}

  void step() {
    cur_frame++;
    if (!active()) return;

    std::cout << "frame " << cur_frame << std::endl;
    this->glread_rgb(&rgb, &pixels, _w, _h);
    this->encode_frame(rgb);
  }

  void stop() {
    frames = cur_frame;
  }
};
