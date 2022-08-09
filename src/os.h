#ifndef OS_H
#define OS_H

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


#endif //OS_H
