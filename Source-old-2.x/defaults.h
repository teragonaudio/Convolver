/*
 *  Convolver2 - defaults.h
 *  Created by Nik Reiman on 23.03.06
 *  Copyright (c) 2006 Teragon Audio, All rights reserved
 */

#ifndef __defaults_H
#define __defaults_H

#define DEF_PRODUCT "Convolver"
#define MAJ_VERSION 1
#define MIN_VERSION 3
#define REL_BRANCH 'a'
#define VERSION ((MAJ_VERSION * 1000) + (MIN_VERSION * 10))

enum {
  PRM_STEREO,
  PRM_GAIN,
  PRM_WETDRY,
  PRM_FILTER_FREQ,
  PRM_FILTER_TYPE,
  PRM_FILTER_RESONANCE,
  PRM_FILENAME,
  PRM_FILE_FRAMES,
  PRM_FILE_SIZE,
  PRM_FILE_DELAY,
  PRM_SAMPLE_RATE,
  NUM_PARAMS
};

enum {
#ifdef MAC
  REZ_BACKGROUND = 10001,
#else
  REZ_BACKGROUND = 1,
#endif
  REZ_OPEN_BUTTON,
  REZ_SCROLLBAR_HANDLE,
  REZ_SCROLLBAR_BG,
  REZ_KNOB,
  REZ_LOAD_BUTTON,
  REZ_SAVE_BUTTON,
  REZ_SPLASHSCREEN,
  REZ_FILE_ICONS,
};

enum {
  TAG_FILECOLUMN = 100,
  TAG_FILE_SAVE,
  TAG_FILE_LOAD,
  TAG_SPLASHSCREEN
};

enum {
  FILT_DISABLED,
  FILT_LOWPASS,
  FILT_HIPASS,
//  FILT_BANDPASS,
  NUM_FILT_TYPES
};

#define DEF_BLOCK_SIZE 8192
// TODO: Eliminate this and use dynamic block sizing
#define DEF_BLOCK_INPUT_SIZE 4096
// Initial gain correction suitable for most IR's
#define DEF_FIX_FACTOR 0.15f
#define DEF_INFO_BOX_TIMEOUT 2000
#define DEF_INFO_BOX_HEIGHT 12
#define DEF_INFO_BOX_X 30
#define DEF_INFO_BOX_X2 70
#define DEF_INFO_BOX_Y 247
#define DEF_KNOB_HEIGHT 54
#define DEF_KNOB_FRAMES 65

#define MAX_CHANNELS 2
#define MAX_FILE_LENGTH 384
#define MAX_UNSPLIT_SIZE 65536

#define NUM_INPUTS 2
#define NUM_OUTPUTS 2
#define UNIQUE_ID 'Cnv2'

#ifdef USE_PC_AU
#define Convolver2_COMP_SUBTYPE UNIQUE_ID
#define Convolver2_COMP_MANF 'Tera'
#define PRP_CORE 64000
#define PRP_EDITOR 64001

#ifdef DEBUG
#define kConvolver2Version 0xFFFFFFFF
#else
#define kConvolver2Version 0x00010000	
#endif
#endif
#endif