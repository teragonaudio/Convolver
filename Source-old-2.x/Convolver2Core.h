/*
 *  Convolver2 - Convolver2Core.h
 *  Created by Nik Reiman on 23.03.06
 *  Copyright (c) 2006 Teragon Audio, All rights reserved
 */


#ifndef __Convolver2Core_H
#define __Convolver2Core_H

#ifndef __pluginCore_H
#include "pluginCore.h"
#endif

#ifndef __Convolver2Editor_H
#include "Convolver2Editor.h"
#endif

#ifndef __defaults_H
#include "defaults.h"
#endif

#include "FFTReal.h"

#ifndef __fxHipassFilter_H
#include "Filters/fxHipassFilter.h"
#endif

#ifndef __fxLopassFilter_H
#include "Filters/fxLopassFilter.h"
#endif

#ifndef __fxLimiter_H
#include "Gains/fxLimiter.h"
#endif

#ifndef SNDFILE_H
#include "sndfile.h"
#endif

#ifdef MAC
#include <sys/sysctl.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifndef DEF_DELIMITER
#ifdef MAC
#define DEF_DELIMITER '/'
#endif
#ifdef WINDOWS
#define DEF_DELIMITER (char)92
#endif
#endif

// Forward declaration, thanks to a recursive include
class Convolver2Editor;

class Convolver2Core : public pluginCore {
public:
  Convolver2Core(int num_params, int version, char *name);
  ~Convolver2Core();
  
  float getDelay() const;
  void getFilename(char *text) const;
  int getFrames() const;
  void getSize(char *text) const;
  bool loadFile(char *filename);
  void setEditor(Convolver2Editor *e) { editor = e; };
  void process(float **inputs, float **outputs, long frames);
  void recache();
  void reset();
  
protected:
private:
  /*
  void convolve(float *src1_fr_l, float *src1_fr_r,
                float *src2_fr_l, float *src2_fr_r,
                float *dest_fr_l, float *dest_fr_r,
                float *out_raw_l, float *out_raw_r);
  */
  void convolve();
  void emptyBuffers(int num_blocks);
  
  fxHipassFilter *m_hipass;
  fxLopassFilter *m_lopass;
  fxLimiter *m_limiter;
  Convolver2Editor *editor;
  FFTReal<float> m_fft;
  bool m_is_stereo;
  unsigned int m_num_frames;
  unsigned int m_num_samples;
  unsigned long m_buffer_index;
  unsigned long m_buffer_size;
  float *m_impulse_left;
  float *m_impulse_right;
  float *m_impulse_left_fr;
  float *m_impulse_right_fr;
  float *m_impulse_orig_left;
  float *m_impulse_orig_right;
  
  float *m_in_left;
  float *m_in_right;
  float *m_in_buf_left;
  float *m_in_buf_right;
  float *m_out_left;
  float *m_out_right;
  float *m_in_left_fr;
  float *m_in_right_fr;
  
  float *m_overflow_left;
  float *m_overflow_right;
  /*
  // Impulse response's raw signal (used only when IR is read in)
  float **m_ir_raw_l;
  float **m_ir_raw_r;
  // Impulse response's frequency domain signal
  float **m_ir_fr_l;
  float **m_ir_fr_r;
  // Output buffer for combined signal
  float **m_output_buf_raw_l;
  float **m_output_buf_raw_r;
  // First part of output buffer
  float *m_output_raw_l;
  float *m_output_raw_r;
  // Output buffer for blockwise convolution
  float m_conv_buf_l[DEF_BLOCK_SIZE];
  float m_conv_buf_r[DEF_BLOCK_SIZE];
  // Original reference copy of input signal (used by wet/dry param)
  float m_input_dry_l[DEF_BLOCK_SIZE];
  float m_input_dry_r[DEF_BLOCK_SIZE];
  // Actual input signal used for processing
  float m_input_raw_l[DEF_BLOCK_SIZE];
  float m_input_raw_r[DEF_BLOCK_SIZE];
  // Input signal's frequency response
  float m_input_fr_l[DEF_BLOCK_SIZE];
  float m_input_fr_r[DEF_BLOCK_SIZE];
  */
  char m_filename[MAX_FILE_LENGTH];
  
  bool m_has_sse;
  bool m_ir_loaded;
  bool m_convolve_on;
  
  int m_num_blocks;
  int m_output_size;
  int m_frames;
  
  float m_size;
  float m_delay;
  unsigned long m_num_samples_processed;
};

#endif
