#pragma once

#include "CoreGlobal.h"
#include "Logger.h"
#include "ErrorCode.h"

// 여러 소스에서 반복 사용하는 헤더의 처리 결과를 미리 컴파일해 재사용한다.
// CMake가 Core·서버·테스트 클라이언트의 각 타깃에 등록된 C++ 소스에 자동으로 포함한다.
#include <iostream>
#include <windows.h>
#include <winsock2.h>

using namespace std;