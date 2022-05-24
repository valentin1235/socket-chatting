## 스펙
- clang 13.1.6
- C89
- Apple M1

## 데모영상
- [LINK](https://youtu.be/ZYQrtAWYKHA)

## 이슈
#### 상황
- 클라이언트가 서버에 접속해있다가 일방적으로 연결을 끊는 즉시에 대부분의 경우는 errno가 설정되지 않는다. 그리고 서버에서 열려있는 클라이언트에 read동작을 할 때 errno가 ECONNRESET(connection reset by peer)로 설정이 된다
- 하지만 어쩌다 한번 씩 끊어진 클라이언트 소켓에 read를 하기전에 errno가 EPIPE(broken pipe)로 설정이 되고 서버가 강제로 종료되어 버린다.
#### 해결
- 
