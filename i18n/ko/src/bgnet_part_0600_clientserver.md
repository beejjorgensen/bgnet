# 클라인트-서버 배경지식

[i[Client/Server]<]

클라이언트-서버에 대해 이야기할 차례이다. 통신망에 있는 거의 모든 것들은
서버 프로세스에게 이야기하는 클라이언트 프로세스를 상대하거나 그 반대이다.
`telnet`을 예로 들어보자. 여러분이 텔넷(클라이언트)로 원격지 호스트의 23번
포트에 접속할 때 그 호스트의 프로그램(`telnetd`라고 불리는 서버)이 생명을
얻는다. 그것이 들어오는 텔넷 요청을 처리하고 당신에게 로그인 프롬프트를
띄워주는 등의 일을 처리한다.

![클라이언트 - 서버 상호작용](cs.pdf "[클라이언트- 서버 상호작용 도표]")

위의 도표에 클라이언트와 서버의 정보 교환이 정리되어 있습니다.

클라이언트-서버 쌍은 `SOCK_STREAM`이나 `SOCK_DGRAM` 또는 다른 어떤 것이라도
말할 수 있음을 기억하십시오.(둘이 같은 방식으로 말하기만 한다면) 클라이언트-서버
쌍의 좋은 예시는 `telnet`/`telnetd`, `ftp`/`ftpd` 또는 `Firefox`/`Apache`
입니다. 당신이 `ftp`를 쓸 때마다 당신의 요청을 받아들이는 원격지 프로그램인
`ftpd`가 있습니다.

흔히 한 대의 장치에는 오직 하나의 서버만이 있을 것이며 그 서버는 [i[`fork()` function]] `fork()`
를 통해서 여러 클라이언트를 처리할 것입니다.(역자 주 : 한 대의 장치에서 여러 개의
서버를 실행하는 많은 방법이 있지만 이 문서의 초판은 90년대에 작성되었습니다. )
기본적인 과정은 아래와 같습니다. 서버가 연결을 기다리고, `accept()`한 후,
요청을 처리할 자식 프로세스를 `fork()`합니다. 이것이 다음 절에서 우리의
예제 서버가 하는 일입니다.

## 단순한 스트림 서버

[i[Server-->stream]<]

이 서버가 하는 일은 스트림 연결에 "`Hello, world!`"을 전송하는 것 뿐입니다.
이 서버를 시험하기 위해서 할 일은 하나의 창에서 이것을 실행한 후 다른 창에서
텔넷에 아래 명령어로 접속하는 일 뿐입니다.

```
$ telnet remotehostname 3490
```

`remotehostname`은 당신이 실행하는 장치의 아이피입니다.

[flx[The server code|server.c]]:

```{.c .numberLines}
/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // 사용자들이 접속할 포트

#define BACKLOG 10   // 몇 개의 대기중인 연결이 유지될 것인가

void sigchld_handler(int s)
{
    // waitpid()이 errno를 덮어쓸 수 있으므로 저장했다가 되살린다.
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// IPv4 또는 IPv6 sockaddr을 받아온다.
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // sock_fd에서 대기하고 들어오는 연결은 new_fd에 저장
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // 접속자의 주소 정보
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // 모든 결과를 조회하고 쓸 수 있는 첫 번째 것을 사용
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // 이 구조체는 더 이상 필요없음

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // 죽은 프로세스를 다 거둬들이자
    sigemptyset(&sa.sa_mask);
    sa.sa_flags =   ;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // 주 accept() 루프
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // 자식 프로세스이다.
            close(sockfd); // 자식은 리스너가 필요없다.
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // 부모는 이것이 필요없다.
    }

    return 0;
}
```

궁금한 독자들을 위해 덧붙이자면 구문의 명료함을 위해서 하나의 큰 `main()`함수
안에 모든 코드를 다 적었습니다. 원한다면 더 작은 함수들로 나누어도 좋습니다.

(아마도 이 [i[`sigaction()` function]] `sigaction()`을 처음 볼 수도 있는데
괜찮다. 이 코드는 `fork()`된 자식 프로세스가 종료되면서 생기는 좀비 프로세스를
거둬들이는 데 사용된다. 좀비 프로세스를 많이 만들고 거둬들이지 않으면 시스템
관리자가 흥분할 것이다.)

다음 절에 나오는 클라이언트를 사용해서 이 서버로부터 데이터를 얻을 수 있다.

[i[Server-->stream]>]

## 단순한 스트림 클라이언트

[i[Client-->stream]<]

이 녀석은 서버보다도 더 쉽다. 이 클라이언트가 하는 일은 당신이 명령줄에
지정한 호스트의 3490번 포트로 접속하는 것이다. 이것은 서버가 보낸 문자열을
받는다.

[flx[The client source|client.c]]:

```{.c .numberLines}
/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // 클라이언트가 접속할 포트

#define MAXDATASIZE 100 // 한 번에 받을 수 있는 최대 바이트 갯수

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // 모든 결과를 순회하면서 쓸 수 있는 가장 첫 번째 것을 씀
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // 이 구조체는 더 이상 필요 없음

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);

    return 0;
}
```

클라이언트를 실행하기 전에 서버를 실행하지 않으면 `connect()`는
[i[Connection refused]] "Connection refused"를 반환한다는 점을
기억하라. 아주 유용하다.

[i[Client-->stream]>]

## 데이터그램 소켓 {#datagram}

[i[Server-->datagram]<]

위에서 `sendto()`과 `recvfrom()`에 대해 논의할 때 UDP 데이터그램 소켓의
기본에 대해서 이미 알아보았다. 그러므로 바로 2개의 예제 프로그램을 제시하겠다.
`talker.c`와 `listener.c`이다.

`listener`는 장치에서 포트 4950으로 들어오는 패킷을 대기한다. `talker`는
지정한 장치의 해당 포트로 사용자가 명령줄에 입력한 내용을 담은 패킷을 보낸다.

데이터그램 소켓은 연결이 없고 소켓을 이더넷에 발송한 후 성공 여부는 신경쓰지
않기 때문에 클라이언트와 서버에 IPv6을 사용하도록 명시할 것이다. 이렇게 하면
서버가 IPv6에서 듣고 클라이언트가 IPv4에서 발송해서 데이터를 받을 수 없는
상황을 피할 수 있을 것이다. (우리의 TCP 스트림 소켓 세상에서도 이런 불일치가
발생할 수 있지만 `connect()`에서 하나의 주소 체계에 대해 에러를 발생시키고
다른 주소체계를 쓰도록 해준다.)

여기에 [flx[`listener.c`의 소스코드가 있다.|listener.c]]:

```{.c .numberLines}
/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "4950"	// the port users will be connecting to

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("listener: got packet from %s\n",
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);

	close(sockfd);

	return 0;
}
```

`getaddrinfo()`에서 우리가 마침내 `SOCK_DGRAM`을 사용한다는 것에
주목하라. 또한, `listen()`와 `accept()`이 필요하지 않다는 점도
기억하라. 이것이 연결 없는 데이터그램 소켓을 사용할 때의 장점
중 하나이다.
Notice that in our call to `getaddrinfo()` we're finally using
`SOCK_DGRAM`. Also, note that there's no need to `listen()` or
`accept()`. This is one of the perks of using unconnected datagram
sockets!

[i[Server-->datagram]>]

[i[Client-->datagram]<]

Next comes the [flx[source for `talker.c`|talker.c]]:

```{.c .numberLines}
/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "4950"	// the port users will be connecting to

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	if (argc != 3) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
	close(sockfd);

	return 0;
}
```

이것이 전부다! 하나의 장치에서 `listener`를 실행하고 `talker`를 다른 장치에서
실행하라.(역자 주 : 하나의 장치에서도 순서만 맞게 실행하면 문제는 없다. 여러
터미널을 동시에 열 수 있는 다양한 방법이 있다.) 그것들이 통신하는 것을 지켜보라.
핵가족 전체를 위한 전연령 엔터테인먼트다.

이번에는 서버르 실행할 필요도 없다! `talker`를 혼자 실행시키면 패킷을 행복하게
발송하고, 아무도 반대쪽에서 `recvfrom()`을 호출하지 않는다면 그저 패킷은 사라질
뿐이다. 기억하라 : UDP 데이터그램 소켓으로 보낸 데이터는 도착을 보장하지 않는다.!

[i[Client-->datagram]>]

전에 몇 번 말한 사소한 것 한 가지를 빼면 전부다: [i[`connect()` function-->on datagram sockets]]
연결된 데이터그램 소켓이 그것이다. 그것에 대해서 여기에서 말해야하는데,
이 문서의 데이터그램에 대한 부분이 바로 여기이기 때문이다. 위의 `talker`
가 `listener`의 주소를 지정하고 `connect()`를 호출한다고 하자. 그 순간부터
`talker`는 `connect()`로 지정한 주소로만 데이터를 보내고 받을 수 있다.
이런 이유로 `sendto()`와 `recvfrom()`을 쓸 필요가 없다. 단순히 `send()`
와 `recv()`를 쓰면 된다.

[i[Client/Server]>]
