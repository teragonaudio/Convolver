/*
 *  cFileColumn.h
 *  Convolver
 *
 *  Created by Nik Reiman on 13.10.05.
 *  Copyright 2005 Teragon Audio. All rights reserved.
 *
 */

#ifndef __cFileColumn_H
#define __cFileColumn_H

#include <vector>

#ifndef __defaults_H
#include "defaults.h"
#endif

#ifndef __vstgui_h__
#include "vstgui.h"
#endif

#ifndef SNDFILE_H
#include "sndfile.h"
#endif

#ifdef MAC
#include <sys/types.h>
#include <dirent.h>
#endif

#ifdef WIN32
#include "resource.h"
#define snprintf _snprintf
#endif

#ifdef MAC
#define DEF_DELIMITER '/'
#define DEF_ROOT "/"
#define MAX_ROOT 2
#endif

#ifdef WINDOWS
// my lord, what a hack this is...
#define DEF_DELIMITER (char)92
#define DEF_ROOT "C:\\"
#define MAX_ROOT 4
#define DEF_DRIVE_NAME "Drive: "
#define MAX_DRIVES 26
#endif

#define DEF_NUM_ICONS 5
#define DEF_ROW_HEIGHT 15
#define DEF_CURRENT_DIR_NAME "Current: "
#define DEF_PARENT_DIR_NAME "Parent: "

#define MAX_LABEL_LENGTH 64
#ifndef MAX_FILE_LENGTH
#define MAX_FILE_LENGTH 512
#endif
#define TOO_BIG 16384

enum {
  TAG_FILEBROWSER_CLICK = 200,
  TAG_FILEBROWSER_SCROLL
};

enum {
  FILE_TYPE_FOLDER,
  FILE_TYPE_AUDIOFILE,
  FILE_TYPE_OTHERFILE,
  FILE_TYPE_EMPTY,
  FILE_TYPE_AUDIOFILEBIG
};

class cFileIcon : public CMovieBitmap {
  public:
    cFileIcon(const CRect &size, CControlListener *listener,
              long tag, CBitmap *pBackground, int index, CPoint &point);
    ~cFileIcon();
    
    void setType(long tag);
  protected:
  private:
    int m_index;
};

//////////////////////////////////////////////////////////////////////

class cFileLabel : public CParamDisplay {
  public:
    cFileLabel(CRect &size, long tag, int index, CControlListener *listener);
    ~cFileLabel();
    
    virtual void draw(CDrawContext *pContext);
    virtual void mouse(CDrawContext *context, CPoint &where, long buttons);
    virtual bool onWheel(CDrawContext *pContext, const CPoint &where, float distance);

    char* getName() const;
    float getValue() const;
    void setLabel(char *text);
    void setClicked(bool on);
    void setScrollbar(CVerticalSlider *vs);
  protected:
    CVerticalSlider *m_scrollbar;
    char m_label[MAX_FILE_LENGTH];
  private:    
    bool m_selected;
    int m_index;
};

//////////////////////////////////////////////////////////////////////

class cFileRow : public CControl {
  public:
    cFileRow(const CRect &size, CControlListener *listener,
            long tag, CBitmap *pBackground, CFrame *frame,
            int index);
    ~cFileRow();
    
    void add();
    void draw(CDrawContext *pContext);
    int getIndex() const;
    bool getClicked() const;
    char* getName() const;
    int getType() const;
    void setClicked(bool on);
    void setName(char *name);
    void setScrollbar(CVerticalSlider *vs);
    void setType(long type);
  protected:
  private:
    cFileIcon *m_type_button;
    cFileLabel *m_file_label;
    CFrame *m_frame;
    
    bool m_selected;
    long m_filetype;
    char *m_name;
    int m_index;
};

//////////////////////////////////////////////////////////////////////

class cFileColumn : public CControl {
  public:
    cFileColumn(const CRect &size, CControlListener *listener,
               long tag, CBitmap *pBackground, CFrame *frame);
    ~cFileColumn();
    
    virtual void draw(CDrawContext *dc);

    void add(long type, char *name);
    void getClicked(char *text) const;
    CRect* getCoords(int num) const;
    char* getDirectory() const;
    CVerticalSlider* getScrollbarHandle() const;
    void getRoot(char *text) const;
    void scroll(float position);
    void selectRow(int row);
    void setDirectory(char *dir);
    void setRoot(char drive);
  protected:
  private:
    CFrame *m_frame;
    CVerticalSlider *m_scrollbar;
    CRect m_coords;
    CBitmap *m_type_bitmap;

    std::vector<cFileRow*> m_rows;
    
    struct rowBuf {
      char name[MAX_FILE_LENGTH];
      int type;
    };
    struct rowBuf *m_scroll_buffer;
    
    char m_dir[MAX_FILE_LENGTH];
    char m_root[MAX_ROOT];
    int m_max_rows;
    int m_num_rows;
    int m_last_index;
};

#endif