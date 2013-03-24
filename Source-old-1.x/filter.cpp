/*
 *  filter.cpp
 *  Convolver
 *
 *  Created by Nik Reiman on 30.12.05.
 *  Copyright 2005 Teragon Audio. All rights reserved.
 *
 */

#include <math.h>

#ifndef __Convolver_H
#include "Convolver.h"
#endif

void Convolver::applyFilters() {
  if(!m_num_samples) {
    return;
  }
  
  // Hipass filter
  register float c = 1.0 / tan(M_PI * m_hicut_val / m_sample_rate);
  register float a1 = 1.0 / (1.0 + (DEF_FILT_RES * c) + (c * c));
  register float a2 = 2.0 * a1;
  register float b1 = a2 * (1.0 - (c * c));
  register float b2 = a1 * (1.0 - (DEF_FILT_RES * c) + (c * c));
    
  memset(m_impulse_left, 0x0, sizeof(float) * m_buffer_size);
  if(m_is_stereo) {
    memset(m_impulse_right, 0x0, sizeof(float) * m_buffer_size);
  }
  
  m_impulse_left[0] = a1 * m_impulse_orig_left[0];
  if(m_is_stereo) {
    m_impulse_right[0] = a1 * m_impulse_orig_right[0];
  }
  
  m_impulse_left[1] = a1 * m_impulse_orig_left[0] +
    a2 * m_impulse_orig_left[0] -
    b1 * m_impulse_left[0];
  if(m_is_stereo) {
    m_impulse_right[1] = a1 * m_impulse_orig_right[0] +
    a2 * m_impulse_orig_right[0] -
    b1 * m_impulse_right[0];
  }
  
  for(unsigned int i = 2; i < m_buffer_size; ++i) {
    m_impulse_left[i] = a1 * m_impulse_orig_left[i] +
    a2 * m_impulse_orig_left[i - 1] +
    a1 * m_impulse_orig_left[i - 2] -
    b1 * m_impulse_left[i - 1] -
    b2 * m_impulse_left[i - 2];
    if(m_is_stereo) {
      m_impulse_right[i] = a1 * m_impulse_orig_right[i] +
      a2 * m_impulse_orig_right[i - 1] +
      a1 * m_impulse_orig_right[i - 2] -
      b1 * m_impulse_right[i - 1] -
      b2 * m_impulse_right[i - 2];
    }
  }
  
  // Then apply a lowpass filter 
  c = tan(M_PI * m_locut_val / m_sample_rate);
  a1 = 1.0 / (1.0 + (DEF_FILT_RES * c) + (c * c));
  a2 = -2.0 * a1;
  b1 = 2.0 * a1 * ((c * c) - 1.0);
  b2 = a1 * (1.0 - (DEF_FILT_RES * c) + (c * c));
  
  m_impulse_left[0] *= a1;
  if(m_is_stereo) {
    m_impulse_right[0] *= a1;
  }
  
  m_impulse_left[1] = a1 * m_impulse_left[0] +
    a2 * m_impulse_left[0] -
    b1 * m_impulse_left[0];
  if(m_is_stereo) {
    m_impulse_right[1] = a1 * m_impulse_right[0] +
    a2 * m_impulse_right[0] -
    b1 * m_impulse_right[0];
  }
  
  for(unsigned int i = 2; i < m_buffer_size; ++i) {
    m_impulse_left[i] = a1 * m_impulse_left[i] +
    a2 * m_impulse_left[i - 1] +
    a1 * m_impulse_left[i - 2] -
    b1 * m_impulse_left[i - 1] -
    b2 * m_impulse_left[i - 2];
    if(m_is_stereo) {
      m_impulse_right[i] = a1 * m_impulse_right[i] +
      a2 * m_impulse_right[i - 1] +
      a1 * m_impulse_right[i - 2] -
      b1 * m_impulse_right[i - 1] -
      b2 * m_impulse_right[i - 2];
    }
  }
  
  m_convolve_on = false;
  m_fft->do_fft(m_impulse_left_fr, m_impulse_left);
  if(m_is_stereo) {
    m_fft->do_fft(m_impulse_right_fr, m_impulse_right);
  }
  m_convolve_on = true;
}