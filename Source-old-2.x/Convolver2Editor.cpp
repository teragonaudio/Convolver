/*
 *  Convolver2 - Convolver2Editor.cpp
 *  Created by Nik Reiman on 23.03.06
 *  Copyright (c) 2006 Teragon Audio, All rights reserved
 */

#ifndef __Convolver2Editor_H
#include "Convolver2Editor.h"
#endif

// VSTGUI implementation //////////////////////////////////////////////////////////////

#if PLUGGUI
Convolver2Editor::Convolver2Editor(void *effect) :
#else
Convolver2Editor::Convolver2Editor(AudioEffect *effect) :
#endif
AEffGUIEditor(effect),
events(0) {
  m_opened = false;
  m_error = false;
  
  m_background = new CBitmap(REZ_BACKGROUND);
  
  rect.left = 0;
  rect.top = 0;
  rect.right = (short)m_background->getWidth();
  rect.bottom = (short)m_background->getHeight();
  
  m_file_column = NULL;
  memset(m_filename, 0x0, sizeof(char) * MAX_PATH_SIZE);
  core = 0;

  setKnobMode(kLinearMode);
}

Convolver2Editor::~Convolver2Editor() {
  if(m_background) {
    m_background->forget();
  }
  m_background = 0;
}

void Convolver2Editor::close() {  
  m_opened = false;
  delete frame;
  frame = 0;
}

void Convolver2Editor::idle() {
  AEffGUIEditor::idle();
  events.update(getTicks());
}

void Convolver2Editor::setAttributes() {
  if(!m_opened) {
    return;
  }
  
  char buffer[MAX_PATH_SIZE];
  memset(buffer, 0x0, sizeof(char) * MAX_PATH_SIZE);
  ((Convolver2Core*)core)->getFilename((char*)&buffer);
  m_filelabel->setLabel(buffer);
  m_framelabel->setLabel(((Convolver2Core*)core)->getFrames());
  ((Convolver2Core*)core)->getSize((char*)&buffer);
  m_filesize->setLabel(buffer);
  m_delaylabel->setLabel(((Convolver2Core*)core)->getDelay());
  
  m_gain_knob->setValue(core->getParameterNormalized(PRM_GAIN));
  m_wetdry_knob->setValue(core->getParameterNormalized(PRM_WETDRY));
  m_filter_knob->setValue(core->getParameterNormalized(PRM_FILTER_FREQ));
}

#ifdef USE_PC_AU
void Convolver2Editor::setAU(AudioUnit unit) {
  au = unit;
}
#endif

void Convolver2Editor::setCore(pluginCore *p) {
  if(!core && p) {
    core = p;
  }
  else {
    core = 0;
  }
}

void Convolver2Editor::setInfoBox(char *text) {
  m_info_box->setLabel(text);
  events.addEvent(m_info_box, DEF_INFO_BOX_TIMEOUT, 0, true);
}

void Convolver2Editor::setInfoBox(char *text, float value, char *label) {
  char ftext[MAX_LABEL_SIZE];
  snprintf(ftext, MAX_LABEL_SIZE, "%s: %2.1f", text, value);
  if(label) {
    if((int)value / 1000 > 0) {
      snprintf(ftext, MAX_LABEL_SIZE, "%s: %2.2f k%s", text, value / 1000.0f, label);
    }
    else {
      snprintf(ftext, MAX_LABEL_SIZE, "%s: %2.1f %s", text, value, label);
    }
  }
  else {
    snprintf(ftext, MAX_LABEL_SIZE, "%s: %2.1f", text, value);
  }
  setInfoBox(ftext);
}

#if PLUGGUI
void Convolver2Editor::setParameter(long index, float value) {
#else
void Convolver2Editor::setParameter(VstInt32 index, float value) {
#endif
  if(!m_opened) {
    return;
  }
  
  switch(index) {
    case TAG_FILEBROWSER_CLICK:
    {
      char filename[MAX_PATH_SIZE];
      m_file_column->getClicked((char*)&filename);
      
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
        break;
      }
      if(st.st_mode & S_IFDIR) {
        m_file_column->setDirectory(filename);
        setStartupPath(m_file_column->getDirectory());
      }
      else if(st.st_mode & S_IFREG) {
        if(((Convolver2Core*)core)->loadFile(filename)) {
          setAttributes();
        }
      }
      else {
        // Unknown file type
      }
    }
      break;
    case PRM_FILTER_FREQ:
    case PRM_FILTER_TYPE:
      break;
    default:
#ifdef USE_PC_AU
      core->setParameterNormalized(index, value);
      AudioUnitParameter param;
      param.mAudioUnit = au;
      param.mParameterID = index;
      param.mScope = kAudioUnitScope_Global;
      param.mElement = 0;
      AUParameterListenerNotify(NULL, NULL, &param);
#endif
#ifdef USE_PC_VST
      effect->setParameterAutomated(index, value);
#endif
      break;
  }
}

void Convolver2Editor::setStartupPath(char *path) {
  char dir[MAX_PATH_SIZE];
  getProductDir(DEF_PRODUCT, (char*)&dir);
  strncat(dir, "lastdir", MAX_DIR);
  
  FILE *fp = fopen(dir, "w");
  if(fp != NULL) {
    fprintf(fp, "%s", path);
    fclose(fp);
  }
}

void Convolver2Editor::valueChanged(CDrawContext* context, CControl* control) {
  float value = control->getValue();
  
  switch(control->getTag()) {
    case TAG_FILEBROWSER_SCROLL:
      m_file_column->scroll(m_file_column->getScrollbarHandle()->getValue());
      break;
    case TAG_FILEBROWSER_CLICK:
      if(value < 0) {
        // Double click has been received
        m_file_column->selectRow((int)value * -1);
        setParameter(TAG_FILEBROWSER_CLICK, value * -1);
      }
      else {
        // Single click received; just select the row
        m_file_column->selectRow((int)value);
      }
      m_file_column->setDirty();
      return;
      break;
    case TAG_FILE_SAVE:
      setInfoBox("Disabled");
      break;
    case TAG_FILE_LOAD:
      setInfoBox("Disabled");
      break;
    case PRM_GAIN:
      core->setParameterNormalized(PRM_GAIN, value);
      setInfoBox("Gain", core->getParameter(REQ_VALUE_RAW, PRM_GAIN), "dB");
      break;
    case PRM_WETDRY:
      core->setParameterNormalized(PRM_WETDRY, value);
      if(value < 0.5) {
        setInfoBox("Dry", (1.0f - core->getParameter(REQ_VALUE_RAW, PRM_WETDRY)) * 100.0f, "%");
      }
      else {
        setInfoBox("Wet", core->getParameter(REQ_VALUE_RAW, PRM_WETDRY) * 100.0f, "%");
      }
      break;
    case PRM_FILTER_FREQ:
      core->setParameterNormalized(PRM_FILTER_FREQ, value);
      setInfoBox("Filter Freq", core->getParameter(REQ_VALUE_RAW, PRM_FILTER_FREQ), "Hz");
      ((Convolver2Core*)core)->recache();
      break;
    case PRM_FILTER_TYPE:
    {
      core->setParameter(PRM_FILTER_TYPE, value);
      ((Convolver2Core*)core)->recache();
      char buf[MAX_LABEL_SIZE];
      snprintf(buf, MAX_LABEL_SIZE, "Filter type: %s", _FILT_NAMES[(int)value]);
      setInfoBox(buf);
    }
      break;
    default:
      break;
  }
  
  setParameter(control->getTag(), value);
}

// AU VSTGUI hooks ///////////////////////////////////////////////////////////////////////

#ifdef USE_PC_AU
COMPONENT_ENTRY(Convolver2View);

Convolver2View::Convolver2View(AudioUnitCarbonView auv) 
: AUCarbonViewBase(auv), editor(0), xOffset(0), yOffset(0) {
}

Convolver2View::~Convolver2View() {
  if(editor) {
    editor->close();
  }
}

void Convolver2View::RespondToEventTimer(EventLoopTimerRef inTimer) {
  if(editor) {
    editor->doIdleStuff();
  }
}

OSStatus Convolver2View::CreateUI(Float32 xoffset, Float32 yoffset) {
  AudioUnit unit = GetEditAudioUnit();
  if(unit) {
    editor = new Convolver2Editor(unit);
    editor->setAU(unit);
    
    UInt32 size;
    Boolean write;
    if(AudioUnitGetPropertyInfo(unit, PRP_CORE, kAudioUnitScope_Global, 0, &size, &write) == noErr) {
      pluginCore *p[1];
      if(AudioUnitGetProperty(unit, PRP_CORE, kAudioUnitScope_Global, 0, p, &size) == noErr) {
        editor->setCore(p[0]);
      }
      else {
        // Core couldn't be set...
        return -1;
      }
    }
    else {
      // Core property information couldn't be obtained
      return -1;
    }
    
    if(AudioUnitSetProperty(unit, PRP_EDITOR, kAudioUnitScope_Global, 0,
                            editor, sizeof(Convolver2Editor*)) != noErr) {
      // Something failed here..
      return -1;
    }
    
    WindowRef window = GetCarbonWindow();
    editor->open(window);
    HIViewMoveBy((HIViewRef)editor->getFrame()->getPlatformControl(), xoffset, yoffset);
    EmbedControl((HIViewRef)editor->getFrame()->getPlatformControl());
    CRect fsize = editor->getFrame()->getViewSize(fsize);
    SizeControl(mCarbonPane, fsize.width(), fsize.height());
    CreateEventLoopTimer(kEventDurationSecond, kEventDurationSecond / 24);
    HIViewSetVisible((HIViewRef)editor->getFrame()->getPlatformControl(), true);
    HIViewSetNeedsDisplay((HIViewRef)editor->getFrame()->getPlatformControl(), true);
  }
  
  return noErr;
}
#endif