#include "main.h"

/* This function is called by the Windows function DispatchMessage() */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static RESFILE res;
	static RESDATA data;

	switch(message)						/* handle the messages */
	{
		case WM_CREATE:
			createGUI(hwnd, &data);
		break;
		case WM_SIZE:
			resizeAllWindows(hwnd);
			makeHexReadable(hwnd, &data);
		break;
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_OPEN)
				clickOpen(hwnd, &res);
			if(HIWORD(wParam) == EN_VSCROLL)
				scrollEqual(hwnd, wParam);
		break;
		case WM_VSCROLL:
			scrollWindow(hwnd, wParam);
		break;
		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->code == TVN_SELCHANGED)
				addTextToEdit(hwnd, &res, &data);
		break;
		case WM_DESTROY:
			PostQuitMessage(0);			/* send a WM_QUIT to the message queue */
		break;
		default:						/* for messages that we don't deal with */
			return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
	HWND hwnd;				/* This is the handle for our window */
	MSG messages;			/* Here messages to the application are saved */
	WNDCLASSEX wincl;		/* Data structure for the windowclass */

	hPrevInstance = 0;		/* Suppress unused variable varnings */
	lpszArgument = "";

	/* The Window structure */
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = "ResolutionApp";
	wincl.lpfnWndProc = WindowProcedure;
	wincl.style = CS_DBLCLKS;
	wincl.cbSize = sizeof(WNDCLASSEX);
	wincl.hIcon = LoadIcon(hThisInstance, "IDI_ICON");
	wincl.hIconSm = LoadIcon(hThisInstance, "IDI_ICON");
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground = (HBRUSH)COLOR_HIGHLIGHT;

	/* Register the window class, and if it fails quit the program */
	if(!RegisterClassEx(&wincl))
		return 0;

	/* The class is registered, let's create the program*/
	hwnd = CreateWindow(
			"ResolutionApp",					/* Classname */
			"Resolution",						/* Title Text */
			WS_OVERLAPPEDWINDOW,				/* default window */
			GetSystemMetrics(SM_CYICONSPACING),	/* Windows decides the position */
			0,									/* where the window ends up on the screen */
			PROGWIDTH,							/* The programs width */
			GetSystemMetrics(SM_CYMAXIMIZED)-8,	/* and height in pixels */
			HWND_DESKTOP,						/* The window is a child-window to desktop */
			NULL,								/* No menu */
			hThisInstance,						/* Program Instance handler */
			NULL								/* No Window Creation data */
	);

	/* Make the window visible on the screen */
	ShowWindow(hwnd, nCmdShow);

	/* Run the message loop. It will run until GetMessage() returns 0 */
	while(GetMessage(&messages, NULL, 0, 0))
	{
		/* Translate virtual-key messages into character messages */
		TranslateMessage(&messages);
		/* Send message to WindowProcedure */
		DispatchMessage(&messages);
	}

	/* The program return-value is 0 - The value that PostQuitMessage() gave */
	return messages.wParam;
}
