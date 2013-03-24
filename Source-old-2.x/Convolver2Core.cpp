/*
 *  Convolver2 - Convolver2Core.cpp
 *  Created by Nik Reiman on 23.03.06
 *  Copyright (c) 2006 Teragon Audio, All rights reserved
 */

// Ok, apparently the fixed length FFTReal hang's the VS8 compiler.  So windows users
// will just have to deal with the runtime initialization, which is about 50% slower

// TODO: These global variables will cause issues with multiple instances of the plugin
/*
#ifdef MAC
#include "FFTRealFixLen.h"
// 2^13 = DEF_BLOCK_SIZE
FFTRealFixLen<13> _FFT;
#endif
#ifdef WINDOWS
#include "FFTReal.h"
// This needs to be set to whatever DEF_BLOCK_SIZE is
FFTReal<float> _FFT(8192);
#endif
*/

#ifndef __Convolver2Core_H
#include "Convolver2Core.h"
#endif

Convolver2Core::Convolver2Core(int num_params, int version, char *name) :
pluginCore(num_params, VERSION, DEF_PRODUCT)
{
  addParameter(PRM_STEREO, "Stereo", TYP_BOOL, 0.0, 1.0, 0.0, true);
  addParameter(PRM_GAIN, "Gain", TYP_DB, -30.0, 10.0, 0.0);
  addParameter(PRM_WETDRY, "Wet/Dry", TYP_GENERIC, 0.0, 1.0, 0.5);
  addParameter(PRM_FILTER_FREQ, "Filter Freq", TYP_HZ_LOG, 50.0, 20000.0f, 50.0);
  addParameter(PRM_FILTER_TYPE, "Filter Type", TYP_GENERIC, 0.0, NUM_FILT_TYPES, 0.0);
  addParameter(PRM_FILTER_RESONANCE, "Filter Resonance", TYP_GENERIC, 0.1f, (float)sqrt(2.0), 1.0f);
  addParameter(PRM_FILENAME, "Filename", TYP_BOOL, 0.0, 0.0, 0.0, true);
  addParameter(PRM_FILE_FRAMES, "File Frames", TYP_GENERIC, 0.0, 0.0, 0.0, true);
  addParameter(PRM_FILE_SIZE, "File Size", TYP_GENERIC, 0.0, 0.0, 0.0, true);
  addParameter(PRM_FILE_DELAY, "IR Delay", TYP_GENERIC, 0.0, 0.0, 0.0, true);
  addParameter(PRM_SAMPLE_RATE, "Sample Rate", TYP_HZ, 32000, 96000, 44100, true);
  /*
  m_ir_raw_l = 0;
  m_ir_raw_r = 0;
  m_ir_fr_l = 0;
  m_ir_fr_r = 0;
  m_output_buf_raw_l = 0;
  m_output_buf_raw_r = 0;
  m_output_raw_l = 0;
  m_output_raw_r = 0;
  */
  m_ir_loaded = false;
  m_frames = 0;
  m_size = 0.0f;
  m_delay = 0.0f;
  m_num_blocks = 0;
  m_output_size = 0;
  m_num_samples_processed = 0;
  memset(m_filename, 0x0, sizeof(char) * MAX_FILE_LENGTH);
  /*
  m_output_raw_l = (float*)malloc(sizeof(float) * DEF_BLOCK_SIZE);
  m_output_raw_r = (float*)malloc(sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_output_raw_l, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_output_raw_r, 0x0, sizeof(float) * DEF_BLOCK_SIZE);

  memset(m_conv_buf_l, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_conv_buf_r, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_input_dry_l, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_input_dry_r, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_input_raw_l, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_input_raw_r, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_input_fr_l, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_input_fr_r, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  */
  // Set up effects 
  m_hipass = new fxHipassFilter(this, 0);
  m_hipass->linkParameter(PRM_SAMPLE_RATE, fxHipassFilter_PRM_SAMPLE_RATE);
  m_hipass->linkParameter(PRM_FILTER_FREQ, fxHipassFilter_PRM_FREQ);
  m_hipass->linkParameter(PRM_FILTER_RESONANCE, fxHipassFilter_PRM_RESONANCE);
  m_hipass->recache();
  
  m_lopass = new fxLopassFilter(this);
  m_lopass->linkParameter(PRM_SAMPLE_RATE, fxLopassFilter_PRM_SAMPLE_RATE);
  m_lopass->linkParameter(PRM_FILTER_FREQ, fxLopassFilter_PRM_FREQ);
  m_lopass->linkParameter(PRM_FILTER_RESONANCE, fxLopassFilter_PRM_RESONANCE);
  m_lopass->recache();
  
  m_limiter = new fxLimiter(this);

  // See if the processor supports SSE
#ifdef MAC
  int hasSSE3 = 0;
  size_t length = sizeof(hasSSE3);
  int error = sysctlbyname("hw.optional.sse3", &hasSSE3, &length, NULL, 0);
  if(hasSSE3 && !error) {
    m_has_sse = true;
  }
  else {
    m_has_sse = false;
  }
#endif
}

Convolver2Core::~Convolver2Core() {
  delete m_limiter;
  delete m_lopass;
  delete m_hipass;

  emptyBuffers(m_num_blocks);
  /*
  free(m_output_raw_l);
  free(m_output_raw_r);
  */
}

void Convolver2Core::convolve()
{
  // Assume here that the tail part of these input buffers is 0-padded
  m_fft.do_fft(m_in_left_fr, m_in_left);
  m_fft.do_fft(m_in_right_fr, m_in_right);
  memcpy(m_in_buf_left, m_in_left, sizeof(float) * m_buffer_size);
  memcpy(m_in_buf_right, m_in_right, sizeof(float) * m_buffer_size);
  
  unsigned int half_buf = m_buffer_size / 2;
  register float tmp;
  if(m_is_stereo) {
    for(register unsigned int real = 0, imag = half_buf; real < half_buf; ++real, ++imag) {
      // Left channel
      tmp = (m_in_left_fr[real] * m_impulse_left_fr[real]) -
      (m_in_left_fr[imag] * m_impulse_left_fr[imag]);
      m_in_left_fr[imag] = (m_in_left_fr[real] * m_impulse_left_fr[imag]) +
        (m_in_left_fr[imag] * m_impulse_left_fr[real]);
      m_in_left_fr[real] = tmp;
      
      // Right channel
      tmp = (m_in_right_fr[real] * m_impulse_right_fr[real]) -
        (m_in_right_fr[imag] * m_impulse_right_fr[imag]);
      m_in_right_fr[imag] = (m_in_right_fr[real] * m_impulse_right_fr[imag]) +
        (m_in_right_fr[imag] * m_impulse_right_fr[real]);
      m_in_right_fr[real] = tmp;
    }
  }
  else {
    for(register unsigned int real = 0, imag = half_buf; real < half_buf; ++real, ++imag) {
      // Left channel
      tmp = (m_in_left_fr[real] * m_impulse_left_fr[real]) -
      (m_in_left_fr[imag] * m_impulse_left_fr[imag]);
      m_in_left_fr[imag] = (m_in_left_fr[real] * m_impulse_left_fr[imag]) +
        (m_in_left_fr[imag] * m_impulse_left_fr[real]);
      m_in_left_fr[real] = tmp;
      
      // Right channel - here is convolved with impulse's left channel,
      // since the impulse is mono
      tmp = (m_in_right_fr[real] * m_impulse_left_fr[real]) -
        (m_in_right_fr[imag] * m_impulse_left_fr[imag]);
      m_in_right_fr[imag] = (m_in_right_fr[real] * m_impulse_left_fr[imag]) +
        (m_in_right_fr[imag] * m_impulse_left_fr[real]);
      m_in_right_fr[real] = tmp;
    }
  }
  
  // Copy back to the input buffers, and then cut up the data
  m_fft.do_ifft(m_in_left_fr, m_out_left);
  m_fft.rescale(m_out_left);
  m_fft.do_ifft(m_in_right_fr, m_out_right);
  m_fft.rescale(m_out_right);
  
  // Toss the old overflow buffers into the convolved output
  for(register unsigned int i = 0; i < m_num_frames - 1; ++i) {
    m_out_left[i] += m_overflow_left[i];
    m_out_right[i] += m_overflow_right[i];
  }
  
  // Grab the last part of the output buffer into the new overflow buffer
  memcpy(m_overflow_left, m_out_left + m_num_samples, sizeof(float) * (m_num_frames - 1));
  memcpy(m_overflow_right, m_out_right + m_num_samples, sizeof(float) * (m_num_frames - 1));
}

/*
inline void Convolver2Core::convolve(float *src1_fr_l, float *src1_fr_r,
                                     float *src2_fr_l, float *src2_fr_r,
                                     float *dest_fr_l, float *dest_fr_r,
                                     float *out_raw_l, float *out_raw_r) {
  unsigned int half_buf = DEF_BLOCK_SIZE / 2;
  
  // We'll assume (for efficiency) that if the first right chan argument is null, then go for mono only.
  // This means that if other null arguments are passed in (ie, stereo out), then it WILL segfault!
  if(src1_fr_r) {
    for(register unsigned int real = 0, imag = half_buf; real < half_buf; ++real, ++imag) {
      dest_fr_l[real] = (src1_fr_l[real] * src2_fr_l[real]) -
      (src1_fr_l[imag] * src2_fr_l[imag]);
      dest_fr_l[imag] = (src1_fr_l[real] * src2_fr_l[imag]) +
        (src1_fr_l[imag] * src2_fr_l[real]);
      
      dest_fr_r[real] = (src1_fr_r[real] * src2_fr_r[real]) -
        (src1_fr_r[imag] * src2_fr_r[imag]);
      dest_fr_r[imag] = (src1_fr_r[real] * src2_fr_r[imag]) +
        (src1_fr_r[imag] * src2_fr_r[real]);
    }
  }
  else {
    for(register unsigned int real = 0, imag = half_buf; real < half_buf; ++real, ++imag) {
      dest_fr_l[real] = (src1_fr_l[real] * src2_fr_l[real]) -
      (src1_fr_l[imag] * src2_fr_l[imag]);
      dest_fr_l[imag] = (src1_fr_l[real] * src2_fr_l[imag]) +
        (src1_fr_l[imag] * src2_fr_l[real]);
    }
  }
  
  // Optionally bypass IFFT by passing NULL to out_raw_l
  if(out_raw_l) {
    if(out_raw_r) {
      _FFT.do_ifft(dest_fr_l, out_raw_l);
      _FFT.do_ifft(dest_fr_r, out_raw_r);
      _FFT.rescale(out_raw_l);
      _FFT.rescale(out_raw_r);
    }
    else {
      _FFT.do_ifft(dest_fr_l, out_raw_l);
      _FFT.rescale(out_raw_l);
    }  
  }
}
*/

void Convolver2Core::emptyBuffers(int num_blocks)
{
/*
 for(int i = 0; i < num_blocks; ++i) {
    free(m_ir_raw_l[i]);
    free(m_ir_raw_r[i]);
    free(m_ir_fr_l[i]);
    free(m_ir_fr_r[i]);
    free(m_output_buf_raw_l[i]);
    free(m_output_buf_raw_r[i]);
  }
  
  free(m_ir_raw_l);
  free(m_ir_raw_r);
  free(m_ir_fr_l);
  free(m_ir_fr_r);
  free(m_output_buf_raw_l);
  free(m_output_buf_raw_r);
*/
}

float Convolver2Core::getDelay() const {
  return m_delay;
}

void Convolver2Core::getFilename(char *text) const {
  char *ch = strrchr((char*)m_filename, (int)DEF_DELIMITER);
  if(ch) {
    strcpy(text, ch + 1);
  }
  else {
    strcpy(text, "None");
  }
}

int Convolver2Core::getFrames() const {
  return m_frames;
}

void Convolver2Core::getSize(char *text) const {
  sprintf(text, "%0.2fKb", m_size);
}

bool Convolver2Core::loadFile(char *filename)
{
  char real_fname[MAX_FILE_LENGTH];
  strncpy(real_fname, filename, MAX_FILE_LENGTH);
  
  SF_INFO sf_info;
  SNDFILE *sf = sf_open(real_fname, SFM_READ, &sf_info);
  if(sf == NULL) {
    return false;
  }
  
  if(sf_info.channels > MAX_CHANNELS) {
    ((Convolver2Editor*)editor)->setInfoBox("Too many channels");
    return false;
  }
  
  else if(sf_info.channels > 1) {
    m_is_stereo = true;
  }
  else {
    m_is_stereo = false;
  }
  
  m_buffer_size = calcBufsize(sf_info.frames);
  fprintf(stderr, "File has %d channels, %d frames, and is type %d.  Bufsize is %d\n",
          sf_info.channels, (int)sf_info.frames, sf_info.format, m_buffer_size);
  
  struct stat ds;
  if(m_buffer_size > MAX_UNSPLIT_SIZE) {
    ((Convolver2Editor*)editor)->setInfoBox("File is too big!");
    stat(filename, &ds);
    int size = (int)ds.st_size / 1024;
    strncpy(real_fname, "No file", MAX_FILE_LENGTH);
    setParameter(PRM_FILENAME, real_fname, MAX_FILE_LENGTH, true);
    setParameter(PRM_FILE_FRAMES, sf_info.frames, true);
    setParameter(PRM_FILE_SIZE, size, true);
    setParameter(PRM_STEREO, m_is_stereo, true);
    setParameter(PRM_FILE_DELAY, 0.0, true);
    ((Convolver2Editor*)editor)->setAttributes(); //"No file", sf_info.frames, size, m_is_stereo, 0.0);
    sf_close(sf);
    return false;
  }
  
  memset(m_impulse_left, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);
  memset(m_impulse_right, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);
  
  unsigned int total_frames;
  if(m_is_stereo) {
    float *tmp_buf = new float[sf_info.frames * sf_info.channels];
    total_frames = sf_readf_float(sf, tmp_buf, sf_info.frames);
    for(unsigned int i = 0; i < sf_info.frames; i += 2) {
      m_impulse_left[i] = tmp_buf[i];
      m_impulse_right[i] = tmp_buf[i + 1];
    }
    delete [] tmp_buf;
  }
  else {
    total_frames = sf_readf_float(sf, m_impulse_left, sf_info.frames);
  }
  
  fprintf(stderr, "Read %d frames from file\n", total_frames);
  if(total_frames != sf_info.frames) {
    ((Convolver2Editor*)editor)->setInfoBox("Corrupt file!");
    sf_close(sf);
    return false;
  }
  sf_close(sf);
  
  //FFTReal<float> m_fft(m_buffer_size);
  /*if(m_fft) {
    delete m_fft;
  }*/
  // FFTReal <float> fft_object (len);   // 1024-point FFT object constructed.
  FFTReal<float> m_fft(m_buffer_size);
  // m_fft = new FFTReal(m_buffer_size);
  
  m_fft.do_fft(m_impulse_left_fr, m_impulse_left);
  if(m_is_stereo) {
    m_fft.do_fft(m_impulse_right_fr, m_impulse_right);
  }
  
  // Copy data over
  for(unsigned int i = 0; i < m_buffer_size; ++i) {
    m_impulse_orig_left[i] = m_impulse_left[i];
    if(m_is_stereo) {
      m_impulse_orig_right[i] = m_impulse_right[i];
    }
  }
  
  memset(m_in_left, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);
  memset(m_in_right, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);  
  memset(m_out_left, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);
  memset(m_out_right, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);
  memset(m_in_buf_left, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);
  memset(m_in_buf_right, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);  
  memset(m_overflow_left, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);
  memset(m_overflow_right, 0x0, sizeof(float) * MAX_UNSPLIT_SIZE);
  
  m_num_samples = m_buffer_size - sf_info.frames;
  //m_buffer_index = 0;
  
  setParameter(PRM_FILE_FRAMES, sf_info.frames, true);
  stat(real_fname, &ds);
  setParameter(PRM_FILE_SIZE, (float)ds.st_size / 1024.0, true);
  setParameter(PRM_STEREO, m_is_stereo, true);
  setParameter(PRM_FILE_DELAY, (m_num_samples - 1.0) / PARAM(PRM_SAMPLE_RATE), true);
  
  char *p = strrchr(real_fname, DEF_DELIMITER);
  if(p) {
    strncpy(m_filename, p, MAX_NAME);
  }
  else {
    strncpy(m_filename, real_fname, MAX_NAME);
  }
  setParameter(PRM_FILENAME, m_filename, MAX_FILE_LENGTH, true);
  if(editor) {
    ((Convolver2Editor*)editor)->setAttributes();
    //m_filename, m_num_frames, (int)m_size, m_is_stereo, m_delay);
  }
  /*
  // Reset frequency indexes and apply the same freq. parameters that are currently set
  setParameter(PRM_LOCUT, m_locut);
  setParameter(PRM_HICUT, m_hicut);
  */
  // Once these values is set, processing will begin!
  m_num_frames = (int)sf_info.frames;
  return true;
  
  
  /*
  SF_INFO sf_info;
  SNDFILE *sf = sf_open(filename, SFM_READ, &sf_info);
  if(sf == NULL) {
    return false;
  }
  
  strncpy(m_filename, filename, MAX_FILE_LENGTH);
  float *tmp_buf = new float[(size_t)(sf_info.frames * sf_info.channels)];
  memset(tmp_buf, 0x0, sizeof(float) * (size_t)sf_info.frames);
  m_frames = (int)sf_readf_float(sf, tmp_buf, sf_info.frames);
  
  int tmp_blocks = (int)ceil((double)m_frames / (double)(DEF_BLOCK_SIZE / 2));
  if(tmp_blocks <= 0) {
    delete [] tmp_buf;
    return false;
  }
 
  int old_blocks = m_num_blocks;
  // Bypass convolution engine by setting this to 0
  m_num_blocks = 0;
  emptyBuffers(old_blocks);
  
  m_ir_raw_l = (float**)malloc(sizeof(float*) * tmp_blocks);
  m_ir_raw_r = (float**)malloc(sizeof(float*) * tmp_blocks);
  m_ir_fr_l = (float**)malloc(sizeof(float*) * tmp_blocks);
  m_ir_fr_r = (float**)malloc(sizeof(float*) * tmp_blocks);
  m_output_buf_raw_l = (float**)malloc(sizeof(float*) * tmp_blocks);
  m_output_buf_raw_r = (float**)malloc(sizeof(float*) * tmp_blocks);
  
  memset(m_input_dry_l, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  memset(m_input_dry_r, 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  
  // TODO: Ok, so this is a bit wasteful..
  m_output_size = DEF_BLOCK_SIZE * (tmp_blocks + 2);
  m_output_raw_l = (float*)realloc(m_output_raw_l, sizeof(float) * m_output_size);
  m_output_raw_r = (float*)realloc(m_output_raw_r, sizeof(float) * m_output_size);

  memset(m_output_raw_l, 0x0, sizeof(float) * m_output_size);
  memset(m_output_raw_r, 0x0, sizeof(float) * m_output_size);
  */
  
  /*
  for(int i = 0; i < tmp_blocks; ++i) {
    m_ir_raw_l[i] = (float*)malloc(sizeof(float) * DEF_BLOCK_SIZE);
    m_ir_raw_r[i] = (float*)malloc(sizeof(float) * DEF_BLOCK_SIZE);
    m_ir_fr_l[i] = (float*)malloc(sizeof(float) * DEF_BLOCK_SIZE);
    m_ir_fr_r[i] = (float*)malloc(sizeof(float) * DEF_BLOCK_SIZE);
    m_output_buf_raw_l[i] = (float*)malloc(sizeof(float) * DEF_BLOCK_SIZE);
    m_output_buf_raw_r[i] = (float*)malloc(sizeof(float) * DEF_BLOCK_SIZE);

    memset(m_ir_raw_l[i], 0x0, sizeof(float) * DEF_BLOCK_SIZE);
    memset(m_ir_raw_r[i], 0x0, sizeof(float) * DEF_BLOCK_SIZE);
    memset(m_ir_fr_l[i], 0x0, sizeof(float) * DEF_BLOCK_SIZE);
    memset(m_ir_fr_r[i], 0x0, sizeof(float) * DEF_BLOCK_SIZE);
    memset(m_output_buf_raw_l[i], 0x0, sizeof(float) * DEF_BLOCK_SIZE);
    memset(m_output_buf_raw_r[i], 0x0, sizeof(float) * DEF_BLOCK_SIZE);
  }

  int block = 0;
  int block_fr = 0;
  
  // TODO: Replace with n-n channel input algo from misswatson
  if(sf_info.channels == 1) {
    for(long fr = 0; fr < m_frames; ++fr, ++block_fr) {
      if(block_fr >= (DEF_BLOCK_SIZE - DEF_BLOCK_INPUT_SIZE)) {
        block++;
        block_fr = 0;
      }
      m_ir_raw_l[block][block_fr] = tmp_buf[fr];
      m_ir_raw_r[block][block_fr] = m_ir_raw_l[block][block_fr];
    }
  }
  else {
    for(long fr = 0; fr < m_frames; fr += 2, ++block_fr) {
      if(block_fr >= (DEF_BLOCK_SIZE - DEF_BLOCK_INPUT_SIZE)) {
        block++;
        block_fr = 0;
      }
      m_ir_raw_l[block][block_fr] = tmp_buf[fr];
      m_ir_raw_r[block][block_fr] = tmp_buf[fr + 1];
    }
  }

  for(int i = 0; i < tmp_blocks; ++i) {
    _FFT.do_fft(m_ir_fr_l[i], m_ir_raw_l[i]);
    _FFT.do_fft(m_ir_fr_r[i], m_ir_raw_r[i]);
  }
    
  struct stat ds;
  stat(filename, &ds);
  m_size = (float)ds.st_size / 1024.0f;
  m_delay = 1000 * DEF_BLOCK_SIZE / PARAM(PRM_SAMPLE_RATE);
  
  delete [] tmp_buf;
  m_num_blocks = tmp_blocks;
  m_ir_loaded = true;
  
  return true;
  */
}

void Convolver2Core::process(float **inputs, float **outputs, long frames)
{
  float *in_left = inputs[0];
  float *in_right = inputs[1];
  float *out_left = outputs[0];
  float *out_right = outputs[1];
  
  float wgain = DEF_FIX_FACTOR * PARAM(PRM_WETDRY) * (float)DB2LIN(PARAM(PRM_GAIN));
  float dgain = 1.0f - PARAM(PRM_WETDRY);
  
  if(!m_num_frames)
  {
    while(--frames >= 0)
    {
      (*out_left++) = (*in_left++);
      (*out_right++) = (*in_right++);
    }
  }
  else
  {
    while(--frames >= 0)
    {
      m_in_left[m_buffer_index] = (*in_left++);
      m_in_right[m_buffer_index] = (*in_right++);
      
      (*out_left++) = (m_out_left[m_buffer_index] * wgain) + (dgain * m_in_buf_left[m_buffer_index]);
      (*out_right++) = (m_out_right[m_buffer_index] * wgain) + (dgain * m_in_buf_right[m_buffer_index]);
      
      if(++m_buffer_index > m_num_samples - 1)
      {
        if(m_convolve_on)
        {
          convolve();
        }
        m_buffer_index = 0;
      }
    }
  }
}

  /*
  float *in_left = inputs[0];
  float *in_right = inputs[1];
  float *out_left = outputs[0];
  float *out_right = outputs[1];
  
  float wgain = DEF_FIX_FACTOR * PARAM(PRM_WETDRY) * (float)DB2LIN(PARAM(PRM_GAIN));
  float dgain = 1.0f - PARAM(PRM_WETDRY);

  // TODO: SSE-ize
  for(register int i = 0; i < frames; ++i, ++m_num_samples_processed) {
    if(m_num_samples_processed > DEF_BLOCK_INPUT_SIZE - 1) {
      if(m_ir_loaded) {
        _FFT.do_fft(m_input_fr_l, m_input_raw_l);
        _FFT.do_fft(m_input_fr_r, m_input_raw_r);
        
        register int j;
        for(j = 0; j < m_num_blocks; ++j) {
          convolve(m_input_fr_l, m_input_fr_r,
                   m_ir_fr_l[j], m_ir_fr_r[j],
                   m_conv_buf_l, m_conv_buf_r,
                   m_output_buf_raw_l[j], m_output_buf_raw_r[j]);
        }
        
        // Copy block one to the output buffer and take care of dry buffer at the same time
        int output_index;
        for(output_index = 0; output_index < DEF_BLOCK_INPUT_SIZE; ++output_index) {
          m_input_dry_l[output_index] = m_input_raw_l[output_index];
          m_input_dry_r[output_index] = m_input_raw_r[output_index];
          
          m_output_raw_l[output_index] = m_output_raw_l[output_index + DEF_BLOCK_INPUT_SIZE];
          m_output_raw_l[output_index] += m_output_buf_raw_l[0][output_index];
          m_output_raw_r[output_index] = m_output_raw_r[output_index + DEF_BLOCK_INPUT_SIZE];
          m_output_raw_r[output_index] += m_output_buf_raw_r[0][output_index];
        }
        
        // Copy overflow from the rest of the blocks
        for(register int bl_prev_index, bl_next_index, bl_num = 1; bl_num < m_num_blocks; ++bl_num) {
          // Write old buffer + end of previous block + beginning of this block
          for(bl_prev_index = DEF_BLOCK_INPUT_SIZE, bl_next_index = 0;
              bl_prev_index < DEF_BLOCK_SIZE - 1;
              ++bl_prev_index, ++bl_next_index, ++output_index) {
            m_output_raw_l[output_index] = m_output_raw_l[output_index + DEF_BLOCK_INPUT_SIZE];
            m_output_raw_l[output_index] += m_output_buf_raw_l[bl_num - 1][bl_prev_index] +
              m_output_buf_raw_l[bl_num][bl_next_index];
            m_output_raw_r[output_index] = m_output_raw_r[output_index + DEF_BLOCK_INPUT_SIZE];
            m_output_raw_r[output_index] += m_output_buf_raw_r[bl_num - 1][bl_prev_index] +
              m_output_buf_raw_r[bl_num][bl_next_index];
          }
          
          // Write old buffer + end of this block 
          for(; bl_next_index < DEF_BLOCK_SIZE - 1; ++bl_next_index, ++output_index) {
            m_output_raw_l[output_index] = m_output_raw_l[output_index + DEF_BLOCK_INPUT_SIZE];
            m_output_raw_l[output_index] += m_output_buf_raw_l[bl_num][bl_next_index];
            m_output_raw_r[output_index] = m_output_raw_r[output_index + DEF_BLOCK_INPUT_SIZE];
            m_output_raw_r[output_index] += m_output_buf_raw_r[bl_num][bl_next_index];
          }
        }
      }
      // IR not loaded
      else {
        for(int j = 0; j < DEF_BLOCK_INPUT_SIZE; ++j) {
          m_input_dry_l[j] = m_input_raw_l[j];
          m_input_dry_r[j] = m_input_raw_r[j];
        }
      }
      
      m_num_samples_processed = 0;
    }
 
    // First store the input array
    m_input_raw_l[m_num_samples_processed] = in_left[i];
    m_input_raw_r[m_num_samples_processed] = in_right[i];
    
    out_left[i] = (m_input_dry_l[m_num_samples_processed] * dgain) +
      (m_output_raw_l[m_num_samples_processed] * wgain);
    out_right[i] = (m_input_dry_r[m_num_samples_processed] * dgain) +
      (m_output_raw_r[m_num_samples_processed] * wgain);

    m_limiter->process(out_left[i]);
    m_limiter->process(out_right[i]);
  }

  switch((int)PARAM(PRM_FILTER_TYPE)) {
    case FILT_LOWPASS:
      m_lopass->process(out_left, out_right, frames);
      break;
    case FILT_HIPASS:
      m_hipass->process(out_left, out_right, frames);
      break;
    default:
      break;
  }
  */

void Convolver2Core::recache() {
  switch((int)PARAM(PRM_FILTER_TYPE)) {
    case FILT_HIPASS:
      m_hipass->recache();
      break;
    case FILT_LOWPASS:
      m_lopass->recache();
      break;
    default:
      break;
  }
}

void Convolver2Core::reset() {
}