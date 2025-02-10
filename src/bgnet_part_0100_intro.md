# Intro
<!--
Beej's Guide to Network Programming book source

# vim: ts=4:sw=4:nosi:et:tw=72
-->

<!--
	History:

	2.3.2:		socket man page
	2.3.3:		sockaddr_in man page
	2.3.4:		bind, listen man page
	2.3.5:		connect man page
	2.3.6:		listen, perror man page
	2.3.7:		errno man page
	2.3.8:		htonl etc man page
	2.3.9:		close man page, expanded man page leader
	2.3.10:		inet_ntoa, setsockopt man pages
	2.3.11:		getpeername man page
	2.3.12:		send/sendto man pages
	2.3.13:		shutdown man pages
	2.3.14:		gethostname man pages, fix inet_aton links
	2.3.15:		fcntl man page
	2.3.16:		recv/recvfrom man page
	2.3.17:		gethostbyname/gethostbyaddr man page
	2.3.18:		changed GET / to GET / HTTP/1.0
	2.3.19:		added select() man page
	2.3.20:		added poll() man page
	2.3.21:		section on NAT and reserved networks
	2.3.22:		typo fixes in sects "man" and "privnet"
	2.3.23:		added broadcast packets section
	2.3.24:		manpage prototype changed to code, subtitle moved out of title
	2.4.0:		big overhaul, serialization stuff
	2.4.1:		minor text changes in intro
	2.4.2:		changed all sizeofs to use variable names instead of types
	2.4.3:		fix myaddr->my_addr in listener.c, sockaddr_inman example
	2.4.4:		fix myaddr->my_addr in server.c
	2.4.5:		fix 14->18 in son of data encap
	3.0.0:		IPv6 overhaul
	3.0.1:		sa-to-sa6 typo fix
	3.0.2:		typo fixes
	3.0.3:		typo fixes
	3.0.4:		cut-n-paste errors, selectserver hints fix
	3.0.5:		typo fixes
	3.0.6:		typo fixes
	3.0.7:		typo fixes, added front matter
	3.0.8:		getpeername() code fixes
	3.0.9:		getpeername() code fixes, this time fer sure
	3.0.10:		bind() man page code fix, comment changes
	3.0.11:		socket syscall section code fix, comment changes
	3.0.12:		typos in "IP Addresses, structs, and Data Munging"
	3.0.13:		amp removals, note about errno and multithreading
	3.0.14:		type changes to listener.c, pack2.c
	3.0.15:		fix inet_pton example
	3.0.16:		fix simple server output, optlen in getsockopt man page
	3.0.17:		fix small typo
	3.0.18:		reverse perror and close calls in getaddrinfo
	3.0.19:		add notes about O_NONBLOCK with select() under Linux
	3.0.20:		fix missing .fd in poll() example
	3.0.21:		change sizeof(int) to sizeof yes
    3.0.22:     C99 updates, bug fixes, markdown
    3.0.23:     Book reference and URL updates
    3.1.0:      Section on poll()
    3.1.1:      Add WSL note, telnot
    3.1.2:      pollserver.c bugfix
    3.1.3:      Fix freeaddrinfo memleak
    3.1.4:      Fix accept example header files
    3.1.5:      Fix dgram AF_UNSPEC
-->

<!-- prevent hyphenation of the following words: -->
[nh[strtol]]
[nh[sprintf]]
[nh[accept]]
[nh[bind]]
[nh[connect]]
[nh[close]]
[nh[getaddrinfo]]
[nh[freeaddrinfo]]
<!--
Don't know how to make this work with underscores. I love
you, Knuth, but... daaahm.

[nh[gai_strerr]]
-->
[nh[gethostname]]
[nh[gethostbyname]]
[nh[gethostbyaddr]]
[nh[getnameinfo]]
[nh[getpeername]]
[nh[errno]]
[nh[fcntl]]
[nh[htons]]
[nh[htonl]]
[nh[ntohs]]
[nh[ntohl]]
<!--
[nh[inet_ntoa]]
[nh[inet_aton]]
[nh[inet_addr]]
[nh[inet_ntop]]
[nh[inet_pton]]
-->
[nh[listen]]
[nh[perror]]
[nh[strerror]]
[nh[poll]]
[nh[recv]]
[nh[recvfrom]]
[nh[select]]
[nh[setsockopt]]
[nh[getsockopt]]
[nh[send]]
[nh[sendto]]
[nh[shutdown]]
[nh[socket]]
[nh[struct]]
[nh[sockaddr]]
<!--
[nh[sockaddr_in]]
[nh[in_addr]]
[nh[sockaddr_in6]]
[nh[in6_addr]]
-->
[nh[hostent]]
[nh[addrinfo]]
[nh[closesocket]]

Hey! Socket programming got you down? Is this stuff just a little too
difficult to figure out from the `man` pages? You want to do cool
Internet programming, but you don't have time to wade through a gob of
`struct`s trying to figure out if you have to call `bind()` before you
`connect()`, etc., etc.

Well, guess what! I've already done this nasty business, and I'm dying
to share the information with everyone! You've come to the right place.
This document should give the average competent C programmer the edge
s/he needs to get a grip on this networking noise.

And check it out: I've finally caught up with the future (just in the
nick of time, too!) and have updated the Guide for IPv6! Enjoy!


## Audience

This document has been written as a tutorial, not a complete reference.
It is probably at its best when read by individuals who are just
starting out with socket programming and are looking for a foothold. It
is certainly not the _complete and total_ guide to sockets programming,
by any means.

Hopefully, though, it'll be just enough for those man pages to start
making sense... `:-)`


## Platform and Compiler

The code contained within this document was compiled on a Linux PC using
Gnu's [i[Compilers-->GCC]] `gcc` compiler. It should, however, build on
just about any platform that uses `gcc`. Naturally, this doesn't apply
if you're programming for Windows---see the [section on Windows
programming](#windows), below.


## Official Homepage and Books For Sale

This official location of this document is:

* [`https://beej.us/guide/bgnet/`](https://beej.us/guide/bgnet/)

There you will also find example code and translations of the guide into
various languages.

To buy nicely bound print copies (some call them "books"), visit:

* [`https://beej.us/guide/url/bgbuy`](https://beej.us/guide/url/bgbuy)

I'll appreciate the purchase because it helps sustain my
document-writing lifestyle!


## Note for Solaris/SunOS/illumos Programmers {#solaris}

When compiling for a [i[Solaris]] Solaris variant or [i[SunOS]] SunOS,
you need to specify some extra command-line switches for linking in the
proper libraries. In order to do this, simply add "`-lnsl -lsocket
-lresolv`" to the end of the compile command, like so:

```
$ cc -o server server.c -lnsl -lsocket -lresolv
```

If you still get errors, you could try further adding a `-lxnet` to the
end of that command line. I don't know what that does, exactly, but some
people seem to need it.

Another place that you might find problems is in the call to
`setsockopt()`. The prototype differs from that on my Linux box, so
instead of:

```{.c}
int yes=1;
```

enter this:

```{.c}
char yes='1';
```

As I don't have a Sun box, I haven't tested any of the above
information---it's just what people have told me through email.


## Note for Windows Programmers {#windows}

At this point in the guide, historically, I've done a bit of bagging on
[i[Windows]] Windows, simply due to the fact that I don't like it very
much. But then Windows and Microsoft (as a company) got a lot better.
Windows 9 and 10 coupled with WSL (below) actually were decent operating
systems. Not really a lot to complain about.

Well, a little—for example, I'm writing this (in 2025) on a 2015 laptop
that used to run Windows 10. Eventually it got too slow and I installed
Linux on it. And have been using it ever since.

And now the news is out that Windows 11 will require beefier hardware
than Windows 10. I'm not a fan of that. The OS should be as unobtrusive
as possible and not require you to spend more money. The extra CPU power
should be for apps, not the OS!

So I still encourage you to try [i[Linux]]
[fl[Linux|https://www.linux.com/]], [i[BSD]] [fl[BSD|https://bsd.org/]],
[i[illumos]] [fl[illumos|https://www.illumos.org/]] or some other flavor
of Unix, instead.

How'd that soapbox get there?

But people like what they like, and you Windows folk will be pleased to
know that this information is generally applicable to Windows, with a
few minor changes.

Oh, hey, the soapbox is back!

One thing that you should strongly consider is [i[WSL]] [i[Windows
Subsystem For Linux]] the [fl[Windows Subsystem for
Linux|https://learn.microsoft.com/en-us/windows/wsl/]]. This basically
allows you to install a Linux VM-ish thing on Windows 10. That will also
definitely get you situated, and you'll be able to build and run these
programs as is.

Another thing you can do is install [i[Cygwin]]
[fl[Cygwin|https://cygwin.com/]], which is a collection of Unix tools
for Windows. I've heard on the grapevine that doing so allows all these
programs to compile unmodified, but I've never tried it.

Some of you might want to do things the Pure Windows Way. That's very
gutsy of you, and this is what you have to do: run out and get Unix
immediately! No, no---I'm kidding. I'm supposed to be
Windows-friendly(er) these days...

Okay, okay. I'll get on with it.

[i[Winsock]]

This is what you'll have to do: first, ignore pretty much all of the
system header files I mention in here. Instead, include:

```{.c}
#include <winsock2.h>
#include <ws2tcpip.h>
```

`winsock2` is the "new" (circa 1994) version of the Windows socket
library.

Unfortunately, if you include `windows.h`, it automatically pulls in
the older `winsock.h` (version 1) header file which conflicts with
`winsock2.h`! Fun times.

So if you have to include `windows.h`, you need to define a macro to get
it to _not_ include the older header:

```{.c}
#define WIN32_LEAN_AND_MEAN  // Say this...

#include <windows.h>         // And now we can include that.
#include <winsock2.h>        // And this.
```

Wait! You also have to make a call to [i[`WSAStartup()` function]]
`WSAStartup()` before doing anything else with the sockets library. You
pass in the Winsock version you desire to this function (e.g. version
2.2). And then you can check the result to make sure that version is
available.

The code to do that looks something like this:

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
        fprintf(stderr,"Version 2.2 of Winsock not available.\n");
        WSACleanup();
        exit(2);
    }
```

Note that call to [i[`WSACleanup()` function]] `WSACleanup()` in there.
That's what you want to call when you're done with the Winsock library.

You also have to tell your compiler to link in the Winsock library,
called `ws2_32.lib` for Winsock 2. Under VC++, this can be done through
the `Project` menu, under `Settings...`. Click the `Link` tab, and look
for the box titled "Object/library modules". Add "ws2_32.lib" (or
whichever lib is your preference) to that list.

Or so I hear.

Once you do that, the rest of the examples in this tutorial should
generally apply, with a few exceptions. For one thing, you can't use
`close()` to close a socket---you need to use [i[`closesocket()`
function]] `closesocket()`, instead. Also, [i[`select()` function]]
`select()` only works with socket descriptors, not file descriptors
(like `0` for `stdin`).

There is also a socket class that you can use, [i[`CSocket` class]]
[`CSocket`](https://learn.microsoft.com/en-us/cpp/mfc/reference/csocket-class?view=msvc-170)
Check your compiler's help pages for more information.

To get more information about Winsock, [check out the official page at
Microsoft](https://learn.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2).

Finally, I hear that Windows has no [i[`fork()` function]] `fork()`
system call which is, unfortunately, used in some of my examples. Maybe
you have to link in a POSIX library or something to get it to work, or
you can use [i[`CreateProcess()` function]] `CreateProcess()` instead.
`fork()` takes no arguments, and `CreateProcess()` takes about 48
billion arguments. If you're not up to that, the [i[`CreateThread()`
function]] `CreateThread()` is a little easier to digest...unfortunately
a discussion about multithreading is beyond the scope of this document.
I can only talk about so much, you know!

Extra finally, Steven Mitchell has [fl[ported a number of the
examples|https://www.tallyhawk.net/WinsockExamples/]] to Winsock. Check
that stuff out.


## Email Policy

I'm generally available to help out with [i[Emailing Beej]] email
questions so feel free to write in, but I can't guarantee a response. I
lead a pretty busy life and there are times when I just can't answer a
question you have. When that's the case, I usually just delete the
message. It's nothing personal; I just won't ever have the time to give
the detailed answer you require.

As a rule, the more complex the question, the less likely I am to
respond. If you can narrow down your question before mailing it and be
sure to include any pertinent information (like platform, compiler,
error messages you're getting, and anything else you think might help me
troubleshoot), you're much more likely to get a response. For more
pointers, read ESR's document, [fl[How To Ask Questions The Smart
Way|http://www.catb.org/~esr/faqs/smart-questions.html]].

If you don't get a response, hack on it some more, try to find the
answer, and if it's still elusive, then write me again with the
information you've found and hopefully it will be enough for me to help
out.

Now that I've badgered you about how to write and not write me, I'd just
like to let you know that I _fully_ appreciate all the praise the guide
has received over the years. It's a real morale boost, and it gladdens
me to hear that it is being used for good! `:-)` Thank you!


## Mirroring

[i[Mirroring the Guide]] You are more than welcome to mirror this site,
whether publicly or privately. If you publicly mirror the site and want
me to link to it from the main page, drop me a line at
[`beej@beej.us`](beej@beej.us).


## Note for Translators

[i[Translating the Guide]] If you want to translate the guide into
another language, write me at [`beej@beej.us`](beej@beej.us) and I'll
link to your translation from the main page. Feel free to add your name
and contact info to the translation.

This source markdown document uses UTF-8 encoding.

Please note the license restrictions in the [Copyright, Distribution,
and Legal](#legal) section, below.

If you want me to host the translation, just ask. I'll also link to it
if you want to host it; either way is fine.


## Copyright, Distribution, and Legal {#legal}

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


## Dedication

Thanks to everyone who has helped in the past and future with me getting
this guide written. And thank you to all the people who produce the Free
software and packages that I use to make the Guide: GNU, Linux,
Slackware, vim, Python, Inkscape, pandoc, many others. And finally a big
thank-you to the literally thousands of you who have written in with
suggestions for improvements and words of encouragement.

I dedicate this guide to some of my biggest heroes and inpirators in the
world of computers: Donald Knuth, Bruce Schneier, W. Richard Stevens,
and The Woz, my Readership, and the entire Free and Open Source Software
Community.


## Publishing Information

This book is written in Markdown using the vim editor on an Arch Linux
box loaded with GNU tools. The cover "art" and diagrams are produced
with Inkscape.  The Markdown is converted to HTML and LaTex/PDF by
Python, Pandoc and XeLaTeX, using Liberation fonts. The toolchain is
composed of 100% Free and Open Source Software.
