/*
 *  Convolver - Convolver.h
 *  Created by Nik Reiman on 05.08.05
 *  Copyright (c) 2005 Teragon Audio, All rights reserved
 */

#ifndef __Convolver_H
#define __Convolver_H

#ifndef __audioeffect__
#include "audioeffectx.h"
#endif

#include "FFTReal.h"

// Custom file attributes for display LCD
#define ATR_FRAMES 0
#define ATR_SIZE 1
#define ATR_STEREO 2
#define ATR_DELAY 3

#define DEF_BUFSIZE 512
// Corresponds to ~0.0dB given range of {-30.0, 12.0}
#define DEF_GAIN 0.716
// Initial gain correction suitable for most IR's
#define DEF_FIX_FACTOR .15
// Filter resonance
#define DEF_FILT_RES 1.0

#define MAX_NAME 24
#define MAX_UNSPLIT_SIZE 65536
#define MAX_CHANNELS 2
#ifndef WINDOWS
#define MAX_PATH 512
#endif
#define MAX_GAIN 12.0
#define MIN_GAIN -30.0
// Limiter
#define MAX_LIMIT 1.5
#define MIN_FILTER_FREQ 60.0

#define NUM_PARAMS 9
#define NUM_PROGRAMS 1
#define NUM_INPUTS 2
#define NUM_OUTPUTS 2

#define PRM_FILE 0
#define PRM_GAIN 1
#define PRM_FILEBOX 2
#define PRM_LOCUT 3
#define PRM_HICUT 4
#define PRM_SAVE 5
#define PRM_LOAD 6
#define PRM_SPLASH 7
#define PRM_WETDRY 8

#define UNIQUE_ID 'cnvl'

class Convolver : public AudioEffectX {
  public:
    Convolver(audioMasterCallback audioMaster);
    ~Convolver();

    // VST overrides
    virtual void getProgramName(char *name);
    virtual float getParameter(long index);
    virtual void getParameterLabel(long index, char *label);
    virtual void getParameterDisplay(long index, char *text);
    virtual void getParameterName(long index, char *text);
    virtual bool getEffectName(char* name);
    virtual bool getVendorString(char* text);
    virtual bool getProductString(char* text);
    virtual long getVendorVersion();
    virtual long getVstVersion();
    virtual VstPlugCategory getPlugCategory();
    virtual void process(float **inputs, float **outputs, long sampleFrames);
    virtual void processReplacing(float **inputs, float **outputs, long sampleFrames);
    virtual void setParameter(long index, float value);
    virtual void setProgramName(char *name);

    void applyFilters();
    float getAttribute(int attrib) const;
    float getDb(float value);
    void getDefaultPresetPath(char *path) const;
    void getFilename(char *name) const;
    float getFreq(long index) const;
    void getProductDir(char *name) const;
    bool loadFile(char *fname);
  protected:
    char programName[32];

  private:
    long calcBufsize(long frames);
    void convolve();
    void initLocals();

    //FFTReal<float> *fft_object;
    FFTReal *m_fft;
    
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
    
    bool m_is_stereo;
    bool m_downsample_enable;
    bool m_convolve_on;
    
    char m_filename[MAX_NAME];
    
    float m_sample_rate;
    float m_gain;
    float m_gain_val;
    float m_locut;
    float m_locut_val;
    float m_hicut;
    float m_hicut_val;
    float m_size;
    float m_delay;
    float m_wetdry;

    int m_downsample_rate;
    
    unsigned int m_num_frames;
    unsigned int m_num_samples;
    unsigned long m_buffer_index;
    unsigned long m_buffer_size;
};

#endif
