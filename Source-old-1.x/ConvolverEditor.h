/*
 *  Convolver - ConvolverEditor.h
 *  Created by Nik Reiman on 05.08.05
 *  Copyright (c) 2005 Teragon Audio, All rights reserved
 */

#ifndef __ConvolverEditor_H
#define __ConvolverEditor_H

#ifndef __Convolver_H
#include "Convolver.h"
#endif

#ifndef __cFileColumn_H
#include "cFileColumn.h"
#endif

#ifndef __vstgui__
#include "vstgui.h"
#endif

#define DEF_ERROR_TIMEOUT 1300
#define DEF_OFFSET 3

#define DEF_INFO_BOX_HEIGHT 12
#define DEF_INFO_BOX_X 30
#define DEF_INFO_BOX_X2 70
#define DEF_INFO_BOX_Y 247
#define DEF_KNOB_HEIGHT 54
#define DEF_KNOB_FRAMES 65

#ifdef MAC
#define MAX_PATH 512
#endif

#define PRM_FILECOLUMN 0

#ifdef MAC 
#define REZ_BACKGROUND 10001
#define REZ_FILE_OPEN 10002
#define REZ_KNOB 10009
#define REZ_LOAD_BUTTON 10010
#define REZ_SAVE_BUTTON 10011
#define REZ_SPLASHSCREEN 10012
#elif WIN32
#include "resource.h"
#endif

class CLabel : public CParamDisplay {
  public:
    CLabel(CRect &size, char *text);
  
    void draw(CDrawContext *pContext);
    void setLabel(char *text);
  protected:
    char label[256];
};

///////////////////////////////////////////////////////////////////////

class ConvolverEditor : public AEffGUIEditor, public CControlListener {
public:
  ConvolverEditor(AudioEffect *effect);
  ~ConvolverEditor();
  
  // VSTGUI Overrides
  virtual void close();
  virtual void idle();
  virtual long open(void *ptr);
  virtual void valueChanged(CDrawContext* context, CControl* control);

  char* getFilename();
  bool isOpen() const;
  
  void presetRead(char *fname);
  void presetWrite(char *fname);
  
  void setAttributes(char *text, int frames, int size, bool stereo, float delay);
  void setFilename(char *text);
  void setParameter(long index, float value);
  void setError(char *text);
  void setStatus(char *param, float value, char *label);
  void setStartupPath(char *path);
protected:
    
private:
  char* getStartupPath();
  
  CBitmap *m_background;
  CBitmap *m_file_choose_bitmap;
  CLabel *m_filelabel;
  CLabel *m_framelabel;
  CLabel *m_filesize;
  CLabel *m_delaylabel;
  CLabel *m_paramlabel;
  CLabel *m_paramvalue;
  CKickButton *m_file_choose_button;
  CKickButton *m_save_button;
  CKickButton *m_load_button;
  CAnimKnob *m_locut_knob;
  CAnimKnob *m_hicut_knob;
  CAnimKnob *m_gain_knob;
  CSplashScreen *m_splashscreen;
  
  cFileColumn *m_file_column;

  bool m_opened;
  bool m_error;
  char m_filename[MAX_PATH];
  char *m_dir;
  unsigned long m_ticks;
};

#endif