# 시스템 콜이 아니면 죽음을

이 절에서 우리는 유닉스 장치나 기타 소켓 API를 지원하는 다른 장치(BSD,
윈도우즈, 리눅스, 맥, 여러분이 가진 다른 장치)에서 네트워크 기능에 접근할
수 있게 해 주는 시스템 호출(*System call)(과 다른 라이브러리 호출
(*Library Call))에 대해서 다룰 것입니다. 이런 함수 중 하나를 호출하면
커널이 넘겨받고 여러분을 위해 모든 일을 자동으로 마법같이 처리합니다.

대부분의 사람들이 어려워하는 점은 이 함수들을 어떤 순서로 호출해야 하는가
입니다. 이미 찾아보셨겠지만 그런 쪽으로는 `man`페이지는 아무 쓸모도 없습니다.
그 끔찍한 상황을 해결하기 위해 시스템콜들을 _정확히_(대략) 여러분의
프로그램에서 호출해야 하는 순서 그대로 아래에 이어지는 절들에 제시했습니다.

그러니까 여기에 있는 몇몇 예제 코드와 우유, 과자(이것들은 직접 준비하셔야 합니다)
그리고 두둑한 배짱과 용기만 있다면 여러분은 인터넷의 세계에서 존 포스텔의 아들처럼
데이터를 나를 수 있게 될 것입니다.(역자 주 : John Postel은 인터넷의 초기에 큰
기여를 한 컴퓨터 과학자 중 한 명입니다.)

_(아래의 예제 코드들은 대개 필수적인 에러코드를 간략함을 얻기 위해서 생략했음을
기억하십시오. 그리고 예제 코드들은 대개 `getaddrinfo()`의 호출이 성공하고
연결리스트로 적절한 결과물을 돌려준다고 가정합니다. 이런 상황은 독립 실행형
프로그램에서는 제대로 처리되어 있으니, 그것들을 지침으로 삼으십시오.)_


## `getaddrinfo()`---발사 준비!

[i[`getaddrinfo()` function]], 이것은 여러 옵션을 가진 진짜 일꾼입니다.
그러나 사용법은 사실 꽤 간단합니다. 이것은 여러분이 나중에 필요로 하는
`struct`들을 초기화합니다.

역사 한토막 : 예전에는 DNS 검색을 위해서 `gethostbyname()`을 호출해야 했습니다.
그리고 그 정보를 수작업으로 `struct sockaddr_in`에 담고 이후의 호출에서
사용해야 했습니다.

고맙게도 더 이상은 그럴 필요가 없습니다. (여러분이 IPv4와 IPv6환경 모두에서
동작하는 코드를 짜고싶다면 그래서도 안 됩니다!) 요새는 `getaddrinfo()`이라는
것이 있어서 DNS 와 서비스 이름 검색, `struct`내용 채워넣기 등을 포함해서
여러분이 필요로 하는 모든 일을 해 줍니다.

이제 살펴봅시다!

(역자 주 : 아래에서부터 입니다, 하세요 등의 표현 대신 이다, 하라 등의 간결한 어미를 섞어서 씁니다.)

```{.c}
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node,     // e.g. "www.example.com" 또ㅡㄴ IP
                const char *service,  // e.g. "http" 또는 포트 숫자를 ""안에 감싸서 넣는다.
                const struct addrinfo *hints,
                struct addrinfo **res);
```

이 함수에는 3개의 입력 매개변수를 넘겨줍니다. 그리고 결과 연결리스트의 포인터인
`res`를 돌려받습니다. 

`node`매개변수는 접속하려는 호스트 이름이나 IP주소입니다.

다음 매개변수는 `service`입니다. 이것은 "80"같은 포트 번호나 "http", "ftp",
"telnet" 또는 "smtp"같은 특정한 서비스 이름이 될 수 있습니다. 
([fl[IANA 포트 목록|https://www.iana.org/assignments/port-numbers]]
혹은 여러분이 유닉스 장치를 쓴다면 `/etc/services`에서 볼 수 있습니다)

마지막으로 `hints`매개변수는 여러분이 관련된 정보로 이미 채워넣은 `struct addrinfo`
를 가리킵니다.

여기에 여러분이 호스트 IP주소의 포트 3490을 듣고자 할 때의 함수 호출 예제가
있습니다. 이것이 듣기 작업이나 네트워크 설정을 하지는 않음을 기억하십시오.
이것은 단지 나중에 사용할 구조체들을 설정할 뿐입니다.

```{.c .numberLines}
int status;
struct addrinfo hints;
struct addrinfo *servinfo;  // 결과를 가리킬 것이다

memset(&hints, 0, sizeof hints); // 구조체를 확실히 비워두라
hints.ai_family = AF_UNSPEC;     // IPv4 이든 IPv6 이든 상관없다
hints.ai_socktype = SOCK_STREAM; // TCP 스트림 소켓
hints.ai_flags = AI_PASSIVE;     // 내 주소를 넣어달라

if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
}

// servinfo는 이제 1개 혹은 그 이상의 addrinfo 구조체에 대한 연결리스트를 가리킨다

// ... servinfo가 더이상 필요없을 때까지 모든 작업을 한다...

freeaddrinfo(servinfo); // 연결리스트를 해제
```

`ai_family`을 `AF_UNSPEC`으로 설정해서 IPv4든 IPv6이든 신경쓰지 않음을 나타낸
것에 주목하라. 만약 특정한 하나를 원한다면 `AF_INET`이나 `AF_INET6`을 쓸 수 있다.

`AI_PASSIVE`도 볼 수 있다. 이것은 `getaddrinfo()`에게 소켓 구조체에
내 로컬 호스트의 주소를 할당해달라고 말해준다. 이것은 여러분이 하드코딩할 필요를
없애주기에 좋다. (아니면 위에서 `NULL`을 넣은 `getaddrinfo()`의 첫 번째
매개변수에 특정한 주소를 넣을 수 있다. )

이렇게 함수를 호출한다. 오류가 있다면(`getaddrinfo()`이 0이 아닌 값을 돌려준다면)
보다시피 그 오류를 `gai_strerror()`함수를 통해서 출력할 수 있다. 만약 모든
것이 제대로 동작한다면 `servinfo`는 각각이 우리가 나중에 쓸 수 있는
`struct sockaddr`나 비슷한 것을 가진 `struct addrinfo`의 연결리스트를 가리킬
것이다. 멋지다!

마지막으로 `getaddrinfo()`가 은혜롭게 우리에게 할당해 준 연결리스트를
다 썼다면 우리는 `freeaddrinfo()`을 호출해서 그것을 할당 해제할 수 있습니다.
(반드시 해야합니다.)

여기에 여러분이 특정한 주소, 예를 들어 "www.example.net"의 3490포트에
잡속하고자 하는 클라이언트일 경우의 호출 예제가 있습니다. 다시 말씀드리지만
이것으로는 실제 연결이 이루어지지 않습니다. 그러나 이것은 우리가 나중에
사용할 구조체를 설정해줍니다.

```{.c .numberLines}
int status;
struct addrinfo hints;
struct addrinfo *servinfo;  // 결과물을 가리킬 것임

memset(&hints, 0, sizeof hints); // 반드시 비워둘 것
hints.ai_family = AF_UNSPEC;     // IPv4나 IPv6은 신경쓰지 않음
hints.ai_socktype = SOCK_STREAM; // TCP 스트림 소켓

// 연결 준비
status = getaddrinfo("www.example.net", "3490", &hints, &servinfo);

// servinfo는 이제 1개 혹은 그 이상의 addrinfo 구조체에 대한 연결리스트를 가리킨다

// 등등.
```

`servinfo`은 모든 종류의 주소 정보를 가진 연결리스트라고 계속 이야기하고 있다.
이 정보를 보기 위한 짧은 시연 프로그램을 작성해보자. [flx[이 짧은 프로그램|showip.c]]
은 여러분이 명령줄에 적는 호스트의 IP주소들을 출력한다.

```{.c .numberLines}
/*
** showip.c -- 명령줄에서 주어진 호스트의 주소들을 출력한다.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: showip hostname\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // 버전을 지정하려면 AF_INET또는 AF_INET6을 사용
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    printf("IP addresses for %s:\n\n", argv[1]);

    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // 주소 자체에 대한 포인터를 받는다. IPv4와 IPv6은 필드가 다르다.
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // IP주소를 문자열로 변환하고 출력한다.
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(res); // 연결 목록을 해제한다.

    return 0;
}
```

보다시피 이 코드는 당신이 명령줄에 넘기는 것이 무엇이든 `getaddrinfo()`을
호출한다. 그리고 `res`에 연결목록의 포인터를 넘겨준다. 그래서 우리는
이 목록을 순회해서 출력하거나 다른 일을 할 수 있다.

(저 예제코드에는 IP 버전에 따라 다른 종류의 `struct sockaddr`을 처리해야 하는
흉한 부분이 있다. 그 점에 대해서 사과한다. 그러나 더 나은 방법이 있는지는 모르겠다.)

실행 예제! 모두가 스크린샷을 좋아합니다.

```
$ showip www.example.net
IP addresses for www.example.net:

  IPv4: 192.0.2.88

$ showip ipv6.example.com
IP addresses for ipv6.example.com:

  IPv4: 192.0.2.101
  IPv6: 2001:db8:8c00:22::171
```

이제 저것을 다룰 수 있으니, `getaddrinfo()`에서 얻은 결과를 다른 소켓 함수에 
넘기고 결과적으로는 네트워크 연결을 성립할 수 있도록 해 보자! 계속 읽어보라!


## `socket()`---파일 설명자를 받아오라! {#socket}

더 이상 미룰 수가 없을 듯 하다. 이제 [i[`socket()` function]] `socket()`
시스템 콜에 대해서 이야기해야 한다. 개요는 이렇다.

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol); 
```

그러나 이 인수들이 무엇인지 모를 것이다. 이것들은 어떤 종류의 소켓을 원하는지
정할 수 있게 해 준다.(IPv4 또는 IPv6, 스트림 혹은 데이터그램, TCP 혹은 UDP)

사용자들이 그 값을 직접 적어야 했고, 지금도 그렇게 할 수 있다.
(`domain`은 `PF_INET`이나 `PF_INET6`이고, `type`은 `SOCK_STREAM`또는 `SOCK_DGRAM`
이며, `protocol`은 주어진 `type`에 적절한 값을 자동으로 선택하게 하려면 `0`을
넘겨주거나 "tcp"나 "udp" 중 원하는 프로토콜의 값을 얻기 위해서 `getprotobyname()`
을 쓸 수도 있다.)

(이 `PF_INET`은 `sin_family`필드에 넣어주는 [i[`AF_INET` macro]]`AF_INET`와 유사한 것이다.
이것을 이해하려면 짧은 이야기가 필요하다. 아주 먼 옛날에는 어쩌면 하나의 주소 체통(*Address Family)
("`AF_INET`"안에 들어있는 "AF")가 여러 종류의 프로토콜 계통(*Protocol Family)("`PF_INET`"의
"PF"))을 지원할 것이라고 생각하던 시절이 있었다. 그런 일은 일어나지 않았다. 그리고 모두 행복하게
오래오래 잘 살았다. 이런 이야기다. 그래서 할 수 있는 가장 정확한 일은
`struct sockaddr_in`에서 `AF_INET`을 쓰고 `socket()`에서 `PF_INET`을 사용하는 것이다.

아무튼 이제 충분하다. 여러분이 정말로 하고싶은 일은 `getaddrinfo()`을
호출한 결과로 돌아오는 값을 아래와 같이 `socket()`에 직접 넘겨주는 것이다.

```{.c .numberLines}
int s;
struct addrinfo hints, *res;

// 탐색 시작
// ["hints"구조체는 이미 채운 것으로 친다]
getaddrinfo("www.example.com", "http", &hints, &res);

// 다시 말하지면 원래는 (이 안내서의 예제들이 하듯이) 첫 번째 것이 좋다고
// 가정하는 대신 getaddrinfo()에 대해서 오류 확인을 하고
// "res"링크드 리스트를 순회해야 한다.
// client/server절의 진짜 예제들을 참고하라.

s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
```

`socket()`은 단순하게 이후의 시스템 호출에서 쓸 수 있는 _소켓 설명자_ 를 돌려준다.
오류가 있으면 -1을 돌려준다. 전역 변수인  `errno`가 오류의 값으로 설정된다.
(자세한 정보는 [`errno`](#errnoman) 의 맨페이지를 참고하라.)

좋다. 그러면 이제 이 소켓을 어디에 쓰는가? 정답은 아직 못 쓴다는 것이다.
실제로 쓰기 위해서는 안내서를 더 읽고 이것이 동작하게 하기 위한 시스템 호출을
더 해야 한다.


## `bind()`---나는 어떤 포트에 있는가? {#bind}

[i[`bind()` function]] 소켓을 가지면 여러분의 기계의 [i[Port]] 포트에 연동하고
싶을 것이다. (이 작업은 보통 여러분이 [i[`listen()` function]] `listen()`
으로 특정 포트에서 들어오는 연결을 듣고자(*listen) 할 때 이루어진다. ---다중 사용자
네트워크 게임들은 "192.168.5.10의 3490포트에 연결합니다"라고 말할 때 이런
작업을 한다.) 포트 번호는 커널이 특정 프로세스의 소켓 설명자를 들어오는 패킷과
연관짓기 위해서 사용한다. 만약 여러분이 [i[`connect()`] function] `connect()`만
할 생각이라면 `bind()`는 불필요하다. 그러나 재미를 위해 읽어두자.

이것이 `bind()` 시스템 콜의 개요다.

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
```

`sockfd`은 `socket()`이 돌려준 소켓 파일 설명자이다. `my_addr`은
여러분의 주소, 말하자면 포트와 [i[IP address]] IP주소를 가진 `struct sockaddr`
에 대한 포인터이다. `addrlen`은 그 주소의 바이트 단위 길이이다.

으엑. 한 번에 많이 배웠다. 프로그램이 실행되는 호스트의 3490번 포트에
소켓을 바인드하는 예제를 보자.

```{.c .numberLines}
struct addrinfo hints, *res;
int sockfd;

// 먼저 getaddrinfo()으로 구조체에 정보를 불러온다.

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;  // IPv4나 IPv6 중 아무 것이나 쓴다
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;     // IP는 나의 아이피로 채운다.

getaddrinfo(NULL, "3490", &hints, &res);

// 소켓을 만든다.

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// getaddrinfo()에 넘겼던 포트에 바인드한다.

bind(sockfd, res->ai_addr, res->ai_addrlen);
```

`AI_PASSIVE`플래그를 써서 프로그램에게 실행중인 호스트의 IP에 바인드하라고
알려줍니다. 특정한 로컬 IP주소에 바인드하고싶다면 `AI_PASSIVE`을 버리고
`getaddrinfo()`의 첫 번째 인수로 IP주소를 넣으라.

`bind()`도 오류가 발생하면 `-1`을 돌려주고 `errno`을 오류의 값으로 설정한다.

많은 오래된 코드들이 `bind()`을 호출하기 전에 `struct sockaddr_in`을 직접
채워넣는다. 이것은 분명히 IPv4 전용이지만 같은 일을 IPv6에 대해서도 못 할
이유는 없다. 단지 `getaddrinfo()`을 쓰는 편이 일반적으로 더 쉽다. 어쨌든
예전 코드는 이런 방식이다.

```{.c .numberLines}
// !!! 이것은 예전 방식이다 !!!

int sockfd;
struct sockaddr_in my_addr;

sockfd = socket(PF_INET, SOCK_STREAM, 0);

my_addr.sin_family = AF_INET;
my_addr.sin_port = htons(MYPORT);     // short, 네트워크 바이트 순서
my_addr.sin_addr.s_addr = inet_addr("10.12.110.57");
memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);

bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr);
```

위의 코드에서 당신의 로컬 IP 주소에 바인드하고 싶었다면(위의 `AI_PASSIVE`처럼)
`s_addr`필드에 `INADDR_ANY`을 대입할 수 있다. IPv6버전의 `INADDR_ANY`은
당신의 `struct sockaddr_in6`의 `sin6_addr`필드에 대입해야 하는 전역변수인
`in6addr_any`이다. (변수 초기화식에 쓸 수 있는 `IN6ADDR_ANY_INIT`이라는 매크로도
있다.)

`bind()`을 쓸 때 주의해야 할 것 : 포트 번호는 낮은 것을 쓰지 말 것.
[i[Port]] 1024번 아래의 모든 포트는 예약되어 있다(슈퍼유저가 아닌 이상)!
그 위의 포트 번호는 (다른 프로그램이 이미
쓰고 있지 않다면) 65535까지 아무 것이나 쓸 수 있다.

눈치챌 수 있듯이 때때로 서버를 다시 실행하려고 하면 `bind()`가 실패하고
[i[Address already in use]] "주소가 이미 사용중입니다.."라고 할 때가 있다.
그것은 연결되었던 소켓 중 일부가 여전히 커널에서 대기중이고 포트를 사용하고
있다는 것을 의미한다. 여러분은 그것이 정리될 때까지 1분 정도를 기다리거나
당신의 프로그램이 포트를 재사용할 수 있도록 하는 코드를 넣을 수도 있다.

[i[`setsockopt()` function]]
 [i[`SO_REUSEADDR` macro]]

```{.c .numberLines}
int yes=1;
//char yes='1'; // 솔라리스는 이것을 사용

// "주소가 이미 사용중입니다"라는 오류 메시지를 제거
if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
    perror("setsockopt");
    exit(1);
} 
```

[i[`bind()` function]] One small extra final note about `bind()`: there
are times when you won't absolutely have to call it. If you are
[i[`connect()` function]] `connect()`ing to a remote machine and you
don't care what your local port is (as is the case with `telnet` where
you only care about the remote port), you can simply call `connect()`,
it'll check to see if the socket is unbound, and will `bind()` it to an
unused local port if necessary.


## `connect()`---Hey, you! {#connect}

[i[`connect()` function]] Let's just pretend for a few minutes that
you're a telnet application. Your user commands you (just like in the
movie [i[TRON]] _TRON_) to get a socket file descriptor. You comply and
call `socket()`. Next, the user tells you to connect to "`10.12.110.57`"
on port "`23`" (the standard telnet port). Yow! What do you do now?

Lucky for you, program, you're now perusing the section on
`connect()`---how to connect to a remote host. So read furiously onward!
No time to lose!

The `connect()` call is as follows:

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen); 
```

`sockfd` is our friendly neighborhood socket file descriptor, as
returned by the `socket()` call, `serv_addr` is a `struct sockaddr`
containing the destination port and IP address, and `addrlen` is the
length in bytes of the server address structure.

All of this information can be gleaned from the results of the
`getaddrinfo()` call, which rocks.

Is this starting to make more sense? I can't hear you from here, so I'll
just have to hope that it is. Let's have an example where we make a
socket connection to "`www.example.com`", port `3490`:

```{.c .numberLines}
struct addrinfo hints, *res;
int sockfd;

// first, load up address structs with getaddrinfo():

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;

getaddrinfo("www.example.com", "3490", &hints, &res);

// make a socket:

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// connect!

connect(sockfd, res->ai_addr, res->ai_addrlen);
```

Again, old-school programs filled out their own `struct sockaddr_in`s to
pass to `connect()`. You can do that if you want to. See the similar
note in the [`bind()` section](#bind), above.

Be sure to check the return value from `connect()`---it'll return `-1`
on error and set the variable `errno`.

[i[`bind()` function-->implicit]]

Also, notice that we didn't call `bind()`. Basically, we don't care
about our local port number; we only care where we're going (the remote
port). The kernel will choose a local port for us, and the site we
connect to will automatically get this information from us. No worries.


## `listen()`---Will somebody please call me? {#listen}

[i[`listen()` function]] OK, time for a change of pace. What if you
don't want to connect to a remote host. Say, just for kicks, that you
want to wait for incoming connections and handle them in some way. The
process is two step: first you `listen()`, then you [i[`accept()`
function]] `accept()` (see below).

The `listen()` call is fairly simple, but requires a bit of explanation:

```{.c}
int listen(int sockfd, int backlog); 
```

`sockfd` is the usual socket file descriptor from the `socket()` system
call.  [i[`listen()` function-->backlog]] `backlog` is the number of
connections allowed on the incoming queue. What does that mean? Well,
incoming connections are going to wait in this queue until you
`accept()` them (see below) and this is the limit on how many can queue
up. Most systems silently limit this number to about 20; you can
probably get away with setting it to `5` or `10`.

Again, as per usual, `listen()` returns `-1` and sets `errno` on error.

Well, as you can probably imagine, we need to call `bind()` before we
call `listen()` so that the server is running on a specific port. (You
have to be able to tell your buddies which port to connect to!)  So if
you're going to be listening for incoming connections, the sequence of
system calls you'll make is:

```{.c .numberLines}
getaddrinfo();
socket();
bind();
listen();
/* accept() goes here */ 
```

I'll just leave that in the place of sample code, since it's fairly
self-explanatory. (The code in the `accept()` section, below, is more
complete.) The really tricky part of this whole sha-bang is the call to
`accept()`.


## `accept()`---"Thank you for calling port 3490."

[i[`accept()` function]] Get ready---the `accept()` call is kinda weird!
What's going to happen is this: someone far far away will try to
`connect()` to your machine on a port that you are `listen()`ing on.
Their connection will be queued up waiting to be `accept()`ed. You call
`accept()` and you tell it to get the pending connection. It'll return
to you a _brand new socket file descriptor_ to use for this single
connection! That's right, suddenly you have _two socket file
descriptors_ for the price of one! The original one is still listening
for more new connections, and the newly created one is finally ready to
`send()` and `recv()`. We're there! 

The call is as follows:

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 
```

`sockfd` is the `listen()`ing socket descriptor. Easy enough. `addr`
will usually be a pointer to a local `struct sockaddr_storage`. This is
where the information about the incoming connection will go (and with it
you can determine which host is calling you from which port). `addrlen`
is a local integer variable that should be set to `sizeof(struct
sockaddr_storage)` before its address is passed to `accept()`.
`accept()` will not put more than that many bytes into `addr`. If it
puts fewer in, it'll change the value of `addrlen` to reflect that.

Guess what? `accept()` returns `-1` and sets `errno` if an error occurs.
Betcha didn't figure that.

Like before, this is a bunch to absorb in one chunk, so here's a sample
code fragment for your perusal:

```{.c .numberLines}
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MYPORT "3490"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

int main(void)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;

    // !! don't forget your error checking for these calls !!

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, MYPORT, &hints, &res);

    // make a socket, bind it, and listen on it:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    // now accept an incoming connection:

    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    // ready to communicate on socket descriptor new_fd!
    .
    .
    .
```

Again, note that we will use the socket descriptor `new_fd` for all
`send()` and `recv()` calls. If you're only getting one single
connection ever, you can `close()` the listening `sockfd` in order to
prevent more incoming connections on the same port, if you so desire.


## `send()` and `recv()`---Talk to me, baby! {#sendrecv}

These two functions are for communicating over stream sockets or
connected datagram sockets. If you want to use regular unconnected
datagram sockets, you'll need to see the section on [`sendto()` and
`recvfrom()`](#sendtorecv), below.

[i[`send()` function]] The `send()` call:

```{.c}
int send(int sockfd, const void *msg, int len, int flags); 
```

`sockfd` is the socket descriptor you want to send data to (whether it's
the one returned by `socket()` or the one you got with `accept()`).
`msg` is a pointer to the data you want to send, and `len` is the length
of that data in bytes.  Just set `flags` to `0`. (See the `send()` man
page for more information concerning flags.)

Some sample code might be:

```{.c .numberLines}
char *msg = "Beej was here!";
int len, bytes_sent;
.
.
.
len = strlen(msg);
bytes_sent = send(sockfd, msg, len, 0);
.
.
. 
```

`send()` returns the number of bytes actually sent out---_this might be
less than the number you told it to send!_  See, sometimes you tell it
to send a whole gob of data and it just can't handle it. It'll fire off
as much of the data as it can, and trust you to send the rest later.
Remember, if the value returned by `send()` doesn't match the value in
`len`, it's up to you to send the rest of the string. The good news is
this: if the packet is small (less than 1K or so) it will _probably_
manage to send the whole thing all in one go.  Again, `-1` is returned
on error, and `errno` is set to the error number.

[i[`recv()` function]] The `recv()` call is similar in many respects:

```{.c}
int recv(int sockfd, void *buf, int len, int flags);
```

`sockfd` is the socket descriptor to read from, `buf` is the buffer to
read the information into, `len` is the maximum length of the buffer,
and `flags` can again be set to `0`. (See the `recv()` man page for flag
information.)

`recv()` returns the number of bytes actually read into the buffer, or
`-1` on error (with `errno` set, accordingly).

Wait! `recv()` can return `0`. This can mean only one thing: the remote
side has closed the connection on you! A return value of `0` is
`recv()`'s way of letting you know this has occurred.

There, that was easy, wasn't it? You can now pass data back and forth on
stream sockets! Whee! You're a Unix Network Programmer!


## `sendto()` and `recvfrom()`---Talk to me, DGRAM-style {#sendtorecv}

[i[`SOCK_DGRAM` macro]] "This is all fine and dandy," I hear you saying,
"but where does this leave me with unconnected datagram sockets?" No
problemo, amigo. We have just the thing.

Since datagram sockets aren't connected to a remote host, guess which
piece of information we need to give before we send a packet? That's
right! The destination address! Here's the scoop:

```{.c}
int sendto(int sockfd, const void *msg, int len, unsigned int flags,
           const struct sockaddr *to, socklen_t tolen); 
```

As you can see, this call is basically the same as the call to `send()`
with the addition of two other pieces of information. `to` is a pointer
to a `struct sockaddr` (which will probably be another `struct
sockaddr_in` or `struct sockaddr_in6` or `struct sockaddr_storage` that
you cast at the last minute) which contains the destination [i[IP
address]] IP address and [i[Port]] port. `tolen`, an `int` deep-down,
can simply be set to `sizeof *to` or `sizeof(struct sockaddr_storage)`.

To get your hands on the destination address structure, you'll probably
either get it from `getaddrinfo()`, or from `recvfrom()`, below, or
you'll fill it out by hand.

Just like with `send()`, `sendto()` returns the number of bytes actually
sent (which, again, might be less than the number of bytes you told it
to send!), or `-1` on error.

Equally similar are `recv()` and [i[`recvfrom()` function]]
`recvfrom()`. The synopsis of `recvfrom()` is:

```{.c}
int recvfrom(int sockfd, void *buf, int len, unsigned int flags,
             struct sockaddr *from, int *fromlen); 
```

Again, this is just like `recv()` with the addition of a couple fields.
`from` is a pointer to a local [i[`struct sockaddr` type]] `struct
sockaddr_storage` that will be filled with the IP address and port of
the originating machine. `fromlen` is a pointer to a local `int` that
should be initialized to `sizeof *from` or `sizeof(struct
sockaddr_storage)`. When the function returns, `fromlen` will contain
the length of the address actually stored in `from`.

`recvfrom()` returns the number of bytes received, or `-1` on error
(with `errno` set accordingly).

So, here's a question: why do we use `struct sockaddr_storage` as the
socket type? Why not `struct sockaddr_in`? Because, you see, we want to
not tie ourselves down to IPv4 or IPv6. So we use the generic `struct
sockaddr_storage` which we know will be big enough for either.

(So... here's another question: why isn't `struct sockaddr` itself big
enough for any address? We even cast the general-purpose `struct
sockaddr_storage` to the general-purpose `struct sockaddr`! Seems
extraneous and redundant, huh. The answer is, it just isn't big enough,
and I'd guess that changing it at this point would be Problematic. So
they made a new one.)

Remember, if you [i[`connect()` function-->on datagram sockets]]
`connect()` a datagram socket, you can then simply use `send()` and
`recv()` for all your transactions. The socket itself is still a
datagram socket and the packets still use UDP, but the socket interface
will automatically add the destination and source information for you.


## `close()` and `shutdown()`---Get outta my face!

Whew! You've been `send()`ing and `recv()`ing data all day long, and
you've had it. You're ready to close the connection on your socket
descriptor. This is easy. You can just use the regular Unix file
descriptor [i[`close()` function]] `close()` function:

```{.c}
close(sockfd); 
```

This will prevent any more reads and writes to the socket. Anyone
attempting to read or write the socket on the remote end will receive an
error.

Just in case you want a little more control over how the socket closes,
you can use the [i[`shutdown()` function]] `shutdown()` function. It
allows you to cut off communication in a certain direction, or both ways
(just like `close()` does). Synopsis:

```{.c}
int shutdown(int sockfd, int how); 
```

`sockfd` is the socket file descriptor you want to shutdown, and `how`
is one of the following:

| `how` | Effect                                                     |
|:-----:|------------------------------------------------------------|
|  `0`  | Further receives are disallowed                            |
|  `1`  | Further sends are disallowed                               |
|  `2`  | Further sends and receives are disallowed (like `close()`) |

`shutdown()` returns `0` on success, and `-1` on error (with `errno` set
accordingly).

If you deign to use `shutdown()` on unconnected datagram sockets, it
will simply make the socket unavailable for further `send()` and
`recv()` calls (remember that you can use these if you `connect()` your
datagram socket).

It's important to note that `shutdown()` doesn't actually close the file
descriptor---it just changes its usability. To free a socket descriptor,
you need to use `close()`.

Nothing to it.

(Except to remember that if you're using [i[Windows]] Windows and
[i[Winsock]] Winsock that you should call [i[`closesocket()` function]]
`closesocket()` instead of `close()`.)


## `getpeername()`---Who are you?

[i[`getpeername()` function]] This function is so easy.

It's so easy, I almost didn't give it its own section. But here it is
anyway.

The function `getpeername()` will tell you who is at the other end of a
connected stream socket. The synopsis:

```{.c}
#include <sys/socket.h>

int getpeername(int sockfd, struct sockaddr *addr, int *addrlen); 
```

`sockfd` is the descriptor of the connected stream socket, `addr` is a
pointer to a `struct sockaddr` (or a `struct sockaddr_in`) that will
hold the information about the other side of the connection, and
`addrlen` is a pointer to an `int`, that should be initialized to
`sizeof *addr` or `sizeof(struct sockaddr)`.

The function returns `-1` on error and sets `errno` accordingly.

Once you have their address, you can use [i[`inet_ntop()` function]]
`inet_ntop()`, [i[`getnameinfo()` function]] `getnameinfo()`, or
[i[`gethostbyaddr()` function]] `gethostbyaddr()` to print or get more
information. No, you can't get their login name. (Ok, ok. If the other
computer is running an ident daemon, this is possible. This, however, is
beyond the scope of this document. Check out [flrfc[RFC 1413|1413]] for
more info.)


## `gethostname()`---Who am I?

[i[`gethostname()` function]] Even easier than `getpeername()` is the
function `gethostname()`. It returns the name of the computer that your
program is running on. The name can then be used by [i[`getaddrinfo()`
function]] `getaddrinfo()`, above, to determine the [i[IP address]] IP
address of your local machine.

What could be more fun? I could think of a few things, but they don't
pertain to socket programming. Anyway, here's the breakdown:

```{.c}
#include <unistd.h>

int gethostname(char *hostname, size_t size); 
```

The arguments are simple: `hostname` is a pointer to an array of chars that will
contain the hostname upon the function's return, and `size` is the length in
bytes of the `hostname` array.

The function returns `0` on successful completion, and `-1` on error, setting
`errno` as usual.
