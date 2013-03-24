/*
 *  Convolver2 - Convolver2Editor.h
 *  Created by Nik Reiman on 23.03.06
 *  Copyright (c) 2006 Teragon Audio, All rights reserved
 */

#ifndef __Convolver2Editor_H
#define __Convolver2Editor_H

#ifdef USE_PC_AU
#include <Carbon/Carbon.h>
#include "AUCarbonViewBase.h"
#endif

#ifndef __pluginCore_H
#include "pluginCore.h"
#endif

#ifndef __Convolver2Core_H
#include "Convolver2Core.h"
#endif

#ifndef __cOptionMenuHidden_H
#include "cOptionMenuHidden.h"
#endif

#ifndef __cFileColumn_H
#include "cFileColumn.h"
#endif

#ifndef __cTransparentLabel_H
#include "cTransparentLabel.h"
#endif

#ifndef __defaults_H
#include "defaults.h"
#endif

#ifndef __vstcontrols__
#include "vstcontrols.h"
#endif

#ifndef __vstgui__
#include "vstgui.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>

static char _FILT_NAMES[NUM_FILT_TYPES][MAX_LABEL_SIZE] = {
  "Disabled",
  "Lowpass",
  "Hipass",
  //"Bandpass"
};

class Convolver2Editor : public AEffGUIEditor, public CControlListener {
  public:
#if PLUGGUI
    Convolver2Editor(void *effect);
#else
    Convolver2Editor(AudioEffect *effect);
#endif
    ~Convolver2Editor();
    
    virtual bool open(void *ptr);
    virtual void close();
    virtual void idle();
    
    void setAttributes();
#ifdef USE_PC_AU
    void setAU(AudioUnit unit);
#endif
    void setCore(pluginCore *p);
    void setInfoBox(char *text);
    void setInfoBox(char *text, float value, char *label = 0);
#if PLUGGUI
    void setParameter(long index, float value);
#else
    void setParameter(VstInt32 index, float value);
#endif
    void setStartupPath(char *path);
    virtual void valueChanged(CDrawContext* context, CControl* control);
    
  protected:
      
  private:
    pluginCore *core;
    eventManager events;
#ifdef USE_PC_AU
    AudioUnit au;
#endif
    
    CBitmap *m_background;
    CBitmap *m_file_choose_bitmap;
    cTransparentLabel *m_filelabel;
    cTransparentLabel *m_framelabel;
    cTransparentLabel *m_filesize;
    cTransparentLabel *m_delaylabel;
    cTransparentLabel *m_info_box;
    CKickButton *m_file_choose_button;
    CKickButton *m_save_button;
    CKickButton *m_load_button;
    CAnimKnob *m_filter_knob;
    CAnimKnob *m_wetdry_knob;
    CAnimKnob *m_gain_knob;
    CSplashScreen *m_splashscreen;
    cFileColumn *m_file_column;
    cOptionMenuHidden *m_filter_type;
    char m_filename[MAX_PATH_SIZE];
    bool m_error;
    bool m_opened;
};

#ifdef USE_PC_AU
class Convolver2View : public AUCarbonViewBase  {
  public:
    Convolver2View(AudioUnitCarbonView auv);  
    virtual ~Convolver2View();
    
    void RespondToEventTimer(EventLoopTimerRef inTimer);
    virtual OSStatus CreateUI(Float32 xoffset, Float32 yoffset);  
    
    Float32 xOffset;
    Float32 yOffset;
  protected:
    Convolver2Editor* editor;
  
  private:
};
#endif

#endif