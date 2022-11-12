# Common Questions

**Where can I get those header files?**

[ix[header files]] If you don't have them on your system already, you
probably don't need them. Check the manual for your particular platform.
If you're building for [ix[Windows]] Windows, you only need to `#include
<winsock.h>`.

**What do I do when `bind()` reports [ix[Address already in use]]
"Address already in use"?**

You have to use [ixtt[setsockopt()]] `setsockopt()` with the
[ixtt[SO\_REUSEADDR]] `SO_REUSEADDR` option on the listening socket.
Check out the [ixtt[bind()]] [section on `bind()`](#bind) and the
[ixtt[select()]] [section on `select()`](#select) for an example.

**How do I get a list of open sockets on the system?**

Use the [ix[netstat]] `netstat`. Check the `man` page for full details,
but you should get some good output just typing:

```
$ netstat
```

The only trick is determining which socket is associated with which
program.  `:-)`

**How can I view the routing table?**

Run the [ix[route]] `route` command (in `/sbin` on most Linuxes) or the
command [ix[netstat]] `netstat -r`.

**How can I run the client and server programs if I only have one
computer?  Don't I need a network to write network programs?**

Fortunately for you, virtually all machines implement a [ix[loopback
device]] loopback network "device" that sits in the kernel and pretends
to be a network card. (This is the interface listed as "`lo`" in the
routing table.)

Pretend you're logged into a machine named [ix[goat]] "`goat`". Run the
client in one window and the server in another. Or start the server in
the background ("`server &`") and run the client in the same window. The
upshot of the loopback device is that you can either `client goat` or
[ix[localhost]] `client localhost` (since "`localhost`" is likely
defined in your `/etc/hosts` file) and you'll have the client talking to
the server without a network!

In short, no changes are necessary to any of the code to make it run on
a single non-networked machine! Huzzah!

**How can I tell if the remote side has closed connection?**

You can tell because `recv()` will return `0`.

**How do I implement a [ixtt[ping]] "ping" utility? What is [ixtt[ICMP]]
ICMP?  Where can I find out more about [ix[raw sockets]] raw sockets and
`SOCK_RAW`?**

All your raw sockets questions will be answered in [W. Richard Stevens'
UNIX Network Programming books](#books). Also, look in the `ping/`
subdirectory in Stevens' UNIX Network Programming source code,
[fl[available online|http://www.unpbook.com/src.html]].

**How do I change or shorten the timeout on a call to `connect()`?**

Instead of giving you exactly the same answer that W. Richard Stevens
would give you, I'll just refer you to [fl[`lib/connect_nonb.c` in the
UNIX Network Programming source code|http://www.unpbook.com/src.html]].

The gist of it is that you make a socket descriptor with `socket()`,
[set it to non-blocking](#blocking), call `connect()`, and if all goes
well `connect()` will return `-1` immediately and `errno` will be set to
`EINPROGRESS`. Then you call [`select()`](#select) with whatever timeout
you want, passing the socket descriptor in both the read and write sets.
If it doesn't timeout, it means the `connect()` call completed. At this
point, you'll have to use `getsockopt()` with the `SO_ERROR` option to
get the return value from the `connect()` call, which should be zero if
there was no error.

Finally, you'll probably want to set the socket back to be blocking
again before you start transferring data over it.

Notice that this has the added benefit of allowing your program to do
something else while it's connecting, too. You could, for example, set
the timeout to something low, like 500 ms, and update an indicator
onscreen each timeout, then call `select()` again. When you've called
`select()` and timed-out, say, 20 times, you'll know it's time to give
up on the connection.

Like I said, check out Stevens' source for a perfectly excellent
example.

**How do I build for Windows?**

First, delete Windows and install Linux or BSD. `};-)`. No, actually,
just see the [section on building for Windows](#windows) in the
introduction.

**How do I build for Solaris/SunOS? I keep getting linker errors when I
try to compile!**

The linker errors happen because Sun boxes don't automatically compile
in the socket libraries. See the [section on building for
Solaris/SunOS](#solaris) in the introduction for an example of how to do
this.

**Why does `select()` keep falling out on a signal?**

Signals tend to cause blocked system calls to return `-1` with `errno`
set to `EINTR`. When you set up a signal handler with
[ixtt[sigaction()]] `sigaction()`, you can set the flag
[ixtt[SA\_RESTART]] `SA_RESTART`, which is supposed to restart the
system call after it was interrupted.

Naturally, this doesn't always work.

My favorite solution to this involves a [ix[goto]] `goto` statement. You
know this irritates your professors to no end, so go for it!

```{.c .numberLines}
select_restart:
if ((err = select(fdmax+1, &readfds, NULL, NULL, NULL)) == -1) {
    if (errno == EINTR) {
        // some signal just interrupted us, so restart
        goto select_restart;
    }
    // handle the real error here:
    perror("select");
} 
```

Sure, you don't _need_ to use `goto` in this case; you can use other
structures to control it. But I think the `goto` statement is actually
cleaner.

**How can I implement a timeout on a call to `recv()`?**

[ix[recv()@\texttt{recv()}!timeout]] Use [ixtt[select()]]
[`select()`](#select)! It allows you to specify a timeout parameter for
socket descriptors that you're looking to read from. Or, you could wrap
the entire functionality in a single function, like this:

```{.c .numberLines}
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

int recvtimeout(int s, char *buf, int len, int timeout)
{
    fd_set fds;
    int n;
    struct timeval tv;

    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(s, &fds);

    // set up the struct timeval for the timeout
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    // wait until timeout or data received
    n = select(s+1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error

    // data must be here, so do a normal recv()
    return recv(s, buf, len, 0);
}
.
.
.
// Sample call to recvtimeout():
n = recvtimeout(s, buf, sizeof buf, 10); // 10 second timeout

if (n == -1) {
    // error occurred
    perror("recvtimeout");
}
else if (n == -2) {
    // timeout occurred
} else {
    // got some data in buf
}
.
.
. 
```

Notice that [ixtt[recvtimeout()]] `recvtimeout()` returns `-2` in case
of a timeout. Why not return `0`? Well, if you recall, a return value of
`0` on a call to `recv()` means that the remote side closed the
connection. So that return value is already spoken for, and `-1` means
"error", so I chose `-2` as my timeout indicator.

**How do I [ix[encryption]] encrypt or compress the data before sending
it through the socket?**

One easy way to do encryption is to use [ix[SSL]] SSL (secure sockets
layer), but that's beyond the scope of this guide. [ix[OpenSSL]] (Check
out the [fl[OpenSSL project|https://www.openssl.org/]] for more info.)

But assuming you want to plug in or implement your own [ix[compression]]
compressor or encryption system, it's just a matter of thinking of your
data as running through a sequence of steps between both ends. Each step
changes the data in some way.

1. server reads data from file (or wherever)
2. server encrypts/compresses data  (you add this part)
3. server `send()`s encrypted data

Now the other way around:

1. client `recv()`s encrypted data
2. client decrypts/decompresses data  (you add this part)
3. client writes data to file (or wherever)

If you're going to compress and encrypt, just remember to compress
first. `:-)`

Just as long as the client properly undoes what the server does, the
data will be fine in the end no matter how many intermediate steps you
add.

So all you need to do to use my code is to find the place between where
the data is read and the data is sent (using `send()`) over the network,
and stick some code in there that does the encryption.

**What is this "`PF_INET`" I keep seeing? Is it related to `AF_INET`?**

[ixtt[PF\_INET]] [ixtt[AF\_INET]]

Yes, yes it is. See [the section on `socket()`](#socket) for details.

**How can I write a server that accepts shell commands from a client and
executes them?**

For simplicity, lets say the client `connect()`s, `send()`s, and
`close()`s the connection (that is, there are no subsequent system calls
without the client connecting again).

The process the client follows is this:

1. `connect()` to server
2. `send("/sbin/ls > /tmp/client.out")`
3. `close()` the connection

Meanwhile, the server is handling the data and executing it:

1. `accept()` the connection from the client
2. `recv(str)` the command string
3. `close()` the connection
4. `system(str)` to run the command

[ix[security]] _Beware!_  Having the server execute what the client says
is like giving remote shell access and people can do things to your
account when they connect to the server. For instance, in the above
example, what if the client sends "`rm -rf ~`"? It deletes everything in
your account, that's what!

So you get wise, and you prevent the client from using any except for a
couple utilities that you know are safe, like the `foobar` utility:

```{.c}
if (!strncmp(str, "foobar", 6)) {
    sprintf(sysstr, "%s > /tmp/server.out", str);
    system(sysstr);
} 
```

But you're still unsafe, unfortunately: what if the client enters
"`foobar; rm -rf ~`"? The safest thing to do is to write a little
routine that puts an escape ("`\`") character in front of all
non-alphanumeric characters (including spaces, if appropriate) in the
arguments for the command.

As you can see, security is a pretty big issue when the server starts
executing things the client sends.

**I'm sending a slew of data, but when I `recv()`, it only receives 536
bytes or 1460 bytes at a time. But if I run it on my local machine, it
receives all the data at the same time. What's going on?**

You're hitting the [ix[MTU]] MTU---the maximum size the physical medium
can handle. On the local machine, you're using the loopback device which
can handle 8K or more no problem. But on Ethernet, which can only handle
1500 bytes with a header, you hit that limit. Over a modem, with 576 MTU
(again, with header), you hit the even lower limit.

You have to make sure all the data is being sent, first of all. (See the
[`sendall()`](#sendall) function implementation for details.) Once
you're sure of that, then you need to call `recv()` in a loop until all
your data is read.

Read the section [Son of Data Encapsulation](#sonofdataencap) for
details on receiving complete packets of data using multiple calls to
`recv()`.

**I'm on a Windows box and I don't have the `fork()` system call or any
kind of `struct sigaction`. What to do?**

[ixtt[fork()]] If they're anywhere, they'll be in POSIX libraries that
may have shipped with your compiler. Since I don't have a Windows box, I
really can't tell you the answer, but I seem to remember that Microsoft
has a POSIX compatibility layer and that's where `fork()` would be. (And
maybe even `sigaction`.)

Search the help that came with VC++ for "fork" or "POSIX" and see if it
gives you any clues.

If that doesn't work at all, ditch the `fork()`/`sigaction` stuff and
replace it with the Win32 equivalent: [ixtt[CreateProcess()]]
`CreateProcess()`. I don't know how to use `CreateProcess()`---it takes
a bazillion arguments, but it should be covered in the docs that came
with VC++.

**[ix[firewall]] I'm behind a firewall---how do I let people outside the
firewall know my IP address so they can connect to my machine?**

Unfortunately, the purpose of a firewall is to prevent people outside
the firewall from connecting to machines inside the firewall, so
allowing them to do so is basically considered a breach of security.

This isn't to say that all is lost. For one thing, you can still often
`connect()` through the firewall if it's doing some kind of masquerading
or NAT or something like that. Just design your programs so that you're
always the one initiating the connection, and you'll be fine.

[ix[firewall!poking holes in]] If that's not satisfactory, you can ask
your sysadmins to poke a hole in the firewall so that people can connect
to you. The firewall can forward to you either through it's NAT
software, or through a proxy or something like that.

Be aware that a hole in the firewall is nothing to be taken lightly. You
have to make sure you don't give bad people access to the internal
network; if you're a beginner, it's a lot harder to make software secure
than you might imagine.

Don't make your sysadmin mad at me. `;-)`

**[ix[packet sniffer]] [ix[promiscuous mode]] How do I write a packet
sniffer? How do I put my Ethernet interface into promiscuous mode?**

For those not in the know, when a network card is in "promiscuous mode",
it will forward ALL packets to the operating system, not just those that
were addressed to this particular machine. (We're talking Ethernet-layer
addresses here, not IP addresses--but since ethernet is lower-layer than
IP, all IP addresses are effectively forwarded as well. See the section
[Low Level Nonsense and Network Theory](#lowlevel) for more info.)

This is the basis for how a packet sniffer works. It puts the interface
into promiscuous mode, then the OS gets every single packet that goes by
on the wire.  You'll have a socket of some type that you can read this
data from.

Unfortunately, the answer to the question varies depending on the
platform, but if you Google for, for instance, "windows promiscuous
[ixtt[ioctl()]] ioctl" you'll probably get somewhere.  For Linux,
there's what looks like a [fl[useful Stack Overflow
thread|https://stackoverflow.com/questions/21323023/]], as well.

**How can I set a custom [ix[timeout, setting]] timeout value for a TCP
or UDP socket?**

It depends on your system. You might search the net for
[ixtt[SO\_RCVTIMEO]] `SO_RCVTIMEO` and [ixtt[SO\_SNDTIMEO]]
`SO_SNDTIMEO` (for use with [ixtt[setsockopt()]] `setsockopt()`) to see
if your system supports such functionality.

The Linux man page suggests using `alarm()` or `setitimer()` as a
substitute.

**How can I tell which ports are available to use? Is there a list of
"official" port numbers?**

Usually this isn't an issue. If you're writing, say, a web server, then
it's a good idea to use the well-known port 80 for your software. If
you're writing just your own specialized server, then choose a port at
random (but greater than 1023) and give it a try.

If the port is already in use, you'll get an "Address already in use"
error when you try to `bind()`. Choose another port. (It's a good idea
to allow the user of your software to specify an alternate port either
with a config file or a command line switch.)

There is a [fl[list of official port
numbers|https://www.iana.org/assignments/port-numbers]] maintained by
the Internet Assigned Numbers Authority (IANA). Just because something
(over 1023) is in that list doesn't mean you can't use the port. For
instance, Id Software's DOOM uses the same port as "mdqs", whatever that
is. All that matters is that no one else _on the same machine_ is using
that port when you want to use it.
