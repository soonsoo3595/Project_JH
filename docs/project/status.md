# Project JH 현재 상태

최종 갱신: 2026-10-29 (Asia/Seoul)
상태 관리: 현재 PM 채팅

## 현재 단계

2026-10-29 사용자 요청으로 StatusCode 번호 대역 제거를 완료했다. SC_SUCCESS=0 이외의 명시 번호와 README의 고정 번호 설명을 제거하고 선언 순서 자동 증가로 정리했다. SC_ 이름·종류별 주석·GetStatusMessage는 유지한다. 과거 검증 기록의 번호는 당시 이력이다.

2026-10-29 사용자가 IOCP 코드 검토 결과의 전체 조치를 명시 요청해 백엔드 세션이 IocpObject·IocpEvent 및 Register/Dispatch 구조를 유지하면서 CMake 등록·생성자/반환/헤더·실패 I/O 완료 전달·입력 검사 수정을 완료했다. 최종 manifest ON 구성과 Debug·Release CTest 각 5/5 통과는 백엔드 보고 기준이다. 사용자는 앞으로 자신의 백엔드 코드에 맞춘 CMake·StatusCode 관리를 백엔드 세션에 맡겼으며 역할 합의를 team.md·repository.md에 반영했다. 별도 리뷰·QA를 새로 배정하지 않았다.

2026-10-25 사용자 요청으로 ErrorCode를 StatusCode, EC_를 SC_, GetErrorMessage를 GetStatusMessage로 변경 완료했다. 아래 현재 API 표기는 새 명칭을 기준으로 하며 동작·C++11 사용 범위와 nodiscard 미사용을 유지한다. 현재 숫자 정책은 아래 최신 결정을 따른다. 검증 이력의 옛 명칭은 당시 상태를 나타낸다.

최신 사용자 결정으로 빌드 설정은 C++17을 유지하되 직접 작성하는 코드는 C++11 범위로 제한하고 기존 코드도 전환한다. 백엔드 세션이 string_view·filesystem·nodiscard·가변 string::data 사용 등을 정리 완료했다. NetAddress는 const String&, Logger는 const std::string&·const char*를 사용한다. 로거 파일 경로는 UTF-8 const std::string&로 받고 내부 Windows UTF-16·절대 경로 변환과 SHCreateDirectoryExW/GetFileAttributesW/_wfsopen으로 처리한다. shell32 링크와 PCH Shell 헤더를 추가했으며 경로 길이 제한은 Backend/README.md를 따른다. 아래 이전 구현·nodiscard 적용 기록은 전환 전 상태다.

백엔드 기반을 먼저 검증한 뒤 UE를 연동한다. 사용자가 백엔드 기본 템플릿 구현을 명시적으로 요청하여 `build/backend-template` 브랜치에 최소 빌드·실행 기반을 구성했다. 별도 코드 리뷰·QA 요청이나 다음 네트워크 기능 구현으로 범위를 확대하지 않는다.

첫 목표는 [백엔드 첫 목표](../specs/backend-first-slice.md)의 IOCP 연결·TCP/Protobuf 송수신·종료 처리다. [Backend/README.md](../../Backend/README.md)에 템플릿 빌드·실행 절차를 정리했다. 서버·테스트 클라이언트는 공통 Winsock 초기화·해제 후 종료하며 제품 연결·송수신 흐름·Protobuf·DB는 아직 구현하지 않았으며 IOCP 완료 큐 기반은 구현했다.

2026-10-22에 백엔드 세션이 사용자 요청으로 `Backend/Core/Pch.h`와 타깃별 PCH 설정을 추가했다. Core·서버·테스트 클라이언트에서 같은 헤더 목록을 사용하되 PCH 산출물은 각각 생성한다. 아래 추가 검증 결과는 백엔드 세션 보고 기준이며 PM이 빌드를 재실행하지 않았다.

같은 날 F5 실행 시 한글 깨짐 보고에 대응해 백엔드 세션이 서버·테스트 클라이언트 진입점에 콘솔 연결 여부 확인과 `SetConsoleOutputCP(CP_UTF8)` 초기화를 추가했다. UTF-8 실행 문자열 및 UTF-16 LE BOM 소스는 유지했다. Visual Studio F5 화면의 수정 결과는 아직 직접 확인하지 않았다.

2026-10-24에 사용자가 콘솔 한글 깨짐을 다시 보고했다. 백엔드 세션은 당시 두 진입점에 콘솔 초기화가 모두 없는 것을 확인하고, 콘솔이 연결된 경우 `SetConsoleOutputCP(CP_UTF8)`를 호출하도록 추가했다. 설정 실패 시 오류 코드 출력 후 종료하고 콘솔이 없으면 건너뛴다. UTF-8 BOM·CRLF·MSVC `/utf-8`은 유지하며 시스템 전체 설정과 공통 로거 설계는 변경하지 않았다. PM은 공유 소스와 README의 반영 상태를 확인했으며 빌드·실행은 다시 수행하지 않았다.

## 확정한 사항

- 현재 IOCP API는 IocpObject/IocpEvent와 IocpCore::Register/Dispatch 구조다. 생성자에서 private Init을 호출하고 IsInitialized로 확인하며 소멸자에서 private Close를 호출한다. 이전 Wait/Post/IocpCompletion API는 현재 계약에서 제외한다. IocpObject는 GetHandle과 Dispatch(IocpEvent*, DWORD numOfBytes, DWORD error)를 구현하고 정상·실패·취소 완료를 모두 받는다. GQCS FALSE라도 이벤트가 있으면 실패 완료를 전달하며 패킷 없는 실패·타임아웃과 구분한다. 콜백 직전에 이벤트 owner를 비우고 지역 shared_ptr로 콜백 수명을 유지해 완료 후 순환 참조를 끊는다. 재요청에는 SetOwner가 필요하고 즉시 실패해 완료가 오지 않으면 호출자가 해제한다. 이벤트 복사·이동은 금지하며 완료 처리·워커 합류 후 포트를 소멸시킨다. 널 객체·무효 핸들을 검사하고 SC_IOCP_INVALID_OBJECT를 추가했다. IOCP 결과는 SC_IOCP_ 항목으로 구분한다. Listener·Session·제품 워커·접속/송수신 흐름은 아직 미구현이다.

- 2026-10-26 사용자 요청으로 출력·갱신 참조에 out 접두사 대신 OUT을 적용했다. Types.h에 미정의 시 빈 매크로를 제공하며 SocketUtils의 소켓 생성·주소 조회·종료와 Logger 내부 출력 참조에 적용했다. 읽기 전용 입력 참조에는 사용하지 않으며 입출력 인자의 기존 값 조건은 주석을 따른다.
- SocketUtils에 `SetReuseAddress(SOCKET, bool)`를 추가했다. SO_REUSEADDR을 Bind 전에 명시적으로 설정하며 자동 활성화하지 않는다. 주소 독점 옵션은 유지하고 두 옵션을 동시에 활성화하지 않는다. Windows의 중복 바인딩 의미는 README를 따르며 OS 설정 실패는 기존 SC_NET_SOCKET_OPTION_FAILED로 반환한다.

- 2026-10-25 사용자 요청으로 NetAddress 생성자는 포트 인자 없는 `NetAddress() noexcept`로 정리했다. 멤버 값 초기화로 IP·포트는 0이고 생성자는 sin_family=AF_INET만 설정한다. 기본 주소는 0.0.0.0:0이며 실제 IP·포트는 SetAddress로 지정한다. 설정 실패 시 기존 값 보존 계약은 유지한다. 백엔드 보고 기준 Debug·Release 빌드 및 기존 backend_network_basics 각 1/1이 통과했다. 전체 CTest·새 테스트·F5·별도 리뷰·QA·커밋은 미수행이며 PM은 생성자 선언·구현·README만 대조했다.

- 사용자 요청으로 `Core/NetAddress.h·cpp`와 `SocketUtils.h·cpp`를 추가했다. NetAddress는 IPv4 숫자 주소·포트 설정/조회·sockaddr_in 변환을 지원하며 기본은 0.0.0.0:0, 설정 실패 시 기존 주소를 보존한다. DNS·IPv6는 미지원이다. SocketUtils는 IPv4 TCP overlapped·상속 금지 소켓 생성, TCP_NODELAY·주소 독점·송수신 버퍼 설정, Bind·Listen·GetLocalAddress·CloseSocket을 제공한다. 열린 출력 핸들 덮어쓰기를 거부하며 닫기 성공 시 INVALID_SOCKET으로 복원하고 반복 종료는 성공 처리한다. 소켓 소유권은 호출자에게 있고 소켓 정리 후 WinsockRuntime을 종료한다. 네트워크 오류는 SC_NET_ 항목으로 구분한다. 제품 진입점은 여전히 초기화 후 종료하며 IOCP 완료 큐 기반은 아래 별도 구현 사항을 따르며 실제 제품 연결 흐름은 미구현이다.

- 사용자 요청으로 `Core/Container.h`에 Array·Vector·List·Map·Set·Deque·Queue·Stack·PriorityQueue·HashMap·HashSet·String을 추가하고 CMake·PCH·README에 연결했다. 표준 할당자를 사용하며 Array 크기는 std::size_t, String은 기존 UTF-8 규칙의 std::string이다. 비교자·해시·기반 컨테이너 인자를 지원한다. 커스텀 Allocator·wchar_t String은 도입하지 않았고 자동 동기화·인코딩 변환은 제공하지 않는다. 백엔드 보고 기준 Debug·Release 빌드와 기존 CTest 각 3/3 및 diff 공백 검사를 통과했다. PM은 헤더·연결 문서를 대조했고 빌드·실행은 재수행하지 않았다. 새 테스트·F5·별도 리뷰·QA·커밋은 미수행이다.

- 사용자 제공 참고에 따라 `Types.h`에 표준 스레드 별칭 `Atomic<T>`·`Mutex`·`CondVar`·`UniqueLock`·`LockGuard`를 추가했다. atomic·condition_variable·mutex 헤더를 직접 포함한다. BYTE는 Windows 타입을 유지하며 미구현 클래스의 Ref 선언·USING_SHARED_PTR와 축소 캐스팅 size/len 매크로는 도입하지 않았다. 백엔드 보고 기준 이 추가 후 Debug·Release 빌드와 기존 CTest 각 3/3이 통과했다. PM은 타입 헤더 제공 목록을 확인했고 빌드·실행은 재수행하지 않았다. 새 테스트·F5·별도 리뷰·QA·커밋은 미수행이다.

- 사용자 요청으로 `Backend/Core/Types.h`에 `int8/16/32/64`·`uint8/16/32/64`·`float32/64`를 정의했다. CMake Core 소스 목록과 PCH에 연결했고 `StatusCode.h`가 직접 include하여 기반 타입을 int32로 사용한다. 당시 타입 변경에서는 오류 항목·번호를 유지했다. bool·char·std::size_t·문자열·포인터·Windows API 원본 타입은 유지하며 도메인 ID를 미리 추가하지 않았다.

- 사용자 요청으로 테스트 소스를 로컬 전용으로 관리한다. `/Backend/Tests/`를 Git에서 제외하고 기존 로컬 파일은 보존했다. `Backend/TestClient/`는 계속 버전 관리 대상이다. 로거 테스트는 `Tests/LoggerTests.cpp`·`LoggerCaller.cpp`가 모두 있을 때, 네트워크 검사는 `Tests/NetworkTests.cpp`가 있을 때 CMake에 등록한다. IOCP 검사 소스도 조건부로 등록하며 현재 로컬 테스트 소스가 모두 있으면 CTest 5개, 없으면 서버·클라이언트 시작 검사 2개가 등록된다.

- 2026-10-24 사용자 요청으로 `Backend/Core/Logger.h·cpp`의 공통 로거를 구현했다. 두 진입점은 직접 출력과 코드 페이지 설정 대신 로거를 사용한다. 현재 동작과 사용법은 [Backend/README.md](../../Backend/README.md)의 공통 로거 항목을 따른다.
- 같은 날 사용자 확정에 따라 전역 로거·`Init`·`LOG_*` 매크로 구조로 변경했다. `CoreGlobal.h·cpp`의 객체가 자동 소유하며 재할당 불가 `GLogger` 포인터로 접근한다. 두 진입점은 `GLogger->Init(LogOutput::Console/File/Both, 최소 레벨, 선택 파일 경로)`로 설정한다. 재초기화 성공 시 설정을 직렬화해 교체하고 실패하면 기존 설정을 유지한다. 기존 `LoggerOptions`·`Log()`와 지역 인스턴스 사용도 유지한다.
- 후속 사용자 요청으로 `CoreGlobal.cpp`의 final `CoreGlobal` 클래스가 첫 멤버 `Logger _logger`를 직접 소유하도록 변경했다. 파일 내부 static 인스턴스가 수명을 관리하며 `Logger* const GLogger` 접근 API는 유지한다. CoreGlobal은 복사·이동을 금지한다. 멤버 선언 순서로 생성하고 역순으로 소멸하며 향후 로거 의존 객체는 로거 아래에 선언한다. 현재 DB·워커 객체를 추가하지 않았고 main의 Init·종료 전 워커 합류·전역 생성자/소멸자 로그 금지 조건은 유지한다.
- `Write()`와 매크로는 UTF-8 printf 형식을 지원하고 `LOG_TRACE/DEBUG/INFO/WARNING/ERROR/FATAL`은 호출 함수명을 자동 기록한다. Release에서는 Trace·Debug 매크로만 인자 평가 없이 제거하고 Info 이상은 최소 레벨에 따라 유지한다. 직접 `Write()`·`Log()` 호출에는 이 컴파일 단계 제거가 적용되지 않는다. 전역 로거를 수동 삭제하지 않으며 모든 워커를 main 종료 전에 합류시키고 전역 객체 생성자·소멸자에서는 로그를 호출하지 않는다.
- 로거 enum 두 이름만 using으로 노출한 단계 이후, 사용자 요청으로 프로젝트 전반의 자체 namespace 제거를 완료했다. 직접 작성한 C++ 소스·헤더 11개에서 `jh`·`jh::detail`·익명 namespace와 using 별칭·`using namespace std`를 제거했다. 파일 내부 전용 함수·객체는 static, Release 무동작 함수는 `SkipLogForRelease` inline으로 유지한다. 표준 라이브러리의 `std::`와 외부·생성 코드의 namespace는 유지한다.
- 로거는 Trace·Debug·Info·Warning·Error·Fatal, 최소 레벨 필터와 실행 중 변경, 콘솔·파일 선택 및 동시 출력을 지원한다. 날짜·시간·밀리초·레벨을 기록하며 인스턴스별 mutex로 동기식 출력을 보호한다. 파일은 UTF-8 append와 레코드별 flush, 실제 콘솔은 UTF-16 변환 후 `WriteConsoleW`, 리다이렉션은 UTF-8 바이트 출력을 사용한다. 콘솔 코드 페이지는 변경하지 않는다. Error·Fatal은 stderr로 출력하고 Fatal 자체는 종료를 수행하지 않는다.
- 서버·테스트 클라이언트의 파일 로그는 실행 작업 디렉터리 기준 `logs/backend-server.log`·`logs/backend-test-client.log`다. `logs/`는 Git에서 제외한다. 사용자 의도에 맞춰 로거를 단순화하여 `Init()`만 StatusCode 반환, `SetMinimumLevel()`·`Log()`·`Write()`·로그 매크로는 void로 변경했다. 개별 출력 반환값 검사와 원본 category/value 출력은 제거했다. 초기화 실패 시 기존 설정을 보존하고 두 진입점은 실패 로그 후 종료 코드 1을 반환한다. 콘솔 실패와 관계없이 파일 기록을 시도하고 파일 쓰기 실패는 stderr에 짧게 알린다. 가변 인수 목록과 FILE 핸들은 RAII로 정리한다.
- 업무상 실패를 판단할 작업에 공통 `StatusCode : int32`를 사용하며 단순 로그 출력과 구분한다. `Core/StatusCode.h·cpp`는 `SC_SUCCESS=0`과 한국어 `GetStatusMessage()`를 제공한다. 항목 이름은 SC_ 대문자 스네이크 표기다. 2026-10-29 최신 결정에 따라 번호 대역과 기존 번호 고정 규칙을 제거하고 SC_SUCCESS=0 이외에는 선언 순서 자동 증가를 사용한다. 종류별 주석과 GetStatusMessage는 유지한다. 상태 bool·값 조회·Winsock 원본 int 조회는 유지한다. `LoggerOptions`·`LoggerDiagnostics`와 개별 출력 오류 코드는 제거했다. 파일은 `_wfsopen` append binary와 unique_ptr 삭제자로 관리하며 UTF-8·동기식 보호·6레벨·함수명·Release 상세 매크로 제거는 유지한다.

- 독립 C++17 Windows IOCP 백엔드, UE 클라이언트·Dedicated Server, MySQL을 기본 기술 방향으로 한다.
- PC와 Android 사용자가 함께 플레이하는 크로스플레이를 지원한다. PC 버전을 먼저 완성한 뒤 Android를 지원한다.
- PC 클라이언트는 Windows만 지원한다. 개발 환경은 사용자 전달 기준 Visual Studio 2026·UE 5.8이다. 실제 설치 및 빌드 호환성은 미검증이다.
- CI는 GitHub Actions로 확정했다. 초기 범위는 GitHub 제공 Windows 실행 환경에서 백엔드 빌드·테스트 자동화다. 실제 워크플로 생성·실행은 수행하지 않았다.
- 독립 백엔드는 CMake + vcpkg manifest 모드(`vcpkg.json`)로 구성했다. 외부 의존성 목록은 현재 비어 있다. 백엔드 세션 보고 기준 VS 2026 x64 Debug·Release 빌드와 각 구성의 CTest 5개(시작 검사 2개·로거 검사 1개·네트워크 기본 검사 1개·IOCP 검사 1개)가 통과했다. UE는 Unreal 빌드 체계를 유지하며 이번에 검증하지 않았다.
- 자체 백엔드 통신은 TCP + Protobuf를 사용한다. 프레이밍·스키마는 미확정이다.
- 게임 플레이 흐름 및 구현 단계·완료 기준은 언제든 변경 가능한 초안이다.
- 사용자는 단독 개발자이자 최종 의사결정자다. 현재 채팅은 PM 역할이며 역할별 세션은 필요할 때 확장한다.
- 구현·코드 리뷰·QA는 각각 사용자 명시 요청에 따라 진행한다. 지정된 프로젝트 세션 간 메시지 전달은 허용한다.
- 공통 지침·운영 문서·작업 로그 템플릿을 작성했다.
- 세션 로그는 역할별 하루 한 파일에 누적하며 중요한 결정·실수·어려웠던 점과 해결 과정 위주로 기록한다. 기존 PM 로그 두 개를 당일 PM.md로 통합했다.
- 날짜별 세션 로그는 로컬 전용으로 Git 추적에서 제외한다. 공통 템플릿은 계속 추적한다. 사용자 요청으로 Git을 새로 초기화해 날짜별 로그를 첫 커밋부터 제외한다.
- 버전 관리는 Git으로 변경했다. 작업 브랜치 → dev(PR 통합) → release(배포 검증) → main 흐름을 적용하며 커밋 메시지는 `태그 | 작업 내용` 형식을 사용한다.
- GitHub 공개 단일 저장소를 사용한다. 유료 에셋과 원본 공개가 허용되지 않는 외부 에셋은 `.gitignore`로 제외한다.
- 식별자는 영어, 프로젝트 주석·진단 로그는 한국어로 작성한다. `auto`는 람다·장황한 반복자 등 타입을 직접 쓰기 어려운 경우로 제한한다.
- 게임 규칙·서버 판정·네트워크 로직은 C++, 에셋 연결·기본값·UI·표현은 Blueprint를 허용한다.
- 2026-10-23에 UTF-8 사용으로 변경했다. 직접 작성하는 C++ 소스·헤더는 UTF-8 BOM, 백엔드 내부 일반 텍스트는 UTF-8 `std::string`, Protobuf 문자열과 로그 파일은 UTF-8을 사용한다. Windows UTF-16 API·UE 문자열은 경계에서 변환한다. 사용자 요청으로 기존 소스·헤더 5개를 모두 UTF-8 BOM으로 맞췄다. UTF-16 파일 4개를 변환하고 이미 UTF-8 BOM이던 서버 Main.cpp는 유지했다. Git 인코딩 재변환을 제거하고 MSVC `/utf-8`을 적용했다.
- 백엔드 일반 변수·매개변수는 camelCase(`sessionId`), 멤버 변수는 `_` 접두사(`_sessionId`)로 작성한다. 클래스·구조체·함수는 PascalCase이며 UE 타입 접두사를 사용하지 않는다. bool은 `isConnected`·`_isConnected`처럼 작성한다. 탭 폭 4와 새 줄 중괄호를 사용하며 한 문장이어도 중괄호를 생략하지 않는다.

- UE는 타입 접두사, 함수·변수·매개변수의 PascalCase와 bool의 `b` 접두사를 사용한다. 멤버에 `_`를 붙이지 않는다. 탭 폭 4, 새 줄 중괄호와 한 문장에서도 중괄호 필수 규칙을 적용한다.

## 결정 대기 및 제한

- 2026-10-26 사용자 요청으로 Git 채팅이 `build/backend-template`에서 추가 분할 로컬 커밋을 완료했다. `80bc534` 공통 타입·동기화·컨테이너 별칭, `15f49df` StatusCode·C++11 정리·TCP 유틸리티, `df90360` 관련 문서다. 이전 2026-10-24 세 커밋 이후 작업을 기록했으며 main·dev에는 아직 통합하지 않았다. Git 보고와 PM 확인 시 작업 트리·스테이징은 깨끗했다. 이 완료 사실을 기록하는 현재 status.md 수정은 후속 미커밋 변경이다. Git 보고 기준 Core·Server·TestClient 파일 해시는 커밋 전후 동일하고 로컬 테스트·산출물·로그는 제외, TEMPLATE 추적은 유지했다. 브랜치 전환·push·PR·병합·추가 빌드·테스트·리뷰·QA는 수행하지 않았다.

- 2026-10-24 사용자 요청으로 Git 담당 채팅을 생성했다. 식별자는 `01a16d0d-e967-7fa3-b4cb-f7c57bd68c6c`(local)이며 PM·백엔드와 같은 체크아웃을 공유한다. 읽기 전용 인수인계와 사용자 작업 요청 대기를 전달했다. 이번 생성으로 스테이징·커밋·브랜치 전환·원격 연결·push·PR·병합을 실행하지 않았다. 실제 역할 등록은 team.md를 따른다.

- 보유 홈서버는 사용자 전달 기준 i7-1260P·RAM 32GB·SSD 1TB 노트북이다. Windows의 Ubuntu WSL에서 Docker 웹 컨테이너를 운영하며 구매한 도메인을 사용한다. CI 용도로 사용할지는 미확정이다.
- 홈서버 참고 저장소는 [homeserver-infra](https://github.com/soonsoo3595/homeserver-infra)다. 저장소 문서에서 WSL2·Ubuntu 24.04 LTS 및 기존 Jenkins·PostgreSQL·Redis·Traefik·Cloudflare Tunnel 구성을 확인했다. 실제 서비스 실행 상태는 미검증이며 Project JH의 GitHub Actions·MySQL 선택은 유지한다.
- 홈서버의 남은 SSD 용량·WSL 자원 제한·Windows 빌드 도구·UE 설치 상태 및 실제 성능은 미확인이다. 서버 접속이나 기존 서비스 변경은 수행하지 않았다.

- 사용자 요청으로 2026-10-21에 로컬 Git을 재초기화했다. main·dev의 초기 커밋은 `d35ba04`이며 날짜별 로그는 포함하지 않았다. 현재 구현 체크아웃은 dev에서 분기한 `build/backend-template`이다. 2026-10-24 사용자 요청으로 Git 채팅이 세 로컬 커밋을 생성했다: `4cf2b32` 저장소 제외·인코딩 속성, `045638b` 백엔드 기반·로거 구현, `7ed7d56` 명세·개발 규약·상태 문서. push·PR·병합은 수행하지 않았다. Git 버전은 `2.56.0.windows.1`이다.
- Git 채팅의 커밋 완료 직후와 PM의 후속 확인 시 작업 트리·스테이징은 깨끗했다. 이 완료 사실을 반영하는 현재 status.md 수정은 후속 미커밋 문서 변경이다. Git 보고 기준 두 Main.cpp·Pch.h의 해시는 커밋 전후 동일하며 로컬 테스트·산출물·실행 로그·날짜별 일지는 제외하고 TEMPLATE.md 추적을 유지했다. 커밋 정리 과정에서 빌드·테스트·별도 리뷰·QA는 재수행하지 않았다.
- 초기 커밋은 main에 남기고 같은 기준에서 dev를 만들어 개발 기준으로 사용한다. 후속 변경은 dev에서 작업 브랜치를 분기한다. release는 배포 검증 시 구성한다.
- GitHub 원격 저장소는 아직 연결하지 않았으며 push·PR·병합은 수행하지 않았다. Git LFS는 아직 설정하지 않았다.
- 원격 저장소 이름·주소, 상세 브랜치·병합·보호 정책, Git LFS 적용 범위와 유료·비공개 에셋의 실제 제외 경로는 미확정이다.
- 백엔드 담당 채팅을 2026-10-22에 생성했다. 식별자는 `01a163b9-43b1-7c11-913a-f47d39ba8bac`(local)이며 같은 프로젝트 체크아웃을 공유한다. 초기 요청은 읽기 전용 인수인계와 사용자 요청 대기다. 추가 구현·코드 리뷰·QA를 배정하지 않았다. Git 담당 채팅은 아래 별도 등록 사항을 따르며 그 외 역할별 채팅은 미등록이다.
- 사용자 선호에 따라 `try-catch`를 가급적 지양하고 예상 가능한 실패는 반환값·오류 코드, 자원 정리는 RAII를 사용한다. 후속 명시 요청으로 기존 Logger·두 진입점·테스트의 try/catch와 직접 throw 제거를 완료했다. 표준 라이브러리의 자원 부족 등 예상 밖 예외 가능성과 컴파일러 예외 지원은 유지한다.
- 코딩 규약의 미확정 제안, 백엔드 소유권·상세 오류 처리·IOCP 수명·스레드 규칙은 별도 논의한다.
- 기존 템플릿의 UTF-8 전환과 Debug·Release 재빌드·시작 검사는 완료했다. 과거 사용자는 서버 Main.cpp를 UTF-8 BOM으로 바꾸고 코드 페이지 초기화 없이 한글이 정상 출력됐다고 보고했다. 이 관찰의 원인을 별도 재현 검증하지 않았다. 현재는 공통 로거로 출력한다. DB 문자 집합·연결 설정과 UE 빌드는 별도 설계·검증한다.
- 로거의 비동기 큐·파일 순환·보관 기간 관리, 여러 로거 인스턴스·프로세스 사이의 같은 파일 쓰기 직렬화는 미구현이다. IOCP 처리 경로에 적용할 때 동기식 출력·flush 지연을 측정하고 방식을 검토한다.
- 개발 기간·주당 투자 시간·첫 완성본 범위는 미정이다.
- Windows 지원 버전·최소 사양, Android 최소 버전·대상 기기, 입력·UI·성능 목표와 검증 시점, 크로스플레이의 세부 매칭 정책은 미확정이다.
- CMake·vcpkg 버전·의존성 버전 고정·빌드 프리셋, 툴셋·SDK, GitHub Actions 실행 이미지·트리거·구체적인 검사 항목과 UE 빌드·패키징 환경은 미확정이다.
- PCH 추가 후 정상 CMake 구성은 vcpkg 7zip v26.02 다운로드의 DNS 오류로 실패했다. 백엔드 세션은 외부 의존성이 비어 있는 상태에서 검증 중에만 `VCPKG_MANIFEST_INSTALL=OFF`로 빌드하고 이후 캐시를 ON으로 복원했다고 보고했다. 2026-10-23 PM이 샌드박스 밖 도구 캐시 접근으로 manifest 설치 ON 상태의 정상 구성을 완료했다. 제한된 실행 환경에서는 같은 다운로드 DNS 오류가 발생할 수 있다. 프리셋과 manifest 설치 설정은 유지했다.

## 검증 상태

- 백엔드 세션의 2026-10-29 StatusCode 번호 대역 폐지 완료 보고: Debug 전체 빌드와 diff 공백 검사 통과. 이번 선언 변경의 테스트 실행·Release 재빌드는 미수행이다. PM은 헤더·README를 대조했으며 빌드·실행·커밋은 하지 않았다.

- 백엔드 세션의 2026-10-29 IOCP 검토 조치 완료 보고: manifest ON 최종 구성·Debug/Release 전체 빌드·CTest 각 5/5 통과. 실제 루프백 정상 수신·취소 995 콜백 전달, 중복 등록 거부, owner 누락/재설정·완료 후 소멸 검사를 포함한다. PCH 없는 검사 컴파일·BOM/CRLF·diff 공백 검사·테스트 Git 제외도 확인했다고 보고했다. WSASocket A 별칭의 C4996은 W 명시로 수정하고 사용자 BYTE 별칭은 보존했다. PM은 공개 API·콜백 연결·README를 대조했고 빌드·실행·별도 리뷰·QA는 재수행하지 않았다. 엄격 C++11·별도 C++14 재컴파일·F5·제품 흐름·OS 실패 강제 주입·커밋은 미수행이다. 아래 2026-10-26 Wait/Post 관련 검증 기록은 당시 API의 이력이다.

- 백엔드 세션의 2026-10-26 IocpCore 완료 보고: CMake 명시 재구성 후 Debug·Release 빌드와 CTest 각 5/5 통과. 실제 루프백 WSARecv 완료·CancelIoEx 취소(native 995)와 실패 완료 보존·포인터 크기 키·수동 알림·다른 스레드 깨우기/합류·타임아웃 초기화·다른 포트 중복 등록 실패·포트 종료 후 소켓 유지를 검증했다. 로컬 Tests/IocpCoreTests.cpp는 Git 제외·소스 존재 시 조건부 등록이다. 새 검사 파일 추가 후 재구성 전에는 이전 4개만 실행돼 최종 등록 수를 확인했다. 별도 ignored 복사본의 MSVC C++14 Debug 전체 빌드·IOCP 검사 1/1도 통과했다고 보고했다. 엄격한 C++11 모드·생성/닫기 OS 실패 강제 주입·F5·제품 연결 흐름·Protobuf·DB·별도 리뷰·QA는 미수행이다. PM은 헤더·상태 코드·CMake·README만 대조했으며 빌드·실행·커밋은 하지 않았다.

- 백엔드 세션의 2026-10-26 OUT·주소 재사용 옵션 완료 보고: Debug·Release 빌드와 CTest 각 4/4 통과. 실제 옵션 on/off 조회·설정 후 바인딩·독점 충돌 실패·무효 소켓 검사를 로컬 NetworkTests에 추가했다. 수정 소스 5개 BOM·CRLF, out 접두사 잔존 없음·공백 검사도 확인했다고 보고했다. PM은 공개 헤더·구현 연결·README를 대조했으며 빌드·실행은 재수행하지 않았다. F5·별도 C++14 재빌드·별도 리뷰·QA·커밋은 미수행이다.

- 사용자 제공 [강의 IOCP 서버](https://github.com/soonsoo3595/Iocp_Server_MatchMaking)의 구조·세션·패킷·로그 경로를 읽고 [참고 정리](../architecture/iocp-course-reference.md)를 작성했다. 코드는 가져오지 않았으며 정식 리뷰·QA·실행 검증은 수행하지 않았다.
- 읽은 자료: 프로젝트 개요, 사용자가 전달한 이전 프로젝트의 team.md·repository.md·coding-standards.md.
- 수행: CMake 구성·vcpkg 빈 manifest 설치, MSVC Debug·Release 빌드, 각 구성의 CTest 시작 검사 2/2 통과, 직접 실행의 한국어 출력, 소스 BOM·Git 인코딩 속성 확인.
- 확인한 도구: Visual Studio 18 Community, MSVC 19.51.36260.0(툴셋 폴더 14.51.36231), Windows SDK 10.0.26100.0, CMake 4.3.1-msvc1, vcpkg 2026-07-27.
- 백엔드 세션의 2026-10-22 보고: 위 임시 manifest 설치 비활성 조건에서 PCH 적용 Debug·Release 빌드 및 각 CTest 시작 검사 2/2 통과, 총 6개 PCH 산출물 생성 확인. 정상 manifest 설치를 포함한 전체 구성 성공과는 구분한다.
- 콘솔 출력 수정의 백엔드 보고: 같은 임시 manifest 설치 비활성 조건에서 Debug·Release 빌드 및 각 CTest 시작 검사 2/2 통과. PTY에서 CP949를 지정한 후 두 Debug 프로그램의 한국어 출력·정상 종료 확인. 검증 후 캐시는 ON으로 복원했고 프리셋은 유지했다. Visual Studio F5 화면은 미확인이다.
- 2026-10-23 PM 수행: UTF-8 BOM 전환 전후 소스 텍스트 동일성, Git 속성에서 인코딩 재변환 제거, 생성 프로젝트의 `/utf-8` 옵션을 확인했다. manifest 설치 ON으로 CMake 구성에 성공했고 PCH를 포함해 Debug·Release를 각각 새로 빌드한 뒤 각 CTest 시작 검사 2/2 통과를 확인했다. 사용자 실행 로직은 변경하지 않았고 Visual Studio F5 화면은 확인하지 않았다.
- 백엔드 세션의 2026-10-24 보고: 콘솔 초기화 추가 후 Debug·Release 빌드와 각 CTest 시작 검사 2/2 통과. PTY에서 두 Debug 프로그램의 한국어 출력·정상 종료 및 소스 UTF-8 BOM·CRLF를 확인했다. Visual Studio F5 화면은 직접 확인하지 않았다. PM의 별도 빌드·실행·코드 리뷰·QA는 수행하지 않았다.
- 미수행: 제품 진입점 간 네트워크 연결·IOCP·Protobuf·DB·UE 검증, GitHub Actions 실행, 별도 코드 리뷰·QA.
- 백엔드 세션의 2026-10-24 네트워크 기본 유틸리티 완료 보고: Debug·Release 빌드·CTest 각 4/4 통과. 주소·인수 오류/보존·실제 소켓 옵션·핸들 상속 금지·생성 전후 조건·임시 포트 루프백 Bind/주소 조회·독점 중복 Bind 실패·Listen·테스트 내 실제 connect/accept·반복 종료를 확인했다. 제한 환경의 Release connect 오류 10013을 확인한 뒤 로컬 네트워크 허용 환경에서 두 구성을 통과했다. 네트워크 테스트는 로컬 Git 제외 대상이다. 소스 7개의 BOM·CRLF와 diff 공백 검사도 확인했다고 보고했다. PM은 공개 헤더·오류 정의·조건부 검사 등록·README를 대조했고 빌드·실행은 재수행하지 않았다. F5·제품 IOCP·Protobuf·DB·별도 리뷰·QA는 미수행이며 커밋하지 않았다.
- 백엔드 세션의 2026-10-24 기본 타입 정의 완료 보고: manifest ON Debug·Release 빌드와 기존 CTest 각 3/3 통과, 수정 헤더 3개의 UTF-8 BOM·CRLF 확인. 새 테스트·F5·별도 리뷰·QA는 미수행이다. PM은 타입 헤더·ErrorCode·PCH·CMake·README를 대조하고 빌드·실행은 재수행하지 않았다. 커밋하지 않았다.
- 백엔드 세션의 2026-10-24 테스트 로컬 전용 처리 보고: 두 테스트 소스의 Git 제외·미추적 확인, manifest ON Debug 빌드와 CTest 3/3 통과. Tests를 제외해 ignored out에 복사한 별도 소스 구성에서는 vcpkg 없이 Debug 빌드·시작 검사 2/2 통과. 이 후속 변경의 Release·F5·별도 리뷰·QA는 미수행이다. PM은 제외 경로·조건부 타깃·README만 대조하고 빌드·실행은 재수행하지 않았다.
- 백엔드 세션의 2026-10-24 로거 단순화 완료 보고: Debug·Release 빌드와 각 CTest 3/3 통과, 실제 콘솔 한글 출력·코드 페이지 불변, 소스 8개의 UTF-8 BOM·CRLF 확인. F5·별도 리뷰·QA는 미수행이다. PM은 현재 헤더·README의 반환 계약을 대조했으며 빌드·실행은 재수행하지 않았다. 이전 ErrorCode·공개 진단 검증 기록은 단순화 전 이력이다.
- 백엔드 세션의 2026-10-24 ErrorCode 적용 완료 보고: Debug·Release 빌드와 각 CTest 3/3 통과. 파일 byte-range 잠금으로 파일 단독·양쪽 출력 실패 및 원본 오류 보존을 확인했다. 두 구성×두 프로그램×디렉터리 생성/파일 열기 실패의 실행 8건에서 코드 5/6·원본 오류·종료 코드 1을 확인했다. 실제 콘솔 한글·코드 페이지 불변, UTF-8 BOM·CRLF 및 try/catch/직접 throw 없음도 확인했다고 보고했다. PM은 헤더 API·README 대조만 수행했고 빌드·실행은 재수행하지 않았다. F5·별도 리뷰·QA는 미수행이다.
- 백엔드 세션의 2026-10-24 try-catch 제거 완료 보고: Debug·Release 빌드와 각 CTest 3/3 통과, 잘못된 옵션·파일·디렉터리 실패 반환과 설정 보존 확인. logs 경로를 일반 파일로 막은 두 구성×두 프로그램의 실행 4건에서 초기화 실패 로그·종료 코드 1을 확인했다. PM은 두 Main.cpp 및 직접 작성한 Core·Server·TestClient·Tests 소스에서 try/catch/throw 잔존 0건을 확인하고 README와 대조했다. 빌드·실행은 재수행하지 않았다. F5·별도 리뷰·QA는 미수행이다.
- 백엔드 세션의 2026-10-24 CoreGlobal 소유 구조 변경 완료 보고: Debug·Release 빌드와 각 CTest 3/3 통과, UTF-8 BOM·CRLF 확인. 새 테스트 없이 기존 전역 공유·로거·시작 검사를 실행했다. PM은 소유 클래스·접근 포인터·README 반영 상태만 대조했으며 빌드·실행을 재수행하지 않았다. F5·별도 리뷰·QA는 미수행이다.
- 백엔드 세션의 2026-10-24 namespace 제거 완료 보고: Debug·Release 빌드와 각 CTest 3/3 통과, 직접 작성한 11개 소스·헤더의 UTF-8 BOM·CRLF 및 namespace·jh:: 잔존 없음 확인. 기존 공유 변경은 보존했고 커밋하지 않았다. PM은 대상 폴더의 namespace·jh:: 검색과 전역 객체·PCH 반영 상태만 대조했으며 빌드·실행·별도 리뷰·QA는 수행하지 않았다.
- 백엔드 세션의 2026-10-24 공통 로거 완료 보고: manifest 설치 ON으로 정상 CMake 구성 후 Debug·Release 빌드와 각 CTest 3/3 통과. 레벨 필터·한글 경로·UTF-8·append·4스레드 400건·stdout/stderr·출력 실패·초기화 실패를 검사했다. 실제 콘솔 핸들의 `--console` 실행에서 6레벨 한글 출력과 코드 페이지 불변을 확인했다고 보고했다. F5 화면·네트워크·별도 리뷰·QA는 미수행이다. PM은 README·CMake·진입점·제외 규칙의 반영 상태만 대조했으며 빌드·실행을 재수행하지 않았다.
- 백엔드 세션의 같은 날 전역 로거 변경 완료 보고: Debug·Release 빌드 및 각 CTest 3/3 통과. 다른 번역단위의 전역 공유, 포맷·함수명·8192문자 기록, Release 상세 매크로 인자 미평가·Info 유지, 재초기화 실패 복구·출력 교체와 기존 테스트를 확인했다. 실제 콘솔의 전역 Write·매크로·서버 한글 출력과 코드 페이지 불변도 확인했다고 보고했다. F5·네트워크·별도 리뷰·QA는 미수행이며 PM은 문서·전역 객체 선언의 반영 상태만 대조하고 빌드·실행을 재수행하지 않았다.

- 백엔드 세션의 2026-10-24 오류 코드 명명·번호 구간 변경 보고: Debug·Release 빌드와 각 CTest 3/3 통과, 이전 열거 이름 잔존 없음 및 수정 소스 6개의 UTF-8 BOM·CRLF 확인. PM은 오류 코드 선언·README를 대조했으며 빌드·실행은 재수행하지 않았다. 새 테스트·F5·별도 리뷰·QA는 미수행이다.

- 백엔드 세션의 2026-10-24 결과 무시 경고 정책 변경: 함수별 속성을 제거하고 enum class [[nodiscard]] ErrorCode : int32에 한 번 적용했다. 결과 확인 요구는 유지한다. Debug·Release 빌드·CTest 각 4/4 통과 및 임시 컴파일 프로브에서 결과 무시 C4834·결과 비교 정상 컴파일을 확인했다고 보고했다. PM은 타입 선언·공개 헤더·README를 대조하고 빌드·실행은 재수행하지 않았다. WinsockRuntime 소유권은 유지하고 AcceptEx·ConnectEx·DisconnectEx는 IOCP 단계로 남겼다. 새 영구 테스트·F5·별도 리뷰·QA·커밋은 미수행이다.

- 백엔드 세션의 2026-10-24 C++11 범위 전환 완료 보고: 원본 C++17 설정 Debug·Release 빌드와 CTest 각 4/4 통과, 실제 콘솔 한글·코드 페이지 불변, 수정 소스 8개의 BOM·CRLF·공백 검사와 C++17 기능 및 try/catch/직접 throw 잔존 검색을 확인했다. MSVC의 최저 지원 C++14 모드로 별도 복사본 Debug 빌드·CTest 4/4도 통과했다. 엄격한 C++11 모드 컴파일은 미수행이다. 사용자 PCH·using namespace std 변경은 보존하고 SDK byte 충돌은 Shell 헤더를 해당 using 전에 포함해 해결했다고 보고했다. PM은 공개 API·CMake·README를 대조하고 빌드·실행은 재수행하지 않았다. F5·별도 리뷰·QA·커밋은 미수행이다.

- 백엔드 세션의 2026-10-25 StatusCode 전환 완료 보고: Debug·Release 빌드·CTest 각 4/4 통과. NetAddress.h의 winsock2 직접 include를 보완해 PCH 없는 네트워크 검사 빌드를 해결했다. 이전 API 명칭 잔존 없음·소스 BOM/CRLF·diff 공백 검사·Tests Git 제외 확인. PM은 새 타입 선언과 이전 API 이름 검색만 대조했으며 빌드·실행은 재수행하지 않았다. 이번 변경의 별도 C++14 재검증·F5·IOCP·별도 리뷰·QA·커밋은 미수행이다.
