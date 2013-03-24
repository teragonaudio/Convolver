/*
 *  Convolver2 - Convolver2VST.h
 *  Created by Nik Reiman on 23.03.06
 *  Copyright (c) 2006 Teragon Audio, All rights reserved
 */

#ifndef __Convolver2VST_H
#define __Convolver2VST_H

#ifndef __Convolver2Core_H
#include "Convolver2Core.h"
#endif

#ifndef __Convolver2Editor_H
#include "Convolver2Editor.h"
#endif

#ifndef __pluginCore_H
#include "pluginCore.h"
#endif

#ifndef __defaults_H
#include "defaults.h"
#endif

#ifndef __audioeffect__
#include "audioeffectx.h"
#endif

class Convolver2 : public AudioEffectX {
  public:
    Convolver2(audioMasterCallback audioMaster);
    ~Convolver2();

    virtual VstInt32 canDo(char *text);
    bool canParameterBeAutomated(VstInt32 index);
    virtual bool getEffectName(char* name);
    virtual float getParameter(VstInt32 index);
    virtual void getParameterDisplay(VstInt32 index, char *text);
    virtual void getParameterLabel(VstInt32 index, char *label);
    virtual void getParameterName(VstInt32 index, char *text);
    virtual VstPlugCategory getPlugCategory();
    virtual bool getProductString(char* text);
    virtual void getProgramName(char *name);
    virtual bool getVendorString(char* text);
    virtual VstInt32 getVendorVersion();
  
    virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);

    virtual void setParameter(VstInt32 index, float value);
    virtual void setProgramName(char *name);

  protected:
    char m_program_name[32];

  private:
    Convolver2Core *core;
};

#endif
