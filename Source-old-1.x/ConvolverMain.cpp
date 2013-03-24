/*
 *  Convolver - ConvolverMain.cpp
 *  Created by Nik Reiman on 05.08.05
 *  Copyright (c) 2005 Teragon Audio, All rights reserved
 */

#ifndef __Convolver_H
#include "Convolver.h"
#endif

bool oome = false;

#if MAC
#pragma export on
#endif

#if BEOS
#define main main_plugin
extern "C" __declspec(dllexport)AEffect *main_plugin(audioMasterCallback audioMaster);
#elif MACX
#define main main_macho
extern "C" AEffect *main_macho(audioMasterCallback audioMaster);
#else
AEffect *main(audioMasterCallback audioMaster);
#endif

AEffect *main(audioMasterCallback audioMaster) {
  // Get VST Version
  if(!audioMaster(0, audioMasterVersion, 0, 0, 0, 0)) {
    return 0; // old version
  }

  // Create the AudioEffect
  Convolver* effect = new Convolver(audioMaster);
  if(!effect) {
    return 0;
  }

  // Check if no problem in constructor of Chopper
  if(oome) {
    delete effect;
    return 0;
  }
  return effect->getAeffect();
}

#if MAC
#pragma export off
#endif

#if WIN32
#include <windows.h>
void* hInstance;

BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved) {
  hInstance = hInst;
  return 1;
}
#endif
