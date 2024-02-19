# 시스템 콜이 아니면 죽음을

이 절에서 우리는 유닉스 장치나 기타 소켓 API를 지원하는 다른 장치(BSD,
윈도우즈, 리눅스, 맥, 여러분이 가진 다른 장치)에서 네트워크 기능에 접근할
수 있게 해 주는 시스템 호출(System call)(과 다른 라이브러리 호출
(Library Call))에 대해서 다룰 것입니다. 이런 함수 중 하나를 호출하면
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
이것을 이해하려면 짧은 이야기가 필요하다. 아주 먼 옛날에는 어쩌면 하나의 주소 계통(Address Family)
("`AF_INET`"안에 들어있는 "AF")가 여러 종류의 프로토콜 계통(Protocol Family)("`PF_INET`"의
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
오류가 있으면 -1을 돌려준다. 전역 변수인 `errno`가 오류의 값으로 설정된다.
(자세한 정보는 [`errno`](#errnoman) 의 맨페이지를 참고하라.)

좋다. 그러면 이제 이 소켓을 어디에 쓰는가? 정답은 아직 못 쓴다는 것이다.
실제로 쓰기 위해서는 안내서를 더 읽고 이것이 동작하게 하기 위한 시스템 호출을
더 해야 한다.

## `bind()`---나는 어떤 포트에 있는가? {#bind}

[i[`bind()` function]] 소켓을 가지면 여러분의 기계의 [i[Port]] 포트에 연동하고
싶을 것이다. (이 작업은 보통 여러분이 [i[`listen()` function]] `listen()`
으로 특정 포트에서 들어오는 연결을 듣고자(listen) 할 때 이루어진다. ---다중 사용자
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

[i[`bind()` function]] `bind()`에 대해서 한마디 더: 이 함수를 호출할 필요가
전혀 없는 경우도 있습니다. [i[`connect()` function]] `connect()`를 호출해서
원격 장치에 연결하려고 하고, 로컬 포트에 대해서는 신경쓰지 않는다면(`telnet`의
경우 처럼 원격지 포트만 신경쓰는 경우) `connect()`가 자동으로 소켓이 바인드되지
않았는지 확인하고 필요하다면 사용하지 않은 로컬 포트에 `bind()`해줄 것입니다.

## `connect()`---이봐, 안녕! {#connect}

[i[`connect()` function]] 몇 분만 여러분이 텔넷 응용프로그램이 되었다고 생각해봅시다.
여러분의 사용자들이 소켓 파일 설명자를 얻기 위해서 여러분에게 명령을 내립니다
(영화 [i[TRON]] _트론_ 에서처럼요). 여러분은 그에 따라 `socket()`을 호출합니다.
다음으로 사용자가 여러분에게 "`10.12.110.57`"의 "`23`"번 포트(텔넷 표준 포트)에
연결하라고 합니다. 어떻게 해야할까요?

응용프로그램 여러분, `connect()`에 대한 절을 읽는 중이라니 운이 좋습니다!
이 절은 원격 호스트에 어떻게 연결하는지에 대해 알려줍니다. 거침없이 읽어봅시다!
낭비할 시간이 없습니다!

`connect()`에 대한 호출은 아래와 같습니다:

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
```

`sockfd`는 `socket()`함수 호출이 돌려주는 우리의 친근한 이웃인 소켓
파일 설명자 입니다. `serv_addr`는 `struct sockaddr`이고 목적지 포트와
아이피 주소를 담고 있습니다. `addrlen`은 서버 주소 구조체의 바이트단위
길이를 담고 있습니다.

모든 정보는 멋진 `getaddrinfo()`호출의 결과에서 추출할 수 있습니다.

이해가 되기 시작합니까? 여기서는 대답을 들을 수 없으니 그럴 것이라 생각하겠습니다.
"`www.example.com`"의 `3490`포트로 소켓 연결을 만드는 예제를 살펴봅시다:

```{.c .numberLines}
struct addrinfo hints, *res;
int sockfd;

// getaddrinfo()으로 주소 구조체를 채웁니다:

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;

getaddrinfo("www.example.com", "3490", &hints, &res);

// 소켓을 만듭니다:

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// 연결합니다!

connect(sockfd, res->ai_addr, res->ai_addrlen);
```

다시 이야기하자면 구식 프로그램들은 `connect()`에 넘겨줄 `struct sockaddr_in`
을 직접 채워넣었습니다. 그렇게 하고싶다면 해도 됩니다. 위에 있는 [`bind()` 절](#bind)
에서 비슷한 내용을 참고하십시오.

`connect()`의 복귀값을 확인하는 것을 잊지 마십시오. 오류가 발생하면 `-1`을
돌려주고 `errno`변수를 설정할 것입니다.

[i[`bind()` function-->implicit]]

우리가 `bind()`를 호출하지 않았음에 주목하십시오. 간단히 말하자면 우리의
로컬 포트 번호에 대해서는 신경쓰지 않습니다. 우리가 어디로 가는지만 신경씁니다
(원격지 포트). 커널이 우리 대신 로컬 포트를 고를 것입니다. 우리가 접속하는
사이트는 이 정보를 자동으로 우리에게서 얻어냅니다. 신경쓰실 필요가 없습니다.

## `listen()`---누가 연락 좀 해주실래요? {#listen}

[i[`listen()` function]] 이제 흐름이 변할 때입니다. 우리가 원격지 호스트에
접속하고 싶지 않은 경우라면 어떻게 하시겠습니까? 재미로 하는 말이지만, 들어오는
연결을 기다리고 그것을 어떤 방식으로 다루고자 한다면 어떻게 하시겠습니까?
그 과정은 두 단계입니다. 먼저 `listen()`를 호출하고, [i[`accept()`
function]] `accept()`를 씁니다.(아래를 참고하십시오.)

`listen()`함수 호출은 꽤 단순하지만 약간의 설명이 필요합니다:

```{.c}
int listen(int sockfd, int backlog);
```

`sockfd`은 `socket()`시스템 함수 호출로 얻어온 평범한 소켓 파일 설명자입니다.
[i[`listen()` function-->backlog]] `backlog`는 들어오는 큐에 허용되는
연결의 숫자입니다. 이것이 무슨 뜻인지 궁금하십니까? 들어오는 연결들은
여러분이 `accept()`를 해주기 전까지(아래를 참고하십시오) 이 큐 안에서
기다릴 것이고 이것은 몇 개의 연결이 대기할 수 있는가를 정합니다. 대개의 시스템은
이 값을 조용히 20 정도로 제한합니다. 그러나 `5`나 `10`정도의 값으로
설정해도 괜찮을 것입니다.

또 평소와 다름없이 `listen()`도 오류가 발생할 경우 `-1`을 돌려주고 `errno`을
설정할 것입니다.

아마도 상상하실 수 있겠지만 서버가 특정 포트에서 실행되도록 하기 위해서는
`listen()`을 호출하기 전에 `bind()`을 호출해야 합니다. (여러분의 친구들에게
어떤 포트로 연결해야 할지 말해줄 수 있어야 합니다.) 이런 식입니다.

```{.c .numberLines}
getaddrinfo();
socket();
bind();
listen();
/* accept()는 아래에 온다 */
```

I'll just leave that in the place of sample code, since it's fairly
self-explanatory. (The code in the `accept()` section, below, is more
complete.) The really tricky part of this whole sha-bang is the call to
`accept()`.

## `accept()`---"3490포트에 접속해주셔서 감사합니다.."

[i[`accept()` function]] 각오하십시오! `accept()`함수는 조금 이상합니다.
이렇게 돌아갑니다: 아주 먼 곳에 있는 누군가가 여러분의 장치에 `connect()`
함수로 연결하려고 합니다. 여러분은 특정 포트에서 `listen()`을 실행하고
있습니다. 그들의 연결은 `accept()`로 받아들여질 때까지 대기열에 쌓일 것입니다.
여러분은 `accept()`을 해서 대기중인 연결을 받아들이겠다고 알려줍니다.
`accept()`는 이 연결만을 위해서 쓸 _완전히 새로운 소켓 파일 설명자_ 를 돌려줄 것입니다.
그렇습니다! 갑자기 `send()`와 `recv()`를 쓸 수 있는 _두 개_ 의 소켓 파일을
가지게 된 것입니다.

호출은 아래와 같이 합니다:

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

`sockfd`은 `listen()`을 하고있는 소켓 설명자입니다. 어렵지 않습니다.
`addr`은 대개 로컬 `struct sockaddr_storage`에 대한 포인터입니다.
여기에 들어오는 연결의 정보가 들어가게 됩니다(그리고 그것을 통해서 어떤 호스트가
어떤 포트에서 여러분을 호출하고 있는지 알 수 있습니다.) `addrlen`은
sockaddr_storage을 `accept()`에 넘기기 전에 `sizeof(struct
sockaddr_storage)`으로 설정되어야 하는 로컬 정수 변수입니다.
`accept()`는 `addr`에 `addrlen`의 크기 이상의 바이트를 적지 않을 것입니다.
예상하셨습니까? `accept()`도 오류가 발생하면 `-1`을 돌려주고 `errno`에
값을 설정합니다. 전혀 예상하지 못하셨으리라 생각합니다.

전과 마찬가지로 한 번에 많은 내용입니다. 여러분의 독서를 위한 예제 코드 조각입니다:

```{.c .numberLines}
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MYPORT "3490"  // 사용자들이 접속할 포트
#define BACKLOG 10     // 대기열에 몇 개의 연결이 대기할 수 있는가

int main(void)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;

    // !! 이 호출들에 대한 오류 확인을 잊지 마십시오 !!

    // getaddrinfo()으로 정보를 채워넣습니다:

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // IPv4또는 IPv6, 아무것이나 씁니다.
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // 나를 위해 자동으로 내 IP를 채워넣을 것.

    getaddrinfo(NULL, MYPORT, &hints, &res);

    // 소켓을 만들고, 바인드하고, 듣기 시작:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    // 들어오는 연결을 받습니다:

    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    // new_fd 소켓 설명자에서 통신할 준비 완료!
    .
    .
    .
```

다시 말하지만 모든 `send()`와 `recv()` 호출에 대해서 `new_fd`를 사용할
것입니다. 만약 단 한 개의 연결만을 받아들이길 원한다면 추가적인 연결이 같은
포트를 통해 들어오는 것을 막기 위해서 `sockfd`을 `close()`처리할 수 있습니다.

## `send()`와 `recv()`---Talk to me, baby! {#sendrecv}

(역자 주 : Talk to me, baby!는 Elmore James의 노래입니다. 그러나 원저자의 의도가 이것인지 확실하지는 않습니다.)
이 두 함수들은 스트림 소켓이나 연결된 데이터그램 소켓을 통해 통신하기 위해서 쓰입니다.
일반적인 연결되지 않은 데이터그램 소켓을 쓰고싶다면 [`sendto()`과
`recvfrom()`](#sendtorecv) 절을 보시면 됩니다.

[i[`send()` function]] `send()` 함수:

```{.c}
int send(int sockfd, const void *msg, int len, int flags);
```

`sockfd` 은 데이터를 보내고 싶은 소켓 설명자(`socket()`으로 만들었든 `accept()`로 만들었든)입니다.
`msg`는 당신이 보낼 데이터에 대한 포인터이며, `len`은 그 길이입니다.

예제 코드는 이렇게 될 수 있겠습니다:

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

한 방에 모든 것을 보냈습니다. 다시 강조하지만 오류가 발생하면 `-1`이 반환되고
`errno`가 오류 번호로 설정됩니다.

[i[`recv()` function]] `recv()`함수는 많은 면에서 유사합니다:

```{.c}
int recv(int sockfd, void *buf, int len, int flags);
```

`sockfd`은 읽어들일 소켓 설명자이며, `buf`는 정보를 읽어들일 버퍼이고, `len`은 버퍼의
최대 길이이고 `flags`는 여기서도 0으로 설정될 수 있습니다. (플래그 정보에 대해서는
`recv()`의 man page를 참고하십시오.)

`recv()`는 실제로 버퍼에 읽어들인 바이트의 수를 돌려주거나 오류가 발생할 경우 (`errno`
를 적절한 값으로 설정하고) `-1`을 돌려줍니다.

잠깐! `recv()`는 `0`을 돌려줄 수 있습니다. 이것은 한 가지 의미입니다: 원격지 측에서
당신에 대한 연결을 닫은 것입니다! 복귀값 `0`은 `recv()`가 연결이 끊어졌음을
알려주는 방식입니다.

자, 정말 쉽지않습니까? 이제 여러분은 스트림 소켓에서 자료를 주고받을 수 있습니다.
와! 이제 여러분은 유닉스 네트워크 프로그래머입니다!

## `sendto()`와 `recvfrom()`---Talk to me, DGRAM-방식 {#sendtorecv}

[i[`SOCK_DGRAM` macro]] "이제 다 깔끔하고 좋네요"라고 말씀하시는 소리가
들립니다. "그렇지만 연결이 없는 데이터그램 소켓은 어떻게 처리하지요?"라고도
하시는군요. 문제 없습니다, 토모다치여(역자 주 : 원문은 amigo). 딱 맞는 것이
있습니다.

데이터그램 소켓은 원격지 호스트에 연결되어 있지 않으므로, 패킷을 보낼 때에
필요한 정보는 조금 다릅니다. 그렇습니다. 목적지 주소가 필요합니다. 이런 식입니다:

```{.c}
int sendto(int sockfd, const void *msg, int len, unsigned int flags,
           const struct sockaddr *to, socklen_t tolen);
```

보시다시피 `send()`와 같지만 두 개의 정보가 더 있습니다. `to`는 목적지의
[i[IP address]] IP주소와 [i[Port]] 포트를 담은 `struct sockaddr`
이며(아마도 여러분이 형변환해서 사용하실 `struct sockaddr_in`이나
`struct sockaddr_in6` 또는 `struct sockaddr_storage`일 것입니다.)
`tolen`은 내부적으로는 `int`이며 간단하게 `sizeof *to`나 `sizeof(struct sockaddr_storage)`로
설정하면 됩니다.

목적지 주소 구조체를 얻으려면 `getaddrinfo()`이나 아래의 `recvfrom()`을 사용하시거나
수작업으로 값을 채워넣을 수도 있습니다.

`send()`와 마찬가지로 `sendto()`도 실제로 보낸 바이트 수를 돌려줍니다. (
그 말은 보내려고 한 바이트의 수보다 적은 수가 돌아올 수도 있다는 의미입니다.)
오류가 발생하면 `-1`을 돌려줍니다.

이와 유사한 관계가 `recv()`과 [i[`recvfrom()` function]] `recvfrom()`입니다.
`recvfrom()`의 개요는 이렇습니다:

```{.c}
int recvfrom(int sockfd, void *buf, int len, unsigned int flags,
             struct sockaddr *from, int *fromlen);
```

또 다시 이것은 몇 개의 추가적인 필드가 있는 `recv()`과 같습니다.
`from`은 근원지 장치의 아이피 주소와 포트로 채워진 로컬 [i[`struct sockaddr` type]]
`struct sockaddr_storage`에 대한 포인터입니다. `fromlen`은 로컬 `int`에 대한 포인터이며
`sizeof *from`이나 `sizeof(struct sockaddr_storage)`으로 초기화되어야
합니다. 함수가 반환될 때 `fromlen`은 `from`에 실제로 저장된 주소의 길이로
설정되어 있을 것입니다.

`recvfrom()`은 받은 바이트의 갯수를 반환하며 오류가 나면 (`errno`를 적절히 설정하고)
`-1`을 돌려줍니다.

여기 질문이 하나 있을 것입니다: 왜 우리는 `struct sockaddr_storage`을 소켓의 타입으로
사용하는가? 왜 그냥 `struct sockaddr_in`을 쓸 수 없는가? 이유는 보시다시피
우리가 IPv4나 IPv6중 하나에 얽메이고 싶지 않기 때문입니다. 그래서 우리는 양쪽 모두에
충분히 크고 일반적인 `struct sockaddr_storage`을 사용합니다.

(그럼... 여기에서 다른 질문 하나: 왜 `struct sockaddr`을 모든 주소를 담을
수 있을 정도로 크게 만들지 않았는가? 우리는 일반 목적의 `struct sockaddr_storage`
을 다시 일반 목적의 `struct sockaddr`으로 형변환하고 있습니다! 이런 동작은
과하고 불필요해 보입니다. 여기에 대한 대답은 그냥 이 `struct sockaddr`은
만들어질 때부터 그렇게 크지 않았다는 것이고, 이제와서 그것을 바꾸는 것은
문제의 소지가 있다는 것입니다. 그래서 그들은 그냥 새로운 타입을 만들었습니다.)
(역자 주 : `struct sockaddr`은 소켓 통신의 초기에 만들어진 구조체이므로
IPv6을 담기에 충분하지 않은 것은 당연한 일입니다.)

만약 여러분이 데이터그램 소켓을 [i[`connect()` function-->on datagram sockets]]
`connect()`하게 되면 모든 통신에 `send()`와 `recv()`을 쓸 수 있음을
기억하십시오. 소켓 자체는 여전히 데이터그램 소켓일 것이고 패킷은 여전히
UDP를 사용할 것이지만 소켓 인터페이스가 자동으로 여러분을 위해서
목적지와 원천지 정보를 추가할 것입니다.

## `close()`와 `shutdown()`---내 앞에서 꺼져!

휴! 여러분은 하루 종일 `send()`와 `recv()`을 사용했고, 이제 충분합니다.
이제 여러분의 소켓 설명자를 닫을 준비가 되었습니다. 이건 쉽습니다. 그냥
평범한 유닉스 파일 설명자 닫기 함수인 [i[`close()` function]] `close()`
를 쓸 수 있습니다:

```{.c}
close(sockfd);
```

이것은 해당 소켓에 대한 후속 읽기와 쓰기를 방지할 것입니다. 원격지에서
이 소켓을 쓰거나 읽으려는 모든 시도는 오류를 반환할 것입니다.

소켓이 어떻게 닫히는지 좀 더 조절하고 싶은 경우에 [i[`shutdown()` function]] `shutdown()`
함수를 사용할 수 있습니다. 이것은 특정 방향으로의 통신만 끊는 일을 할
수 있으며 양쪽 모두 막을 수도 있습니다(마치 `close()`가 하듯이).
개요는 이렇습니다:

```{.c}
int shutdown(int sockfd, int how);
```

`sockfd`는 종료하고 싶은 소켓 파일 설명자이고, `how`는 다음 중 하나입니다:

| `how` | 효과                                      |
| :---: | ----------------------------------------- |
|  `0`  | 후속 수신이 금지됩니다.                   |
|  `1`  | 후속 송신이 금지됩니다.                   |
|  `2`  | 후속 송수신이 금지됩니다. (`close()`처럼) |

`shutdown()`은 성공시에 `0`을 반환하고, 오류가 발생하면 (`errno`를 적절한 값으로 설정하고)
`-1`을 반환합니다.

연결되지 않은 데이터그램 소켓에 기꺼이 `shutdown()`을 해주신다면,
그것은 단순히 해당 소켓에 `send()`와 `recv()`를 사용할 수 없도록
만들 것입니다(데이터그램 소켓에 `connect()`를 사용하면 이 두 함수를
사용할 수 있음을 기억하십시오).

`shutdown()`이 실제로 파일 설명자를 닫지는 않음에 주목하십시오. 소켓 설명자를
해제하기 위해서는 `close()`를 호출해야 합니다.

별 것 없군요.

(예외적으로 여러분이 [i[Windows]] 윈도우즈와 [i[Winsock]] Winsock을
사용하실 경우 `close()`대신 [i[`closesocket()` function]]
`closesocket()`을 호출해야 합니다.)

## `getpeername()`---누구십니까?

[i[`getpeername()` function]] 이 함수는 너무 쉽습니다.

너무 쉬워서 이 함수에 별도의 장을 주지도 않았습니다. 아무튼 알려드리겠습니다.

`getpeername()`함수는 연결된 스트림 소켓의 반대편 끝에 누가 있는지를 알려줄
것입니다. 개요입니다:

```{.c}
#include <sys/socket.h>

int getpeername(int sockfd, struct sockaddr *addr, int *addrlen);
```

`sockfd`는 연결된 스트림 소켓의 설명자입니다. `addr`은 연결의 반대편
끝에 대한 정보를 담을 `struct sockaddr` (또는 `struct sockaddr_in`)
에 대한 포인터입니다. `addrlen`은 `int`에 대한 포인터이며 `sizeof *addr`이나
`sizeof(struct sockaddr)`으로 초기화되어야 합니다.
(역자 주 : 이 함수도 IPv6과 동작하기 위해서 `struct sockaddr_storage`
을 사용할 수 있습니다.)

이 함수는 오류가 발생하면 `-1`을 돌려주고 `errno`를 알맞게 설정합니다.

여러분이 상대방의 주소를 가지면 그것을 [i[`inet_ntop()` function]]
`inet_ntop()`, [i[`getnameinfo()` function]] `getnameinfo()` 또는
[i[`gethostbyaddr()` function]] `gethostbyaddr()`에 넣어서 화면에
출력하거나 추가적인 정보를 가져올 수 있습니다. 그들의 로그인 이름을 가져올
수는 없습니다. (좋습니다, 좋아요. 만약 저쪽 컴퓨터가 ident 데몬을 실행중이라면
가능합니다. 그러나 그것은 이 문서의 범위를 넘어섭니다. 더 자세한 정보를
원한다면 [flrfc[RFC 1413|1413]]을 참고하십시오.)

## `gethostname()`---나는 누구인가?

[i[`gethostname()` function]] `getpeername()`보다 더 쉬운 것이 바로 `gethostname()`
함수입니다. 이것은 여러분의 프로그램이 실행되고 있는 컴퓨터의 이름을 돌려줍니다.
돌려받은 이름은 위에 있는 [i[`getaddrinfo()` function]] `getaddrinfo()`
을 써서 여러분의 로컬 장치의 [i[IP address]] IP주소를 알아내는 일에
쓰일 수 있습니다.

이보다 더 재미있는 일이 있을 수 있겠습니까? 사실 몇 가지 생각나긴 합니다만
소켓 프로그래밍에 대한 것이 아니군요. 아무튼 정리하자면 이렇습니다:

```{.c}
#include <unistd.h>

int gethostname(char *hostname, size_t size);
```

인수들은 단순합니다: `hostname`은 함수가 반환하는 호스트 이름을 담을
char의 배열에 대한 포인터입니다. `size`는 `hostname`배열의 길이입니다.

함수는 성공적인 완료 후에 `0`을 반환하고, 오류에 대해서는 흔히 그렇듯
`errno`를 설정하고 `-1`을 반환합니다.
