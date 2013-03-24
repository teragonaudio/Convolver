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

#ifndef __vstgui_h__
#include "vstgui.h"
#endif

#ifdef MAC
#define DEF_DELIMITER '/'
#elif WIN32
// my lord, what a hack this is...
#define DEF_DELIMITER (char)92
#ifdef DEBUG
#define MAX_DRIVES 4
#else
#define MAX_DRIVES 32
#endif
#endif
#define DEF_NUM_ICONS 5
#define DEF_ROW_HEIGHT 15
#define DEF_CURRENT_DIR_NAME "Current: "
#define DEF_PARENT_DIR_NAME "Parent: "
#define DEF_PRODUCT_NAME "Convolver"
#ifdef MAC
#define DEF_ROOT "/"
#define MAX_ROOT 2
#elif WINDOWS
#define DEF_ROOT "C:\\"
#define MAX_ROOT 4
#define DEF_DRIVE_NAME "Drive: "
#endif

#define TOO_BIG 16384
#define MAX_FILENAME 64
#define MAX_FILE 512
#define MAX_DIR 512

#define PRM_FILECLICK 200
#define PRM_FILESCROLL 100

#define FILE_TYPE_FOLDER 1
#define FILE_TYPE_AUDIOFILE 2
#define FILE_TYPE_OTHERFILE 3
#define FILE_TYPE_EMPTY 4
#define FILE_TYPE_AUDIOFILEBIG 5

#ifdef MAC
#define REZ_SCROLLBAR_HANDLE 10006
#define REZ_SCROLLBAR_BG 10007
#define REZ_FILE_ICONS 10013
#endif

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
    
    // VSTGUI overrides
    virtual void draw(CDrawContext *pContext);
    virtual void mouse(CDrawContext *context, CPoint &where, long buttons);
    virtual bool onWheel(CDrawContext *pContext, const CPoint &where, float distance);

    char* getName() const;
    virtual float getValue() const;
    void setLabel(char *text);
    void setClicked(bool on);
  protected:
    char m_label[MAX_FILENAME];
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
    void setType(long type);
  protected:
  private:
    cFileIcon *m_type_button;
    cFileLabel *m_file_label;
    CFrame *m_frame;
    
    bool m_selected;
    long m_filetype;
    char m_name[MAX_FILENAME];
    int m_index;
};

//////////////////////////////////////////////////////////////////////

class cFileColumn : public CControl {
  public:
    cFileColumn(const CRect &size, CControlListener *listener,
               long tag, CBitmap *pBackground, CFrame *frame);
    ~cFileColumn();
    
    // VSTGUI overrides
    virtual void draw(CDrawContext *dc);

    void add(long type, char *name);
    char* getClicked() const;
    CRect* getCoords(int num) const;
    char* getDirectory() const;
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
      char name[MAX_FILENAME];
      int type;
    };
    struct rowBuf *m_scroll_buffer;
    
    char m_dir[MAX_DIR];
    char m_root[MAX_ROOT];
	  char m_file[MAX_DIR];
    int m_max_rows;
    int m_num_rows;
    int m_last_index;
};

#endif