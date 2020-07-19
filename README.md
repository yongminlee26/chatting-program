# 멀티플렉싱 기반 다중 접속 채팅 프로그램 만들기

클라이언트가 다중으로 접속 가능한 멀티플렉싱 채팅 서버
클라이언트끼리 서로의 채팅을 볼 수 있음
Select timeout값 = sec 5, usec 5000
입출력 버퍼 최대값 100
클라이언트는 입출력 루틴이 분할된 TCP 멀티프로세스 기반 클라이언트 사용
클라이언트 종료 문자 입력 시 half-close 기법 사용해서 종료


# 멀티쓰레드 기반 1-1 채팅 프로그램

TCP client
연결설정 직후 TCP 서버로 클라이언트의 사용자 이름을 전송
즉, 사용자 이름을 서버에 등록합니다.
입출력 루틴이 분리된 쓰레드를 생성
발신자의 이름과 함께 메시지를 출력
예시
[Derrick] Hey Ashley!        
[Ashley] Hi Derrick, what‘h up?

TCP server
각 클라이언트 연결을 위한 쓰레드를 생성하고 해당 클라이언트 사용자 이름을 등록
수신한 메시지를 지정된 클라이언트로 전송
User A가 user B에게 메시지 전송 :  AserverB
User A가 user C에게 메시지 전송 :  AserverC
모두에게 보내는 메시지를 제외한 1대1 메시지는 지정된 상대에게만 출력
사용자가 채팅에 참여할 때와 떠날때 서버에 안내문을 출력
참여 시:  User @derrick has joined chat
종료 시:  User @derrick has left the chat