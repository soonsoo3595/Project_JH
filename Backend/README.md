# 백엔드 기본 템플릿

Windows IOCP 백엔드를 만들기 위한 C++17 실행 기반이다. 현재 서버·테스트 클라이언트 모두 Winsock 초기화 후 메시지를 출력하고 종료한다. 소켓 연결·IOCP·Protobuf 송수신·MySQL은 아직 구현하지 않았다.

## 구성

- `Core/`: Winsock 초기화·해제, 공통 오류 코드와 로거를 담당하는 라이브러리. WinsockRuntime은 복사·이동을 금지해 초기화와 해제 횟수를 맞춘다.
- `Server/`: `jh_backend_server` 실행 진입점.
- `TestClient/`: `jh_backend_test_client` 실행 진입점.
- `Tests/`: 로거의 레벨 필터·UTF-8 출력·동시 기록·실패 처리를 확인하는 로컬 전용 테스트. Git에서 제외한다.
- `vcpkg.json`: manifest 모드 의존성 목록. 현재는 외부 의존성 없이 비워 두었으며 Protobuf는 통신 구현 시 추가한다.
- `CMakePresets.json`: VS 2026 x64 구성과 Debug·Release 빌드·CTest 실행 설정.

## 준비와 빌드

Visual Studio 2026의 C++ 데스크톱 개발 도구·Windows SDK·CMake·vcpkg가 필요하다. CMake 최소 버전은 VS 2026 생성기를 지원하는 4.2다. 다음 명령은 이 `Backend` 폴더에서 실행한다.

PowerShell에서 `cmake`·`ctest`가 PATH에 없다면 Visual Studio Developer PowerShell을 사용하거나 설치된 실행 파일의 전체 경로로 호출한다. `VCPKG_ROOT`는 자신의 vcpkg 설치 경로로 설정한다. 아래 경로는 이번 PC에서 확인한 예시이며 프로젝트 설정에 하드코딩하지 않았다.

```powershell
$env:VCPKG_ROOT = 'C:\Program Files\Microsoft Visual Studio\18\Community\VC\vcpkg'
cmake --preset windows-vs2026
cmake --build --preset windows-debug
ctest --preset windows-debug
cmake --build --preset windows-release
ctest --preset windows-release
```

실행 파일은 `out/build/windows-vs2026/Debug/` 또는 `Release/`에 생성된다. CTest에는 서버·클라이언트 시작 검사 2개가 등록되며, 로컬 `Tests/` 소스가 있으면 로거 검사를 추가해 총 3개가 된다. 테스트 소스가 없는 체크아웃에서도 빌드할 수 있다. 두 시작 검사는 실제 프로그램 실행·Winsock 초기화·정상 종료만 확인한다. 네트워크 통신 테스트로 해석하지 않는다.

## 공통 로거

`Core/Logger.h`의 전역 `GLogger`를 사용한다. `main()`에서 출력 대상과 최소 레벨을 설정하고, 이후에는 로그 매크로만 호출한다.

```cpp
const ErrorCode result = GLogger->Init(LogOutput::Both, LogLevel::Info,
    "logs/backend-server.log");
if (result != ErrorCode::EC_SUCCESS)
{
    LOG_ERROR("로그 초기화 실패: %s", GetErrorMessage(result));
    return 1;
}

LOG_INFO("서버 시작");
LOG_WARNING("접속 수: %d", 100);
LOG_ERROR("연결 실패: %d", error);
```

- 출력 대상: `LogOutput::Console`, `File`, `Both`. 파일 기본 경로는 `logs/backend.log`다.
- 레벨: `Trace < Debug < Info < Warning < Error < Fatal`. 기본 최소 레벨은 `Info`이며 `SetMinimumLevel()`로 변경한다.
- 매크로: `LOG_TRACE / LOG_DEBUG / LOG_INFO / LOG_WARNING / LOG_ERROR / LOG_FATAL`. `Fatal`도 프로그램을 자동 종료하지 않는다.
- 형식: `[로컬 날짜와 시간.밀리초] [레벨] [호출 함수] 메시지`. Release에서는 `LOG_TRACE`·`LOG_DEBUG`와 그 인자 평가를 제거한다.
- UTF-8 printf 형식의 `%d`·`%s` 등을 사용한다. 동적 문자열은 `LOG_INFO("%s", message.c_str())` 또는 `GLogger->Log(LogLevel::Info, message)`로 전달한다.

`Init()`만 `[[nodiscard]] ErrorCode`를 반환한다. 잘못된 출력·레벨, 빈 경로, 디렉터리 생성 실패, 파일 열기 실패를 구분하며 실패 시 기존 설정을 유지한다. 개별 로그 호출은 `void`이며 반환값을 검사하지 않는다. 공개 진단 구조체와 별도 옵션 구조체는 두지 않는다. 잘못된 레벨이나 빈 형식 포인터로 출력하면 기록하지 않으며, 잘못된 최소 레벨 변경은 무시한다.

파일은 부모 폴더를 자동 생성하고 기존 파일에 UTF-8 BOM 없이 추가 기록한다. 실제 콘솔에서는 UTF-16 `WriteConsoleW`를 사용하고 리다이렉션에는 UTF-8을 유지한다. 콘솔 코드 페이지는 변경하지 않는다. `Error`·`Fatal`은 표준 오류, 나머지는 표준 출력으로 보낸다. 한글 파일 경로는 `std::filesystem::u8path()`로 만든다.

같은 로거의 호출은 mutex로 직렬화하며 호출한 스레드에서 출력·파일 flush를 완료한다. 콘솔 출력에 실패해도 파일 기록을 시도하고, 파일 기록에 실패하면 표준 오류에 짧게 알린다. 출력 성공을 호출부에 반환하거나 보장하지 않는다. 비동기 큐·파일 순환·보관 기간 관리는 포함하지 않는다.

서버는 `logs/backend-server.log`, 테스트 클라이언트는 `logs/backend-test-client.log`에 기록한다. 상대 경로는 실행 시 작업 디렉터리 기준이며 `logs/`는 Git에서 제외한다.

`CoreGlobal`이 로거를 멤버로 소유하고 `GLogger`는 재할당할 수 없는 접근 포인터다. 기본 상태는 콘솔·Info다. 작업 스레드는 `main()` 종료 전에 모두 종료·합류해야 한다. `GLogger`를 직접 삭제하거나 다른 전역 객체의 생성자·소멸자에서 로그를 호출하지 않는다. 독립 출력이 필요하면 지역 `Logger`를 생성해 같은 `Init()`을 사용한다.

오류 코드 항목은 `EC_대문자_스네이크` 형식으로 명명하고 종류별로 100개 단위 구간을 사용한다. 현재 로거는 100~199이며 정의된 항목은 100~104다. 다른 종류의 구간은 해당 기능 구현 시 추가한다. 공통 오류 코드는 `Core/ErrorCode.h`에서 정의한다. 성공은 `EC_SUCCESS = 0`이며 `GetErrorMessage()`로 한국어 설명을 얻는다. 업무상 실패를 판단해야 하는 작업은 오류 코드를 반환하고, 로그를 남기는 호출에는 이 규칙을 적용하지 않는다. 단순 상태 판단·값 조회 및 Winsock 원본 오류 조회도 기존 반환형을 유지한다. 프로젝트의 `try-catch`와 직접 `throw`는 사용하지 않으며 자원은 RAII로 정리한다. 표준 라이브러리의 자원 부족 예외까지 반환값으로 바꾸지는 않는다.

CTest는 레벨 필터·한글·추가 기록·동시 기록·출력 대상·초기화 실패 및 매크로 동작을 확인한다. `jh_backend_logger_tests --console`은 실제 콘솔의 한글 출력과 코드 페이지 유지 여부를 확인하며, 일반 검사는 빌드 폴더의 `logger-tests-*`에 산출물을 남긴다.

## 미리 컴파일된 헤더(PCH)

`Core/Pch.h`에 현재 사용하는 시스템·표준 헤더인 `winsock2.h`와 `iostream`을 모았다. CMake의 `target_precompile_headers`로 Core·서버·테스트 클라이언트에 적용하며 각 타깃·구성별로 PCH를 생성한다.

CMake가 PCH 생성용 소스와 자동 포함 설정을 관리하므로 별도 `Pch.cpp`나 소스별 `#include "Core/Pch.h"`는 필요하지 않다. 소스에서 사용하는 헤더의 직접 include는 유지한다. 프로젝트 헤더는 PCH에 넣지 않고 각 소스에서 직접 포함한다.

## 인코딩

Visual Studio CMake 출력 창에서 vcpkg의 한국어 메시지가 깨지는 경우를 피하기 위해 프리셋에 `VSLANG=1033`을 설정했다. 빌드 도구 메시지는 영어로 출력하며 IDE 표시 언어와 프로젝트가 작성한 한국어 실행 로그는 변경하지 않는다. 수정 후 Visual Studio에서 CMake 구성을 다시 실행한다. 이는 출력 인코딩을 직접 수정하는 대신 비ASCII 메시지를 피하는 우회 방법이다.

직접 작성한 C++ 소스·헤더는 UTF-8 BOM과 CRLF를 사용한다. MSVC의 `/utf-8` 옵션으로 소스 문자 집합과 실행 문자열을 UTF-8로 지정한다. 백엔드 내부 일반 텍스트는 UTF-8 `std::string`을 사용하며 Windows UTF-16 API와는 경계에서 변환한다.

이 폴더의 `.gitattributes`는 해당 C++ 파일을 텍스트로 관리하고 체크아웃 시 CRLF를 유지한다. 인코딩 재변환은 사용하지 않는다. CMake·JSON·Markdown은 UTF-8이다. 생성 코드·외부 라이브러리에는 소스 변환 규칙을 적용하지 않는다.

UTF-8 BOM과 `/utf-8`만으로 콘솔 출력 코드 페이지가 바뀌지는 않는다. 현재 공통 로거는 실제 콘솔에서 `WriteConsoleW`를 사용하므로 코드 페이지에 의존하지 않으며 두 진입점의 `SetConsoleOutputCP` 초기화는 제거했다. 파일·파이프에는 UTF-8을 유지한다.

## 다음 작업

IOCP·세션 수명·종료 절차를 설계하고, 이후 프레이밍·메시지 크기·Protobuf 스키마를 정한다. 이번 템플릿은 이 설계 선택을 확정하지 않는다. GitHub Actions 워크플로도 아직 구성하지 않았다.

## 확인 결과와 근거

2026-10-21에 MSVC 19.51.36260.0, SDK 10.0.26100.0, CMake 4.3.1-msvc1, vcpkg 2026-07-27에서 Debug·Release 빌드 및 각 CTest 시작 검사 2/2 통과를 확인했다. 서버·클라이언트 직접 실행의 한국어 출력과 UTF-16 LE BOM도 확인했다. UE·네트워크·DB·GitHub Actions는 검증하지 않았다.

2026-10-23에 직접 작성한 소스·헤더 5개의 UTF-8 BOM과 변환 전후 텍스트 동일성을 확인했다. manifest 설치 ON으로 CMake 구성 후 PCH 포함 Debug·Release 재빌드와 각 CTest 시작 검사 2/2가 통과했다. Visual Studio F5 화면은 확인하지 않았다.

- [MSVC 소스 문자 집합과 BOM 감지](https://learn.microsoft.com/en-us/cpp/build/reference/source-charset-set-source-character-set)
- [CMake VS 2026 생성기](https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2018%202026.html)
- [Git working-tree-encoding](https://git-scm.com/docs/gitattributes)
