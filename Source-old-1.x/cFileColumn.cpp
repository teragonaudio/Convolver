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

#ifndef SNDFILE_H
#include "sndfile.h"
#endif

#ifdef MAC
#include <sys/types.h>
#include <dirent.h>
#endif

#include <vector>

#ifdef WIN32
#include "resource.h"
#define snprintf _snprintf
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
      CMovieBitmap::setValue(0.0);
      break;
    case FILE_TYPE_OTHERFILE:
      CMovieBitmap::setValue(0.25);
      break;
    case FILE_TYPE_AUDIOFILE:
      CMovieBitmap::setValue(0.45);
      break;
    case FILE_TYPE_AUDIOFILEBIG:
      CMovieBitmap::setValue(0.65);
      break;
    case FILE_TYPE_EMPTY:
      CMovieBitmap::setValue(1.0);
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
  value = m_index;
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
      value = m_index * -1.0;
#ifdef DEBUG
      fprintf(stderr, "Got a double click, and value is %f, %f\n", getValue(), value);
#endif
      listener->valueChanged((CDrawContext*)context, (CControl*)this);    
    }
    else {
      value = m_index;
      listener->valueChanged((CDrawContext*)context, (CControl*)this);
    }
  }
}

bool cFileLabel::onWheel(CDrawContext *pContext, const CPoint &where, float distance) {
  /* So, this kinda scrolls the display... but not really so well
  value += (1.0 / distance);
  listener->valueChanged((CDrawContext*)pContext, (CControl*)this);
  value -= (1.0 / distance);
  */
  return true;
}

void cFileLabel::setClicked(bool on) {
  m_selected = on;
  setDirty();
}

void cFileLabel::setLabel(char *text) {
	if(text) {
    strncpy(m_label, text, MAX_FILENAME);
	}
  setDirty();
}

///////////////////////////////////////////////////////////////////////

cFileRow::cFileRow(const CRect &size, CControlListener *listener,
                   long tag, CBitmap *pBackground, CFrame *frame,
                   int index) :
CControl(size, listener, tag, pBackground) {
  m_frame = frame;
  m_index = index;
  m_selected = false;
  memset(m_name, 0x0, MAX_FILENAME);
  
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
  
  m_file_label = new cFileLabel(tmp_size, PRM_FILECLICK, m_index, listener);
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
  if(m_selected) {
    on = false;
  }
  if(m_filetype != FILE_TYPE_EMPTY) {
    m_selected = on;
    m_file_label->setClicked(on);
  }
}

void cFileRow::setName(char *name) {
  strncpy(m_name, name, MAX_FILENAME);
  m_file_label->setLabel(m_name);
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
  
#ifdef MAC
  m_type_bitmap = new CBitmap(REZ_FILE_ICONS);
#elif WIN32
  m_type_bitmap = new CBitmap(IDB_BITMAP13);
#endif
  m_max_rows = (int)((m_coords.bottom - m_coords.top) / DEF_ROW_HEIGHT);
  m_coords.bottom = (m_max_rows * DEF_ROW_HEIGHT) + m_coords.top;
  setMouseEnabled(true);

  CPoint point(0, 0);
#ifdef MAC
  CBitmap *handle = new CBitmap(REZ_SCROLLBAR_HANDLE);
  CBitmap *bg = new CBitmap(REZ_SCROLLBAR_BG);
#elif WIN32
  CBitmap *handle = new CBitmap(IDB_BITMAP6);
  CBitmap *bg = new CBitmap(IDB_BITMAP7);
#endif
  
  // Dynamic scrollbar sizing... not being used because it's not precise enough
  // for this particular implementation
	// CRect bar_size(0,0, bg->getWidth(), m_coords->bottom - m_coords->top);
	// bar_size.offset(m_coords->right - bg->getWidth(), m_coords->top);
  
  CRect bar_size(203,29, 220,196);
  CPoint handleOffset(0, 0);
	m_scrollbar = new CVerticalSlider(bar_size, listener, 
                                    PRM_FILESCROLL, handleOffset, 
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
  
  memset(m_file, 0x0, sizeof(char) * MAX_DIR);
  memset(m_dir, 0x0, sizeof(char) * MAX_DIR);
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
#ifdef DEBUG
  fprintf(stderr, "(%d), with coords of (%d,%d, %d,%d)\n",
          m_rows.size(), r->left,r->top,r->right,r->bottom);
#endif
  
  cFileRow *row = new cFileRow(*r, (CControlListener*)listener,
                               (long int)type, m_type_bitmap, m_frame, m_rows.size());
  
  if(type != FILE_TYPE_EMPTY) {
    row->setName(name);
  }
  
  if(row->getIndex() < m_max_rows) {
    row->add();
  }
  m_rows.push_back(row);
#ifdef DEBUG
  fprintf(stderr, "Added row to vector of type ");
#endif
}

void cFileColumn::draw(CDrawContext *dc) {
  for(int i = 0; i < m_max_rows; ++i) {
    m_rows.at(i)->draw(dc);
  }
}

char* cFileColumn::getClicked() const {
  for(unsigned int i = 0; i < m_rows.size(); ++i) {
    if(m_rows.at(i)->getClicked()) {
      if(m_dir[0] == '\0') {
        strcpy((char*)m_file, m_root);
      }
      else {
        char tmp[MAX_DIR];
        strcpy(tmp, m_rows.at(i)->getName());
#if WINDOWS
        if(!strncmp(tmp, DEF_DRIVE_NAME, strlen(DEF_DRIVE_NAME))) {
          sprintf((char*)m_file, "%s:%c%", tmp + strlen(DEF_DRIVE_NAME), DEF_DELIMITER);
        }
        else {
#endif
          sprintf((char*)m_file, "%s%c%s", m_dir, DEF_DELIMITER, m_rows.at(i)->getName());
#if WINDOWS
        }
#endif
      }
      return (char*)&m_file;
    }
  }
  // Pseudo-perl saves the day
  return "";
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
#ifdef DEBUG
    fprintf(stderr, "Scrolling down by %d rows\n", diff);
#endif
    
    // Copy scrolled out rows into temporary buffer
    if(m_scroll_buffer) {
      delete [] m_scroll_buffer;
    }
    m_scroll_buffer = new struct rowBuf[diff];
    
    for(int i = 0; i < diff; ++i) {
      strncpy(m_scroll_buffer[i].name, m_rows.at(i)->getName(), MAX_FILENAME);
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
#ifdef DEBUG
    fprintf(stderr, "Scrolling up by %d rows\n", diff);
#endif
    
    if(m_scroll_buffer) {
      delete [] m_scroll_buffer;
    }
    m_scroll_buffer = new struct rowBuf[diff];
    
    // Copy tail to buffer
    for(int i = m_rows.size() - diff, j = 0; j < diff; ++i, ++j) {
      strncpy(m_scroll_buffer[j].name, m_rows.at(i)->getName(), MAX_FILENAME);
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
  if(row > m_max_rows) {
    return;
  }
  
#ifdef DEBUG
  fprintf(stderr, "Selecting row #%d\n", row);
#endif
  
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
  // Clear out all text manually, because otherwise transparency gets fucked up.
  // Stupid VSTGUI
  for(unsigned int i = 0; i < m_rows.size(); ++i) {
    m_rows.at(i)->setName("");
    m_rows.at(i)->setClicked(false);
    m_rows.at(i)->setType(FILE_TYPE_EMPTY);
  }
  m_rows.clear();
  
#ifdef DEBUG
  fprintf(stderr, "Changing directory to %s\n", dir);
#endif
  char *ch = 0;
  char flabel[MAX_FILENAME];

  if(dir == NULL || dir[0] == 0x0) {
#if MAC
    return setDirectory("/Volumes");
#elif WINDOWS

#endif
  }
  else {
    ch = strrchr(dir, DEF_DELIMITER);
  }
#ifdef MAC
  char root[2];
  root[0] = DEF_DELIMITER;
  root[1] = '\0';
  
  if(!strcmp(dir, root)) {
    ch = NULL;
  }
#endif

  if(ch == NULL) { // Top level directory
    snprintf(flabel, MAX_FILENAME, "%s(Top Level)", DEF_CURRENT_DIR_NAME);
    add(FILE_TYPE_FOLDER, flabel);
    
    memset(flabel, 0x0, MAX_FILENAME * sizeof(char));
    snprintf(flabel, MAX_FILENAME, "%s(None)", DEF_PARENT_DIR_NAME);
    add(FILE_TYPE_FOLDER, flabel);
#if MAC
    dir = NULL;
#elif WINDOWS
    // Display the drive list for windows
    DWORD d = GetLogicalDrives();
    for(int i = 0; i < MAX_DRIVES; ++i) {
	    if((1 << i)&d) {
        snprintf(flabel, MAX_FILENAME, "%s%c", DEF_DRIVE_NAME, 'A' + i);
        add(FILE_TYPE_FOLDER, flabel);
	    }
    }
    return;
#endif
  }
  else {
    // Parent directory
    char tmp_dir[MAX_FILENAME];
    int s = strlen(dir) - strlen(ch);
    strncpy(tmp_dir, dir, s);
    tmp_dir[s] = '\0';

    char *ch2 = strrchr(tmp_dir, DEF_DELIMITER);
    if(ch2 == NULL || s <= 1) { // Top level directory, for windows
      snprintf(flabel, MAX_FILENAME, "%s%s%s", DEF_CURRENT_DIR_NAME, m_root, ch + 1);
      add(FILE_TYPE_FOLDER, flabel);
      snprintf(flabel, MAX_FILENAME, "%s(Top Level)", DEF_PARENT_DIR_NAME);
    }
    else {      
      snprintf(flabel, MAX_FILENAME, "%s%s", DEF_CURRENT_DIR_NAME, ch + 1);
      add(FILE_TYPE_FOLDER, flabel);
      snprintf(flabel, MAX_FILENAME, "%s%s", DEF_PARENT_DIR_NAME, ch2 + 1);
    }
    add(FILE_TYPE_FOLDER, flabel);
  }
  m_num_rows = 2;

  SF_INFO info;
  SNDFILE *sf;
  char tmp_name[MAX_DIR];
  if(dir == NULL) {
    strcpy(m_dir, m_root);
  }
  else {
    strncpy(m_dir, dir, MAX_DIR);
  }
  
#ifdef MAC
  DIR *dp;
  if(dir == NULL) {
    dp = opendir("/");
  }
  else {
    dp = opendir(dir);
    if(dp == NULL) {
      return;
    }
  }
  
  dirent *dir_st;
  while((dir_st = readdir(dp))) {
#ifdef DEBUG
    fprintf(stderr, "Adding file %s ", dir_st->d_name);
#endif
    if(dir_st->d_name[0] == '.') {
#ifdef DEBUG
      fprintf(stderr, "(skipping)\n");
#endif
      continue;
    }
    if(dir_st->d_type == DT_DIR) {
      add(FILE_TYPE_FOLDER, dir_st->d_name);
#ifdef DEBUG
      fprintf(stderr, "(folder)\n");
#endif
    }
    else {
      sprintf(tmp_name, "%s/%s", m_dir, dir_st->d_name);
      sf = sf_open(tmp_name, SFM_READ, &info);
      if(sf == NULL) {
        add(FILE_TYPE_OTHERFILE, dir_st->d_name);
#ifdef DEBUG
        fprintf(stderr, "(other)\n");
#endif
      }
      else {
        if(info.frames > TOO_BIG) {
          add(FILE_TYPE_AUDIOFILEBIG, dir_st->d_name);
        }
        else {
          add(FILE_TYPE_AUDIOFILE, dir_st->d_name);
        }
        
        sf_close(sf);
#ifdef DEBUG
        fprintf(stderr, "(audio)\n");
#endif
      }
    }
    ++m_num_rows;
  }
  closedir(dp);
#elif WINDOWS
  HANDLE hlist;
  WIN32_FIND_DATA filedata;

  strcat(dir, "\\*");
  hlist = FindFirstFile(dir, &filedata);
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
        sprintf(tmp_name, "%s\\%s", m_dir, filedata.cFileName);
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
      else {
        ++m_num_rows;
      }
    }

    FindClose(hlist);
  }
#endif
  
  for(int i = m_num_rows; i < m_max_rows; ++i) {
    add(FILE_TYPE_EMPTY, "");
#ifdef DEBUG
    fprintf(stderr, "(empty)\n");
#endif
  }
  
  m_last_index = 0;
  m_scrollbar->setValue(1.0);
}

void cFileColumn::setRoot(char drive) {
#if MAC
  m_root[0] = drive;
#elif WINDOWS
  snprintf(m_root, MAX_ROOT, "%c:%c", drive, DEF_DELIMITER);
#endif
}