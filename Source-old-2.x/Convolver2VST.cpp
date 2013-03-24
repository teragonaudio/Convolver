/*
 *  Convolver2 - Convolver2VST.cpp
 *  Created by Nik Reiman on 23.03.06
 *  Copyright (c) 2006 Teragon Audio, All rights reserved
 */

#ifndef __Convolver2VST_H
#include "Convolver2VST.h"
#endif

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new Convolver2(audioMaster);
}

Convolver2::Convolver2(audioMasterCallback audioMaster)
: AudioEffectX(audioMaster, 0, NUM_PARAMS) {
  setNumInputs(NUM_INPUTS);
  setNumOutputs(NUM_OUTPUTS);
  setUniqueID(UNIQUE_ID);

  editor = new Convolver2Editor(this);
  core = new Convolver2Core(NUM_PARAMS, VERSION, DEF_PRODUCT);
  core->setParameter(PRM_SAMPLE_RATE, getSampleRate(), true);
  ((Convolver2Editor*)editor)->setCore(core);
  ((Convolver2Core*)core)->setEditor((Convolver2Editor*)editor);
}

Convolver2::~Convolver2() {
  delete core;
}

VstInt32 Convolver2::canDo(char *text) {
  // TODO: Fill in according to your plugin's capabilities
  
  return 0;
}

bool Convolver2::canParameterBeAutomated(VstInt32 index) {
  return true;
}

bool Convolver2::getEffectName(char* name) {
  strcpy(name, "Convolver2");
  return true;
}

float Convolver2::getParameter(VstInt32 index) {
  return core->getParameter(REQ_VALUE, index, NULL);
}

void Convolver2::getParameterDisplay(VstInt32 index, char *text) {
  core->getParameter(REQ_DISPLAY, index, text);
}

void Convolver2::getParameterLabel(VstInt32 index, char *text) {
  core->getParameter(REQ_LABEL, index, text);
}

void Convolver2::getParameterName(VstInt32 index, char *text) {
  core->getParameter(REQ_NAME, index, text);
}

VstPlugCategory Convolver2::getPlugCategory() {
  return kPlugCategEffect;
}

bool Convolver2::getProductString(char* text) {
  strcpy(text, "Convolver2");
  return true;
}

void Convolver2::getProgramName(char *name) {
  strcpy(name, m_program_name);
}

bool Convolver2::getVendorString(char* text) {
  strcpy(text, "Teragon Audio");
  return true;
}

VstInt32 Convolver2::getVendorVersion() {
  return 1000;
}

void Convolver2::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
  core->process(inputs, outputs, sampleFrames);
}

void Convolver2::setParameter(VstInt32 index, float value) {
  core->setParameter(index, value);
}

void Convolver2::setProgramName(char *name) {
  strcpy(m_program_name, name);
}