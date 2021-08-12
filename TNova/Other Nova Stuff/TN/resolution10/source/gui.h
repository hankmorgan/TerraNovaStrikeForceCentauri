#ifndef GUI_H
#define GUI_H

#include <windows.h>
#include <commctrl.h>
#include "res.h"

#define IDC_TOOL	1
#define IDC_OPEN	2
#define IDC_STAT	4
#define IDC_TREE	8
#define IDC_SCRL	16
#define IDC_EDITHEX	32
#define IDC_EDITTXT	64

#define PROGWIDTH	675		/* Progran width, fits 16 bytes per line */
#define TREEWIDTH	125		/* Treeview width, enough for large .res files */

void createGUI(HWND hwnd, RESDATA *data);
void createTool(HWND hwnd);
void createStat(HWND hwnd);
void createTree(HWND hwnd);
void createScroll(HWND hwnd);
void createEdit(HWND hwnd, RESDATA *data);
int hwndH(HWND hwnd);
int hwndW(HWND hwnd);
void resizeAllWindows(HWND hwnd);
void clickOpen(HWND hwnd, RESFILE *res);
void verifyFile(HWND hwnd, RESFILE *res, char szFileName[MAX_PATH]);
void addItemToTree(HWND hwnd, LPTSTR lpszItem, int nLevel);
void addTextToEdit(HWND hwnd, RESFILE *res, RESDATA *data);
void makeHexReadable(HWND hwnd, RESDATA *data);
void scrollWindow(HWND hwnd, WPARAM wParam);
void scrollEqual(HWND hwnd, WPARAM wParam);

#endif
