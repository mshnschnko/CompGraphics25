#include "common.h"

void beginEvent(const wchar_t* str) {
#ifdef _DEBUG
  DebugEvents::GetInstance().beginEvent(str);
#endif
}

void endEvent() {
#ifdef _DEBUG
  DebugEvents::GetInstance().endEvent();
#endif
}