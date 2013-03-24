/*
 *  Convolver2 - Convolver2AU.cpp
 *  Created by Nik Reiman on 23.03.06
 *  Copyright (c) 2006 Teragon Audio, All rights reserved
 */

#ifndef __Convolver2AU_H
#include "Convolver2AU.h"
#endif

COMPONENT_ENTRY(Convolver2);

Convolver2::Convolver2(AudioUnit component) : AUEffectBase(component) {
	CreateElements();
	Globals()->UseIndexedParameters(NUM_PARAMS);
  
  editor = NULL;
  core = new Convolver2Core(NUM_PARAMS, VERSION, DEF_PRODUCT);
  Float64 srate;
  if(DispatchGetProperty(kAudioUnitProperty_SampleRate, kAudioUnitScope_Global, 0, &srate) !=
     kAudioUnitErr_InvalidProperty) {
    core->setParameter(PRM_SAMPLE_RATE, srate, true);
  }
  
#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher(this);
#endif
}

Convolver2::~Convolver2() {
  delete core;
  
#if AU_DEBUG_DISPATCHER
  delete mDebugDispatcher;
#endif
}

int Convolver2::GetNumCustomUIComponents() {
  return 1;
}

ComponentResult Convolver2::GetParameter(AudioUnitParameterID inID,
                                            AudioUnitScope inScope,
                                            AudioUnitElement inElement,
                                            Float32& outValue) {
  outValue = core->getParameter(REQ_VALUE, inID, NULL);
  return noErr;
}

ComponentResult	Convolver2::GetParameterInfo(AudioUnitScope inScope,
                                                AudioUnitParameterID inParameterID,
                                                AudioUnitParameterInfo &outParameterInfo) {
	ComponentResult result = noErr;
  
	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable
    |	kAudioUnitParameterFlag_IsReadable;
  
  if(inScope == kAudioUnitScope_Global) {
    char *cname = new char[24];
    core->getParameter(REQ_NAME, inParameterID, cname);
    CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, cname, 0);
    delete [] cname;
    
    AUBase::FillInParameterName(outParameterInfo, name, false);
    outParameterInfo.unit = (AudioUnitParameterUnit)core->getParameter(REQ_LABEL, inParameterID, NULL);
    outParameterInfo.minValue = core->getParameter(REQ_VALUE_MIN, inParameterID, NULL);
    outParameterInfo.maxValue = core->getParameter(REQ_VALUE_MAX, inParameterID, NULL);
    outParameterInfo.defaultValue = core->getParameter(REQ_VALUE_DEF, inParameterID, NULL);
	}
  else {
    result = kAudioUnitErr_InvalidParameter;
  }
  
	return result;
}

ComponentResult	Convolver2::GetParameterValueStrings(AudioUnitScope inScope,
                                                        AudioUnitParameterID inParameterID,
                                                        CFArrayRef *outStrings) {
  return kAudioUnitErr_InvalidProperty;
}

ComponentResult Convolver2::GetProperty(AudioUnitPropertyID inID,
                                                       AudioUnitScope inScope,
                                                       AudioUnitElement inElement,
                                                       void *outData) {
  if(inScope == kAudioUnitScope_Global) {
    switch(inID) {
      case PRP_CORE:
        pluginCore **pc = (pluginCore**)outData;
        pc[0] = core;
        return noErr;
      default:
        return AUEffectBase::GetProperty(inID, inScope, inElement, outData);
    }
  }
  return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
}

ComponentResult Convolver2::GetPropertyInfo(AudioUnitPropertyID inID,
                                                           AudioUnitScope inScope,
                                                           AudioUnitElement inElement,
                                                           UInt32& outDataSize,
                                                           Boolean& outWritable) {
  if(inScope == kAudioUnitScope_Global) {
    switch(inID) {
      case PRP_CORE:
        outDataSize = sizeof(pluginCore*);
        return noErr;
      case PRP_EDITOR:
        outDataSize = sizeof(Convolver2Editor*);
        break;
      default:
        return AUEffectBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
    }
  }
  return AUEffectBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}

void Convolver2::GetUIComponentDescs(ComponentDescription* inDescArray) {
  inDescArray[0].componentType = kAudioUnitCarbonViewComponentType;
  inDescArray[0].componentSubType = Convolver2_COMP_SUBTYPE;
  inDescArray[0].componentManufacturer = Convolver2_COMP_MANF;
  inDescArray[0].componentFlags = 0;
  inDescArray[0].componentFlagsMask = 0;
}

OSStatus Convolver2::ProcessBufferLists(AudioUnitRenderActionFlags &ioActionFlags,
                                           const AudioBufferList &inBuffer,
                                           AudioBufferList &outBuffer,
                                           UInt32 inFramesToProcess) {
  // call the kernels to handle either interleaved or deinterleaved
	if(inBuffer.mNumberBuffers == 1) {
    m_inputs[0] = (float*)inBuffer.mBuffers[0].mData;
    m_inputs[1] = (float*)inBuffer.mBuffers[0].mData + 1;
    m_outputs[0] = (float*)outBuffer.mBuffers[0].mData;
    m_outputs[1] = (float*)outBuffer.mBuffers[0].mData + 1;
	}
  else {
    m_inputs[0] = (float*)inBuffer.mBuffers[0].mData;
    m_inputs[1] = (float*)inBuffer.mBuffers[1].mData;
    m_outputs[0] = (float*)outBuffer.mBuffers[0].mData;
    m_outputs[1] = (float*)outBuffer.mBuffers[1].mData;
  }
  
  core->process(m_inputs, m_outputs, inFramesToProcess);
  return noErr;
}

ComponentResult Convolver2::SetParameter(AudioUnitParameterID inID,
                                         AudioUnitScope inScope,
                                         AudioUnitElement inElement,
                                         Float32 inValue,
                                         UInt32 inBufferOffsetInFrames) {
  core->setParameter(inID, inValue);
  if(editor) {
    ((Convolver2Editor*)editor)->setAttributes();
  }
  return noErr;
}

ComponentResult Convolver2::SetProperty(AudioUnitPropertyID inID,
                                        AudioUnitScope inScope,
                                        AudioUnitElement inElement,
                                        const void *inData,
                                        UInt32 inDataSize) {
  if(inScope == kAudioUnitScope_Global) {
    switch(inID) {
      case PRP_EDITOR:
        editor = (Convolver2Editor*)inData;
        ((Convolver2Core*)core)->setEditor((Convolver2Editor*)editor);
        return noErr;
      default:
        return AUEffectBase::SetProperty(inID, inScope, inElement, inData, inDataSize);
    }
  }
  return AUEffectBase::SetProperty(inID, inScope, inElement, inData, inDataSize);
}

bool Convolver2::SupportsTail() {
  return false;
}

ComponentResult Convolver2::Version() {
  return kConvolver2Version;
}