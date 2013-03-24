/*
 *  Convolver - ConvolverEditor.cpp
 *  Created by Nik Reiman on 05.08.05
 *  Copyright (c) 2005 Teragon Audio, All rights reserved
 */

#ifndef __ConvolverEditor_H
#include "ConvolverEditor.h"
#endif

#ifndef __AudioEffect__
#include "AudioEffect.hpp"
#endif

#ifndef __aeffectx__
#include "aeffectx.h"
#endif

#ifndef __cfileselector__
#include "cfileselector.h"
#endif

#ifndef __cFileColumn_H
#include "cFileColumn.h"
#endif

#ifdef MAC
#include <dirent.h>
#include <unistd.h>
#include <Quicktime/Quicktime.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include "resource.h"

#define snprintf _snprintf
#endif

CLabel::CLabel(CRect &size, char *text) : CParamDisplay(size) {
	strcpy(label, "");
  setBackColor(kTransparentCColor);
  setFrameColor(kTransparentCColor);
	setFont(kNormalFontVerySmall);
	setFontColor(kBlackCColor);
	setLabel(text);
}

void CLabel::setLabel(char *text) {
	if(text) {
    sprintf(label, " %s", text);
	}
  setDirty();
}

void CLabel::draw(CDrawContext *pContext) {
	pContext->setFont(kNormalFontVerySmall);
	pContext->setFontColor(kBlackCColor);
	pContext->drawString(label, size, false, kLeftText);
}

///////////////////////////////////////////////////////////////////////////////

ConvolverEditor::ConvolverEditor(AudioEffect *effect) : AEffGUIEditor(effect) {
  m_opened = false;
  m_ticks = 0;
  m_error = false;

#ifdef MAC
  m_background = new CBitmap(REZ_BACKGROUND);
#elif WIN32
  m_background = new CBitmap(IDB_BITMAP1);
#endif

  rect.left = 0;
  rect.top = 0;
  rect.right = (short)m_background->getWidth();
  rect.bottom = (short)m_background->getHeight();
  
  m_file_column = NULL;
  m_dir = new char[MAX_PATH];
  memset(m_filename, 0x0, sizeof(char) * MAX_PATH);
  
  setKnobMode(kLinearMode);
}

ConvolverEditor::~ConvolverEditor() {
  if(m_background) {
    m_background->forget();
  }
  m_background = 0;
  delete [] m_dir;
}

void ConvolverEditor::close() {
#ifdef DEBUG
  fprintf(stderr, "Editor closed\n\n");
#endif
  m_opened = false;
  delete frame;
  frame = 0;
}

char* ConvolverEditor::getFilename() {
  if(!m_opened) {
    return "";
  }
  else {
    return m_filename;
  }
}

char* ConvolverEditor::getStartupPath() {
  char path[MAX_PATH];

#ifdef MAC
  snprintf(path, MAX_PATH, "%s/Library/Application Support/Teragon Audio/%s/lastdir",
           getenv("HOME"), DEF_PRODUCT_NAME);
  FILE *fp = fopen(path, "r");
  if(fp == NULL) {
    m_dir = NULL;
  }
  else {
    fgets(m_dir, MAX_PATH, fp);
    fclose(fp);
  }
#elif WINDOWS
  HKEY key;
  DWORD max = MAX_DIR;

  if(RegOpenKeyEx(HKEY_CURRENT_USER,
                  "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                  0, KEY_READ, &key) == ERROR_SUCCESS) {
    if(RegQueryValueEx(key, "AppData", NULL, NULL,
                       (LPBYTE)path, &max) == ERROR_SUCCESS) {
      HANDLE hlist;
      WIN32_FIND_DATA filedata;

      sprintf(path, "%s\\Teragon Audio\\%s\\lastdir", path, DEF_PRODUCT_NAME);
      FILE *fp = fopen(path, "r");
      if(fp == NULL) {
        strcpy(m_dir, "");
      }
      else {
        fgets(m_dir, MAX_PATH, fp);
        fclose(fp);
      }

      RegCloseKey(key);
    }
  }
#endif

  return m_dir;
}

void ConvolverEditor::idle() {
  AEffGUIEditor::idle();

  if(m_error) {
    if(getTicks() > m_ticks + DEF_ERROR_TIMEOUT) {
      m_paramlabel->setLabel("");
      m_paramvalue->setLabel("");
      m_error = false;
    }
  }
}

long ConvolverEditor::open(void *ptr) {
  AEffGUIEditor::open(ptr);
  
  CPoint point;
  CRect size(0, 0, m_background->getWidth(), m_background->getHeight());
  frame = new CFrame(size, ptr, this);
  frame->setBackground(m_background);
  
  // Source filename boxes
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y,
       DEF_INFO_BOX_X + 35, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 1));
  CLabel *filebox = new CLabel(size, "File");
  frame->addView(filebox);  
  size(DEF_INFO_BOX_X2, DEF_INFO_BOX_Y,
       DEF_INFO_BOX_X2 + 62, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 1));
  m_filelabel = new CLabel(size, "None");
  frame->addView(m_filelabel);
  
  // Frames boxes
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 1),
       DEF_INFO_BOX_X + 35, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 2));
  CLabel *framesbox = new CLabel(size, "Frames");
  frame->addView(framesbox);
  size(DEF_INFO_BOX_X2, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 1),
       DEF_INFO_BOX_X2 + 62, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 2));
  m_framelabel = new CLabel(size, "0");
  frame->addView(m_framelabel);
  
  // Filesize boxes
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 2),
       DEF_INFO_BOX_X + 35,DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 3));
  CLabel *sizebox = new CLabel(size, "Size");
  frame->addView(sizebox);
  size(DEF_INFO_BOX_X2, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 2),
       DEF_INFO_BOX_X2 + 62, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 3));
  m_filesize = new CLabel(size, "0");
  frame->addView(m_filesize);
  
  // Delay calculation boxes
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 3),
       DEF_INFO_BOX_X + 35,DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 4));
  CLabel *samplerate = new CLabel(size, "Delay");
  frame->addView(samplerate);
  size(DEF_INFO_BOX_X2,DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 3),
       DEF_INFO_BOX_X2 + 62, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 4));
  m_delaylabel = new CLabel(size, "0");
  frame->addView(m_delaylabel);
  
  // Parameter boxes
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 4),
       DEF_INFO_BOX_X + 35, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 5));
  m_paramlabel = new CLabel(size, "");
  frame->addView(m_paramlabel);
  size(DEF_INFO_BOX_X2, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 4),
       DEF_INFO_BOX_X2 + 62 , DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 5));
  m_paramvalue = new CLabel(size, "");
  frame->addView(m_paramvalue);
  
  // See if we should be actually setting info for a preloaded file
  if(((Convolver*)effect)->getAttribute(ATR_FRAMES)) {
    char name[MAX_NAME];
    ((Convolver*)effect)->getFilename(name);
    bool stereo = false;
    if(((Convolver*)effect)->getAttribute(ATR_STEREO)) {
      stereo = true;
    }
    setAttributes(name, (int)((Convolver*)effect)->getAttribute(ATR_FRAMES),
                  (int)((Convolver*)effect)->getAttribute(ATR_SIZE),
                  stereo, ((Convolver*)effect)->getAttribute(ATR_DELAY));
  }

  // File open button
#ifdef MAC
  CBitmap *bmap = new CBitmap(REZ_FILE_OPEN);
#elif WIN32
  CBitmap *bmap = new CBitmap(IDB_BITMAP2);
#endif
  size(169,241, 169 + 42,241 + 19);
  point(0, 0);
  m_file_choose_button = new CKickButton(size, this, PRM_FILE,
                                         bmap->getHeight() / 2,
                                         bmap, point);
  frame->addView(m_file_choose_button);
  bmap->forget();
  
  // Save Button
#ifdef MAC
  bmap = new CBitmap(REZ_SAVE_BUTTON);
#elif WIN32
  bmap = new CBitmap(IDB_BITMAP11);
#endif

  size(169,267, 169 + 42,267 + 19);
  m_save_button = new CKickButton(size, this, PRM_SAVE,
                                  bmap->getHeight() / 2,
                                  bmap, point);
  frame->addView(m_save_button);
  bmap->forget();
  
  // Load Button
#ifdef MAC
  bmap = new CBitmap(REZ_LOAD_BUTTON);
#elif WIN32
  bmap = new CBitmap(IDB_BITMAP10);
#endif
  size(169,292, 169 + 42,292 + 19);
  m_load_button = new CKickButton(size, this, PRM_LOAD,
                                  bmap->getHeight() / 2,
                                  bmap, point);
  frame->addView(m_load_button);
  bmap->forget();
  
  // Locut Knob
#ifdef MAC
  bmap = new CBitmap(REZ_KNOB);
#elif WIN32
  bmap = new CBitmap(IDB_BITMAP9);
#endif
  size(24,339, 24 + DEF_KNOB_HEIGHT,339 + DEF_KNOB_HEIGHT);
  m_locut_knob = new CAnimKnob(size, this, PRM_LOCUT, DEF_KNOB_FRAMES,
                               DEF_KNOB_HEIGHT, bmap, point);
  frame->addView(m_locut_knob);
  m_locut_knob->setValue(effect->getParameter(PRM_LOCUT));
  m_locut_knob->setDirty();
  
  // Gain Knob
  size(96,356, 96 + DEF_KNOB_HEIGHT,356 + DEF_KNOB_HEIGHT);
  m_gain_knob = new CAnimKnob(size, this, PRM_GAIN, DEF_KNOB_FRAMES,
                              DEF_KNOB_HEIGHT, bmap, point);
  frame->addView(m_gain_knob);
  m_gain_knob->setValue(effect->getParameter(PRM_GAIN));
  m_gain_knob->setDirty();
  
  // Hicut knob
  size(167,339, 167 + DEF_KNOB_HEIGHT, 339 + DEF_KNOB_HEIGHT);
  m_hicut_knob = new CAnimKnob(size, this, PRM_HICUT, DEF_KNOB_FRAMES,
                               DEF_KNOB_HEIGHT, bmap, point);
  frame->addView(m_hicut_knob);
  m_hicut_knob->setValue(effect->getParameter(PRM_HICUT));
  m_hicut_knob->setDirty();
  bmap->forget();

  // File browser
  size(30,28, 195,195);
  m_file_column = new cFileColumn(size, this, PRM_FILECOLUMN, NULL, frame);
  getStartupPath();
  m_file_column->setDirectory(m_dir);
  
  // About screen
#ifdef MAC
  bmap = new CBitmap(REZ_SPLASHSCREEN);
#elif WIN32
  bmap = new CBitmap(IDB_BITMAP12);
#endif
  size(0,433, 240,450);
  point(0, 0);
  CRect display_area(0, 0, bmap->getWidth(), bmap->getHeight());
  display_area.offset(0, 0);
  m_splashscreen = new CSplashScreen(size, this, PRM_SPLASH, bmap, display_area, point);
  frame->addView(m_splashscreen);
  bmap->forget();
  
  m_opened = true;
  return true;
}

void ConvolverEditor::presetRead(char *fname) {
  FILE *fp = fopen(fname, "r");
  if(fp == NULL) {
    setError("Invalid file");
    return;
  }
  char *linebuf = new char[MAX_PATH];
  fgets(linebuf, MAX_PATH, fp);
#ifdef DEBUG
  long num_params = 
#endif
    atoi(linebuf);
#ifdef DEBUG
  fprintf(stderr, "Found %ld parameters\n", num_params);
#endif
  
  long param = 0;
  while(fgets(linebuf, MAX_PATH, fp)) {
    linebuf[strlen(linebuf) - 1] = '\0';
    switch(param) {
      case PRM_FILE:
        ((Convolver*)effect)->loadFile(linebuf);
        break;
      case PRM_GAIN:
        setParameter(param, atof(linebuf));
        break;
      case PRM_FILEBOX:
        break;
      case PRM_LOCUT:
        setParameter(param, atof(linebuf));
        break;
      case PRM_HICUT:
        setParameter(param, atof(linebuf));
        break;
      case PRM_SAVE:
        break;
      case PRM_LOAD:
        break;
      case PRM_SPLASH:
        break;
        /*
      case PRM_DOWNSAMPLE_ENABLE:
        break;
      case PRM_DOWNSAMPLE_RATE:
        break;
        */
      default:
        setError("Unknown param");
        break;
    }
    ++param;
  }
  
  delete [] linebuf;
  fclose(fp);
}

void ConvolverEditor::presetWrite(char *fname) {
  FILE *fp = fopen(fname, "w");
  if(fp == NULL) {
    setError("Invalid file");
    return;
  }
  char *text = new char[MAX_PATH];
  fprintf(fp, "%d\n", NUM_PARAMS);
  
  for(long i = 0; i < NUM_PARAMS; ++i) {   
    switch(i) {
      case PRM_FILE:
        effect->getParameterDisplay(i, text);
        fprintf(fp, "%s\n", text);
        break;
      case PRM_GAIN:
        fprintf(fp, "%f\n", effect->getParameter(PRM_GAIN));
        break;
      case PRM_FILEBOX:
        fprintf(fp, "0.0\n");
        break;
      case PRM_LOCUT:
        fprintf(fp, "%f\n", effect->getParameter(PRM_LOCUT));
        break;
      case PRM_HICUT:
        fprintf(fp, "%f\n", effect->getParameter(PRM_HICUT));
        break;
      case PRM_SAVE:
        fprintf(fp, "0.0\n");
        break;
      case PRM_LOAD:
        fprintf(fp, "0.0\n");
        break;
      case PRM_SPLASH:
        fprintf(fp, "0.0\n");
        break;
        /*
      case PRM_DOWNSAMPLE_ENABLE:
        fprintf(fp, "0.0\n");
        break;
      case PRM_DOWNSAMPLE_RATE:
        fprintf(fp, "0.0\n");
        break;
        */
      default:
        setError("Unknown param");
        break;
    }
  }
  delete [] text;
  fclose(fp);
}

void ConvolverEditor::setAttributes(char *text, int frames, int size, bool stereo, float delay) {
  char *p = strrchr(text, DEF_DELIMITER);
  if(p) {
    m_filelabel->setLabel(p + 1);
  }
  else {
    m_filelabel->setLabel(text);
  }
  
  char s[12];
  sprintf(s, "%d Kb", size);
  m_filesize->setLabel(s);
  
  sprintf(s, "%d ", frames);
  if(stereo) {
    strcat(s, "(2)");
  }
  else {
    strcat(s, "(1)");
  }
  m_framelabel->setLabel(s);
  sprintf(s, "%2.04f sec", delay);
  m_delaylabel->setLabel(s);
}

void ConvolverEditor::setFilename(char *text) {
  strncpy(m_filename, text, MAX_PATH);
}

void ConvolverEditor::setError(char *text) {
  m_error = true;
  m_ticks = getTicks();
  m_paramlabel->setLabel("ERROR");
  m_paramvalue->setLabel(text);
}

void ConvolverEditor::setParameter(long index, float value) {
  if(!m_opened) {
    return;
  }
  
  switch(index) {
    case PRM_FILESCROLL:
      m_file_column->scroll(value);
      break;
    case PRM_LOAD:
      {
        CFileSelector *fs = new CFileSelector(effect);
        VstFileSelect *vfs = new VstFileSelect;
        char *path = new char[MAX_PATH];
        path = getenv("HOME");
        char *ret = new char[MAX_PATH];
        long num;
        
#ifdef MAC
        Handle filename;
        CFStringRef tmp_string;
        CFURLRef tmp_url;
        CFStringRef tmp_path;
        char *fix;
#endif
        
#ifdef WINDOWS
        VstFileType vft("Impulse Response Settings", "irs", "irs", 0, 0, 0);
#endif
        vfs->command = kVstFileLoad;
        vfs->type = kVstFileType;
#ifdef WINDOWS
        vfs->nbFileTypes = 1;
#elif MAC
        vfs->nbFileTypes = 0;
#endif
        vfs->macCreator = 0;
        vfs->fileTypes = &vft;
        strcpy(vfs->title, "Load User Preset...");
        vfs->initialPath = path;
        vfs->returnPath = NULL;
        vfs->sizeReturnPath = MAX_PATH;
        num = fs->run(vfs);
        
        if(num) {
#ifdef MAC
          filename = NewHandle(MAX_PATH);
          HRUtilGetURLFromFSSpec((FSSpec*)vfs->returnPath, filename);
          tmp_string = CFStringCreateWithCString(NULL, *filename, kCFStringEncodingASCII);
          tmp_url = CFURLCreateWithString(NULL, tmp_string, NULL);
          tmp_path = CFURLCopyPath(tmp_url);
          CFStringGetCString(tmp_path, ret, MAX_PATH, kCFStringEncodingASCII);
          fix = strchr(ret + 1, '/');
          strncpy(ret, fix, MAX_PATH);
#elif WINDOWS
          strncpy(ret, vfs->returnPath, MAX_PATH);
#endif

          presetRead(ret);
        }
      }
      break;
    case PRM_SAVE:
      /* TODO: Automatic file naming, someday when we're not so lazy
      if(((AudioEffectX*)effect)->canHostDo("openFileSelector") <= 0) {
        char *path = new char[MAX_PATH];
        ((Convolver*)effect)->getDefaultPresetPath(path);
        presetWrite(path);
      }
      */
      {
        CFileSelector *fs = new CFileSelector(effect);
        VstFileSelect *vfs = new VstFileSelect;
        char *path = new char[MAX_PATH];
        path = getenv("HOME");
        char *ret = new char[MAX_PATH];
        long num = 0;

#ifdef MAC
        Handle filename;
        CFStringRef tmp_string;
        CFURLRef tmp_url;
        CFStringRef tmp_path;
        char *fix;
#endif
        
#ifdef WINDOWS
        VstFileType vft("Impulse Response Settings", "irs", "irs", 0, 0, 0);
#endif
        vfs->command = kVstFileSave;
        vfs->type = kVstFileType;
#ifdef WINDOWS
        vfs->nbFileTypes = 1;
#elif MAC
        vfs->nbFileTypes = 0;
#endif
        vfs->macCreator = 0;
        vfs->fileTypes = &vft;
        strcpy(vfs->title, "Save User Preset...");
        vfs->initialPath = path;
        vfs->returnPath = NULL;
        vfs->sizeReturnPath = MAX_PATH;
        num = fs->run(vfs);
        
        if(num) {
#ifdef MAC
          filename = NewHandle(MAX_PATH);
          HRUtilGetURLFromFSSpec((FSSpec*)vfs->returnPath, filename);
          tmp_string = CFStringCreateWithCString(NULL, *filename, kCFStringEncodingASCII);
          tmp_url = CFURLCreateWithString(NULL, tmp_string, NULL);
          tmp_path = CFURLCopyPath(tmp_url);
          CFStringGetCString(tmp_path, ret, MAX_PATH, kCFStringEncodingASCII);
          fix = strchr(ret + 1, '/');
          strncpy(ret, fix, MAX_PATH);
#elif WINDOWS
          strncpy(ret, vfs->returnPath, MAX_PATH);
#endif
          presetWrite(ret);
        }

        delete [] path;
        delete [] ret;
      }
      break;
    case PRM_FILE:
      {
        char filename[MAX_PATH];
        strcpy(filename, m_file_column->getClicked());

 #if WINDOWS
        // See if we need to change drives
        if(!strncmp(filename, DEF_DRIVE_NAME, strlen(DEF_DRIVE_NAME))) {
          m_file_column->setRoot(filename[strlen(DEF_DRIVE_NAME)]);
          m_file_column->getRoot(filename);
        }
#endif

        // Check for special directories (parent, current)
        char *ch = strrchr(filename, DEF_DELIMITER);
        if(ch == NULL) {
          break;
        }
        else {
          // Check for current directory
          if(!strncmp(ch + 1, DEF_CURRENT_DIR_NAME, strlen(DEF_CURRENT_DIR_NAME))) {
            break;
          }
        
          // Check for parent directory
          if(!strncmp(ch + 1, DEF_PARENT_DIR_NAME, strlen(DEF_PARENT_DIR_NAME))) {
            *ch = '\0';
            ch = strrchr(filename, DEF_DELIMITER);
            if(ch != NULL) {
              *ch = '\0';
              m_file_column->setDirectory(filename);
              setStartupPath(m_file_column->getDirectory());
              break;
            }
          }
        }
        
        struct stat st;
        if(stat(filename, &st) != 0) {
#ifdef DEBUG
          fprintf(stderr, "Could not open %s for analysis\n", filename);
          break;
#endif
        }
        if(st.st_mode & S_IFDIR) {
#ifdef DEBUG
          fprintf(stderr, "Opening directory %s\n", filename);
#endif
          m_file_column->setDirectory(filename);
          setStartupPath(m_file_column->getDirectory());
        }
        else if(st.st_mode & S_IFREG) {
#ifdef DEBUG
          fprintf(stderr, "Opening file %s\n", filename);
#endif
          setFilename(filename);
          effect->setParameter(PRM_FILE, 1.00);
        }
        else {
#ifdef DEBUG
          fprintf(stderr, "Unknown file type %d\n", st.st_mode);
#endif
        }
      }
      break;
    case PRM_GAIN:
      effect->setParameter(PRM_GAIN, value);
      setStatus("Gain", ((Convolver*)effect)->getDb(value), "dB");
      m_gain_knob->setValue(value);
      m_gain_knob->setDirty();
      break;
    case PRM_HICUT:
      effect->setParameter(PRM_HICUT, value);
      setStatus("Freq", ((Convolver*)effect)->getFreq(PRM_HICUT), "Hz");
      m_hicut_knob->setValue(value);
      m_hicut_knob->setDirty();
      break;
    case PRM_LOCUT:
      effect->setParameter(PRM_LOCUT, value);
      setStatus("Freq", ((Convolver*)effect)->getFreq(PRM_LOCUT), "Hz");
      m_locut_knob->setValue(value);
      m_locut_knob->setDirty();
      break;
    default:
      break;
  }
  
  postUpdate();
}

void ConvolverEditor::setStartupPath(char *path) {
  char *dir = new char[MAX_DIR];
  ((Convolver*)effect)->getProductDir(dir);
  strncat(dir, "lastdir", MAX_DIR);

  FILE *fp = fopen(dir, "w");
  if(fp != NULL) {
    fprintf(fp, "%s", path);
    fclose(fp);
  }

  delete [] dir;
}

void ConvolverEditor::setStatus(char *param, float value, char *label) {
  m_error = true;
  m_ticks = getTicks();
  m_paramlabel->setLabel(param);
  char display[12];
  snprintf(display, 12, "%3.02f %s", value, label);
  m_paramvalue->setLabel(display);
}

void ConvolverEditor::valueChanged(CDrawContext* context, CControl* control) {
  float value = control->getValue();
  switch(control->getTag()) {
    case PRM_FILESCROLL:
      setParameter(PRM_FILESCROLL, value);
      break;
    case PRM_FILECLICK:
      if(value < 0) {
        // Hack for double click support
        setParameter(PRM_FILE, (int)(value * -1));
      }
      else if((value - (int)value) > 0) {
        // Hack for scrolling support from browser
        setParameter(PRM_FILESCROLL, 1.0 / (value - (int)value));
      }
      else {
        m_file_column->selectRow((int)value);
      }
      break;
    case PRM_SAVE:
      if(value) {
        setParameter(PRM_SAVE, value);
      }
      break;
    case PRM_LOAD:
      if(value) {
        setParameter(PRM_LOAD, value);
      }
      break;
    case PRM_GAIN:
      setParameter(PRM_GAIN, value);
      break;
    case PRM_HICUT:
      setParameter(PRM_HICUT, value);
      break;
    case PRM_LOCUT:
      setParameter(PRM_LOCUT, value);
      break;
    case PRM_FILE:
      if(value) {
        setParameter(PRM_FILE, value);
      }
      break;
    default:
      break;
  }
}
