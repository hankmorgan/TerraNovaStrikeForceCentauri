#include "gui.h"

void createGUI(HWND hwnd, RESDATA *data)
{
	InitCommonControls();
	createTool(hwnd);
	createStat(hwnd);
	createTree(hwnd);
	createScroll(hwnd);
	createEdit(hwnd, data);
}

void createTool(HWND hwnd)
{
	HWND hTool;
	TBADDBITMAP tbab;
	TBBUTTON tbb[1];

	hTool = CreateWindow(TOOLBARCLASSNAME, "", WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hwnd, (HMENU)IDC_TOOL, GetModuleHandle(NULL), NULL);

	if(hTool == NULL)
		MessageBox(NULL, "Could not create toolbar.", "Error", MB_OK | MB_ICONERROR);

	SendMessage(hTool, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	tbab.hInst = HINST_COMMCTRL;
	tbab.nID = IDB_STD_SMALL_COLOR;
	SendMessage(hTool, TB_ADDBITMAP, 0, (LPARAM)&tbab);

	ZeroMemory(tbb, sizeof(tbb));
	tbb[0].iBitmap = STD_FILEOPEN;
	tbb[0].idCommand = IDC_OPEN;
	tbb[0].fsState = TBSTATE_ENABLED;
	tbb[0].fsStyle = TBSTYLE_BUTTON;
	SendMessage(hTool, TB_ADDBUTTONS, sizeof(tbb)/sizeof(TBBUTTON), (LPARAM)&tbb);
}

void createStat(HWND hwnd)
{
	HWND hStat;

	hStat = CreateWindow(STATUSCLASSNAME, "", WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hwnd, (HMENU)IDC_STAT, GetModuleHandle(NULL), NULL);

	if(hStat == NULL)
		MessageBox(NULL, "Could not create status bar.", "Error", MB_OK | MB_ICONERROR);
}

void createTree(HWND hwnd)
{
	HWND hTree;

	hTree = CreateWindow(WC_TREEVIEW, "",
			TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | WS_BORDER,
			0, hwndH(GetDlgItem(hwnd, IDC_TOOL)), TREEWIDTH,
			hwndH(hwnd)-2*GetSystemMetrics(SM_CXSIZEFRAME)-GetSystemMetrics(SM_CYSIZE)-
			hwndH(GetDlgItem(hwnd, IDC_TOOL))-hwndH(GetDlgItem(hwnd, IDC_STAT)),
			hwnd, (HMENU)IDC_TREE, GetModuleHandle(NULL), NULL);

	if(hTree == NULL)
		MessageBox(hwnd, "Could not create treeview.", "Error", MB_OK | MB_ICONERROR);
}

void createScroll(HWND hwnd)
{
	HWND hScroll;

	hScroll = CreateWindow("SCROLLBAR", "", SBS_RIGHTALIGN | SBS_VERT | WS_CHILD,
		0, hwndH(GetDlgItem(hwnd, IDC_TOOL)), PROGWIDTH-2*GetSystemMetrics(SM_CXSIZEFRAME),
		GetSystemMetrics(SM_CYMAXIMIZED)-GetSystemMetrics(SM_CYMIN)-2*GetSystemMetrics(SM_CYSIZEFRAME)-
		hwndH(GetDlgItem(hwnd, IDC_TOOL))-hwndH(GetDlgItem(hwnd, IDC_STAT)),
		hwnd, (HMENU)IDC_SCRL, GetModuleHandle(NULL), NULL);

	if(hScroll == NULL)
		MessageBox(hwnd, "Could not create scrollbar.", "Error", MB_OK | MB_ICONERROR);
}

void createEdit(HWND hwnd, RESDATA *data)
{
	HWND hEdit1, hEdit2;
	HFONT hFont;

	hEdit1 = CreateWindow("EDIT", "", ES_MULTILINE | ES_AUTOVSCROLL | WS_CHILD | WS_BORDER,
			TREEWIDTH, hwndH(GetDlgItem(hwnd, IDC_TOOL)),
			(hwndW(hwnd)-TREEWIDTH-hwndW(GetDlgItem(hwnd, IDC_SCRL)))*0.74-GetSystemMetrics(SM_CXSIZEFRAME),
			hwndH(hwnd)-hwndH(GetDlgItem(hwnd, IDC_TOOL))-hwndH(GetDlgItem(hwnd, IDC_STAT))-
			GetSystemMetrics(SM_CYMIN), hwnd, (HMENU)IDC_EDITHEX, GetModuleHandle(NULL), NULL);

	hEdit2 = CreateWindow("EDIT", "", ES_MULTILINE | ES_AUTOVSCROLL | WS_CHILD | WS_BORDER,
			TREEWIDTH+hwndW(hEdit1), hwndH(GetDlgItem(hwnd, IDC_TOOL)),
			(hwndW(hwnd)-TREEWIDTH-hwndW(GetDlgItem(hwnd, IDC_SCRL)))*0.26-GetSystemMetrics(SM_CXSIZEFRAME),
			hwndH(hwnd)-hwndH(GetDlgItem(hwnd, IDC_TOOL))-hwndH(GetDlgItem(hwnd, IDC_STAT))-
			GetSystemMetrics(SM_CYMIN),	hwnd, (HMENU)IDC_EDITTXT, GetModuleHandle(NULL), NULL);

	if(hEdit1 == NULL || hEdit2 == NULL)
		MessageBox(hwnd, "Could not create edit control.", "Error", MB_OK | MB_ICONERROR);

	hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, "Terminal");
	SendMessage(hEdit1, WM_SETFONT, (WPARAM)hFont, FALSE);
	SendMessage(hEdit2, WM_SETFONT, (WPARAM)hFont, FALSE);

	SetWindowText(hEdit2, "\r\r\nSecond row");
	data->fontheight = HIWORD(SendMessage(hEdit2, EM_POSFROMCHAR, 3, 0));
}

int hwndH(HWND hwnd)
{
	RECT window;

	GetWindowRect(hwnd, &window);
	return window.bottom - window.top;
}

int hwndW(HWND hwnd)
{
	RECT window;

	GetWindowRect(hwnd, &window);
	return window.right - window.left;
}

void resizeAllWindows(HWND hwnd)
{
	HWND hTool = GetDlgItem(hwnd, IDC_TOOL);
	HWND hStat = GetDlgItem(hwnd, IDC_STAT);
	HWND hTree = GetDlgItem(hwnd, IDC_TREE);
	HWND hScrl = GetDlgItem(hwnd, IDC_SCRL);
	HWND hEditHex = GetDlgItem(hwnd, IDC_EDITHEX);
	HWND hEditTxt = GetDlgItem(hwnd, IDC_EDITTXT);

	SendMessage(hTool, TB_AUTOSIZE, 0, 0);
	SendMessage(GetDlgItem(hwnd, IDC_STAT), WM_SIZE, 0, 0);

	SetWindowPos(hTree, HWND_TOP, 0, 0, TREEWIDTH,
		hwndH(hwnd)-hwndH(hTool)-hwndH(hStat)-GetSystemMetrics(SM_CYMIN), SWP_NOMOVE);

	SetWindowPos(hScrl, HWND_TOP,
		hwndW(hwnd)-GetSystemMetrics(SM_CXHTHUMB)-2*GetSystemMetrics(SM_CXSIZEFRAME),
		hwndH(hTool), GetSystemMetrics(SM_CXHTHUMB),
		hwndH(hwnd)-hwndH(hTool)-hwndH(hStat)-GetSystemMetrics(SM_CYMIN), SWP_NOZORDER);

	SetWindowPos(hEditHex, HWND_TOP, 0, 0,
		(hwndW(hwnd)-TREEWIDTH-hwndW(hScrl))*0.74-GetSystemMetrics(SM_CXSIZEFRAME),
		hwndH(hwnd)-hwndH(hTool)-hwndH(hStat)-GetSystemMetrics(SM_CYMIN), SWP_NOMOVE);

	SetWindowPos(hEditTxt, HWND_TOP, TREEWIDTH+hwndW(hEditHex), hwndH(hTool),
		(hwndW(hwnd)-TREEWIDTH-hwndW(hScrl))*0.26-GetSystemMetrics(SM_CXSIZEFRAME),
		hwndH(hwnd)-hwndH(hTool)-hwndH(hStat)-GetSystemMetrics(SM_CYMIN), SWP_NOZORDER);
}

void clickOpen(HWND hwnd, RESFILE *res)
{
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "LG resource files (*.res, *.dat)\0*.res;*.dat\0\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn))
		verifyFile(hwnd, res, szFileName);
}

void verifyFile(HWND hwnd, RESFILE *res, char szFileName[MAX_PATH])
{
	HANDLE hFile;
	DWORD dwRead;
	char magic[14], message[MAX_PATH+32];

	hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		if(ReadFile(hFile, magic, 14, &dwRead, NULL))
		{
			if(strncmp(magic, "LG Res File v2", 14) != 0)
			{
				snprintf(message, MAX_PATH, "%s\nis not a LG resource file.", szFileName);
				MessageBox(hwnd, message, "Error", MB_OK | MB_ICONERROR);
			}
			else
			{
				TreeView_Select(GetDlgItem(hwnd, IDC_TREE), NULL, TVGN_CARET);
				TreeView_DeleteAllItems(GetDlgItem(hwnd, IDC_TREE));
				ShowWindow(GetDlgItem(hwnd, IDC_SCRL), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, IDC_EDITHEX), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, IDC_EDITTXT), SW_SHOW);
				SetWindowText(GetDlgItem(hwnd, IDC_EDITHEX), "");
				SetWindowText(GetDlgItem(hwnd, IDC_EDITTXT), "");
				SendMessage(GetDlgItem(hwnd, IDC_STAT), SB_SETTEXT, 0, (LPARAM)"");
				SetScrollPos(GetDlgItem(hwnd, IDC_SCRL), SB_CTL, 0, TRUE);
				strncpy(res->file, szFileName, strlen(szFileName)+1);
				readchunkdir(hwnd, res);
			}
		}
	}
	else
	{
		snprintf(message, MAX_PATH, "%s\nwas not found.", szFileName);
		MessageBox(hwnd, message, "Error", MB_OK | MB_ICONERROR);
	}

	CloseHandle(hFile);
}

void addItemToTree(HWND hwnd, LPTSTR lpszItem, int nLevel)
{
	TVITEM tvi;
	TVINSERTSTRUCT tvins;
	static HTREEITEM hPrev = (HTREEITEM)TVI_FIRST;
	static HTREEITEM hPrevRootItem = NULL;
	static HTREEITEM hPrevLev2Item = NULL;

	tvi.mask = TVIF_TEXT;
	tvi.pszText = lpszItem;
	tvi.lParam = (LPARAM)nLevel;
	tvins.item = tvi;
	tvins.hInsertAfter = hPrev;

	/* Set the parent item based on the specified level. */
	if (nLevel == 1)
		tvins.hParent = TVI_ROOT;
	if (nLevel == 2)
		tvins.hParent = hPrevRootItem;

	/* Add the item to the tree-view control. */
	hPrev = (HTREEITEM)SendMessage(hwnd, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);

	/* Save the handle to the item. */
	if (nLevel == 1)
		hPrevRootItem = hPrev;
	if (nLevel == 2)
		hPrevLev2Item = hPrev;
}

/*	Loop and count nodes until root is found, then we know which node was selected.
	Block -1 means that a parent is selected, don't display anything */
void addTextToEdit(HWND hwnd, RESFILE *res, RESDATA *data)
{
	HTREEITEM selected, test;
	unsigned int i;
	char buf[256];

	SendMessage(GetDlgItem(hwnd, IDC_STAT), SB_SETTEXT, 0, (LPARAM)"");
	test = selected = (HTREEITEM)SendDlgItemMessage(hwnd, IDC_TREE, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)selected);
	if(selected == NULL)
		return;

	for(i=-1; test!=NULL; i++)
		test = TreeView_GetPrevSibling(GetDlgItem(hwnd, IDC_TREE), test);

	test = TreeView_GetParent(GetDlgItem(hwnd, IDC_TREE), selected);
	if(test == NULL)
	{
		res->currchunk = i;
		res->currblock = -1;
	}
	else
	{
		res->currblock = i;
		for(i=-1; test!=NULL; i++)
			test = TreeView_GetPrevSibling(GetDlgItem(hwnd, IDC_TREE), test);
		res->currchunk = i;
	}

	if((res->type[res->currchunk] >= 2 &&
		(res->currblock < -1 || res->currblock > res->blocks[res->currchunk])) ||
		res->currchunk < 0 || res->currchunk > res->chunks)
		return;
	if(res->type[res->currchunk] >= 2 && res->currblock == -1)
	{
		SetWindowText(GetDlgItem(hwnd, IDC_EDITHEX), "");
		SetWindowText(GetDlgItem(hwnd, IDC_EDITTXT), "");
		return;
	}

	SendMessage(GetDlgItem(hwnd, IDC_STAT), SB_SETTEXT, 0, (LPARAM)"Loading...");
	readChunkOrBlock(res, data);
	makeHexReadable(hwnd, data);
	SetScrollPos(GetDlgItem(hwnd, IDC_SCRL), SB_CTL, 0, TRUE);
	snprintf(buf, 255, "[%d bytes]", data->size);
	SendMessage(GetDlgItem(hwnd, IDC_STAT), SB_SETTEXT, 0, (LPARAM)buf);
}

/*	Left window (75% width, 3 chars/byte) shows data as hex.
	Right window (25% width, 1 char/byte) shows data as text.
	Ensure that both windows show equal amount of chars per line */
void makeHexReadable(HWND hwnd, RESDATA *data)
{
	SCROLLINFO si;
	unsigned int i, j, cols;

	data->hex = malloc(3*data->size+1);
	for(i=0; i<data->size; i++)
		sprintf(&data->hex[3*i], "%X%X ", data->unpack[i]>>4, data->unpack[i]&0x0F);
	SetWindowText(GetDlgItem(hwnd, IDC_EDITHEX), data->hex);

	cols = floor(SendMessage(GetDlgItem(hwnd, IDC_EDITHEX), EM_LINELENGTH, 0, 0)/3);
	cols = (cols < 1)? 1:cols;

	data->txt = malloc(4*data->size+1);
	for(i=0, j=0; i<data->size; i++, j++)
	{
		if(i%cols == 0 && i>0)
		{
			sprintf(&data->txt[j], "\r\r\n");			/* EOL, insert soft linebreak */
			j = j + 3;
		}
		data->txt[j] = data->unpack[i];
		if(data->txt[j] == 0x00 || data->txt[j] == 0x09 || data->txt[j] == 0x0D)	/* NULL, tab, CR */
			data->txt[j] = ' ';
	}
	data->txt[j] = 0x00;

	SetWindowText(GetDlgItem(hwnd, IDC_EDITTXT), data->txt);
	free(data->hex);
	free(data->txt);

	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nMax = SendMessage(GetDlgItem(hwnd, IDC_EDITHEX), EM_GETLINECOUNT, 0, 0);
	si.nPage = hwndH(GetDlgItem(hwnd, IDC_EDITHEX)) / data->fontheight;
	si.nPos = 0;
	SetScrollInfo(GetDlgItem(hwnd, IDC_SCRL), SB_CTL, &si, TRUE);
}

void scrollWindow(HWND hwnd, WPARAM wParam)
{
	SCROLLINFO si;

	if(LOWORD(wParam) == SB_THUMBTRACK || LOWORD(wParam) == SB_THUMBPOSITION)
	{
		SendMessage(GetDlgItem(hwnd, IDC_EDITHEX), WM_VSCROLL, wParam, 0);
		SendMessage(GetDlgItem(hwnd, IDC_EDITTXT), WM_VSCROLL, wParam, 0);
	}
	else
	{
		SendMessage(GetDlgItem(hwnd, IDC_EDITHEX), WM_VSCROLL, LOWORD(wParam), 0);
		SendMessage(GetDlgItem(hwnd, IDC_EDITTXT), WM_VSCROLL, LOWORD(wParam), 0);
	}

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(GetDlgItem(hwnd, IDC_SCRL), SB_CTL, &si);
	switch(LOWORD(wParam))
	{
		case SB_LINEUP:
			si.nPos--; break;
		case SB_LINEDOWN:
			si.nPos++; break;
		case SB_PAGEUP:
			si.nPos -= si.nPage; break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage; break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos; break;
	}
	si.fMask = SIF_POS;
	SetScrollInfo(GetDlgItem(hwnd, IDC_SCRL), SB_CTL, &si, TRUE);
}

/*	When one window is scrolled (arrow keys, select text), the other window
	must be scrolled same amount in same direction */
void scrollEqual(HWND hwnd, WPARAM wParam)
{
	int hexpos, txtpos, scroll;

	hexpos = SendMessage(GetDlgItem(hwnd, IDC_EDITHEX), EM_GETFIRSTVISIBLELINE, 0, 0);
	txtpos = SendMessage(GetDlgItem(hwnd, IDC_EDITTXT), EM_GETFIRSTVISIBLELINE, 0, 0);
	SetScrollPos(GetDlgItem(hwnd, IDC_SCRL), SB_CTL, hexpos, TRUE);

	if(LOWORD(wParam) == IDC_EDITHEX)
	{
		scroll = hexpos - txtpos;
		SendMessage(GetDlgItem(hwnd, IDC_EDITTXT), EM_LINESCROLL, 0, scroll);
	}
	if(LOWORD(wParam) == IDC_EDITTXT)
	{
		scroll = txtpos - hexpos;
		SendMessage(GetDlgItem(hwnd, IDC_EDITHEX), EM_LINESCROLL, 0, scroll);
	}
}
