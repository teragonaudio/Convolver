/*
 *  cFileColumn.cpp
 *  Convolver
 *
 *  Created by Nik Reiman on 13.10.05.
 *  Copyright 2005 Teragon Audio. All rights reserved.
 *
 */

#ifndef __cFileColumn_H
#include "cFileColumn.h"
#endif

cFileIcon::cFileIcon(const CRect &size, CControlListener *listener,
                     long tag, CBitmap *pBackground, int index, CPoint& point) :
CMovieBitmap(size, (CControlListener*)listener, tag, DEF_NUM_ICONS,
             (CCoord)DEF_ROW_HEIGHT, pBackground, point) {
  m_index = index;
}

cFileIcon::~cFileIcon() {
}

void cFileIcon::setType(long tag) {
  switch(tag) {
    case FILE_TYPE_FOLDER:
      CMovieBitmap::setValue(0.0f);
      break;
    case FILE_TYPE_OTHERFILE:
      CMovieBitmap::setValue(0.25f);
      break;
    case FILE_TYPE_AUDIOFILE:
      CMovieBitmap::setValue(0.45f);
      break;
    case FILE_TYPE_AUDIOFILEBIG:
      CMovieBitmap::setValue(0.65f);
      break;
    case FILE_TYPE_EMPTY:
      CMovieBitmap::setValue(1.0f);
      break;
      
    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////

cFileLabel::cFileLabel(CRect &size, long tag, int index, CControlListener *listener) :
CParamDisplay(size) {  
  setTag(tag);
  setListener(listener);
  setLabel("");
  
  setBackColor(kTransparentCColor);
  setFrameColor(kTransparentCColor);
	setFont(kNormalFontVerySmall);
	setFontColor(kBlackCColor);
  
  m_index = index;
  value = (float)m_index;
  m_selected = false;
}

cFileLabel::~cFileLabel() {
}

void cFileLabel::draw(CDrawContext *pContext) {
  if(m_selected) {
    // This transparency doesn't completely work on windows for some reason..
    // but whatever, it still looks alright
    CColor highlight = {255, 255, 255, 100};
    pContext->setFillColor(highlight);
    pContext->fillRect(size);
    pContext->setFrameColor(kBlackCColor);
    pContext->drawRect(size);
  }
  
	pContext->setFont(kNormalFontSmall);
	pContext->setFontColor(kBlackCColor);
	pContext->drawString(m_label, size, false, kLeftText);
  setDirty(false);
}

char* cFileLabel::getName() const {
  return (char*)&m_label;
}

float cFileLabel::getValue() const {
  return value;
}

void cFileLabel::mouse(CDrawContext *context, CPoint &where, long buttons) {
  if(listener && buttons == 1) {
    if(isDoubleClick()) {
      value = (float)m_index * -1.0f;
      listener->valueChanged((CDrawContext*)context, (CControl*)this);    
    }
    else {
      value = (float)m_index;
      listener->valueChanged((CDrawContext*)context, (CControl*)this);
    }
  }
}

bool cFileLabel::onWheel(CDrawContext *pContext, const CPoint &where, float distance) {
  if(m_scrollbar) {
    return m_scrollbar->onWheel(pContext, where, distance);
  }
  return false;
}

void cFileLabel::setClicked(bool on) {
  m_selected = on;
  setDirty();
}

void cFileLabel::setLabel(char *text) {
	if(text) {
    strncpy(m_label, text, MAX_LABEL_LENGTH);
	}
  setDirty();
}

void cFileLabel::setScrollbar(CVerticalSlider *vs) {
  m_scrollbar = vs;
}

///////////////////////////////////////////////////////////////////////

cFileRow::cFileRow(const CRect &size, CControlListener *listener,
                   long tag, CBitmap *pBackground, CFrame *frame,
                   int index) :
CControl(size, listener, tag, pBackground) {
  m_frame = frame;
  m_index = index;
  m_selected = false;
  
  CPoint point(0, 0);
  setMouseableArea(size);
  setMouseEnabled(true);
  CRect tmp_size(size.left, size.top, size.left + pBackground->getWidth(),
                 size.top + (pBackground->getHeight() / DEF_NUM_ICONS));
  m_type_button = new cFileIcon(tmp_size, 
                                (CControlListener*)listener,
                                tag, (CBitmap*)pBackground,
                                index, point);
  m_type_button->setValue(0.0);
  tmp_size(size.left + pBackground->getWidth(), size.top,
           size.right, size.bottom);
  
  m_file_label = new cFileLabel(tmp_size, TAG_FILEBROWSER_CLICK, m_index, listener);
  setType(tag);
}

cFileRow::~cFileRow() {
  delete m_type_button;
  delete m_file_label;
}

void cFileRow::add() {
  m_frame->addView(m_type_button);
  m_frame->addView(m_file_label);
}

void cFileRow::draw(CDrawContext *pContext) {
  m_type_button->draw(pContext);
  m_file_label->draw(pContext);
  setDirty(false);
}

bool cFileRow::getClicked() const {
  return m_selected;
}

int cFileRow::getIndex() const {
  return m_index;
}

char* cFileRow::getName() const {
  return m_file_label->getName();
}

int cFileRow::getType() const {
  return m_filetype;
}

void cFileRow::setClicked(bool on) {
  if(m_filetype != FILE_TYPE_EMPTY) {
    m_selected = on;
    m_file_label->setClicked(on);
  }
  setDirty(true);
}

void cFileRow::setName(char *name) {
  m_file_label->setLabel(name);
}

void cFileRow::setScrollbar(CVerticalSlider *vs) {
  m_file_label->setScrollbar(vs);
}

void cFileRow::setType(long type) {
  m_filetype = type;
  m_type_button->setType(type);
}

///////////////////////////////////////////////////////////////////////

cFileColumn::cFileColumn(const CRect &size, CControlListener *listener,
                         long tag, CBitmap *pBackground, CFrame *frame) :
CControl(size, listener, tag, pBackground) {
  m_coords = size;
  m_frame = frame;
  
  m_type_bitmap = new CBitmap(REZ_FILE_ICONS);
  m_max_rows = (int)((m_coords.bottom - m_coords.top) / DEF_ROW_HEIGHT);
  m_coords.bottom = (m_max_rows * DEF_ROW_HEIGHT) + m_coords.top;
  setMouseEnabled(true);
  
  CPoint point(0, 0);
  CBitmap *handle = new CBitmap(REZ_SCROLLBAR_HANDLE);
  CBitmap *bg = new CBitmap(REZ_SCROLLBAR_BG);
  
  // Dynamic scrollbar sizing... not being used because it's not precise enough
  // for this particular implementation
	// CRect bar_size(0,0, bg->getWidth(), m_coords->bottom - m_coords->top);
	// bar_size.offset(m_coords->right - bg->getWidth(), m_coords->top);
  
  CRect bar_size(203,29, 220,196);
  CPoint handleOffset(0, 0);
	m_scrollbar = new CVerticalSlider(bar_size, listener, 
                                    TAG_FILEBROWSER_SCROLL, handleOffset, 
                                    bar_size.height() - (2 * handleOffset.v),
                                    handle, bg, point, kBottom);
  m_scrollbar->setMin(0.0);
  m_scrollbar->setMax(1.0);
  m_scrollbar->setDefaultValue(1.0);
  m_scrollbar->setValue(1.0);
  m_scrollbar->setFreeClick(false);
	frame->addView(m_scrollbar);
  
	bg->forget();
	handle->forget();
  
  memset(m_dir, 0x0, sizeof(char) * MAX_FILE_LENGTH);
  strncpy(m_root, DEF_ROOT, MAX_ROOT);
  m_scroll_buffer = NULL;
}

cFileColumn::~cFileColumn() {
  m_type_bitmap->forget();
  delete m_type_bitmap;
  delete m_scrollbar;
  if(m_scroll_buffer) {
    delete [] m_scroll_buffer;
  }
  
  for(unsigned int i = 0; i < m_rows.size(); ++i) {
    delete m_rows.at(i);
  }
}

void cFileColumn::add(long type, char *name) {
  CRect *r = getCoords(m_rows.size());  
  cFileRow *row = new cFileRow(*r, (CControlListener*)listener,
                               (long int)type, m_type_bitmap, m_frame, m_rows.size());
  row->setScrollbar(m_scrollbar);
  if(type != FILE_TYPE_EMPTY) {
    row->setName(name);
  }
  
  if(row->getIndex() < m_max_rows) {
    row->add();
  }
  m_rows.push_back(row);
  ++m_num_rows;
}

void cFileColumn::draw(CDrawContext *dc) {
  for(int i = 0; i < m_max_rows; ++i) {
    m_rows.at(i)->draw(dc);
  }
  setDirty(false);
}

void cFileColumn::getClicked(char* text) const {
  for(unsigned int i = 0; i < m_rows.size(); ++i) {
    if(m_rows.at(i)->getClicked()) {
      if(m_dir[0] == '\0') {
        strncpy(text, m_root, MAX_FILE_LENGTH);
      }
      else {
#if WINDOWS
        if(!strncmp(m_rows.at(i)->getName(), DEF_DRIVE_NAME, strlen(DEF_DRIVE_NAME))) {
          _snprintf(text, MAX_FILE_LENGTH, "%s:%c",
                    m_rows.at(i)->getName() + strlen(DEF_DRIVE_NAME), DEF_DELIMITER);
        }
        else {
#endif
          snprintf(text, MAX_FILE_LENGTH, "%s%c%s", m_dir, DEF_DELIMITER, m_rows.at(i)->getName());
#if WINDOWS
        }
#endif
      }
      return;
    }
  }
}

CRect* cFileColumn::getCoords(int num) const {
  CRect *size;
  if(num > m_max_rows) {
    size = new CRect(0,0, 0,0);
  }
  else {
    size = new CRect(m_coords.left, m_coords.top + (num * DEF_ROW_HEIGHT),
                     m_coords.right, m_coords.top + ((num + 1) * DEF_ROW_HEIGHT));
  }
  return size;
}

char* cFileColumn::getDirectory() const {
  return (char*)&m_dir;
}

CVerticalSlider* cFileColumn::getScrollbarHandle() const {
  return m_scrollbar;
}

void cFileColumn::getRoot(char *text) const {
  strncpy(text, m_root, MAX_ROOT);
}

void cFileColumn::scroll(float position) {
  if(m_num_rows <= m_max_rows) {
    return;
  }
  
  float ticks = 1.0 / (float)(m_rows.size() - m_max_rows + 1);
  unsigned int index = (int)((1.0 - position) / ticks);
  if(index + m_max_rows > m_rows.size()) {
    return;
  }
  
  // Scrolling down
  if(index > m_last_index) {
    int diff = index - m_last_index;
    
    // Copy scrolled out rows into temporary buffer
    if(m_scroll_buffer) {
      delete [] m_scroll_buffer;
    }
    m_scroll_buffer = new struct rowBuf[diff];
    
    for(int i = 0; i < diff; ++i) {
      strncpy(m_scroll_buffer[i].name, m_rows.at(i)->getName(), MAX_LABEL_LENGTH);
      m_scroll_buffer[i].type = m_rows.at(i)->getType();
    }
    
    // Move the rest of the data forward
    for(unsigned int i = diff, j = 0; i < m_rows.size(); ++i, ++j) {
      m_rows.at(j)->setName(m_rows.at(i)->getName());
      m_rows.at(j)->setType(m_rows.at(i)->getType());
    }
    
    // Copy buffer back onto the tail
    for(int i = 0, j = m_rows.size() - diff; i < diff; ++i, ++j) {
      m_rows.at(j)->setName(m_scroll_buffer[i].name);
      m_rows.at(j)->setType(m_scroll_buffer[i].type);
    }
    
    m_last_index = index;
  }
  // Scrolling up
  else if(index < m_last_index) {
    int diff = m_last_index - index;
    
    if(m_scroll_buffer) {
      delete [] m_scroll_buffer;
    }
    m_scroll_buffer = new struct rowBuf[diff];
    
    // Copy tail to buffer
    for(int i = m_rows.size() - diff, j = 0; j < diff; ++i, ++j) {
      strncpy(m_scroll_buffer[j].name, m_rows.at(i)->getName(), MAX_LABEL_LENGTH);
      m_scroll_buffer[j].type = m_rows.at(i)->getType();
    }
    
    // Move the tail data forward
    for(int i = m_rows.size() - diff - 1, j = m_rows.size() - 1;
        i >= 0; --i, --j) {
      m_rows.at(j)->setName(m_rows.at(i)->getName());
      m_rows.at(j)->setType(m_rows.at(i)->getType());
    }
    
    // Copy buffer back onto the tail
    for(int i = 0; i < diff; ++i) {
      m_rows.at(i)->setName(m_scroll_buffer[i].name);
      m_rows.at(i)->setType(m_scroll_buffer[i].type);
    }
    
    m_last_index = index;
  }
  // Parameter is between scrollpoints
  else {
  }
  
  // Deselect everybody.  Sorry.
  for(int i = 0; i < m_max_rows; ++i) {
    m_rows.at(i)->setClicked(false);
  }
}

void cFileColumn::selectRow(int row) {
  if(row > m_max_rows || row < 0) {
    return;
  }
  
  for(unsigned int i = 0; i < m_rows.size(); ++i) {
    if(i == row) {
      m_rows.at(i)->setClicked(true);
    }
    else {
      m_rows.at(i)->setClicked(false);
    }
  }
}

void cFileColumn::setDirectory(char *dir) {
  // Clear out all text manually, because otherwise the transparency gets fucked up... stupid VSTGUI
  for(unsigned int i = 0; i < m_rows.size(); ++i) {
    m_rows.at(i)->setName("");
    m_rows.at(i)->setClicked(false);
    m_rows.at(i)->setType(FILE_TYPE_EMPTY);
  }
  m_rows.clear();
  
  char *ch = 0;
  char flabel[MAX_LABEL_LENGTH];
  
  if(dir != NULL) {
    ch = strrchr(dir, DEF_DELIMITER);
  }
  
  m_num_rows = 0;
  // Top level directory -- do special listing
  if(ch == NULL) { 
    snprintf(flabel, MAX_LABEL_LENGTH, "%s(Top Level)", DEF_CURRENT_DIR_NAME);
    add(FILE_TYPE_FOLDER, flabel);
    
    memset(flabel, 0x0, MAX_LABEL_LENGTH * sizeof(char));
    snprintf(flabel, MAX_LABEL_LENGTH, "%s(None)", DEF_PARENT_DIR_NAME);
    add(FILE_TYPE_FOLDER, flabel);
#ifdef MAC
    return setDirectory("/Volumes");
#elif WINDOWS
    // Display the drive list for windows
    DWORD d = GetLogicalDrives();
    for(int i = 0; i < MAX_DRIVES; ++i) {
	    if((1 << i)&d) {
        snprintf(flabel, MAX_LABEL_LENGTH, "%s%c", DEF_DRIVE_NAME, 'A' + i);
        add(FILE_TYPE_FOLDER, flabel);
	    }
    }
    return;
#endif
  }
  else {
    // Get the parent directory
    char tmp_dir[MAX_FILE_LENGTH];
    int s = strlen(dir) - strlen(ch);
    strncpy(tmp_dir, dir, s);
    tmp_dir[s] = '\0';
    
    char *ch2 = strrchr(tmp_dir, DEF_DELIMITER);
    if(ch2 == NULL || s <= 1) { // Top level directory, for windows
      strncpy(m_root, tmp_dir, MAX_ROOT);
      snprintf(flabel, MAX_LABEL_LENGTH, "%s%s%s", DEF_CURRENT_DIR_NAME, m_root, ch + 1);
      add(FILE_TYPE_FOLDER, flabel);
      snprintf(flabel, MAX_LABEL_LENGTH, "%s(Top Level)", DEF_PARENT_DIR_NAME);
    }
    else {      
      snprintf(flabel, MAX_LABEL_LENGTH, "%s%s", DEF_CURRENT_DIR_NAME, ch + 1);
      add(FILE_TYPE_FOLDER, flabel);
      if(strlen(ch2) == 1) {
        // This means that only the directory delimiter was found, so the top level dir
        // is the next one up
        snprintf(flabel, MAX_LABEL_LENGTH, "%s%s", DEF_PARENT_DIR_NAME, m_root);
      }
      else {
        snprintf(flabel, MAX_LABEL_LENGTH, "%s%s", DEF_PARENT_DIR_NAME, ch2 + 1);
      }
    }
    add(FILE_TYPE_FOLDER, flabel);
  }
  
  SF_INFO info;
  SNDFILE *sf;
  char tmp_name[MAX_FILE_LENGTH];
  if(dir == NULL) {
    strncpy(m_dir, m_root, MAX_FILE_LENGTH);
  }
  else {
    strncpy(m_dir, dir, MAX_FILE_LENGTH);
  }
  
#ifdef MAC
  DIR *dp;
  if(dir == NULL) {
    dp = opendir(DEF_ROOT);
  }
  else {
    dp = opendir(dir);
    if(dp == NULL) {
      return;
    }
  }
  
  dirent *dir_st;
  while((dir_st = readdir(dp))) {
    if(dir_st->d_name[0] == '.') {
      continue;
    }
    // TODO: Get listing of files from finder, not the old-school unix way..
    //if(FPGetFileDirParms(kFPGetFileDirParms, 0, 0, 0, 0,
    //                     kFPAttributeBit, kFPShortName, dir_st->d_name) == kFPNoErr)
    
    if(dir_st->d_type == DT_DIR) {
      add(FILE_TYPE_FOLDER, dir_st->d_name);
    }
    else {
      snprintf(tmp_name, MAX_FILE_LENGTH, "%s%c%s", m_dir, DEF_DELIMITER, dir_st->d_name);
      sf = sf_open(tmp_name, SFM_READ, &info);
      if(sf == NULL) {
        add(FILE_TYPE_OTHERFILE, dir_st->d_name);
      }
      else {
        if(info.frames > TOO_BIG) {
          add(FILE_TYPE_AUDIOFILEBIG, dir_st->d_name);
        }
        else {
          add(FILE_TYPE_AUDIOFILE, dir_st->d_name);
        }
        
        sf_close(sf);
      }
    }
  }
  closedir(dp);
#elif WINDOWS
  HANDLE hlist;
  WIN32_FIND_DATA filedata;
  
  sprintf(tmp_name, "%s%c*", dir, DEF_DELIMITER);
  hlist = FindFirstFile(tmp_name, &filedata);
  if(hlist == INVALID_HANDLE_VALUE) {
    FindClose(hlist);
    return;
  }
  else {
    bool finished = false;
    while(!finished) {
      if(filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if(filedata.cFileName[0] != '.') {
          add(FILE_TYPE_FOLDER, filedata.cFileName);
        }
      }
      else {
        sprintf(tmp_name, "%s%c%s", m_dir, DEF_DELIMITER, filedata.cFileName);
        sf = sf_open(tmp_name, SFM_READ, &info);
        if(sf == NULL) {
          add(FILE_TYPE_OTHERFILE, filedata.cFileName);
        }
        else {
          add(FILE_TYPE_AUDIOFILE, filedata.cFileName);
          sf_close(sf);
        }
      }
      
      if(!FindNextFile(hlist, &filedata)) {
        if(GetLastError() == ERROR_NO_MORE_FILES) {
          finished = true;
        }
      }
    }
    
    FindClose(hlist);
  }
#endif
  
  for(int i = m_num_rows; i < m_max_rows; ++i) {
    add(FILE_TYPE_EMPTY, "");
  }
  
  m_last_index = 0;
  m_scrollbar->setValue(1.0);
  setDirty(true);
}

void cFileColumn::setRoot(char drive) {
#if MAC
  m_root[0] = drive;
#elif WINDOWS
  snprintf(m_root, MAX_ROOT, "%c:%c", drive, DEF_DELIMITER);
#endif
}