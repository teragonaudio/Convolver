/*
 *  ConvolverProc.cpp
 *  Convolver
 *
 *  Created by Nik Reiman on 11.08.05.
 *  Copyright 2005 Teragon Audio. All rights reserved.
 *
 */

#ifndef __Convolver_H
#include "Convolver.h"
#endif

void Convolver::convolve() {
  // Assume here that the tail part of these input buffers is 0-padded
  m_fft->do_fft(m_in_left_fr, m_in_left);
  m_fft->do_fft(m_in_right_fr, m_in_right);
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
  m_fft->do_ifft(m_in_left_fr, m_out_left);
  m_fft->rescale(m_out_left);
  m_fft->do_ifft(m_in_right_fr, m_out_right);
  m_fft->rescale(m_out_right);
  
  // Toss the old overflow buffers into the convolved output
  for(register unsigned int i = 0; i < m_num_frames - 1; ++i) {
    m_out_left[i] += m_overflow_left[i];
    m_out_right[i] += m_overflow_right[i];
  }
  
  // Grab the last part of the output buffer into the new overflow buffer
  memcpy(m_overflow_left, m_out_left + m_num_samples, sizeof(float) * (m_num_frames - 1));
  memcpy(m_overflow_right, m_out_right + m_num_samples, sizeof(float) * (m_num_frames - 1));
}

void Convolver::process(float **inputs, float **outputs, long sampleFrames) {
  processReplacing(inputs, outputs, sampleFrames);
}

void Convolver::processReplacing(float **inputs, float **outputs, long sampleFrames) {
  float *in_left = inputs[0];
  float *in_right = inputs[1];
  float *out_left = outputs[0];
  float *out_right = outputs[1];
  
  float dry = 1.0 - m_wetdry;
  float gain = m_gain * DEF_FIX_FACTOR * m_wetdry;
  
  if(!m_num_frames) {
    while(--sampleFrames >= 0) {
      (*out_left++) = (*in_left++);
      (*out_right++) = (*in_right++);
    }
  }
  else {
    while(--sampleFrames >= 0) {
      m_in_left[m_buffer_index] = (*in_left++);
      m_in_right[m_buffer_index] = (*in_right++);
      
      (*out_left++) = (m_out_left[m_buffer_index] * gain) + (dry * m_in_buf_left[m_buffer_index]);
      (*out_right++) = (m_out_right[m_buffer_index] * gain) + (dry * m_in_buf_right[m_buffer_index]);
      
      if(++m_buffer_index > m_num_samples - 1) {
        if(m_convolve_on) {
          convolve();
        }
        m_buffer_index = 0;
      }
    }
  }
}