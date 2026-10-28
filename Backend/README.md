# 백엔드 기본 템플릿

Windows IOCP 백엔드를 만들기 위한 실행 기반이다. 빌드 설정은 C++17로 유지하고 직접 작성하는 코드는 C++11의 문법·표준 라이브러리 범위로 제한한다. 현재 서버·테스트 클라이언트 모두 Winsock 초기화 후 메시지를 출력하고 종료한다. IPv4 주소·TCP 소켓 함수와 IOCP 완료 큐 기본 기능은 준비했으며 제품의 접속·송수신 흐름·Protobuf·MySQL은 아직 구현하지 않았다.

## 구성

- `Core/`: Winsock 초기화·해제, IPv4 주소·TCP 소켓·IOCP 기본 기능, 공통 타입·상태 코드·로거를 제공하는 라이브러리. WinsockRuntime은 복사·이동을 금지해 초기화와 해제 횟수를 맞춘다.
- `Server/`: `jh_backend_server` 실행 진입점.
- `TestClient/`: `jh_backend_test_client` 실행 진입점.
- `Tests/`: 로거·네트워크·IOCP 기본 기능을 확인하는 로컬 전용 테스트. Git에서 제외한다.
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

실행 파일은 `out/build/windows-vs2026/Debug/` 또는 `Release/`에 생성된다. CTest에는 서버·클라이언트 시작 검사 2개가 등록되며, 로컬 `Tests/` 소스가 있으면 로거·네트워크·IOCP 검사를 각각 추가해 현재 총 5개가 된다. 테스트 소스가 없는 체크아웃에서도 빌드할 수 있다. 새 로컬 검사 소스를 추가하면 CMake 구성을 다시 실행한다. 두 시작 검사는 실제 프로그램 실행·Winsock 초기화·정상 종료만 확인한다. 네트워크·IOCP 검사는 검사 프로세스 안의 루프백 소켓을 사용하며 제품의 접속·송수신 흐름 검증과 구분한다.

## 언어 사용 기준

기존 코드도 C++11 범위로 정리했다. `std::string_view` 대신 문자열은 `const std::string&`, 포맷·함수명은 `const char*`로 받는다. `std::filesystem` 대신 파일 경로를 UTF-8 `std::string`으로 받고 Windows API 경계에서 UTF-16으로 변환한다. `[[nodiscard]]`는 사용하지 않으며 오류 반환값은 호출부에서 명시적으로 확인한다. 문자열 버퍼에 쓸 때는 크기를 확보한 후 `&buffer[0]`을 사용한다.

현 MSVC에는 C++11 모드가 없어 별도 복사본에서 가장 낮은 지원 모드인 C++14로 호환성 빌드·검사를 수행했다. 이는 엄격한 C++11 모드의 컴파일 검증과는 구분한다. 원본 CMake·프리셋은 C++17을 유지한다. [MSVC 표준 모드 지원 범위](https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version).

## 기본 타입

`Core/Types.h`에서 고정 폭 정수 `int8/16/32/64`, `uint8/16/32/64`와 실수 `float32/64`를 정의한다. 정수는 `<cstdint>`의 표준 타입을 사용하고 실수는 컴파일 시 비트 수를 확인한다. Core·서버·테스트 클라이언트의 PCH에 포함하며, 헤더에서 별칭을 사용하면 `Core/Types.h`를 직접 포함한다.

크기가 중요한 값에는 이 별칭을 사용한다. `bool`, 문자 `char`, 컨테이너 크기 `std::size_t`, 문자열·포인터 및 Windows API의 타입은 원래 타입을 유지한다. `StatusCode`의 기반 타입은 `int32`다. 별칭은 별도의 새 타입이나 직렬화 규칙을 만들지 않으며 패킷의 바이트 순서는 별도로 처리한다.

스레드 관련 기본 별칭으로 `Atomic<T>`, `Mutex`, `CondVar`, `UniqueLock`, `LockGuard`를 제공한다. 현재 잠금 별칭은 `Mutex`를 대상으로 한다. 클래스별 스마트 포인터 별칭은 해당 클래스 구현 시 추가하며 크기·길이를 작은 정수로 강제 변환하는 매크로는 두지 않는다.

출력·갱신 매개변수는 `out` 이름 접두사 대신 `OUT` 매크로로 표시한다. 예를 들어 `CreateTcpSocket(OUT SOCKET& socket)`, `GetLocalAddress(SOCKET socket, OUT NetAddress& address)`처럼 선언한다. `OUT`은 실행 동작을 바꾸지 않는 표기이며 Windows SDK에 정의되어 있지 않으면 `Types.h`에서 빈 매크로로 정의한다. 읽기 전용 참조 입력에는 붙이지 않는다. 소켓 종료처럼 기존 값을 읽고 갱신하는 인자에도 `OUT`을 붙이며 입력 조건은 함수 주석을 따른다.

## 컨테이너

`Core/Container.h`는 표준 컨테이너의 프로젝트 별칭을 제공한다. `Array`, `Vector`, `List`, `Map`, `Set`, `Deque`, `Queue`, `Stack`, `PriorityQueue`, `HashMap`, `HashSet`을 사용하며 현재는 표준 할당자를 유지한다. `Array`의 크기는 `std::size_t`이고 정렬·해시 비교자와 어댑터의 기반 컨테이너는 템플릿 인자로 지정할 수 있다.

`String`은 UTF-8 텍스트를 위한 `std::string` 별칭이다. 자동 인코딩 변환·검증이나 동기화를 제공하지 않는다. Windows UTF-16 API와의 변환은 기존처럼 경계에서 처리한다. Core·서버·클라이언트 PCH에 포함하며, 헤더에서 사용하면 `Core/Container.h`를 직접 포함한다. 기존 표준 컨테이너를 일괄 변경하지 않는다.

## 주소·소켓 기본 기능

`Core/NetAddress.h`는 IPv4 주소와 포트를 보관한다. 인자 없는 생성자는 구조체의 0 초기화와 `AF_INET` 설정으로 기본 상태 `0.0.0.0:0`을 만든다. 실제 IP·포트는 `SetAddress("127.0.0.1", port)`로 지정하고 `GetIp()`·`GetPort()`로 읽는다. DNS 이름과 IPv6는 현재 지원하지 않는다. `SetAddress()` 실패 시 기존 주소를 보존하며 내부 포트는 네트워크 바이트 순서로 저장한다. `GetPort()`는 호스트 바이트 순서다.

`Core/SocketUtils.h`는 다음 정적 함수를 제공한다. 사용할 소스에서 직접 포함한다.

- `CreateTcpSocket(SOCKET&)`: IPv4 TCP 소켓 생성. IOCP에서 사용할 수 있도록 overlapped 플래그를 지정하고 핸들 상속을 끈다. 출력 변수는 `INVALID_SOCKET`으로 초기화하며 이미 열린 소켓을 덮어쓰지 않는다.
- `SetTcpNoDelay()`, `SetExclusiveAddressUse()`, `SetSendBufferSize()`, `SetReceiveBufferSize()`: TCP 옵션·주소 독점·송수신 버퍼 설정. 주소 독점 옵션은 `Bind()` 전에 지정한다. 옵션은 자동으로 켜지지 않는다.
- `SetReuseAddress()`: `SO_REUSEADDR` 활성화·비활성화. `Bind()` 전에 명시적으로 설정하며 기본 동작을 변경하지 않는다. Windows에서는 이미 사용 중인 포트에 중복 바인딩을 허용할 수 있으므로 단순 재시작 옵션으로 취급하지 않는다. `SetExclusiveAddressUse(true)`와 동시에 활성화하지 않으며 충돌 등 OS 설정 실패는 `SC_NET_SOCKET_OPTION_FAILED`로 반환한다.
- `Bind()`, `Listen()`: 주소 바인딩과 연결 대기 설정. 포트 0으로 바인딩하면 OS가 빈 포트를 할당하며 `GetLocalAddress()`로 확인한다.
- `CloseSocket(SOCKET&)`: 소켓 닫기. 성공하면 `INVALID_SOCKET`으로 바꾸고 이미 무효인 소켓에는 성공을 반환한다. 실패하면 기존 값을 유지한다.

작업 결과는 공통 `StatusCode`의 네트워크 관련 항목으로 반환한다. Winsock API 자체가 실패한 경우 원본 번호가 필요하면 반환 직후 `WSAGetLastError()`를 읽는다. 인수 검증 실패에는 원본 Winsock 오류를 새로 설정하지 않는다. 공개 진단 구조체나 내부 로그 출력은 추가하지 않았다.

`WinsockRuntime` 초기화 성공 후 소켓을 만들고, 모든 소켓을 정리한 뒤 런타임을 소멸시킨다. 소켓 소유권은 호출자에게 있으며 복사한 핸들을 각각 닫지 않는다. 같은 소켓의 종료와 다른 호출을 동시에 수행하지 않는다. 향후 비동기 I/O를 붙일 때 `CloseSocket()` 성공을 I/O 완료·세션 해제와 동일하게 취급하지 않는다. 현재 이 유틸리티는 IOCP 등록·접속 수락·연결·송수신을 수행하지 않는다.

구현 근거: Microsoft의 [WSASocketW](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsasocketw), [주소 재사용·독점 옵션](https://learn.microsoft.com/en-us/windows/win32/winsock/using-so-reuseaddr-and-so-exclusiveaddruse), [closesocket](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-closesocket) 문서.

## IOCP 기본 기능

`Core/IocpCore.h`는 완료 포트 핸들을 소유하는 `IocpCore`와 등록 객체의 인터페이스 `IocpObject`를 제공한다. `Core/IocpEvent.h`는 `OVERLAPPED`를 상속한 이벤트를 정의한다. 필요한 소스에서 직접 포함하며 전역 객체나 워커 풀을 생성하지 않는다.

| 함수 | 역할 |
| --- | --- |
| 생성자 | 내부 `Init()`으로 완료 포트를 생성한다. 동시 실행 수는 OS 기본값이며 워커를 생성하지 않는다. 생성 후 `IsInitialized()`를 확인한다. |
| `Register(iocpObject)` | 객체의 `GetHandle()`을 포트에 연결한다. 널 객체와 무효 핸들을 거부하며 소켓을 소유하거나 닫지 않는다. |
| `Dispatch(timeoutMs = INFINITE)` | 완료 패킷 하나를 기다려 이벤트의 소유 객체에 전달한다. 0ms는 즉시 확인한다. |
| 소멸자 | 내부 `Close()`로 완료 포트만 해제한다. 남은 I/O 완료 처리와 워커 합류를 먼저 마쳐야 한다. |
| `IsInitialized()` | 현재 핸들 보유 여부 조회. |

`IocpObject`는 `GetHandle()`과 `Dispatch(IocpEvent*, DWORD numOfBytes, DWORD error)`를 구현한다. 바이트 수와 오류는 Windows 원본 타입으로 전달한다. `error == ERROR_SUCCESS`이면 정상 완료이며 취소·실패 완료도 같은 콜백으로 전달하므로 해당 작업과 버퍼를 정리해야 한다. 객체 콜백 호출 후 `IocpCore::Dispatch()`의 반환값은 다음처럼 구분한다.

- `SC_SUCCESS`: 정상 완료를 전달했다. 이벤트 없는 수동 알림은 콜백 없이 성공으로 처리한다.
- `SC_IOCP_IO_FAILED`: 실패한 I/O의 완료를 원본 오류와 함께 객체에 전달했다. 반환값 확인 후 같은 작업을 다시 정리하지 않는다.
- `SC_IOCP_WAIT_TIMEOUT`: 완료 패킷 없이 제한 시간이 지났다. 콜백을 호출하지 않는다.
- `SC_IOCP_WAIT_FAILED`: 완료 패킷 없이 대기 자체가 실패했다. 콜백을 호출하지 않으며 원본 오류는 반환 직후 `GetLastError()`로 확인한다.
- `SC_IOCP_INVALID_OBJECT`: 등록 객체가 널이거나 완료 이벤트의 소유 객체가 없다. 후자는 I/O 요청 전 소유자 설정 누락에 해당한다.
- `SC_IOCP_NOT_INITIALIZED`: 생성자에서 포트 생성에 실패했다.

IOCP 결과는 공통 `StatusCode`의 IOCP 관련 항목으로 구분한다. `Register()`의 Windows API 실패 원인이 필요하면 반환 직후 `GetLastError()`를 확인한다. 인수·상태 검사에는 새 Windows 오류를 설정하지 않는다. `WSA_IO_PENDING`은 비동기 요청을 시작할 때 확인하는 값이므로 완료 대기 결과의 상태 코드로 추가하지 않는다.

I/O 요청 전 `IocpEvent::SetOwner()`로 소유 객체를 설정하고 완료 처리까지 이벤트·버퍼의 주소와 수명을 유지한다. 이벤트는 복사·이동하지 않는다. `Init()`은 이전 완료 처리 후 `OVERLAPPED` 영역만 초기화하며 종류와 소유 참조는 유지한다. 요청이 즉시 실패하고 완료 통지가 발생하지 않는 경우에는 호출자가 `SetOwner(nullptr)`로 참조를 해제한다. 소켓 종료나 취소 요청만으로 이벤트를 해제하지 않는다.

완료를 받으면 이벤트의 소유 참조를 콜백 직전에 해제하고 지역 `shared_ptr`로 콜백 동안 객체 수명을 유지한다. 객체가 이벤트를 멤버로 소유해도 완료 후 순환 참조가 남지 않게 하며 콜백이 다음 I/O를 시작한다면 소유자를 다시 설정해야 한다. 콜백 이후에는 이벤트를 다시 접근하지 않는다. 객체 내부 상태와 같은 이벤트의 재사용은 호출자가 동기화해야 한다.

새 I/O 중지와 남은 완료 처리, 워커 합류를 마친 뒤 포트를 소멸시킨다. 생성·소멸과 다른 호출을 동시에 수행하지 않는다. 현재 수동 알림 등록 API는 없으므로 워커 종료는 유한한 대기 시간과 별도 종료 조건 등으로 구성해야 한다. 제품 워커는 아직 구현하지 않았다.

현재 `IocpEvent` 기반 완료 분배까지 제공한다. `Listener`, `Session`, `AcceptEx` 로딩, 제품의 비동기 송수신 흐름은 다음 단계다. 로컬 검사는 입력 검증·이벤트 초기화·타임아웃, 실제 루프백 `WSARecv` 완료·취소, 다른 포트 중복 등록 실패, 소유자 누락 처리·콜백에서의 소유 참조 재설정·완료 후 객체 소멸을 확인한다.

구현 근거: Microsoft의 [CreateIoCompletionPort](https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-createiocompletionport), [GetQueuedCompletionStatus](https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-getqueuedcompletionstatus), [PostQueuedCompletionStatus](https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-postqueuedcompletionstatus), [CancelIoEx](https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-cancelioex) 문서.

## 공통 로거

`Core/Logger.h`의 전역 `GLogger`를 사용한다. `main()`에서 출력 대상과 최소 레벨을 설정하고, 이후에는 로그 매크로만 호출한다.

```cpp
const StatusCode result = GLogger->Init(LogOutput::Both, LogLevel::Info,
    "logs/backend-server.log");
if (result != StatusCode::SC_SUCCESS)
{
    LOG_ERROR("로그 초기화 실패: %s", GetStatusMessage(result));
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

`Init()`만 `StatusCode`를 반환한다. 잘못된 출력·레벨, 빈 경로, 디렉터리 생성 실패, 파일 열기 실패를 구분하며 실패 시 기존 설정을 유지한다. 개별 로그 호출은 `void`이며 반환값을 검사하지 않는다. 공개 진단 구조체와 별도 옵션 구조체는 두지 않는다. 잘못된 레벨이나 빈 형식 포인터로 출력하면 기록하지 않으며, 잘못된 최소 레벨 변경은 무시한다.

파일은 부모 폴더를 자동 생성하고 기존 파일에 UTF-8 BOM 없이 추가 기록한다. 실제 콘솔에서는 UTF-16 `WriteConsoleW`를 사용하고 리다이렉션에는 UTF-8을 유지한다. 콘솔 코드 페이지는 변경하지 않는다. `Error`·`Fatal`은 표준 오류, 나머지는 표준 출력으로 보낸다. 한글 파일 경로도 UTF-8 `std::string`으로 전달한다. 내부에서 절대 경로·UTF-16으로 변환하고 `SHCreateDirectoryExW`로 부모 폴더를 생성하므로 Windows Shell API의 경로 길이 제한을 따른다.

같은 로거의 호출은 mutex로 직렬화하며 호출한 스레드에서 출력·파일 flush를 완료한다. 콘솔 출력에 실패해도 파일 기록을 시도하고, 파일 기록에 실패하면 표준 오류에 짧게 알린다. 출력 성공을 호출부에 반환하거나 보장하지 않는다. 비동기 큐·파일 순환·보관 기간 관리는 포함하지 않는다.

서버는 `logs/backend-server.log`, 테스트 클라이언트는 `logs/backend-test-client.log`에 기록한다. 상대 경로는 실행 시 작업 디렉터리 기준이며 `logs/`는 Git에서 제외한다.

`CoreGlobal`이 로거를 멤버로 소유하고 `GLogger`는 재할당할 수 없는 접근 포인터다. 기본 상태는 콘솔·Info다. 작업 스레드는 `main()` 종료 전에 모두 종료·합류해야 한다. `GLogger`를 직접 삭제하거나 다른 전역 객체의 생성자·소멸자에서 로그를 호출하지 않는다. 독립 출력이 필요하면 지역 `Logger`를 생성해 같은 `Init()`을 사용한다.

상태 코드 항목은 `SC_대문자_스네이크` 형식으로 명명한다. 종류별 번호 대역은 사용하지 않으며 `SC_SUCCESS = 0` 이후 값은 선언 순서대로 자동 부여한다. 상태 판단에는 숫자 리터럴 대신 열거 항목을 사용한다. 공통 상태 코드는 `Core/StatusCode.h`에서 정의한다. C++11 범위에 맞춰 반환값 경고 속성은 제거했으며 호출자는 성공·실패를 명시적으로 확인한다. 성공은 `SC_SUCCESS = 0`이며 `GetStatusMessage()`로 한국어 설명을 얻는다. 업무상 실패를 판단해야 하는 작업은 오류 코드를 반환하고, 로그를 남기는 호출에는 이 규칙을 적용하지 않는다. 단순 상태 판단·값 조회 및 Winsock 원본 오류 조회도 기존 반환형을 유지한다. 프로젝트의 `try-catch`와 직접 `throw`는 사용하지 않으며 자원은 RAII로 정리한다. 표준 라이브러리의 자원 부족 예외까지 반환값으로 바꾸지는 않는다.

CTest는 레벨 필터·한글·추가 기록·동시 기록·출력 대상·초기화 실패 및 매크로 동작을 확인한다. `jh_backend_logger_tests --console`은 실제 콘솔의 한글 출력과 코드 페이지 유지 여부를 확인하며, 일반 검사는 빌드 폴더의 `logger-tests-*`에 산출물을 남긴다.

## 미리 컴파일된 헤더(PCH)

`Core/Pch.h`에 현재 사용하는 공통 타입·컨테이너·전역 로거·오류 코드 헤더와 Windows·표준 헤더를 모았다. 폴더 생성에 사용하는 Shell 헤더도 포함한다. CMake의 `target_precompile_headers`로 Core·서버·테스트 클라이언트에 적용하며 각 타깃·구성별로 PCH를 생성한다.

CMake가 PCH 생성용 소스와 자동 포함 설정을 관리하므로 별도 `Pch.cpp`나 소스별 `#include "Core/Pch.h"`는 필요하지 않다. 공개 헤더는 PCH 없이도 사용할 수 있도록 필요한 헤더를 직접 포함한다.

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
