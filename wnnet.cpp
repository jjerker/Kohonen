#ifndef WX_PRECOMP
#include <wx/wx.h>
#include <wx/resource.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include "bmp.h"
#include "kohonen.h"
#include "nnet.h"
#include "wnnet.h"
#include "wnnetdia.h"
#include "wnnetdia.wxr"
#endif

#define BITMAPSIZE 64

MyThread *tthread;
MyTimer timer;

wxNnetApp *papp; 
wxNnetFrame *gframe;
int npics=0;
static binaryBitmap **tpics;
static binaryBitmap *pic=0;
static int iter=0;
int mutex=0;
int training=0;
int train_stop=0;
KOHONET *knet=0;


enum {
  ID_Quit =1,
  ID_Open,
  ID_Save,
  ID_About,
  ID_InpLevel,
  ID_Stop,
  ID_Start,
  ID_Pause,
  ID_Eval,
  ID_Zoom1,
  ID_Zoom2,
  ID_Zoom3,
  ID_Print
};

BEGIN_EVENT_TABLE(wxNnetFrame, wxFrame)
  EVT_MENU(ID_Quit, wxNnetFrame::OnQuit)
  EVT_MENU(ID_Open, wxNnetFrame::OnOpen)
  EVT_MENU(ID_Save, wxNnetFrame::OnSave)
  EVT_MENU(ID_Print, wxNnetFrame::OnPrint)
  EVT_MENU(ID_Stop, wxNnetFrame::OnStop)
  EVT_MENU(ID_Start, wxNnetFrame::OnStart)
  EVT_MENU(ID_Pause, wxNnetFrame::OnPause)
  EVT_MENU(ID_Eval, wxNnetFrame::OnEval)
  EVT_MENU(ID_About, wxNnetFrame::OnAbout)
  EVT_MENU(ID_Zoom1, wxNnetFrame::OnZoom1)
  EVT_MENU(ID_Zoom2, wxNnetFrame::OnZoom2)
  EVT_MENU(ID_Zoom3, wxNnetFrame::OnZoom3)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(PicFrame, wxFrame)
  EVT_PAINT(PicFrame::OnPaint)
  EVT_CLOSE(PicFrame::OnClose)
END_EVENT_TABLE()



IMPLEMENT_APP(wxNnetApp)

binaryBitmap *GetImage(wxString name) {

	binaryBitmap *bmp;
	unsigned char *data;
	int count=0;
	int i,j;
	wxImage *image = new wxImage(name);

	if (!image) return NULL;
	if(!image->Ok()) {
		//char buf[300];
		//sprintf(buf, "Could not load image '%s'", name);
		//wxMessageBox(buf);
	    delete image;
		return NULL;
	}
    bmp = (binaryBitmap *)calloc(sizeof(binaryBitmap), 1);
	bmp->data = (unsigned char *)calloc(sizeof(unsigned char), BITMAPSIZE*BITMAPSIZE);
	bmp->size = BITMAPSIZE*BITMAPSIZE;
	bmp->width= BITMAPSIZE;
	bmp->height= BITMAPSIZE;


	image->Rescale(BITMAPSIZE,BITMAPSIZE);
	if (!(data = image->GetData())) {
		wxMessageBox("Uuups, errors");
		return NULL;
	}
	for (i=0; i<BITMAPSIZE;i++) {
		for (j=0; j<BITMAPSIZE;j++) {
			int sum=data[count++];
			sum+=data[count++];
			sum+=data[count++];
			bmp->data[(BITMAPSIZE-i-1)*BITMAPSIZE+j] = sum/3;
		}
	}
	delete image;
	return bmp;
}

wxBitmap * wxNnetFrame::GetBitmap() {
  wxBitmap *bmp;
  if (mutex) ;
  mutex=1;

  bmp = ToBitmap(pic, 1);

  mutex=0;
  return bmp;
}


wxBitmap * wxNnetFrame::ToBitmap(binaryBitmap *p, int addframe) {
  
  static unsigned char *data=0;
  int i,j, count=0;
  wxBitmap *bitmap;
  wxImage *image;
  int x,y;

  if (!p) return 0;

  // Construct image
  data = (unsigned char *)calloc(sizeof(unsigned char), p->width*p->height*3);
  
  image = new wxImage(p->width, p->height);

  for (i=0; i<p->height; i++) { 
    for (j=0; j<p->width; j++) { 
      data[count++] = p->data[(p->height-i-1)*p->width+j];
      data[count++] = p->data[(p->height-i-1)*p->width+j];
      data[count++] = p->data[(p->height-i-1)*p->width+j];
    }
  }


  if (addframe) {
    x = p->width / BITMAPSIZE;
    y = p->height / BITMAPSIZE;
    if (matchidx!=-1) {
      for (i=0; i<64;i++) {
	int offset = (y-matchidx/y-1)*BITMAPSIZE*p->width+(matchidx%x)*BITMAPSIZE;
#define SETRED(p,idx) p[3*idx]=255; p[3*idx+1]=0; p[3*idx+2]=0;

	SETRED(data,(offset+i))
	SETRED(data,(offset+p->width*(BITMAPSIZE-1)+i))
	SETRED(data,(offset+i*p->width))
	SETRED(data,(offset+i*p->width+BITMAPSIZE-1))
      }
    }
  }

  image->SetData(data);

  bitmap = new wxBitmap(*image);

  delete image;

  return bitmap;
 
}


bool wxNnetApp::OnInit() {
	char buf[5000];
	
  papp = this;
  gframe = pframe = new wxNnetFrame("Kohonen network", wxPoint(50,50), wxSize(450,350));
  pframe->Show (true);
  SetTopWindow(pframe);

  strcpy(buf, dialog1_a);
  strcat(buf, dialog1_b);
  wxResourceParseData(buf);


  return true;
}

void wxNnetFrame::OnPaint() {
  wxPaintDC dc(this);
  wxBitmap *bitmap;

  bitmap = GetBitmap();
  if (bitmap) {
    dc.Clear();
    dc.SetUserScale(scale, scale);
    dc.BeginDrawing();
    dc.DrawBitmap(*bitmap, 0, 0, 0);
    dc.EndDrawing();
    delete bitmap;
  }
  return;
}  

void wxNnetFrame::OnStop(wxCommandEvent& event) {
    train_stop = 1;
	SetStatusText("Training stopped");
}

void wxNnetFrame::OnPause(wxCommandEvent& event) {
	if (tthread->IsRunning()) {
		while (mutex) ; mutex=1;
		tthread->Pause();
		mutex=0;
		SetStatusText("Training paused");
		return;
	}
	if (tthread->IsPaused()) {
		tthread->Resume();
		SetStatusText("Training continued");
		return;
	}
}

void wxNnetFrame::OnAbout(wxCommandEvent& event) {
  wxMessageBox("This is a small test program for Kohonen networks\n\nUsage\n 1. Load (multiple) bitmapfiles using 'File->Load'\n 2. Start training using 'Train->Start training'\n 3. Watch the screen for changes!!\n\nC J. Björkqvist, 2001", "About");
  return;
}


wxNnetFrame::wxNnetFrame(const wxString& title, const wxPoint& pos, const wxSize& size):wxFrame((wxFrame*)NULL,-1,title,pos,size) {


  // Create Menubar
  wxMenuBar *menuBar = new wxMenuBar;
  
  // Create menu
  wxMenu *menuFile = new wxMenu;
  wxMenu *menuTrain = new wxMenu;
  wxMenu *menuHelp = new wxMenu;
  wxMenu *menuEval = new wxMenu;
  wxMenu *menuView = new wxMenu;

  // Append menu titles
  menuFile->Append(ID_Open, "&Load");
  menuFile->Append(ID_Save, "&Save...");
  menuFile->AppendSeparator();
  menuFile->Append(ID_Print, "&Print");
  menuFile->AppendSeparator();
  menuFile->Append(ID_Quit, "E&xit");
  menuHelp->Append(ID_About, "&About");

  menuTrain->Append(ID_Start, "&Start training");
  menuTrain->Append(ID_Pause, "&Pause/cont training");
  menuTrain->Append(ID_Stop, "&Stop training");
  menuEval->Append(ID_Eval, "&Evaluate bitmap...");

  menuView->Append(ID_Zoom1, "Zoom &1.0");
  menuView->Append(ID_Zoom2, "Zoom &2.0");
  menuView->Append(ID_Zoom3, "Zoom &3.0");


  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuTrain, "&Train");
  menuBar->Append(menuEval, "&Evaluate");
  menuBar->Append(menuView, "&View");
  menuBar->Append(menuHelp, "&Help");

  SetMenuBar(menuBar);

  // Create pic frame

  picframe = new PicFrame(this, "Eval Picture", wxPoint(50,50), wxSize(BITMAPSIZE, BITMAPSIZE));


  //Connect event to callback

  //Connect(ID_Quit, wxEVT_COMMAND_MENU_SELECTED,
  //	  (wxObjectEventFunction)&wxNnetFrame::OnQuit);
  //  Connect(ID_Open, wxEVT_COMMAND_MENU_SELECTED,
  //	  (wxObjectEventFunction)&wxNnetFrame::OnOpen);
    Connect(-1, wxEVT_PAINT,
    (wxObjectEventFunction)&wxNnetFrame::OnPaint);


  //
  matchidx=-1;
  scale = 1.0;

  CreateStatusBar();
  SetStatusText("Application initialized");
}

void wxNnetFrame::OnQuit(wxCommandEvent& event) {
    Close(true);
}

void wxNnetFrame::OnPrint(wxCommandEvent& event) {

  int res;
  wxPrintData *g_printData = new wxPrintData;

#ifdef __WXGTK__
  (*g_printData) = *wxThePrintSetupData;
#endif
  wxPrintDialogData printDialogData(*g_printData);
  wxPrinter printer(& printDialogData);
  MyPrintout printout("Kohonen network");
  if (!printer.Print(this, &printout, true)) {
    wxMessageBox("Printing problems");
  }
}


void wxNnetFrame::OnSave(wxCommandEvent& event) {
  int res;
  int i;
  char buf[200];
  wxFileDialog *filedialog = new wxFileDialog(this, 
     "Save picture in", "", "", "*.bmp", wxSAVE);
  res = filedialog->ShowModal();
  if (res==wxID_OK) {
	wxString path = filedialog->GetPath();
	wxBitmap *bitmap;

    bitmap = GetBitmap();
	if (bitmap) {
		bitmap->SaveFile(path, wxBITMAP_TYPE_BMP);
		delete bitmap;
	}
  }
  filedialog->Destroy();
}


void wxNnetFrame::OnOpen(wxCommandEvent& event) {
  
  int res;
  int i;
  char buf[200];
  wxFileDialog *filedialog = new wxFileDialog(this, 
     "Choose files for training", "", "", "*.bmp", wxMULTIPLE|wxOPEN);
  
  res = filedialog->ShowModal();
  if (res==wxID_OK) {
    wxArrayString filenames;

    filedialog->GetPaths(filenames);
    if (tpics) free(tpics);
    tpics = (binaryBitmap **)calloc(sizeof(binaryBitmap *), filenames.GetCount());

	npics=0;
    for (i=0; i<filenames.GetCount(); i++) {
		if (tpics[npics] = GetImage((char *)filenames.Item(i).c_str()))
		  npics++;	    
//		if (tpics[npics] = readBMPfile((char *)filenames.Item(i).c_str()))
//		  npics++;
      SetStatusText(filenames.Item(i));      
    }
	sprintf(buf, "Loaded %i pictures\n", npics);
	SetStatusText(buf);
    
  }
  filedialog->Destroy();
	
  return;
}


void wxNnetFrame::OnStart(wxCommandEvent& event) {

	int res;
	long x,y,n,iter,alpha, beta;
	static wxString xtext="10", ytext="10", ntext="4096", itertext="100", alphatext="10", betatext="50";
	static int transform;
	wxTextValidator xvali(wxFILTER_NUMERIC, &xtext);
	wxTextValidator yvali(wxFILTER_NUMERIC, &ytext);
	wxTextValidator nvali(wxFILTER_NUMERIC, &ntext);
	wxTextValidator itervali(wxFILTER_NUMERIC, &itertext);
	wxTextValidator alphavali(wxFILTER_NUMERIC, &alphatext);
	wxTextValidator betavali(wxFILTER_NUMERIC, &betatext);

	if (!npics) {
		wxMessageBox("No pictures selected, please load pictures before training");
	return;
	}

  // project.wxr contains dialog1
  wxDialog *dialog = new wxDialog;
  if (dialog->LoadFromResource(this, "dialog1"))
  {
	wxTextCtrl *w = (wxTextCtrl *)wxFindWindowByName("textctrl4", dialog);
	wxCheckBox *b;
	w->SetValidator(xvali);
	w = (wxTextCtrl *)wxFindWindowByName("textctrl5", dialog);
	w->SetValidator(yvali);
//	wxTextCtrl *w = (wxTextCtrl *)wxFindWindowByName("textctrl4", dialog);
//	w->SetValidator(nvali);
 	w = (wxTextCtrl *)wxFindWindowByName("textctrl12", dialog);
	w->SetValidator(itervali);
	w = (wxTextCtrl *)wxFindWindowByName("textctrl8", dialog);
	w->SetValidator(alphavali);
	w = (wxTextCtrl *)wxFindWindowByName("textctrl10", dialog);
	w->SetValidator(betavali);
	b = (wxCheckBox *)wxFindWindowByName("transfcheck", dialog);
	b->SetValue(transform);
	

    res = dialog->ShowModal();
	transform = b->GetValue();
	
  } else wxMessageBox("Could not load dialog box...");

  dialog->Destroy();

  if (res == wxID_OK) {
	xtext.ToLong(&x);
	ytext.ToLong(&y);
	ntext.ToLong(&n);
	itertext.ToLong(&iter);
	alphatext.ToLong(&alpha);
	betatext.ToLong(&beta);
	if (knet) koho_free(knet);
	knet = koho_init((int)x,(int)y,(int)n,(int)iter,(int)alpha,(int)beta, transform);
	SetStatusText("Training pictures....");      
	matchidx = -1;
	timer.Start(100);
	tthread = new MyThread;
	tthread->Create();
	tthread->SetPriority(WXTHREAD_MIN_PRIORITY);;
	tthread->Run();
  }

}




void wxNnetFrame::OnZoom1(wxCommandEvent& event) {
  scale=1.0;
  Refresh();
}

void wxNnetFrame::OnZoom2(wxCommandEvent& event) {
  scale=2.0;
  Refresh();
}

void wxNnetFrame::OnZoom3(wxCommandEvent& event) {
  scale=3.0;
  Refresh();
}

void wxNnetFrame::OnEval(wxCommandEvent& event) {
  int res;
  binaryBitmap *pic;
  wxBitmap *wxpic;

  if (!knet) {
    wxMessageBox("Please train network before evaluating");
    return;
  }

  wxFileDialog *filedialog = new wxFileDialog(this, 
     "Choose file for evaluating", "", "", "*.bmp", wxOPEN);
  res = filedialog->ShowModal();
  if (res==wxID_OK) {
    wxString path = filedialog->GetPath();
    pic = GetImage(path);
    matchidx = kmatch((KOHONET *)knet, pic);

    wxpic = ToBitmap(pic,0);

    picframe->SetPic(wxpic);
    picframe->Show(true);
    picframe->Refresh();

    delete wxpic;

    disposeBinaryBitmap(pic);

    Refresh();

  }
  filedialog->Destroy();

}


  
  


void * MyThread::Entry() {

  if (!knet) {return 0;}
  train_stop=0;
  ktrain((KOHONET *)knet, npics, tpics);
  timer.Stop(); 
  return 0;
   
}  

int MyThread::Callback(int t, binaryBitmap *p) {
  int i;

  if (mutex) ;
  mutex=1;
  if (!pic)
    pic = (binaryBitmap *)calloc(sizeof(binaryBitmap),1);
  pic->size=p->size;
  pic->width=p->width;
  pic->height=p->height;
  pic->data = (unsigned char *)realloc(pic->data, p->size);
  iter = t;

  memcpy(pic->data, p->data, p->size);
  mutex=0;
  if (TestDestroy()) return 1;
  return train_stop;
}  


void MyTimer::Notify() {
  static int tc;
  if (tc!=iter) {
	char buf[200];

	tc=iter;
	if (!train_stop) {
      sprintf(buf, "Training, iteration %i...", iter);
      gframe->SetStatusText(buf);
	}

	gframe->Refresh(NULL, NULL);

  }
}



extern "C" int callback(int t, binaryBitmap *p) {
  return tthread->Callback(t, p);
}  

 bool MyPrintout::HasPage(int page) {
   if (page==1) return true;
   return false;
 }
 

 bool MyPrintout::OnPrintPage(int page) {
   wxDC *dc = GetDC();
   wxBitmap *bitmap;
 
   dc->SetUserScale(6,6);

   if (dc) {
          
     bitmap = gframe->GetBitmap();

     if (bitmap) {
       dc->Clear();
       dc->BeginDrawing();
       dc->DrawBitmap(*bitmap, 80, 80, 0);
       dc->EndDrawing();
       delete bitmap;
     }
   }

   return true;
 }



PicFrame::PicFrame(wxWindow *win, const wxString title="Pic Frame",const wxPoint& pos=wxPoint(50,50), const wxSize& size=wxSize(64,64)):wxFrame(win, 10, title, pos, size, wxSTAY_ON_TOP|wxFRAME_TOOL_WINDOW|wxRESIZE_BORDER|wxCAPTION) {
  bitmap = NULL;
  //Connect(-1, wxEVT_PAINT,
  //  (wxObjectEventFunction)&PicFrame::OnPaint);

}

void PicFrame::SetPic(wxBitmap *bm) {

  if (bm) {

    if (bitmap) delete bitmap;

    bitmap = new wxBitmap(*bm);

  }
}

void PicFrame::OnPaint() {

  wxPaintDC dc(this);

  if (bitmap) {      
    dc.Clear();
    dc.SetUserScale(gframe->scale, gframe->scale);
    dc.BeginDrawing();
    dc.DrawBitmap(*bitmap, 0, 0, 0);
    dc.EndDrawing();
  }
  return;
}  

void PicFrame::OnClose(wxCloseEvent& event) {
    Show(false);
}





