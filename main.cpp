#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "solver.h"
#include "resource.h"

// Windowpointers
HWND hwnd,SolveKnop,ClearKnop,ReadKnop;
HWND LogWindow,ThreadText,ThreadInput,PassesText,PassesInput;
HWND InvoerVakjes[9][9];

// De vaste matrix
short GlobalMatrix[9][9] = {0};

void LogWrite(const char *format,...) {
	va_list parameters;

	char text[2000] = "";

	va_start(parameters,format);
	vsprintf(text,format,parameters);
	va_end(parameters);
	
	// Update het log
	int len = SendMessage(LogWindow,WM_GETTEXTLENGTH,0,0);
	SendMessage(LogWindow,EM_SETSEL,(WPARAM)len,(LPARAM)len);
	SendMessage(LogWindow,EM_REPLACESEL,0,(LPARAM)text);
}

void SolverThread(ArgStruct *args) {
	for(unsigned int i = 0;i < args->numpasses;i++) {
		args->Solver->AssignWork(GlobalMatrix);
		args->Solver->Solve();
	}
}

void ReadSudokuWindows(short (*output)[9]) {
	LogWrite("Reading input fields...\r\n");
	char msg[10] = "";
	for(int i = 0;i < 9;i++) { // Down
		for(int j = 0;j < 9;j++) { // Right
			SendMessage(InvoerVakjes[i][j],WM_GETTEXT,2,(LPARAM)msg);
			if(sscanf(msg,"%hd",&output[i][j]) == 0) {
				output[i][j] = 0;
			}
		}
	}
}

void UpdateSudokuWindows(const short (*input)[9]) {
	LogWrite("Updating input fields...\r\n");
	char msg[10] = "";
	for(int i = 0;i < 9;i++) { // Down
		for(int j = 0;j < 9;j++) { // Right
			sprintf(msg,"%d",input[i][j]);
			SendMessage(InvoerVakjes[i][j],WM_SETTEXT,0,(LPARAM)msg);
		}
	}	
}

bool OpenSudoku(const short (*output)[9]) {
	// Storage
	char sudokupath[MAX_PATH] = "";
	char currentregel[100] = "";

	// Open bestand-window
	OPENFILENAME ofn;
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Sudokubestand (*.sud; *.txt)\0*.sud;*.txt\0";
	ofn.lpstrFile = sudokupath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Selecteer sudoku";
	ofn.Flags = OFN_PATHMUSTEXIST;
	if(GetOpenFileName(&ofn)) {
		FILE* sudokufile = fopen(sudokupath,"r");
		if(sudokufile == NULL) {
			LogWrite("Error opening file:\r\n%s",sudokupath);
			return false;
		} else {
			int regelidx = 0;
	
			while(fgets(currentregel,sizeof(currentregel),sudokufile)) {
				// Hier staat in currentregel de huidige regel
				sscanf(currentregel,"%hd %hd %hd %hd %hd %hd %hd %hd %hd\n",
					&output[regelidx][0],
					&output[regelidx][1],
					&output[regelidx][2],
					&output[regelidx][3],
					&output[regelidx][4],
					&output[regelidx][5],
					&output[regelidx][6],
					&output[regelidx][7],
					&output[regelidx][8]
				);
				regelidx++;
			}
		}
		fclose(sudokufile);		
	} else {
		return false;
	}
	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case ID_OPEN: {
					OpenSudoku(GlobalMatrix);
					UpdateSudokuWindows(GlobalMatrix);
					break;
				}
				case ID_CLEAR: {
					for(int i = 0;i < 9;i++) { // Down
						for(int j = 0;j < 9;j++) { // Right
							GlobalMatrix[i][j] = 0;
						}
					}
					UpdateSudokuWindows(GlobalMatrix);
					break;
				}
				case ID_SOLVE: {
					// Lees de vakjes
					ReadSudokuWindows(GlobalMatrix);
		
					// Ingevoerde gegevens
					int numthreads = 1;
					char msg[10] = "";
					SendMessage(ThreadInput,WM_GETTEXT,10,(LPARAM)msg);
					if(sscanf(msg,"%d",&numthreads) != 1) {
						numthreads = 1;
						sprintf(msg,"%d",numthreads);
						SendMessage(ThreadInput,WM_SETTEXT,0,(LPARAM)msg);
					}

					// Ingevoerde gegevens #2
					int numpasses = 1;
					SendMessage(PassesInput,WM_GETTEXT,10,(LPARAM)msg);
					if(sscanf(msg,"%d",&numpasses) != 1) {
						numpasses = 1000;
						sprintf(msg,"%d",numpasses);
						SendMessage(PassesInput,WM_SETTEXT,0,(LPARAM)msg);
					}
					
					ArgStruct *ArgStructs = (ArgStruct*)malloc(numthreads*sizeof(ArgStruct));
					HANDLE *Threads = (HANDLE*)malloc(numthreads*sizeof(HANDLE));
					ClassSolver *Solvers = (ClassSolver*)malloc(numthreads*sizeof(ClassSolver));

					// Tijdspulletjes
					__int64 beg = 0;
					__int64 end = 0;
					__int64 countspersec = 0;
					float secpercount = 0.0f;

					// Hoe snel tikt de klok?
					QueryPerformanceFrequency((LARGE_INTEGER*)&countspersec);
					secpercount = 1.0f/(float)countspersec;
					
					for(unsigned int i = 0;i < numthreads;i++) {
						ArgStructs[i].numpasses = numpasses;
						ArgStructs[i].Solver = &Solvers[i];
						Threads[i] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SolverThread,&ArgStructs[i],CREATE_SUSPENDED,NULL);
					}
					
					// START!
					QueryPerformanceCounter((LARGE_INTEGER*)&beg);
					
					// Produces slightly more even timings per thread compared to starting immediately
					for(unsigned int i = 0;i < numthreads;i++) {
						ResumeThread(Threads[i]);
					}
					
					// Wachten tot de laatste thread klaar is
					WaitForSingleObject(Threads[numthreads-1],INFINITE);
					
					// STOP!
					QueryPerformanceCounter((LARGE_INTEGER*)&end);
					
					// Schrijf de data van de laatste thread op
					UpdateSudokuWindows(Solvers[numthreads-1].matrix);

					// Print tijdverschil
					if(Solvers[numthreads-1].solveresult) {
						LogWrite("\r\nSuccess!\r\n\r\n");
						LogWrite("- Time needed for %d passes: %gms\r\n",numpasses*numthreads,(end-beg)*secpercount*1000.0f);
						LogWrite("- Time needed for one pass: %gms\r\n",(end-beg)*secpercount*1000.0f/(float)(numpasses*numthreads));
						LogWrite("- Number of sudokus per second: %g\r\n",1.0f/((end-beg)*secpercount/(float)(numpasses*numthreads)));
					} else {
						LogWrite("Failed to solve Sudoku...\r\n");
					}
					SendMessage(LogWindow,EM_SCROLL,(WPARAM)SB_LINEUP,0);
					
					// Ruim je zooi op - gaat nie goed
					//free(ArgStructs);
					//free(Threads);
					//free(Solvers);
					break;
				}
			}
			break;
		}
		case WM_CREATE: {
			SolveKnop = CreateWindow(
				"BUTTON","Solve",WS_CHILD|WS_VISIBLE,5,480,80,30,hwnd,(HMENU)ID_SOLVE,GetModuleHandle(NULL),NULL);
			ClearKnop = CreateWindow(
				"BUTTON","Clear",WS_CHILD|WS_VISIBLE,90,480,80,30,hwnd,(HMENU)ID_CLEAR,GetModuleHandle(NULL),NULL);
			ReadKnop = CreateWindow(
				"BUTTON","Open...",WS_CHILD|WS_VISIBLE,175,480,80,30,hwnd,(HMENU)ID_OPEN,GetModuleHandle(NULL),NULL);
			LogWindow = CreateWindowEx(
				WS_EX_CLIENTEDGE,"EDIT","",WS_CHILD|WS_VISIBLE|ES_READONLY|ES_MULTILINE|WS_VSCROLL,5,520,465,100,hwnd,(HMENU)ID_LOG,GetModuleHandle(NULL),NULL);
			
			// Opties
			ThreadText = CreateWindow(
				"STATIC","Threads:",WS_CHILD|WS_VISIBLE,260,485,60,30,hwnd,(HMENU)ID_THREADSTEXT,GetModuleHandle(NULL),NULL);
			ThreadInput = CreateWindowEx(
				WS_EX_CLIENTEDGE,"EDIT","1",WS_CHILD|WS_VISIBLE|ES_NUMBER,320,482,30,26,hwnd,(HMENU)ID_THREADS,GetModuleHandle(NULL),NULL);
			SendMessage(ThreadInput,EM_LIMITTEXT,(WPARAM)2,0);
			PassesText = CreateWindow(
				"STATIC","Passes:",WS_CHILD|WS_VISIBLE,360,485,60,30,hwnd,(HMENU)ID_PASSESTEXT,GetModuleHandle(NULL),NULL);
			PassesInput = CreateWindowEx(
				WS_EX_CLIENTEDGE,"EDIT","1000",WS_CHILD|WS_VISIBLE|ES_NUMBER,410,482,60,26,hwnd,(HMENU)ID_PASSES,GetModuleHandle(NULL),NULL);
			SendMessage(PassesInput,EM_LIMITTEXT,(WPARAM)6,0);

			// Maak mooi font
			HDC hdc = GetDC(hwnd);
			HFONT font = CreateFont(-MulDiv(10,GetDeviceCaps(hdc, LOGPIXELSY),72),0,0,0,0,0,0,0,0,0,0,0,0,"Segoe UI");
			SendMessage(SolveKnop,WM_SETFONT,(WPARAM)font,0);
			SendMessage(ClearKnop,WM_SETFONT,(WPARAM)font,0);
			SendMessage(ReadKnop,WM_SETFONT,(WPARAM)font,0);
			SendMessage(LogWindow,WM_SETFONT,(WPARAM)font,0);
			SendMessage(ThreadText,WM_SETFONT,(WPARAM)font,0);
			SendMessage(ThreadInput,WM_SETFONT,(WPARAM)font,0);
			SendMessage(PassesText,WM_SETFONT,(WPARAM)font,0);
			SendMessage(PassesInput,WM_SETFONT,(WPARAM)font,0);
			HFONT vakfont = CreateFont(-MulDiv(22,GetDeviceCaps(hdc, LOGPIXELSY),72),0,0,0,0,0,0,0,0,0,0,0,0,"Segoe UI");

			int left = 5, top = 5;
			for(int i = 0;i < 9;i++) { // Down
				for(int j = 0;j < 9;j++) { // Right
					InvoerVakjes[i][j] = CreateWindow(
						"EDIT","0",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|ES_NUMBER|ES_CENTER,left,top,46,46,hwnd,0,GetModuleHandle(NULL),NULL);
					SendMessage(InvoerVakjes[i][i],EM_LIMITTEXT,(WPARAM)1,0);
					SendMessage(InvoerVakjes[i][j],WM_SETFONT,(WPARAM)vakfont,0);
					
					left += 50;
					if(j%3 == 2) {
						left += 10;
					}
				}
				
				top += 50;
				if(i%3 == 2) {
					top += 10;
				}
				left = 5;
			}
			ReleaseDC(hwnd,hdc);
			break;
		}
		case WM_CLOSE: {
			DestroyWindow(hwnd);
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	MSG Msg;

	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(IDI_MAINICON),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	wc.hIconSm		 = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(IDI_MAINICON),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL,"Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindow("WindowClass","Sudoku-oplosser",WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,50,50,482,655,NULL,NULL,hInstance,NULL);
	if(hwnd == NULL) {
		MessageBox(NULL,"Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
