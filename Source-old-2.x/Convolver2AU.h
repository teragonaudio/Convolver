/*
 *  Convolver2 - Convolver2AU.h
 *  Created by Nik Reiman on 23.03.06
 *  Copyright (c) 2006 Teragon Audio, All rights reserved
 */

#ifndef __Convolver2AU_H
#define __Convolver2AU_H

#ifndef __Convolver2Core_H
#include "Convolver2Core.h"
#endif

#ifndef __Convolver2Editor_H
#include "Convolver2Editor.h"
#endif

#ifndef __defaults_H
#include "defaults.h"
#endif

#include "AUCarbonViewBase.h"
#include "AUCarbonViewControl.h"
#include "AUEffectBase.h"

#if AU_DEBUG_DISPATCHER
#include "AUDebugDispatcher.h"
#endif

class Convolver2 : public AUEffectBase {
public:
  Convolver2(AudioUnit component);
  virtual ~Convolver2();
  
  virtual int GetNumCustomUIComponents();  
  virtual	ComponentResult GetParameterValueStrings(AudioUnitScope inScope,
                                                   AudioUnitParameterID inParameterID,
                                                   CFArrayRef *outStrings);
  virtual ComponentResult GetParameter(AudioUnitParameterID inID,
                                       AudioUnitScope inScope,
                                       AudioUnitElement inElement,
                                       Float32& outValue);
  virtual	ComponentResult GetParameterInfo(AudioUnitScope inScope,
                                           AudioUnitParameterID	inParameterID,
                                           AudioUnitParameterInfo	&outParameterInfo);
  virtual ComponentResult GetProperty(AudioUnitPropertyID inID,
                                      AudioUnitScope inScope,
                                      AudioUnitElement inElement,
                                      void *outData);  
  virtual ComponentResult GetPropertyInfo(AudioUnitPropertyID inID,
                                          AudioUnitScope inScope,
                                          AudioUnitElement inElement,
                                          UInt32& outDataSize,
                                          Boolean& outWritable);  
  virtual void GetUIComponentDescs(ComponentDescription* inDescArray);
  virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags &ioActionFlags,
                                      const AudioBufferList &inBuffer,
                                      AudioBufferList &outBuffer,
                                      UInt32 inFramesToProcess);
  virtual ComponentResult SetParameter(AudioUnitParameterID inID,
                                       AudioUnitScope inScope,
                                       AudioUnitElement inElement,
                                       Float32 inValue,
                                       UInt32 inBufferOffsetInFrames);
  virtual ComponentResult SetProperty(AudioUnitPropertyID inID,
                                      AudioUnitScope inScope,
                                      AudioUnitElement inElement,
                                      const void *inData,
                                      UInt32 inDataSize);
  virtual	bool SupportsTail();
  virtual ComponentResult	Version();
  
protected:
private:
  PluginGUIEditor *editor;
  Convolver2Core *core;
  float *m_inputs[2];
  float *m_outputs[2];
};

#endif