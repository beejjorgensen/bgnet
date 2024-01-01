# 도입부

이봐요! 소켓 프로그래밍 때문에 힘든가요? `man`페이지로 공부하기가 좀 지나치게
어려운가요? 멋진 인터넷 프로그래밍을 하고싶지만 `connect()`를 호출하기 전에
`bind()`를 호출해야 한다는 것을 알아내기 위해서 한 무더기의 `struct`를 헤집고다닐
시간이 없나요?

음, 그런데 제가 그 귀찮은 일을 이미 다 해놨습니다. 그리고 그 정보를 여러분에게
공유하고 싶어서 죽을 지경입니다. 제대로 찾아오셨습니다. 이 문서는 보통 수준의
C 프로그래머가 귀찮은 네트워킹을 처리할 수 있게 도와줄 것입니다.

그리고 확인하실 것: 제가 드디어 미래의 기술을 따라잡았고(정말 겨우 시간을 맟췄죠)
IPv6에 대한 안내를 갱신했습니다. 재밌게 보십시오!

## 읽는이에게

이 문서는 완전한 참고문서가 아닌 튜토리얼로서 작성되었습니다.
아마도 이제 막 소켓 프로그래밍을 시작해서 발받침이 필요한 사람이 읽기에 적합할 것입니다.
이것은 분명히 어떤 의미로든 _완벽하고 완전한_ 소켓프로그래밍 가이드는 아닙니다.

그럼에도 희망적으로, 이 문서를 읽고나면 man page가 이해되기 시작할 것입니다.

## 실행환경과 컴파일러

이 문서에 포함된 코드는 Gnu의 [i[Compilers-->GCC]] `gcc` 컴파일러를 사용하는
리눅스 PC에서 컴파일 되었습니다. 그러나 그 코드들은 `gcc`를 사용하는 어떤
실행환경에서도 빌드가 되어야 합니다. 그 말인즉 당신이 윈도우를 위해서 프로그램을
만들고 있다면 해당사항이 없다는 의미입니다. 그런 경우라면 [윈도우즈 프로그래밍을 위한 절](#windows)을 참고하십시오.

## 공식 홈페이지와 책 구매

이 문서의 공식 위치는 아래와 같습니다:

- [`https://beej.us/guide/bgnet/`](https://beej.us/guide/bgnet/)

이 곳에서 예제 코드와 이 안내서의 여러 언어로 된 번역본도 찾을 수 있습니다.

사람들이 책이라고 부르는 잘 제본된 인쇄된 복사본을 사고싶다면 여기에 방문하십시오:

- [`https://beej.us/guide/url/bgbuy`](https://beej.us/guide/url/bgbuy)

구매해주신다면 저의 먹물밥으로 먹고 사는 라이프 스타일을 유지하는 일에 도움이
되므로 감사하겠습니다.

## Solaris/SunOS 프로그래머들을 위한 노트 {#solaris}

[i[Solaris]] Solaris 또는 [i[SunOS]] SunOS를 위해서 컴파일할 때, 정확한
라이브러리에 링크하기 위해서 약간의 추가적인 명령줄 스위치를 지정해야 합니다.
그러기 위해서 컴파일 명령의 끝에 "`-lnsl -lsocket -lresolv`"를 아래와 같이
덧붙이십시오.

```
$ cc -o server server.c -lnsl -lsocket -lresolv
```

여전히 에러가 있다면, 그 명령 뒤에 `-lxnet`를 덧붙여보십시오. 그것이 정확히 무엇을
하는지는 모르지만, 몇몇 사람들은 그렇게 해야 했다고 합니다.

당신이 문제를 발견할 수도 있는 또 다른 곳은 `setsockopt()`을 호출하는 곳입니다.
제 리눅스 장치와 함수 원형이 다릅니다. 그러므로 아래의 코드 대신:

```{.c}
int yes=1;
```

이렇게 입력하십시오:

```{.c}
char yes='1';
```

제가 Sun 장치를 가지지 않았으므로, 위에 적은 내용들을 시험해보지는 않았습니다.
저 내용들은 단지 사람들이 저에게 이메일로 알려준 것입니다.

## Windows 프로그래머들을 위한 노트 {#windows}

안내서의 이 부분을 적는 시점에 저는 더이상 제가 싫어한다는 이유로
[i[Windows]] Windows를 욕하는 일은 하지 말자고 다짐했습니다.
공평해야 하니까 미리 말해두자면 윈도우는 널리 사용되고 있고 분명히 완전히 멀쩡한
운영체제입니다.

추억은 미화된다고 하던데, 이 경우엔 사실인 듯 합니다.(아니면 제가 나이를 먹어서 그런가봅니다.)
제가 말할 수 있는 것은 마이크로소프트의 운영체제를 제 개인적 작업에 10년 이상
쓰지 않은 결과, 저는 더 행복하다는 것입니다. 얼마나 편하냐면 저는 의자에 기대서
편하게 "그럼요, 윈도우 써도 좋죠!"라고 말할 수 있습니다. 사실 그렇게 말하자니
어금니를 꽉 깨물게 되는군요.

그래서 저는 여전히 윈도우 대신 [i[Linux]]
[fl[Linux|https://www.linux.com/]], [i[BSD]] [fl[BSD|https://bsd.org/]],
아니면 다른 종류의 유닉스를 써 보라고 권하고 싶습니다.

하지만 사람들은 좋아하던 것을 계속 좋아하는 법이고, 윈도우를 쓰는 친구들은 이
문서의 정보가 그들에게도 보통 적용된다는 것을 알면 기뻐할 것입니다. 때때로 약간의
차이는 있겠지요.

당신이 진지하게 고려해봐야 할 다른 것은 [i[WSL]] [i[Windows
Subsystem For Linux]] [fl[Windows Subsystem for
Linux|https://learn.microsoft.com/en-us/windows/wsl/]] 입니다. 이것은 간단히
말하자면 Windows 10에 리눅스 VM 비슷한 것을 깔게 해 줍니다. 그것도 당신이 충분히
준비할 수 있게 해 줄 것이고, 예제 프로그램을 있는 그대로 빌드할 수 있게 해 줄 것입니다.

당신이 할 수 있는 다른 멋진 일은 [i[Cygwin]][fl[Cygwin|https://cygwin.com/]]을
설치하는 것입니다. 이것은 Windows를 위한 유닉스 도구 모음입니다. 그렇게 하면 예제
프로그램을 수정 없이 컴파일 할 수 있다고 들었습니다만 직접 해 보지는 않았습니다.

그러나 여러분 중 몇몇은 순수한 Windows방식으로 이걸 하고싶을지도 모르겠습니다.
그렇다면 아주 배짱이 두둑한 일이 되겠군요. 그렇게 하고싶다면 당장 집 밖으로 가서
유닉스를 돌릴 기계를 사십시오! 장난입니다. 요새는 윈도우에 (좀 더) 친화적으로 행동하려고
노력하고 있습니다.

[i[Winsock]]

이게 당신이 해야 할 일입니다. 첫 번째로 제가 이 문서에서 언급하는 거의 모든 시스템
헤더 파일을 무시하십시오. 그 대신 아래의 헤더파일을 포함하십시오.

```{.c}
#include <winsock2.h>
#include <ws2tcpip.h>
```

`winsock`은 "새로운"(1994년 기준으로) 윈도우즈 소켓 라이브러리입니다.

불행하게도 당신이 `windows.h`를 인클루드하면 그것이 자동으로 버전1인 오래된 `winsock.h`
를 끌어오고 `winsock2.h`와 충돌을 일으킬 것입니다. 정말 재밌지요.

그러므로 만약 `windows.h`를 인클루드해야 한다면 그것이 오래된 헤더를 _포함하지 않도록_
아래의 매크로를 정의해야 합니다.

```{.c}
#define WIN32_LEAN_AND_MEAN  // 이렇게 적으십시오.

#include <windows.h>         // 이제 이걸 인클루드해도 됩니다.
#include <winsock2.h>        // 이것도요.
```

잠깐! 소켓 라이브러리를 쓰기 전에 [i[`WSAStartup()` function]]
`WSAStartup()`을 호출해야 합니다. 이 함수에게 사용하길 원하는 Winsock
버전(예를 들어 2.2)을 넘겨주고 결과값을 확인해서 쓰고자 하는 버전이
사용 가능한지 확인해야 합니다.

그 작업을 하는 코드는 아래와 비슷할 것입니다.

[[book-pagebreak]]

```{.c .numberLines}
#include <winsock2.h>

{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }

    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2)
    {
        fprintf(stderr,"Versiion 2.2 of Winsock is not available.\n");
        WSACleanup();
        exit(2);
    }
```

저기 보이는 [i[`WSACleanup()` function]] `WSACleanup()` 호출부를 주목하십시오.
Winsock라이브러리를 다 쓴 후에 저것을 호출해야 합니다.

또한 컴파일러에게 `ws2_32.lib`라는 Winsock2 라이브러리를 링크하라고 말해줘야 합니다.
VC++에서는 `프로젝트` 메뉴에서 `설정`으로 가서 `링크`탭을 클릭하고 "오브젝트/라이브러리 모듈"
이라는 제목이 붙은 상자를 찾으십시오. 그리고 거기에 "ws2_32.lib"나 당신이 원하는
다른 라이브러리를 추가하십시오.
(옮긴이 주) 최신 Visual Studio에서는 [이 링크|https://learn.microsoft.com/en-us/cpp/build/reference/dot-lib-files-as-linker-input?view=msvc-170]를 참고해보십시오.

직접 해 본 것은 아닙니다.

그렇게 하고나면, 이 튜토리얼의 나머지 예제들은 거의 그대로 쓸 수 있을 것입니다.
몇 가지 예외가 있는데 소켓을 닫기 위해서 `close()`를 쓸 수 없습니다.
대신 [i[`closesocket()`function]] `closesocket()`을 써야합니다.
또한 [i[`select()` function]]`select()`는 파일 설명자가 아닌 소켓 설명자에만
쓸 수 있습니다. (`stdin`의 `0` 같은 파일 설명자)

당신이 쓸 수 있는 소켓 클래스도 있습니다.
[i[`CSocket` class]][`CSocket`](https://learn.microsoft.com/en-us/cpp/mfc/reference/csocket-class?view=msvc-170)입니다.
자세한 정보는 컴파일러의 도움말 페이지를 참고하십시오.

Winsock에 대한 정보를 더 얻고싶다면 [마이크로소프트의 공식 홈페이지](https://learn.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2)를 참고하십시오.

마지막으로 윈도우에는 [i[`fork()` function]] `fork()`가 없다고 들었습니다.
불행히도 제 예제코드 중 일부는 `fork()`를 사용합니다. 아마 그것을 동작하게 하려면
POSIX라이브러리에 링크하거나 다른 작업이 필요할 것입니다. 아니면 [i[`CreateProcess()` function]] `CreateProcess()`
를 대신 쓸 수도 있습니다. `fork()`는 인수를 받지 않지만 `CreateProcess()`는
인수를 4800만개 정도 받습니다. 그게 부담스럽다면 [i[`CreateThread()`
function]] `CreateThread()` 이 조금 더 쓰기 쉬울겁니다. 불행히도
멀티스레딩에 대한 논의는 이 문서의 범위를 벗어납니다. 저는 이 정도 까지밖에
말씀드릴 수가 없습니다.

정말 마지막으로, Steven Mitchell이 [fl[몇몇 예제들을 Winsock으로 변환했습니다.|https://www.tallyhawk.net/WinsockExamples/]]
확인해보십시오.

## 이메일 정책

저는 대체로 [i[Emailing Beej]] 이메일로 오는 문의사항에 답을 드리고자 하니
이메일 보내기를 주저하지 마십시오. 그러나 응답을 보장하지는 못합니다. 저는 바쁜
삶을 살고 있고 제가 당신이 가진 궁금증에 대답할 수 없는 때가 많이 있습니다.
그런 경우라면 저는 그 메시지를 그냥 삭제합니다. 개인적인 감정이 아닙니다.
그저 당신이 필요로 하는 자세한 답을 할 시간이 없을 것이라 생각하기 때문입니다.

규칙을 제시하자면 질문이 복잡할수록 제가 응답할 가능성이 적어질 것입니다.
메일을 보내기 전에 질문의 범위를 좁히고 적절한 정보(실행환경, 컴파일러, 당신이
접하는 에러메시지, 문제 해결에 도움이 될 만한 다른 정보)를 첨부해주신다면
제 응답을 받을 확률이 올라갈 것입니다. 더 자세한 지침은 ESR의 문서인
[fl[How To Ask Questions The Smart
Way|http://www.catb.org/~esr/faqs/smart-questions.html]]을 참고하십시오.

당신이 제 회신을 받지 못한다면, 문제를 더 파고들어보고, 답을 찾기 위해 노력해보십시오.
그래도 확실한 답을 얻지 못했다면 그동안 알아낸 정보를 가지고 저에게 다시
메일을 보내십시오. 어쩌면 제가 답을 드릴 수 있을지도 모릅니다.

저에게 메일을 보낼 때 이렇게 해라 저렇게 해라 말이 많았습니다만 이 안내서에
지난 몇 년 동안 보내주신 모든 칭찬에 _정말로_ 감사한다는 사실을 말씀드리고 싶습니다.
그것은 정말로 정신적인 힘이 됩니다. 이 안내서가 좋은 일에 쓰였다는 말을 듣는 일은
저를 기쁘게 합니다. `:-)` 감사합니다!

## 미러링

이 웹사이트를 공개로든 사적으로든 미러링하는 것은 정말로 환영합니다. 이 웹사이트를
공개적으로 미러링하고 제가 메인 페이지에 링크를 걸게 하고 싶다면 [`beej@beej.us`](beej@beej.us)
로 메일을 보내주십시오.

## Note for Translators 번역가들을 위한 노트

[i[Translating the Guide]] 이 안내서를 다른 언어로 번역하고 싶다면 [`beej@beej.us`](beej@beej.us)에게
메일을 보내주십시오. 당신의 번역본의 링크를 제 메인 페이지에 걸어두겠습니다.
당신의 이름과 연락처 정보를 번역본에 추가하셔도 좋습니다.

이 원본 마크다운 문서는 UTF-8로 인코드 되었습니다.

아래의 Copyright, Distribution, and Legal 절을 참고하십시오.

제가 번역본을 호스트하길 바란다면, 말씀해주십시오. 당신이 호스트하길 원한다면
그것도 링크하겠습니다. 어느 쪽이든 좋습니다.

## Copyright, Distribution, and Legal {#legal}

(Translator's Note : This section has not been translated to keep it's legal information)
(역자 주 : 이 절은 법적 정보를 보존하기 위해 번역하지 않았습니다.)

Beej's Guide to Network Programming is Copyright © 2019 Brian "Beej
Jorgensen" Hall.

With specific exceptions for source code and translations, below, this
work is licensed under the Creative Commons Attribution- Noncommercial-
No Derivative Works 3.0 License. To view a copy of this license, visit

[`https://creativecommons.org/licenses/by-nc-nd/3.0/`](https://creativecommons.org/licenses/by-nc-nd/3.0/)

or send a letter to Creative Commons, 171 Second Street, Suite 300, San
Francisco, California, 94105, USA.

One specific exception to the "No Derivative Works" portion of the
license is as follows: this guide may be freely translated into any
language, provided the translation is accurate, and the guide is
reprinted in its entirety. The same license restrictions apply to the
translation as to the original guide. The translation may also include
the name and contact information for the translator.

The C source code presented in this document is hereby granted to the
public domain, and is completely free of any license restriction.

Educators are freely encouraged to recommend or supply copies of this
guide to their students.

Unless otherwise mutually agreed by the parties in writing, the author
offers the work as-is and makes no representations or warranties of any
kind concerning the work, express, implied, statutory or otherwise,
including, without limitation, warranties of title, merchantability,
fitness for a particular purpose, noninfringement, or the absence of
latent or other defects, accuracy, or the presence of absence of errors,
whether or not discoverable.

Except to the extent required by applicable law, in no event will the
author be liable to you on any legal theory for any special, incidental,
consequential, punitive or exemplary damages arising out of the use of
the work, even if the author has been advised of the possibility of such
damages.

Contact [`beej@beej.us`](mailto:beej@beej.us) for more information.

## 헌사

이 안내서를 쓸 수 있도록 저를 과거에 도와주신, 그리고 미래에 도와주실 모든 분들에게
감사합니다. 이 안내서를 만들기 위해 사용한 자유 소프트웨어와 패키지(GNU, Linux,
Slackware, vim, Python, Inkscape, pandoc, 기타 등등...)를 만든 모든
분들에게 감사드립니다. 또한 이 안내서의 발전을 위한 제안을 해주시고 응원의 말씀을
보내주신 수천명의 사람들에게 감사드립니다.

이 안내서를 컴퓨터 세계에서 나의 가장 위대한 영웅이자 영감을 주는 이들에게 바칩니다.
Donald Knuth, Bruce Schneier, W. Richard Stevens, Steve The Woz Wozniak,
독자 여러분, 그리고 모든 자유 및 공개 소프트웨어 커뮤니티

## 출판 정보

이 책은 GNU 도구를 적재한 Arch Linux장치에서 Vim 편집기를 사용해서 Markdown
으로 작성되었습니다. 표지 "미술"과 다이어그램은 Inkscape로 작성되었습니다.
Markdown은 Python과 Pandoc, XeLaTex를 통해 HTML과 Latex/PDF로 변환되었습니다.
문서에는 Liberation 폰트를 사용했습니다. 툴체인은 전적으로 자유/공개 소프트웨어를
사용해서 구성했습니다.

## 옮긴이의 말

이 문서의 첫 한국어 번역 버전은 박성호(tempter@fourthline.com)님이 1998/08/20(yyyy/MM/d)에 인터넷의 어딘가에 게시하신 것으로 보입니다. 현재는 KLDP에 있습니다.

이 문서의 두 번째 한국어 번역 버전은 김수봉(연락처 없음)님이 2003/12/15(yyyy/MM/d)에 KLDP에 게시하셨습니다.
첫 번역 문서를 html에서 wiki형태로 변환했다고 적혀 있습니다.

지금 읽고 계시는 세 번째 버전은 정민석(javalia.javalia@gmail.com)이 작업했으며, 2023년 5월 28일부터 작업했습니다.

이 문서의 번역본은 1998년에 최초로 한국어 버전이 작성된 이래로 한국인이 Socket 프로그래밍에 대해서 참고할 수 있는 문서 중 리눅스 man page를 제외하면
가장 1차적인 문서였습니다. 실제로 많은 소켓 프로그래밍 관련 글과 출판된 책의 예제 코드도 이 문서의 것을 차용하고 있습니다.
원문은 세월이 흐르면서 조금씩 개정되어 내용이 상세해지고 IPv6에 대해서도 다루고 있으나 번역본은 20년 전의 모습으로 멈추어 있었습니다.
소켓 프로그래밍을 공부하던 시절 가장 큰 도움을 받은 문서의 번역본이 개정되지 않음을 안타깝게 생각해서 이렇게 새로운 번역본을 만들게 되었습니다.
이 글이 새로운 네트워크 라이브러리를 개발하는 프로그래머에게 도움이 되기를 바랍니다.

이 문서는 원문의 3.1.5 버전을 기반으로 번역되었습니다. 원문에 등장하는 고유명사는 해당 명사에 통용되는 한국어 번역이 없는 한 원어 그대로 실었습니다.
영어 일반명사는 음역을 기본으로 하였으나, 일부는 의역하기도 하였고 혼동을 줄이기 위해서 병기한 부분도 있습니다.
작업 과정에서 이전 번역자들의 원문을 유지하지는 못했습니다. 원문/번역본/새 번역본 사이의 대조/교정 작업은 개인적인 시간을 짜내서 진행하는 이 일에는
너무 큰 작업이었습니다. 이러한 사정으로 인해 이전 번역자들의 작업이 직접적으로 유지되지는 못하지만, 다른 프로그래머들을 위해 수 십년 전 글을 남기신
번역자 분들의 노력을 이어받고 그 작업을 존중하는 마음으로 다음 몇 년간 사람들이 읽을 수 있는 번역을 제공하기 위해 노력했습니다.

읽어주셔서 감사합니다.
