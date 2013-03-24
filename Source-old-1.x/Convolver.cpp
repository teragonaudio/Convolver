/*
 *  Convolver - Convolver.cpp
 *  Created by Nik Reiman on 05.08.05
 *  Copyright (c) 2005 Teragon Audio, All rights reserved
 */

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef MAC
#include <dirent.h>
#endif

#ifndef __Convolver_H
#include "Convolver.h"
#endif

#ifndef __ConvolverEditor_H
#include "ConvolverEditor.h"
#endif

#ifndef __vstgui__
#include "vstgui.h"
#endif

#ifndef SNDFILE_H
#include "sndfile.h"
#endif

#define SAFE_DELETE(x) if(x) { delete x; x = 0; }
#define SAFE_DELETE_ARR(x) if(x) { delete [] x; x = 0; }

Convolver::Convolver(audioMasterCallback audioMaster)
  : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS) {
  setNumInputs(NUM_INPUTS);
  setNumOutputs(NUM_OUTPUTS);
  setUniqueID(UNIQUE_ID);
  hasClip(false);
  
  strcpy(programName, "Default"); // default program name
  m_sample_rate = getSampleRate();

  initLocals();

  editor = new ConvolverEditor(this);
}

void Convolver::initLocals() {
  m_num_frames = 0;
  m_num_samples = 0;
  m_buffer_size = 0;
  m_buffer_index = 0;
  m_wetdry = 1.0;
  m_convolve_on = false;
  m_fft = 0;
  
  m_in_left = new float[MAX_UNSPLIT_SIZE];
  m_in_right = new float[MAX_UNSPLIT_SIZE];
  m_out_left = new float[MAX_UNSPLIT_SIZE];
  m_out_right = new float[MAX_UNSPLIT_SIZE];
  m_in_left_fr = new float[MAX_UNSPLIT_SIZE];
  m_in_right_fr = new float[MAX_UNSPLIT_SIZE];
  m_in_buf_left = new float[MAX_UNSPLIT_SIZE];
  m_in_buf_right = new float[MAX_UNSPLIT_SIZE];
  m_overflow_left = new float[MAX_UNSPLIT_SIZE];
  m_overflow_right = new float[MAX_UNSPLIT_SIZE];
  m_impulse_left = new float[MAX_UNSPLIT_SIZE];
  m_impulse_right = new float[MAX_UNSPLIT_SIZE];
  m_impulse_left_fr = new float[MAX_UNSPLIT_SIZE];
  m_impulse_right_fr = new float[MAX_UNSPLIT_SIZE];
  m_impulse_orig_left = new float[MAX_UNSPLIT_SIZE];
  m_impulse_orig_right = new float[MAX_UNSPLIT_SIZE];
  
  setParameter(PRM_GAIN, DEF_GAIN);
  setParameter(PRM_LOCUT, 0.0);
  setParameter(PRM_HICUT, 1.0);
  
  memset(m_filename, 0x0, sizeof(char) * MAX_NAME);
}

Convolver::~Convolver() {  
  SAFE_DELETE_ARR(m_impulse_left);
  SAFE_DELETE_ARR(m_impulse_left_fr);
  SAFE_DELETE_ARR(m_impulse_right);
  SAFE_DELETE_ARR(m_impulse_right_fr);
  SAFE_DELETE_ARR(m_in_left);
  SAFE_DELETE_ARR(m_in_right);
  SAFE_DELETE_ARR(m_out_left);
  SAFE_DELETE_ARR(m_out_right);
  SAFE_DELETE_ARR(m_in_left_fr);
  SAFE_DELETE_ARR(m_in_right_fr);
  SAFE_DELETE_ARR(m_overflow_left);
  SAFE_DELETE_ARR(m_overflow_right);
  SAFE_DELETE_ARR(m_in_buf_left);
  SAFE_DELETE_ARR(m_in_buf_right);
  SAFE_DELETE_ARR(m_impulse_orig_left);
  SAFE_DELETE_ARR(m_impulse_orig_right);
}

// Calculate the buffer size to be equal to a power of 2 which is also the sum of the
// number of frames for the impulse response, plus the minimum buffer size, plus some
// arbitrary number added to clean things up
long Convolver::calcBufsize(long frames) {
#ifdef MAC
  return (long)pow(2, (long)ceill((double)log2(frames + DEF_BUFSIZE)));
#elif WINDOWS
  return (long)pow(2, (long)ceill(log((double)(frames + DEF_BUFSIZE)) /
                                  log((double)2.0)));
#endif
}

float Convolver::getAttribute(int attrib) const {
  switch(attrib) {
    case ATR_FRAMES:
      return m_num_frames;
    case ATR_SIZE:
      return m_size;
    case ATR_STEREO:
      if(m_is_stereo) {
        return 1.0;
      }
      else {
        return 0.0;
      }
      break;
    case ATR_DELAY:
      return m_delay;
    default:
      return 0.0;
  }
}

float Convolver::getDb(float value) {
  return ((float)(MAX_GAIN - MIN_GAIN) * value) + (float)MIN_GAIN;
}

void Convolver::getDefaultPresetPath(char *path) const {
  getProductDir(path);
}

bool Convolver::getEffectName(char* name) {
  strcpy(name, "Convolver");
  return true;
}

void Convolver::getFilename(char *name) const {
  strncpy(name, m_filename, MAX_NAME);
}

float Convolver::getFreq(long index) const {
  switch(index) {
    case PRM_LOCUT:
      return m_locut_val;
    case PRM_HICUT:
      return m_hicut_val;
    default:
      return 0.0;
  }
}

void Convolver::getProductDir(char *name) const {
  char dir[MAX_DIR];
#ifdef MAC
  // Check for prefs dir, create if not there
  snprintf(dir, MAX_DIR, "%s/Library/Application Support/Teragon Audio/%s",
           getenv("HOME"), DEF_PRODUCT_NAME);
  DIR *dp = opendir(dir);
  if(dp == NULL) {
    if(mkdir(dir, 0) == -1) {
      // Maybe we don't have a Teragon Audio dir.  Pity.
      char tmp[MAX_DIR];
      snprintf(tmp, MAX_DIR, "%s/Library/Application Support/Teragon Audio", getenv("HOME"));
      if(mkdir(tmp, 0) == -1) {
        return;
      }
      else {
        chmod(tmp, S_IRWXU | S_IRWXG | S_IROTH);
      }
    }
    else {
      chmod(dir, S_IRWXU | S_IRWXG | S_IROTH);
    }
    return;
  }
  else {
    closedir(dp);
  }

  snprintf(name, MAX_DIR, "%s/Library/Application Support/Teragon Audio/%s/",
           getenv("HOME"), DEF_PRODUCT_NAME);
#elif WINDOWS
  HKEY key;
  DWORD max = MAX_DIR;

  if(RegOpenKeyEx(HKEY_CURRENT_USER,
                  "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                  0, KEY_READ, &key) == ERROR_SUCCESS) {
    if(RegQueryValueEx(key, "AppData", NULL, NULL,
                       (LPBYTE)dir, &max) == ERROR_SUCCESS) {
      char teragon[MAX_DIR];
      HANDLE hlist;
      WIN32_FIND_DATA filedata;

      sprintf(teragon, "%s\\Teragon Audio\\*", dir);
      hlist = FindFirstFile(teragon, &filedata);
      if(hlist == INVALID_HANDLE_VALUE) {
        char tmp[MAX_DIR];
        sprintf(tmp, "%s\\Teragon Audio", dir);
        if(!CreateDirectory(tmp, NULL)) {
          return;
        }
      }
      
      char convolver[MAX_DIR];
      sprintf(convolver, "%s\\Teragon Audio\\%s\\*", dir, DEF_PRODUCT_NAME);
      hlist = FindFirstFile(convolver, &filedata);
      if(hlist == INVALID_HANDLE_VALUE) {
        char tmp[MAX_DIR];
        sprintf(tmp, "%s\\Teragon Audio\\%s", dir, DEF_PRODUCT_NAME);
        if(!CreateDirectory(tmp, NULL)) {
          return;
        }
      }
      
      FindClose(hlist);
      RegCloseKey(key);

      _snprintf(name, MAX_DIR, "%s\\Teragon Audio\\%s\\", dir, DEF_PRODUCT_NAME);
    }
  }
#endif
}

void Convolver::getProgramName(char *name) {
  strcpy(name, programName);
}

float Convolver::getParameter(long index) {
  switch(index) {
    case PRM_FILE:
      return 0.0;
    case PRM_GAIN:
      return m_gain_val;
    case PRM_FILEBOX:
      return 0.0;
    case PRM_LOCUT:
      return m_locut;
    case PRM_HICUT:
      return m_hicut;
    case PRM_SAVE:
      return 0.0;
    case PRM_LOAD:
      return 0.0;
    case PRM_SPLASH:
      return 0.0;
    case PRM_WETDRY:
      return m_wetdry;
    default:
      return -1.0;
  }
}

void Convolver::getParameterDisplay(long index, char *text) {
  switch(index) {
    case PRM_FILE:
      if(editor) {
        sprintf(text, "%s", ((ConvolverEditor*)editor)->getFilename());
      }
      else {
        strcpy(text, "0.0");
      }
      break;
    case PRM_GAIN:
      sprintf(text, "%2.02f", m_gain);
      break;
    case PRM_FILEBOX:
      strcpy(text, "0.0");
      break;
    case PRM_LOCUT:
      sprintf(text, "%5.02f", m_locut_val);
      break;
    case PRM_HICUT:
      sprintf(text, "%5.02f", m_hicut_val);
      break;
    case PRM_SAVE:
      strcpy(text, "0.0");
      break;
    case PRM_LOAD:
      strcpy(text, "0.0");
      break;
    case PRM_SPLASH:
      strcpy(text, "0.0");
      break;
    case PRM_WETDRY:
      sprintf(text, "%3.02f", m_wetdry * 100);
      break;
    default:
      strcpy(text, "ERROR");
      break;
  }
}

void Convolver::getParameterLabel(long index, char *text) {
  switch(index) {
    case PRM_FILE:
      break;
    case PRM_GAIN:
      strcpy(text, "dB");
      break;
    case PRM_FILEBOX:
      break;
    case PRM_LOCUT:
      strcpy(text, "Hz");
      break;
    case PRM_HICUT:
      strcpy(text, "Hz");
      break;
    case PRM_SAVE:
      break;
    case PRM_LOAD:
      break;
    case PRM_SPLASH:
      break;
    case PRM_WETDRY:
      strcpy(text, "%");
      break;
    default:
      strcpy(text, "ERROR");
      break;
  }
}

void Convolver::getParameterName(long index, char *text) {
  switch(index) {
    case PRM_FILE:
      strcpy(text, "Filename");
      break;
    case PRM_GAIN:
      strcpy(text, "Gain");
      break;
    case PRM_FILEBOX:
      strcpy(text, "(Internal)");
      break;
    case PRM_LOCUT:
      strcpy(text, "Low Cutoff");
      break;
    case PRM_HICUT:
      strcpy(text, "High Cutoff");
      break;
    case PRM_SAVE:
      strcpy(text, "Save");
      break;
    case PRM_LOAD:
      strcpy(text, "Load");
      break;
    case PRM_SPLASH:
      strcpy(text, "(Internal)");
      break;
    case PRM_WETDRY:
      strcpy(text, "Wet/Dry Mix");
      break;
    default:
      strcpy(text, "ERROR");
      break;
  }
}

VstPlugCategory Convolver::getPlugCategory() {
  return kPlugCategEffect;
}

bool Convolver::getProductString(char* text) {
  strcpy(text, "Convolver");
  return true;
}

bool Convolver::getVendorString(char* text) {
  strcpy(text, "Teragon Audio");
  return true;
}

long Convolver::getVendorVersion() {
  return 1000;
}

long Convolver::getVstVersion() {
  return 2300;
}

bool Convolver::loadFile(char *fname) {
  if(fname == NULL) {
    return false;
  }
  
  // Bypass convolution
  m_convolve_on = false;
  //m_num_frames = 0;
  float wet = m_wetdry;
  //m_wetdry = 0.0;
  
  SF_INFO sf_info;
  SNDFILE *sf = sf_open(fname, SFM_READ, &sf_info);
  if(sf == NULL) {
    ((ConvolverEditor*)editor)->setError("Invalid File");
#ifdef DEBUG
    fprintf(stderr, "Libsndfile error number %d thrown\n", sf_error(sf));
#endif
    return false;
  }
#ifdef DEBUG
  else {
    fprintf(stderr, "Opening file %s for reading\n", fname);
  }
#endif
  
  if(sf_info.channels > MAX_CHANNELS) {
    ((ConvolverEditor*)editor)->setError("Too many channels");
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
  
  // TODO: Make this obsolete
  if(m_buffer_size > MAX_UNSPLIT_SIZE) {
    ((ConvolverEditor*)editor)->setError("File is too big!");
    struct stat ds;
    stat(fname, &ds);
    int size = (int)ds.st_size / 1024;
    ((ConvolverEditor*)editor)->setAttributes("No file", sf_info.frames, size, m_is_stereo, 0.0);
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
  
#ifdef DEBUG
  fprintf(stderr, "Read %d frames from file\n", total_frames);
#endif
  if(total_frames != sf_info.frames) {
#ifdef DEBUG
    fprintf(stderr, "Total frames not the same as number of frames read\n");
#endif
    sf_close(sf);
    return false;
  }
  sf_close(sf);
  
  //FFTReal<float> m_fft(m_buffer_size);
  if(m_fft) {
    delete m_fft;
  }
  m_fft = new FFTReal(m_buffer_size);
  
  m_fft->do_fft(m_impulse_left_fr, m_impulse_left);
  if(m_is_stereo) {
    m_fft->do_fft(m_impulse_right_fr, m_impulse_right);
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
    
  struct stat ds;
  stat(fname, &ds);
  m_size = (float)ds.st_size / 1024.0;
  m_delay = (m_num_samples - 1.0) / m_sample_rate;
  
  char *p = strrchr(fname, DEF_DELIMITER);
  if(p) {
    strncpy(m_filename, p, MAX_NAME);
  }
  else {
    strncpy(m_filename, fname, MAX_NAME);
  }
  if(editor) {
    ((ConvolverEditor*)editor)->setAttributes(m_filename, m_num_frames, (int)m_size,
                                              m_is_stereo, m_delay);
  }
  
  // Reset frequency indexes and apply the same freq. parameters that are currently set
  setParameter(PRM_LOCUT, m_locut);
  setParameter(PRM_HICUT, m_hicut);
  
  // Once these values is set, processing will begin!
  m_num_frames = (int)sf_info.frames;
  m_convolve_on = true;
  m_wetdry = wet;
  
  return true;
}

void Convolver::setParameter(long index, float value) {
  switch(index) {
    case PRM_FILE:
      if(editor) {
        loadFile(((ConvolverEditor*)editor)->getFilename());
      }
      break;
    case PRM_GAIN:
    {
      float db = getDb(value);
      m_gain = powf(10, db / 20.0);
      m_gain_val = value;
      break;
    }
    case PRM_FILEBOX:
      break;    
    case PRM_HICUT:
      m_hicut = value;
      m_hicut_val = (value * ((m_sample_rate / 4.0) - MIN_FILTER_FREQ)) + MIN_FILTER_FREQ;
      applyFilters();
      break;
    case PRM_LOCUT:
      m_locut = value;
      m_locut_val = (value * ((m_sample_rate / 4.0) - MIN_FILTER_FREQ)) + MIN_FILTER_FREQ;
      applyFilters();
      break;
    case PRM_SAVE:
      break;
    case PRM_LOAD:
      break;
    case PRM_SPLASH:
      break;
    case PRM_WETDRY:
      m_wetdry = value;
      break;
    default:
      break;
  }
}

void Convolver::setProgramName(char *name) {
  strcpy(programName, name);
}