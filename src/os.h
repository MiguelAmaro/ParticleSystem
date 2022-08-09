#ifndef OS_H
#define OS_H

typedef struct time_measure time_measure;
struct time_measure
{
  f64 MSDelta;
  f64 MSElapsed;
  u64 TickFrequency;
  u64 WorkStartTick;
  u64 WorkEndTick;
  u64 WorkTickDelta;
  f64 MicrosElapsedWorking;
  f64 TargetMicrosPerFrame;
  f64 TicksToMicros;
};

time_measure OSInitTimeMeasure(void)
{
  time_measure Result = {0};
  Result.MSDelta   = 0.0;
  Result.MSElapsed = 0.0;
  Result.TickFrequency = 0;
  Result.WorkStartTick = 0;
  Result.WorkEndTick = 0;
  Result.WorkTickDelta = 0;
  Result.MicrosElapsedWorking = 0.0;
  QueryPerformanceFrequency((LARGE_INTEGER *)&Result.TickFrequency);
  Result.TargetMicrosPerFrame = 16666.0;
  Result.TicksToMicros = 1000000.0/(f64)Result.TickFrequency;
  return Result;
}
f64 OSTimeMeasureElapsed(time_measure *Time)
{
  QueryPerformanceCounter((LARGE_INTEGER *)&Time->WorkEndTick);
  Time->WorkTickDelta  = Time->WorkEndTick - Time->WorkStartTick;
  Time->MicrosElapsedWorking = (f64)Time->WorkTickDelta*Time->TicksToMicros;
  u64 IdleTickDelta = 0;
  u64 IdleStartTick = Time->WorkEndTick;
  u64 IdleEndTick = 0;
  f64 MicrosElapsedIdle = 0.0;
  while((Time->MicrosElapsedWorking+MicrosElapsedIdle)<Time->TargetMicrosPerFrame)
  {
    QueryPerformanceCounter((LARGE_INTEGER *)&IdleEndTick);
    IdleTickDelta = IdleEndTick-IdleStartTick;
    MicrosElapsedIdle = (f64)IdleTickDelta*Time->TicksToMicros;
  }
  f64 FrameTimeMS = (Time->MicrosElapsedWorking+MicrosElapsedIdle)/1000.0;
  Time->MSDelta    = FrameTimeMS;
  Time->MSElapsed += FrameTimeMS;
  return Time->MSDelta;
}
//~ MEMORY
fn void *OSMemoryAlloc(u64 Size)
{
  void *Result = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  Assert(Result);
  return Result;
}
//~ FILE STUFF
fn u64 OSFileGetSize(HANDLE File)
{
  LARGE_INTEGER Whatever = { 0 };
  u64 Result = 0;
  b32 Status = GetFileSizeEx(File, &Whatever);
  Result = Whatever.QuadPart;
  Assert(Status != 0);
  return Result;
}
fn str8 OSFileRead(str8 Path, arena *Arena)
{
  str8 Result = {0};
  HANDLE File = CreateFileA((LPCSTR)Path.Data, GENERIC_READ, FILE_SHARE_READ,
                            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if(File)
  {
    u64 ExpectedSize = OSFileGetSize(File); Assert(ExpectedSize<U32MAX);
    u64 BytesRead    = 0;
    Result = Str8(Arena, ExpectedSize);
    ReadFile(File, Result.Data, (u32)ExpectedSize, (LPDWORD)&BytesRead, NULL);
    Assert(BytesRead == ExpectedSize);
    Result.Size = BytesRead;
  }
  CloseHandle(File);
  return Result;
}
//~ CONSOLE STUFF
HANDLE gConsole = NULL;
#define ConsoleLog(...) _Generic(ARG1(__VA_ARGS__), \
arena: OSConsoleLogF, \
char *: OSConsoleLogStrLit)(__VA_ARGS__)
fn u32 OSConsoleLogF(arena Scratch, char *Format, ...)
{
  va_list ArgList;
  va_start(ArgList, Format);
  str8 String = Str8FromArena(&Scratch, vsnprintf(NULL, 0, Format, ArgList) + 1);
  vsnprintf((char *)String.Data, String.Size, Format, ArgList);
  u32 WriteCount = 0;
  WriteConsoleA(gConsole, String.Data, (u32)String.Size, (LPDWORD)&WriteCount, NULL);
  return WriteCount;
}
fn u32 OSConsoleLogStrLit(char *StringLit)
{
  str8 String = Str8((u8 *)StringLit, CStrGetSize(StringLit, 1));
  u32 WriteCount = 0;
  WriteConsoleA(gConsole, String.Data, (u32)String.Size, (LPDWORD)&WriteCount, NULL);
  return WriteCount;
}
fn void OSConsoleCreate(void)
{
  b32 Status = AllocConsole();
  Assert(Status != 0);
  gConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                       FILE_SHARE_READ, NULL,
                                       CONSOLE_TEXTMODE_BUFFER, NULL);
  Status = SetConsoleActiveScreenBuffer(gConsole);
  return;
}
fn static void FatalError(const char* message)
{
  MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
  ExitProcess(0);
}
//~ WINDOW
fn v2s OSWindowGetSize(HWND Window)
{
  RECT Rect = {0};
  GetClientRect(Window, &Rect); // get current size for window client area
  v2s WindowDim = V2s(Rect.right  - Rect.left, Rect.bottom - Rect.top);
  return WindowDim;
}
//~ EVENTS
fn b32 OSProcessMessges(void)
{
  MSG Msg;
  b32 ShouldQuit = 0;
  while(PeekMessageW(&Msg, NULL, 0, 0, PM_REMOVE))
  {
    if (Msg.message == WM_QUIT)
    { 
      ShouldQuit = 1;
      break;
    }
    TranslateMessage(&Msg);
    DispatchMessageW(&Msg);
  }
  return ShouldQuit;
}
static LRESULT CALLBACK WindowEventProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(wnd, msg, wparam, lparam);
}
fn HWND OSWindowCreate(HINSTANCE Instance, v2s WindowDim)
{
  // register window class to have custom WindowProc callback
  WNDCLASSEXW WindowClass =
  {
    .cbSize = sizeof(WindowClass),
    .lpfnWndProc = WindowEventProc,
    .hInstance = Instance,
    .hIcon = LoadIcon(NULL, IDI_APPLICATION),
    .hCursor = LoadCursor(NULL, IDC_ARROW),
    .lpszClassName = L"d3d11_window_class",
  };
  ATOM atom = RegisterClassExW(&WindowClass);
  Assert(atom && "Failed to register window class");
  // window properties - width, height and style
  // WS_EX_NOREDIRECTIONBITMAP flag here is needed to fix ugly bug with Windows 10
  // when window is resized and DXGI swap chain uses FLIP presentation model
  // DO NOT use it if you choose to use non-FLIP presentation model
  DWORD exstyle = WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
  DWORD style = WS_OVERLAPPEDWINDOW;
  // uncomment in case you want fixed size window
  //style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
  //RECT rect = { 0, 0, 1280, 720 };
  //AdjustWindowRectEx(&rect, style, FALSE, exstyle);
  //width = rect.right - rect.left;
  //height = rect.bottom - rect.top;
  // create window
  HWND Window = CreateWindowExW(exstyle, WindowClass.lpszClassName, L"D3D11 Window", style,
                                CW_USEDEFAULT, CW_USEDEFAULT, WindowDim.x, WindowDim.y,
                                NULL, NULL, WindowClass.hInstance, NULL);
  return Window;
}

#endif //OS_H
