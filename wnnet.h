#include <wx/wx.h>

class PicFrame:public wxFrame {
 public:
  PicFrame(wxWindow *, const wxString,const wxPoint&, const wxSize&);
  void SetPic(wxBitmap *bitmap);
  void OnPaint();
  void OnClose(wxCloseEvent& event);
  wxBitmap *bitmap;

  DECLARE_EVENT_TABLE()

};




class wxNnetFrame:public wxFrame {
 public:

  wxNnetFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
  void OnQuit(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnSave(wxCommandEvent& event);
  void OnPrint(wxCommandEvent& event);
  void OnStop(wxCommandEvent& event);
  void OnStart(wxCommandEvent& event);
  void OnPause(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnEval(wxCommandEvent& event);
  void OnZoom1(wxCommandEvent& event);
  void OnZoom2(wxCommandEvent& event);
  void OnZoom3(wxCommandEvent& event);

  wxBitmap *GetBitmap();
  wxBitmap *ToBitmap(binaryBitmap *pic, int addframe);

  //  void Draw(unsigned char *bitmapdata, int width, int height);

  void OnPaint();

  int matchidx;
  double scale;
  // protected:
  //  int l;

  PicFrame *picframe;
  DECLARE_EVENT_TABLE()

};

class wxNnetApp: public wxApp {
 public:
  virtual bool OnInit();
  wxNnetFrame *pframe;
  int pics;


};


class MyThread: public wxThread{
 public:
  virtual void *Entry();
  int Callback(int t, binaryBitmap *p);
  
  int running;

};

class MyTimer: public wxTimer{
 public:
  virtual void Notify();
};


class MyPrintout: public wxPrintout {
 public:
  MyPrintout(char *title="My Printout"):wxPrintout(title){}
  bool HasPage(int page);
  bool OnPrintPage(int page);
};
    

//class ParaDialog: public wxDialog {
// public:
