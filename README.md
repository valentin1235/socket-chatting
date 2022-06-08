## 목적
- 간단한 채팅기능을 제공합니다

## 스펙
- clang 13.1.6
- C89
- Apple M1

## 서비스
- `/client` 채팅 클라이언트
- `/server` 채팅 서버

## 실행하기
### 채팅 서버
1. 채팅 서버 폴더로 이동한다 `cd ./server`
2. 컴파일 한다 `clang -std=c89 -W -Wall -pedantic-errors *.c`
3. 실행한다 `./a.out`
### 채팅 클라이언트
1. 클라이언트 폴더로 이동한다 `cd ./client`
2. 컴파일 한다 `clang -std=c89 -W -Wall -pedantic-errors *.c`
3. 실행한다 `./a.out 127.0.0.1 3000`
4. 터미널을 하나 더 열고 채팅 클라이언트를 하나 더 실행한다 `./a.out 127.0.0.1 3000`
5. 채팅 클라이언트 끼리 메세지를 주고받을 수 있다

## 데모영상
- [LINK](https://youtu.be/ZYQrtAWYKHA)

## 이슈 로그
### SIGPIE 이슈
##### 상황
- 클라이언트가 서버에 접속해있다가 일방적으로 연결을 끊는 즉시에 대부분의 경우는 errno가 설정되지 않는다. 그리고 서버에서 열려있는 클라이언트에 read동작을 할 때 errno가 ECONNRESET(connection reset by peer)로 설정이 된다
- 하지만 어쩌다 한번 씩 끊어진 클라이언트 소켓에 read를 하기전에 errno가 EPIPE(broken pipe)로 설정이 되고 서버가 강제로 종료되어 버린다.
##### 원인
- 리눅스 운영체제에서 소켓연결이 끊어지고, 프로그램에서 끊어진 소켓에 데이터를 쓰거나 읽으려 하면 SIGPIPE 시그널을 발생시킨다. 특별한 설정이 없으면 프로세스는 SIGPIPE시그널을 받으면 프로그램을 종료시킨다.
##### 해결
- `signal(SIGPIPE, SIG_IGN);` 를 프로그램 시작 직후에 넣어주어서 운영체제에서 SIGPIPE 시그널을 무시하도록 한다
