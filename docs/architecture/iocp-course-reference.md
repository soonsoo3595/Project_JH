# 강의 기반 IOCP 서버 참고

- 확인일: 2026-10-22 (Asia/Seoul)
- 저장소: [Iocp_Server_MatchMaking](https://github.com/soonsoo3595/Iocp_Server_MatchMaking)
- 확인 범위: master의 README, 프로젝트 목록, IOCP·Listener·Session·Service·패킷 핸들러·JobQueue·Logger·PCH 및 게임 서버 프로젝트 설정 일부.
- 목적: 사용자가 학습한 구조를 이해하고 Project JH 설계 논의의 참고 자료로 사용한다. 정식 코드 리뷰·QA나 코드 재사용 승인으로 취급하지 않는다.

## 구조 대응

| 참고 저장소 | 역할 | Project JH에서 비교할 부분 |
| --- | --- | --- |
| ServerCore | IOCP·소켓·세션·버퍼 등 공통 엔진 | Backend/Core의 책임과 서비스 경계 |
| GameServer | 로그인·매칭 등 콘텐츠 처리 | Backend/Server 및 후속 서비스 로직 |
| GameClient | 테스트 클라이언트 | Backend/TestClient |
| Common·Tools | Protobuf 정의·코드 생성 | 후속 Shared/Protocol과 빌드 연계 |

## 읽은 흐름

- IocpCore가 완료 포트 등록·대기를 담당하고 IocpEvent가 작업 종류와 소유 객체를 보관한다.
- Listener가 AcceptEx를 등록하고 접속 수락 후 세션 초기화로 연결한다.
- Session이 연결·수신·송신·해제 작업과 완료 처리를 나눈다.
- PacketSession이 누적 수신 데이터에서 size·id 헤더를 읽어 메시지를 구분하고 패킷 핸들러에서 Protobuf를 파싱한다.
- GameServer의 워커 루프는 IOCP dispatch와 예약 작업·전역 작업 큐 처리를 수행한다.

이는 원본 구조 설명이다. Project JH의 프레이밍·워커 수·소유권·스레드 모델을 이대로 확정하지 않는다. 부분 송신·입력 길이 검증·종료 중 대기 I/O 수명 등의 처리 기준은 새 설계 시 별도로 정한다. 커스텀 메모리 풀·락·작업 스케줄러·DB 계층을 초기에 한꺼번에 도입하지 않는다.

## 한글 출력 관련 관찰

원본 `ServerCore/Logger.cpp`는 WCHAR 문자열을 `WriteConsoleW`로 콘솔에 출력하고, 파일은 `_wfopen_s`의 `ccs=UTF-8` 모드로 기록한다. 현재 Project JH의 `cout` 기반 좁은 문자열 출력과 다른 경로다. 원본에서 한글이 출력되는 이유를 소스 저장 인코딩만으로 설명해서는 안 된다.

`WriteConsoleW`를 활용하는 방식을 추후 선택하면 콘솔 연결과 파일·파이프 리디렉션을 구분해야 한다. [Microsoft WriteConsole 문서](https://learn.microsoft.com/en-us/windows/console/writeconsole)

## 적용 원칙과 한계

- 기존 합의대로 강의 프로젝트를 기반 코드로 복사하지 않고 개념과 필요한 경계를 새로 설계한다.
- 이번에는 원격 파일 읽기만 수행했으며 원본·Project JH 코드를 수정하거나 빌드·실행하지 않았다.
- 원본 프로젝트의 실행 안정성·성능·인코딩 문제 원인은 이번 읽기로 검증하지 않았다.
