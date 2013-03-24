/*
 *  ConvolverEditorOpen.cpp
 *  Convolver2
 *
 *  Created by Nik Reiman on 05.05.06.
 *  Copyright 2006 Teragon Audio. All rights reserved.
 *
 */

#ifndef __Convolver2Editor_H
#include "Convolver2Editor.h"
#endif

bool Convolver2Editor::open(void *ptr) {
  AEffGUIEditor::open(ptr);
  
  CPoint point;
  CRect size(0, 0, m_background->getWidth(), m_background->getHeight());
  frame = new CFrame(size, ptr, this);
  frame->setBackground(m_background);
  
  // Source filename boxes
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y,
       DEF_INFO_BOX_X + 35, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 1));
  cTransparentLabel *filebox = new cTransparentLabel(size, "File");
  frame->addView(filebox);  
  size(DEF_INFO_BOX_X2, DEF_INFO_BOX_Y,
       DEF_INFO_BOX_X2 + 62, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 1));
  m_filelabel = new cTransparentLabel(size, "None");
  frame->addView(m_filelabel);
  
  // Frames boxes
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 1),
       DEF_INFO_BOX_X + 35, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 2));
  cTransparentLabel *framesbox = new cTransparentLabel(size, "Frames");
  frame->addView(framesbox);
  size(DEF_INFO_BOX_X2, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 1),
       DEF_INFO_BOX_X2 + 62, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 2));
  m_framelabel = new cTransparentLabel(size, "0");
  frame->addView(m_framelabel);
  
  // Filesize boxes
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 2),
       DEF_INFO_BOX_X + 35, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 3));
  cTransparentLabel *sizebox = new cTransparentLabel(size, "Size");
  frame->addView(sizebox);
  size(DEF_INFO_BOX_X2, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 2),
       DEF_INFO_BOX_X2 + 62, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 3));
  m_filesize = new cTransparentLabel(size, "0");
  frame->addView(m_filesize);
  
  // Delay calculation boxes
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 3),
       DEF_INFO_BOX_X + 35, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 4));
  cTransparentLabel *samplerate = new cTransparentLabel(size, "Delay");
  frame->addView(samplerate);
  size(DEF_INFO_BOX_X2,DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 3),
       DEF_INFO_BOX_X2 + 62, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 4));
  m_delaylabel = new cTransparentLabel(size, "0");
  frame->addView(m_delaylabel);
  
  // Info box
  size(DEF_INFO_BOX_X, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 4),
       DEF_INFO_BOX_X + 62 + 35, DEF_INFO_BOX_Y + (DEF_INFO_BOX_HEIGHT * 5));
  m_info_box = new cTransparentLabel(size, "");
  frame->addView(m_info_box);
  
  // File open button
  CBitmap *bmap = new CBitmap(REZ_OPEN_BUTTON);
  size(169,241, 169 + 42,241 + 19);
  point(0, 0);
  m_file_choose_button = new CKickButton(size, this, TAG_FILEBROWSER_CLICK,
                                         bmap->getHeight() / 2,
                                         bmap, point);
  frame->addView(m_file_choose_button);
  bmap->forget();
  
  // Save Button
  bmap = new CBitmap(REZ_SAVE_BUTTON);
  size(169,267, 169 + 42,267 + 19);
  m_save_button = new CKickButton(size, this, TAG_FILE_SAVE,
                                  bmap->getHeight() / 2,
                                  bmap, point);
  frame->addView(m_save_button);
  bmap->forget();
  
  // Load Button
  bmap = new CBitmap(REZ_LOAD_BUTTON);
  size(169,292, 169 + 42,292 + 19);
  m_load_button = new CKickButton(size, this, TAG_FILE_LOAD,
                                  bmap->getHeight() / 2,
                                  bmap, point);
  frame->addView(m_load_button);
  bmap->forget();
  
  // Gain Knob
  bmap = new CBitmap(REZ_KNOB);
  size(24,339, 24 + DEF_KNOB_HEIGHT,339 + DEF_KNOB_HEIGHT);
  m_gain_knob = new CAnimKnob(size, this, PRM_GAIN, DEF_KNOB_FRAMES,
                              DEF_KNOB_HEIGHT, bmap, point);
  frame->addView(m_gain_knob);
  
  // Filter Knob
  size(96,356, 96 + DEF_KNOB_HEIGHT,356 + DEF_KNOB_HEIGHT);
  m_filter_knob = new CAnimKnob(size, this, PRM_FILTER_FREQ, DEF_KNOB_FRAMES,
                                DEF_KNOB_HEIGHT, bmap, point);
  frame->addView(m_filter_knob);
  
  // Filter type menu
  size(103,325, 147,337);
  m_filter_type = new cOptionMenuHidden(size, this, PRM_FILTER_TYPE);
  for(int i = 0; i < NUM_FILT_TYPES; ++i) {
    m_filter_type->addEntry(_FILT_NAMES[i]);
  }
  frame->addView(m_filter_type);
  
  // Wet/Dry knob
  size(167,339, 167 + DEF_KNOB_HEIGHT, 339 + DEF_KNOB_HEIGHT);
  m_wetdry_knob = new CAnimKnob(size, this, PRM_WETDRY, DEF_KNOB_FRAMES,
                                DEF_KNOB_HEIGHT, bmap, point);
  frame->addView(m_wetdry_knob);
  bmap->forget();
  
  // File browser
  size(30,28, 195,195);
  m_file_column = new cFileColumn(size, this, TAG_FILECOLUMN, NULL, frame);
  char buf[MAX_PATH_SIZE];
  getProductDir(DEF_PRODUCT, (char*)&buf);
  strcat(buf, "lastdir");
  FILE *fp = fopen(buf, "r");
  if(fp) {
    fgets((char*)&buf, MAX_PATH_SIZE, fp);
    if(strlen(buf)) {
      m_file_column->setDirectory(buf);
    }
    fclose(fp);
  }
  else {
    m_file_column->setDirectory(NULL);
  }
  
  // About screen
  bmap = new CBitmap(REZ_SPLASHSCREEN);
  size(0,433, 240,450);
  point(0, 0);
  CRect display_area(0, 0, bmap->getWidth(), bmap->getHeight());
  display_area.offset(0, 0);
  m_splashscreen = new CSplashScreen(size, this, TAG_SPLASHSCREEN, bmap, display_area, point);
  frame->addView(m_splashscreen);
  bmap->forget();
  
  this->frame = frame;
  m_opened = true;
  // Place all the knobs in their correct position and sets up the stuff in the info panel
  setAttributes();
  
  return true;  
}