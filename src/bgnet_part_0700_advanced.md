# Slightly Advanced Techniques

These aren't _really_ advanced, but they're getting out of the more
basic levels we've already covered. In fact, if you've gotten this far,
you should consider yourself fairly accomplished in the basics of Unix
network programming!  Congratulations!

So here we go into the brave new world of some of the more esoteric
things you might want to learn about sockets. Have at it!


## Blocking {#blocking}

[i[Blocking]<]

Blocking. You've heard about it---now what the heck is it? In a
nutshell, "block" is techie jargon for "sleep". You probably noticed
that when you run `listener`, above, it just sits there until a packet
arrives. What happened is that it called `recvfrom()`, there was no
data, and so `recvfrom()` is said to "block" (that is, sleep there)
until some data arrives.

Lots of functions block. `accept()` blocks. All the `recv()` functions
block.  The reason they can do this is because they're allowed to. When
you first create the socket descriptor with `socket()`, the kernel sets
it to blocking.  [i[Non-blocking sockets]] If you don't want a socket to
be blocking, you have to make a call to [i[`fcntl()` function]]
`fcntl()`:

```{.c .numberLines}
#include <unistd.h>
#include <fcntl.h>
.
.
.
sockfd = socket(PF_INET, SOCK_STREAM, 0);
fcntl(sockfd, F_SETFL, O_NONBLOCK);
.
.
. 
```

By setting a socket to non-blocking, you can effectively "poll" the
socket for information. If you try to read from a non-blocking socket
and there's no data there, it's not allowed to block---it will return
`-1` and `errno` will be set to [i[`EAGAIN` macro]] `EAGAIN` or
[i[`EWOULDBLOCK` macro]] `EWOULDBLOCK`.

(Wait---it can return [i[`EAGAIN` macro]] `EAGAIN` _or_
[i[`EWOULDBLOCK` macro]] `EWOULDBLOCK`? Which do you check for?  The
specification doesn't actually specify which your system will return, so
for portability, check them both.)

Generally speaking, however, this type of polling is a bad idea. If you
put your program in a busy-wait looking for data on the socket, you'll
suck up CPU time like it was going out of style. A more elegant solution
for checking to see if there's data waiting to be read comes in the
following section on [i[`poll()` function]] `poll()`.

[i[Blocking]>]

## `poll()`---Synchronous I/O Multiplexing {#poll}

[i[poll()]<]

What you really want to be able to do is somehow monitor a _bunch_ of
sockets at once and then handle the ones that have data ready. This way
you don't have to continuously poll all those sockets to see which are
ready to read.

> _A word of warning: `poll()` is horribly slow when it comes to giant
> numbers of connections. In those circumstances, you'll get better
> performance out of an event library such as
> [fl[libevent|https://libevent.org/]] that attempts to use the fastest
> possible method availabile on your system._

So how can you avoid polling? Not slightly ironically, you can avoid
polling by using the `poll()` system call. In a nutshell, we're going to
ask the operating system to do all the dirty work for us, and just let
us know when some data is ready to read on which sockets. In the
meantime, our process can go to sleep, saving system resources.

The general gameplan is to keep an array of `struct pollfd`s with
information about which socket descriptors we want to monitor, and what
kind of events we want to monitor for. The OS will block on the `poll()`
call until one of those events occurs (e.g. "socket ready to read!") or
until a user-specified timeout occurs.

Usefully, a `listen()`ing socket will return "ready to read" when a new
incoming connection is ready to be `accept()`ed.

That's enough banter. How do we use this?

``` {.c}
#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

`fds` is our array of information (which sockets to monitor for what),
`nfds` is the count of elements in the array, and `timeout` is a timeout
in milliseconds. It returns the number of elements in the array that
have had an event occur.

Let's have a look at that `struct`:

[i[`struct pollfd` type]]

``` {.c}
struct pollfd {
    int fd;         // the socket descriptor
    short events;   // bitmap of events we're interested in
    short revents;  // on return, bitmap of events that occurred
};
```

So we're going to have an array of those, and we'll set the `fd` field
for each element to a socket descriptor we're interested in monitoring.
And then we'll set the `events` field to indicate the type of events
we're interested in.

The `events` field is the bitwise-OR of the following:

| Macro     | Description                                                  |
|-----------|--------------------------------------------------------------|
| `POLLIN`  | Alert me when data is ready to `recv()` on this socket.      |
| `POLLOUT` | Alert me when I can `send()` data to this socket without blocking.|
| `POLLHUP` | Alert me when the remote closed the connection.|

Once you have your array of `struct pollfd`s in order, then you can pass
it to `poll()`, also passing the size of the array, as well as a timeout
value in milliseconds. (You can specify a negative timeout to wait
forever.)

After `poll()` returns, you can check the `revents` field to see if
`POLLIN` or `POLLOUT` is set, indicating that event occurred.

(There's actually more that you can do with the `poll()` call. See the
[`poll()` man page, below](#pollman), for more details.)

Here's [flx[an example|poll.c]] where we'll wait 2.5 seconds for data to
be ready to read from standard input, i.e. when you hit `RETURN`:

``` {.c .numberLines}
#include <stdio.h>
#include <poll.h>

int main(void)
{
    struct pollfd pfds[1]; // More if you want to monitor more

    pfds[0].fd = 0;          // Standard input
    pfds[0].events = POLLIN; // Tell me when ready to read

    // If you needed to monitor other things, as well:
    //pfds[1].fd = some_socket; // Some socket descriptor
    //pfds[1].events = POLLIN;  // Tell me when ready to read

    printf("Hit RETURN or wait 2.5 seconds for timeout\n");

    int num_events = poll(pfds, 1, 2500); // 2.5 second timeout

    if (num_events == 0) {
        printf("Poll timed out!\n");
    } else {
        int pollin_happened = pfds[0].revents & POLLIN;

        if (pollin_happened) {
            printf("File descriptor %d is ready to read\n",
                    pfds[0].fd);
        } else {
            printf("Unexpected event occurred: %d\n",
                    pfds[0].revents);
        }
    }

    return 0;
}
```

Notice again that `poll()` returns the number of elements in the `pfds`
array for which events have occurred. It doesn't tell you _which_
elements in the array (you still have to scan for that), but it does
tell you how many entries have a non-zero `revents` field (so you can
stop scanning after you find that many).

A couple questions might come up here: how to add new file descriptors
to the set I pass to `poll()`? For this, simply make sure you have
enough space in the array for all you need, or `realloc()` more space as
needed.

What about deleting items from the set? For this, you can copy the last
element in the array over-top the one you're deleting. And then pass in
one fewer as the count to `poll()`. Another option is that you can set
any `fd` field to a negative number and `poll()` will ignore it.

How can we put it all together into a chat server that you can `telnet`
to?

What we'll do is start a listener socket, and add it to the set of file
descriptors to `poll()`. (It will show ready-to-read when there's an
incoming connection.)

Then we'll add new connections to our `struct pollfd` array. And we'll
grow it dynamically if we run out of space.

When a connection is closed, we'll remove it from the array.

And when a connection is ready-to-read, we'll read the data from it and
send that data to all the other connections so they can see what the
other users typed.

So give [flx[this poll server|pollserver.c]] a try. Run it in one
window, then `telnet localhost 9034` from a number of other terminal
windows. You should be able to see what you type in one window in the
other ones (after you hit RETURN).

Not only that, but if you hit `CTRL-]` and type `quit` to exit `telnet`,
the server should detect the disconnection and remove you from the array
of file descriptors.

``` {.c .numberLines}
/*
** pollserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define PORT "9034"   // Port we're listening on

/*
 * Convert socket to IP address string.
 * addr: struct sockaddr_in or struct sockaddr_in6
 */
const char *inet_ntop2(void *addr, char *buf, size_t size)
{
    struct sockaddr_storage *sas = addr;
    struct sockaddr_in *sa4;
    struct sockaddr_in6 *sa6;
    void *src;

    switch (sas->ss_family) {
        case AF_INET:
            sa4 = addr;
            src = &(sa4->sin_addr);
            break;
        case AF_INET6:
            sa6 = addr;
            src = &(sa6->sin6_addr);
            break;
        default:
            return NULL;
    }

    return inet_ntop(sas->ss_family, src, buf, size);
}

/*
 * Return a listening socket.
 */
int get_listener_socket(void)
{
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    freeaddrinfo(ai); // All done with this

    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

/*
 * Add a new file descriptor to the set.
 */
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count,
        int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size) {
        *fd_size *= 2; // Double it
        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}

/*
 * Remove a file descriptor at a given index from the set.
 */
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}

/*
 * Handle incoming connections.
 */
void handle_new_connection(int listener, int *fd_count,
        int *fd_size, struct pollfd **pfds)
{
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    int newfd;  // Newly accept()ed socket descriptor
    char remoteIP[INET6_ADDRSTRLEN];

    addrlen = sizeof remoteaddr;
    newfd = accept(listener, (struct sockaddr *)&remoteaddr,
            &addrlen);

    if (newfd == -1) {
        perror("accept");
    } else {
        add_to_pfds(pfds, newfd, fd_count, fd_size);

        printf("pollserver: new connection from %s on socket %d\n",
                inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
                newfd);
    }
}

/*
 * Handle regular client data or client hangups.
 */
void handle_client_data(int listener, int *fd_count,
        struct pollfd *pfds, int *pfd_i)
{
    char buf[256];    // Buffer for client data

    int nbytes = recv(pfds[*pfd_i].fd, buf, sizeof buf, 0);

    int sender_fd = pfds[*pfd_i].fd;

    if (nbytes <= 0) { // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("pollserver: socket %d hung up\n", sender_fd);
        } else {
            perror("recv");
        }

        close(pfds[*pfd_i].fd); // Bye!

        del_from_pfds(pfds, *pfd_i, fd_count);

        // reexamine the slot we just deleted
        (*pfd_i)--;

    } else { // We got some good data from a client
        printf("pollserver: recv from fd %d: %.*s", sender_fd,
                nbytes, buf);
        // Send to everyone!
        for(int j = 0; j < *fd_count; j++) {
            int dest_fd = pfds[j].fd;

            // Except the listener and ourselves
            if (dest_fd != listener && dest_fd != sender_fd) {
                if (send(dest_fd, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    }
}

/*
 * Process all existing connections.
 */
void process_connections(int listener, int *fd_count, int *fd_size,
        struct pollfd **pfds)
{
    for(int i = 0; i < *fd_count; i++) {

        // Check if someone's ready to read
        if ((*pfds)[i].revents & (POLLIN | POLLHUP)) {
            // We got one!!

            if ((*pfds)[i].fd == listener) {
                // If we're the listener, it's a new connection
                handle_new_connection(listener, fd_count, fd_size,
                        pfds);
            } else {
                // Otherwise we're just a regular client
                handle_client_data(listener, fd_count, *pfds, &i);
            }
        }
    }
}

/*
 * Main: create a listener and connection set, loop forever
 * processing connections.
 */
int main(void)
{
    int listener;     // Listening socket descriptor

    // Start off with room for 5 connections
    // (We'll realloc as necessary)
    int fd_size = 5;
    int fd_count = 0;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    // Set up and get a listening socket
    listener = get_listener_socket();

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    // Add the listener to set;
    // Report ready to read on incoming connection
    pfds[0].fd = listener;
    pfds[0].events = POLLIN;

    fd_count = 1; // For the listener

    puts("pollserver: waiting for connections...");

    // Main loop
    for(;;) {
        int poll_count = poll(pfds, fd_count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        // Run through connections looking for data to read
        process_connections(listener, &fd_count, &fd_size, &pfds);
    }

    free(pfds);
}
```

In the next section, we'll look at a similar, older function called
`select()`. Both `select()` and `poll()` offer similar functionality and
performance, and only really differ in how they're used. `select()`
might be slightly more portable, but is perhaps a little clunkier in
use. Choose the one you like the best, as long as it's supported on your
system.

[i[poll()]>]


## `select()`---Synchronous I/O Multiplexing, Old School {#select}

[i[`select()` function]<]

This function is somewhat strange, but it's very useful. Take the
following situation: you are a server and you want to listen for
incoming connections as well as keep reading from the connections you
already have.

No problem, you say, just an `accept()` and a couple of `recv()`s. Not
so fast, buster! What if you're blocking on an `accept()` call? How are
you going to `recv()` data at the same time? "Use non-blocking sockets!"
No way! You don't want to be a CPU hog. What, then?

`select()` gives you the power to monitor several sockets at the same
time.  It'll tell you which ones are ready for reading, which are ready
for writing, and which sockets have raised exceptions, if you really
want to know that.

> _A word of warning: `select()`, though very portable, is terribly slow
> when it comes to giant numbers of connections. In those circumstances,
> you'll get better performance out of an event library such as
> [fl[libevent|https://libevent.org/]] that attempts to use the fastest
> possible method availabile on your system._

Without any further ado, I'll offer the synopsis of `select()`:

```{.c}
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int select(int numfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout); 
```

The function monitors "sets" of file descriptors; in particular
`readfds`, `writefds`, and `exceptfds`. If you want to see if you can
read from standard input and some socket descriptor, `sockfd`, just add
the file descriptors `0` and `sockfd` to the set `readfds`. The
parameter `numfds` should be set to the values of the highest file
descriptor plus one. In this example, it should be set to `sockfd+1`,
since it is assuredly higher than standard input (`0`).

When `select()` returns, `readfds` will be modified to reflect which of
the file descriptors you selected which is ready for reading. You can
test them with the macro `FD_ISSET()`, below.

Before progressing much further, I'll talk about how to manipulate these
sets.  Each set is of the type `fd_set`. The following macros operate on
this type:

| Function                         | Description                          |
|----------------------------------|--------------------------------------|
| [i[`FD_SET()` macro]]`FD_SET(int fd, fd_set *set);`   | Add `fd` to the `set`.               |
| [i[`FD_CLR()` macro]]`FD_CLR(int fd, fd_set *set);`   | Remove `fd` from the `set`.          |
| [i[`FD_ISSET()` macro]]`FD_ISSET(int fd, fd_set *set);` | Return true if `fd` is in the `set`. |
| [i[`FD_ZERO()` macro]]`FD_ZERO(fd_set *set);`          | Clear all entries from the `set`.    |

[i[`struct timeval` type]<]

Finally, what is this weirded-out  `struct timeval`? Well, sometimes you
don't want to wait forever for someone to send you some data. Maybe
every 96 seconds you want to print "Still Going..." to the terminal even
though nothing has happened. This time structure allows you to specify a
timeout period. If the time is exceeded and `select()` still hasn't
found any ready file descriptors, it'll return so you can continue
processing.

The `struct timeval` has the follow fields:

```{.c}
struct timeval {
    int tv_sec;     // seconds
    int tv_usec;    // microseconds
}; 
```

Just set `tv_sec` to the number of seconds to wait, and set `tv_usec` to
the number of microseconds to wait. Yes, that's _micro_seconds, not
milliseconds.  There are 1,000 microseconds in a millisecond, and 1,000
milliseconds in a second. Thus, there are 1,000,000 microseconds in a
second. Why is it "usec"?  The "u" is supposed to look like the Greek
letter μ (Mu) that we use for "micro". Also, when the function returns,
`timeout` _might_ be updated to show the time still remaining. This
depends on what flavor of Unix you're running.

Yay! We have a microsecond resolution timer! Well, don't count on it.
You'll probably have to wait some part of your standard Unix timeslice
no matter how small you set your `struct timeval`.

Other things of interest:  If you set the fields in your `struct
timeval` to `0`, `select()` will timeout immediately, effectively
polling all the file descriptors in your sets. If you set the parameter
`timeout` to NULL, it will never timeout, and will wait until the first
file descriptor is ready. Finally, if you don't care about waiting for a
certain set, you can just set it to NULL in the call to `select()`.

[flx[The following code snippet|select.c]] waits 2.5 seconds for
something to appear on standard input:

```{.c .numberLines}
/*
** select.c -- a select() demo
*/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define STDIN 0  // file descriptor for standard input

int main(void)
{
    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = 2;
    tv.tv_usec = 500000;

    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);

    // don't care about writefds and exceptfds:
    select(STDIN+1, &readfds, NULL, NULL, &tv);

    if (FD_ISSET(STDIN, &readfds))
        printf("A key was pressed!\n");
    else
        printf("Timed out.\n");

    return 0;
} 
```

If you're on a line buffered terminal, the key you hit should be RETURN
or it will time out anyway.

Now, some of you might think this is a great way to wait for data on a
datagram socket---and you are right: it _might_ be. Some Unices can use
select in this manner, and some can't. You should see what your local
man page says on the matter if you want to attempt it.

Some Unices update the time in your `struct timeval` to reflect the
amount of time still remaining before a timeout. But others do not.
Don't rely on that occurring if you want to be portable. (Use
[i[`gettimeofday()` function]] `gettimeofday()` if you need to track time
elapsed. It's a bummer, I know, but that's the way it is.)

[i[`struct timeval` type]>]

What happens if a socket in the read set closes the connection? Well, in
that case, `select()` returns with that socket descriptor set as "ready
to read".  When you actually do `recv()` from it, `recv()` will return
`0`. That's how you know the client has closed the connection.

One more note of interest about `select()`: if you have a socket that is
[i[`select()` function-->with `listen()`]]
[i[`listen()` function-->with `select()`]]
`listen()`ing, you can check to see if there is a new connection by
putting that socket's file descriptor in the `readfds` set.

And that, my friends, is a quick overview of the almighty `select()`
function.

But, by popular demand, here is an in-depth example. Unfortunately, the
difference between the dirt-simple example, above, and this one here is
significant. But have a look, then read the description that follows it.

[flx[This program|selectserver.c]] acts like a simple multi-user chat
server. Start it running in one window, then `telnet` to it ("`telnet
hostname 9034`") from multiple other windows. When you type something in
one `telnet` session, it should appear in all the others.

```{.c .numberLines}
/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9034"   // port we're listening on

/*
 * Convert socket to IP address string.
 * addr: struct sockaddr_in or struct sockaddr_in6
 */
const char *inet_ntop2(void *addr, char *buf, size_t size)
{
    struct sockaddr_storage *sas = addr;
    struct sockaddr_in *sa4;
    struct sockaddr_in6 *sa6;
    void *src;

    switch (sas->ss_family) {
        case AF_INET:
            sa4 = addr;
            src = &(sa4->sin_addr);
            break;
        case AF_INET6:
            sa6 = addr;
            src = &(sa6->sin6_addr);
            break;
        default:
            return NULL;
    }

    return inet_ntop(sas->ss_family, src, buf, size);
}

/*
 * Return a listening socket
 */
int get_listener_socket(void)
{
    struct addrinfo hints, *ai, *p;
    int yes=1;    // for setsockopt() SO_REUSEADDR, below
    int rv;
    int listener;

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    return listener;
}

/*
 * Add new incoming connections to the proper sets
 */
void handle_new_connection(int listener, fd_set *master, int *fdmax)
{
    socklen_t addrlen;
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    char remoteIP[INET6_ADDRSTRLEN];

    addrlen = sizeof remoteaddr;
    newfd = accept(listener,
        (struct sockaddr *)&remoteaddr,
        &addrlen);

    if (newfd == -1) {
        perror("accept");
    } else {
        FD_SET(newfd, master); // add to master set
        if (newfd > *fdmax) {  // keep track of the max
            *fdmax = newfd;
        }
        printf("selectserver: new connection from %s on "
            "socket %d\n",
            inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
            newfd);
    }
}

/*
 * Broadcast a message to all clients
 */
void broadcast(char *buf, int nbytes, int listener, int s,
               fd_set *master, int fdmax)
{
    for(int j = 0; j <= fdmax; j++) {
        // send to everyone!
        if (FD_ISSET(j, master)) {
            // except the listener and ourselves
            if (j != listener && j != s) {
                if (send(j, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    }
}

/*
 * Handle client data and hangups
 */
void handle_client_data(int s, int listener, fd_set *master,
                        int fdmax)
{
    char buf[256];    // buffer for client data
    int nbytes;

    // handle data from a client
    if ((nbytes = recv(s, buf, sizeof buf, 0)) <= 0) {
        // got error or connection closed by client
        if (nbytes == 0) {
            // connection closed
            printf("selectserver: socket %d hung up\n", s);
        } else {
            perror("recv");
        }
        close(s); // bye!
        FD_CLR(s, master); // remove from master set
    } else {
        // we got some data from a client
        broadcast(buf, nbytes, listener, s, master, fdmax);
    }
}

/*
 * Main
 */
int main(void)
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    listener = get_listener_socket();

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data
        // to read
        for(int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener)
                    handle_new_connection(i, &master, &fdmax);
                else
                    handle_client_data(i, listener, &master, fdmax);
            }
        }
    }

    return 0;
}
```

Notice I have two file descriptor sets in the code: `master` and
`read_fds`. The first, `master`, holds all the socket descriptors that
are currently connected, as well as the socket descriptor that is
listening for new connections.

The reason I have the `master` set is that `select()` actually _changes_
the set you pass into it to reflect which sockets are ready to read.
Since I have to keep track of the connections from one call of
`select()` to the next, I must store these safely away somewhere. At the
last minute, I copy the `master` into the `read_fds`, and then call
`select()`.

But doesn't this mean that every time I get a new connection, I have to
add it to the `master` set? Yup! And every time a connection closes, I
have to remove it from the `master` set? Yes, it does.

Notice I check to see when the `listener` socket is ready to read. When
it is, it means I have a new connection pending, and I `accept()` it and
add it to the `master` set. Similarly, when a client connection is ready
to read, and `recv()` returns `0`, I know the client has closed the
connection, and I must remove it from the `master` set.

If the client `recv()` returns non-zero, though, I know some data has
been received. So I get it, and then go through the `master` list and
send that data to all the rest of the connected clients.

And that, my friends, is a less-than-simple overview of the almighty
`select()` function.

Quick note to all you Linux fans out there: sometimes, in rare
circumstances, Linux's `select()` can return "ready-to-read" and then
not actually be ready to read! This means it will block on the `read()`
after the `select()` says it won't! Why you little---! Anyway, the
workaround solution is to set the [i[`O_NONBLOCK` macro]] `O_NONBLOCK`
flag on the receiving socket so it errors with `EWOULDBLOCK` (which you
can just safely ignore if it occurs). See the [`fcntl()` reference
page](#fcntlman) for more info on setting a socket to non-blocking.

In addition, here is a bonus afterthought: there is another function
called [i[`poll()` function]] `poll()` which behaves much the same way
`select()` does, but with a different system for managing the file
descriptor sets. [Check it out!](#pollman)

[i[`select()` function]>]

## Handling Partial `send()`s {#sendall}

Remember back in the [section about `send()`](#sendrecv), above, when I
said that `send()` might not send all the bytes you asked it to? That
is, you want it to send 512 bytes, but it returns 412. What happened to
the remaining 100 bytes?

Well, they're still in your little buffer waiting to be sent out. Due to
circumstances beyond your control, the kernel decided not to send all
the data out in one chunk, and now, my friend, it's up to you to get the
data out there.

[i[`sendall()` function]<]
You could write a function like this to do it, too:

```{.c .numberLines}
#include <sys/types.h>
#include <sys/socket.h>

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 
```

In this example, `s` is the socket you want to send the data to, `buf`
is the buffer containing the data, and `len` is a pointer to an `int`
containing the number of bytes in the buffer.

The function returns `-1` on error (and `errno` is still set from the
call to `send()`). Also, the number of bytes actually sent is returned
in `len`. This will be the same number of bytes you asked it to send,
unless there was an error. `sendall()` will do its best, huffing and
puffing, to send the data out, but if there's an error, it gets back to
you right away.

For completeness, here's a sample call to the function:

```{.c .numberLines}
char buf[10] = "Beej!";
int len;

len = strlen(buf);
if (sendall(s, buf, &len) == -1) {
    perror("sendall");
    printf("We only sent %d bytes because of the error!\n", len);
} 
```

[i[`sendall()` function]>]

What happens on the receiver's end when part of a packet arrives? If the
packets are variable length, how does the receiver know when one packet
ends and another begins? Yes, real-world scenarios are a royal pain in
the [i[Donkeys]] donkeys. You probably have to [i[Data encapsulation]]
_encapsulate_ (remember that from the [data encapsulation
section](#lowlevel) way back there at the beginning?)  Read on for
details!


## Serialization---How to Pack Data {#serialization}

[i[Serialization]<]

It's easy enough to send text data across the network, you're finding,
but what happens if you want to send some "binary" data like `int`s or
`float`s? It turns out you have a few options.

1. Convert the number into text with a function like `sprintf()`, then
   send the text. The receiver will parse the text back into a number
   using a function like `strtol()`.

2. Just send the data raw, passing a pointer to the data to `send()`.

3. Encode the number into a portable binary form. The receiver will
   decode it.

Sneak preview! Tonight only!

[_Curtain raises_]

Beej says, "I prefer Method Three, above!"

[_THE END_]

(Before I begin this section in earnest, I should tell you that there
are libraries out there for doing this, and rolling your own and
remaining portable and error-free is quite a challenge. So hunt around
and do your homework before deciding to implement this stuff yourself. I
include the information here for those curious about how things like
this work.)

Actually all the methods, above, have their drawbacks and advantages,
but, like I said, in general, I prefer the third method. First, though,
let's talk about some of the drawbacks and advantages to the other two.

The first method, encoding the numbers as text before sending, has the
advantage that you can easily print and read the data that's coming over
the wire.  Sometimes a human-readable protocol is excellent to use in a
non-bandwidth-intensive situation, such as with [i[IRC]] [fl[Internet
Relay Chat (IRC)|https://en.wikipedia.org/wiki/Internet_Relay_Chat]].
However, it has the disadvantage that it is slow to convert, and the
results almost always take up more space than the original number!

Method two: passing the raw data. This one is quite easy (but
dangerous!): just take a pointer to the data to send, and call send with
it.

```{.c}
double d = 3490.15926535;

send(s, &d, sizeof d, 0);  /* DANGER--non-portable! */
```

The receiver gets it like this:

```{.c}
double d;

recv(s, &d, sizeof d, 0);  /* DANGER--non-portable! */
```

Fast, simple---what's not to like? Well, it turns out that not all
architectures represent a `double` (or `int` for that matter) with the
same bit representation or even the same byte ordering! The code is
decidedly non-portable. (Hey---maybe you don't need portability, in
which case this is nice and fast.)

When packing integer types, we've already seen how the [i[`htons()`
function]] `htons()`-class of functions can help keep things portable by
transforming the numbers into [i[Byte ordering]] Network Byte Order, and
how that's the Right Thing to do. Unfortunately, there are no similar
functions for `float` types. Is all hope lost?

Fear not! (Were you afraid there for a second? No? Not even a little
bit?) There is something we can do: we can pack (or "marshal", or
"serialize", or one of a thousand million other names) the data into a
known binary format that the receiver can unpack on the remote side.

What do I mean by "known binary format"? Well, we've already seen the
`htons()` example, right? It changes (or "encodes", if you want to think
of it that way) a number from whatever the host format is into Network
Byte Order. To reverse (unencode) the number, the receiver calls
`ntohs()`.

But didn't I just get finished saying there wasn't any such function for
other non-integer types? Yes. I did. And since there's no standard way
in C to do this, it's a bit of a pickle (that a gratuitous pun there for
you Python fans).

The thing to do is to pack the data into a known format and send that
over the wire for decoding. For example, to pack `float`s, here's
[flx[something quick and dirty with plenty of room for
improvement|pack.c]]:

```{.c .numberLines}
#include <stdint.h>

uint32_t htonf(float f)
{
    uint32_t p;
    uint32_t sign;

    if (f < 0) { sign = 1; f = -f; }
    else { sign = 0; }
        
    // whole part and sign
    p = ((((uint32_t)f)&0x7fff)<<16) | (sign<<31);

    // fraction
    p |= (uint32_t)(((f - (int)f) * 65536.0f))&0xffff;

    return p;
}

float ntohf(uint32_t p)
{
    float f = ((p>>16)&0x7fff); // whole part
    f += (p&0xffff) / 65536.0f; // fraction

    if (((p>>31)&0x1) == 0x1) { f = -f; } // sign bit set

    return f;
}
```

The above code is sort of a naive implementation that stores a `float`
in a 32-bit number. The high bit (31) is used to store the sign of the
number ("1" means negative), and the next seven bits (30-16) are used to
store the whole number portion of the `float`. Finally, the remaining
bits (15-0) are used to store the fractional portion of the number.

Usage is fairly straightforward:

```{.c .numberLines}
#include <stdio.h>

int main(void)
{
    float f = 3.1415926, f2;
    uint32_t netf;

    netf = htonf(f);  // convert to "network" form
    f2 = ntohf(netf); // convert back to test

    printf("Original: %f\n", f);        // 3.141593
    printf(" Network: 0x%08X\n", netf); // 0x0003243F
    printf("Unpacked: %f\n", f2);       // 3.141586

    return 0;
}
```

On the plus side, it's small, simple, and fast. On the minus side, it's
not an efficient use of space and the range is severely restricted---try
storing a number greater-than 32767 in there and it won't be very happy!
You can also see in the above example that the last couple decimal
places are not correctly preserved.

What can we do instead? Well, _The_ Standard for storing floating point
numbers is known as [i[IEEE-754]]
[fl[IEEE-754|https://en.wikipedia.org/wiki/IEEE_754]].  Most computers
use this format internally for doing floating point math, so in those
cases, strictly speaking, conversion wouldn't need to be done. But if
you want your source code to be portable, that's an assumption you can't
necessarily make.

Or can you? Very probably your systems are IEEE-754, just like they're
probably 2's complement for integers. So if you know that's what you
have, you can just pass the data over the wire (though you need to fix the
endianness with `htonl()` or the appropriate function—`float`s have
endianness, too). And this is what `htons()` and its ilk do on
big-endian systems where no conversion is necessary.

But just in case you are on a system that isn't IEEE-754, [flx[here's
some code that encodes `float`s and `double`s into IEEE-754
format|ieee754.c]].  (Mostly---it doesn't encode NaN or Infinity, but it
could be modified to do that.)

```{.c .numberLines}
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;

    // -1 for sign bit
    unsigned significandbits = bits - expbits - 1;

    if (f == 0.0) return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;
    long long shift;
    unsigned bias;

    // -1 for sign bit
    unsigned significandbits = bits - expbits - 1;

    if (i == 0) return 0.0;

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}
```

I put some handy macros up there at the top for packing and unpacking
32-bit (probably a `float`) and 64-bit (probably a `double`) numbers,
but the `pack754()` function could be called directly and told to encode
`bits`-worth of data (`expbits` of which are reserved for the normalized
number's exponent).

Here's sample usage:

```{.c .numberLines}

#include <stdio.h>
#include <stdint.h> // defines uintN_t types
#include <inttypes.h> // defines PRIx macros

int main(void)
{
    float f = 3.1415926, f2;
    double d = 3.14159265358979323, d2;
    uint32_t fi;
    uint64_t di;

    fi = pack754_32(f);
    f2 = unpack754_32(fi);

    di = pack754_64(d);
    d2 = unpack754_64(di);

    printf("float before : %.7f\n", f);
    printf("float encoded: 0x%08" PRIx32 "\n", fi);
    printf("float after  : %.7f\n\n", f2);

    printf("double before : %.20lf\n", d);
    printf("double encoded: 0x%016" PRIx64 "\n", di);
    printf("double after  : %.20lf\n", d2);

    return 0;
}
```


The above code produces this output:

```
float before : 3.1415925
float encoded: 0x40490FDA
float after  : 3.1415925

double before : 3.14159265358979311600
double encoded: 0x400921FB54442D18
double after  : 3.14159265358979311600
```

Another question you might have is how do you pack `struct`s?
Unfortunately for you, the compiler is free to put padding all over the
place in a `struct`, and that means you can't portably send the whole
thing over the wire in one chunk.  (Aren't you getting sick of hearing
"can't do this", "can't do that"? Sorry! To quote a friend, "Whenever
anything goes wrong, I always blame Microsoft."  This one might not be
Microsoft's fault, admittedly, but my friend's statement is completely
true.)

Back to it: the best way to send the `struct` over the wire is to pack
each field independently and then unpack them into the `struct` when
they arrive on the other side.

That's a lot of work, is what you're thinking. Yes, it is. One thing you
can do is write a helper function to help pack the data for you. It'll
be fun! Really!

In the book [flr[_The Practice of Programming_|tpop]] by Kernighan and
Pike, they implement `printf()`-like functions called `pack()` and
`unpack()` that do exactly this. I'd link to them, but apparently those
functions aren't online with the rest of the source from the book.

(_The Practice of Programming_ is an excellent read. Zeus saves a kitten
every time I recommend it.)

At this point, I'm going to drop a pointer to a [fl[Protocol Buffers
implementation in C|https://github.com/protobuf-c/protobuf-c]] which
I've never used, but looks completely respectable. Python and Perl
programmers will want to check out their language's `pack()` and
`unpack()` functions for accomplishing the same thing. And Java has a
big-ol' Serializable interface that can be used in a similar way.

But if you want to write your own packing utility in C, K&P's trick is
to use variable argument lists to make `printf()`-like functions to
build the packets.  [flx[Here's a version I cooked up|pack2.c]] on my
own based on that which hopefully will be enough to give you an idea of
how such a thing can work.

(This code references the `pack754()` functions, above. The `packi*()`
functions operate like the familiar `htons()` family, except they pack
into a `char` array instead of another integer.)

```{.c .numberLines}
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

/*
** packi16() -- store a 16-bit int into a char buffer (like htons())
*/
void packi16(unsigned char *buf, unsigned int i)
{
    *buf++ = i>>8; *buf++ = i;
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/
void packi32(unsigned char *buf, unsigned long int i)
{
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

/*
** packi64() -- store a 64-bit int into a char buffer (like htonl())
*/
void packi64(unsigned char *buf, unsigned long long int i)
{
    *buf++ = i>>56; *buf++ = i>>48;
    *buf++ = i>>40; *buf++ = i>>32;
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like
**                ntohs())
*/
int unpacki16(unsigned char *buf)
{
    unsigned int i2 = ((unsigned int)buf[0]<<8) | buf[1];
    int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffu) { i = i2; }
    else { i = -1 - (unsigned int)(0xffffu - i2); }

    return i;
}

/*
** unpacku16() -- unpack a 16-bit unsigned from a char buffer (like
**                ntohs())
*/
unsigned int unpacku16(unsigned char *buf)
{
    return ((unsigned int)buf[0]<<8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like
**                ntohl())
*/
long int unpacki32(unsigned char *buf)
{
    unsigned long int i2 = ((unsigned long int)buf[0]<<24) |
                           ((unsigned long int)buf[1]<<16) |
                           ((unsigned long int)buf[2]<<8)  |
                           buf[3];
    long int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffu) { i = i2; }
    else { i = -1 - (long int)(0xffffffffu - i2); }

    return i;
}

/*
** unpacku32() -- unpack a 32-bit unsigned from a char buffer (like
**                ntohl())
*/
unsigned long int unpacku32(unsigned char *buf)
{
    return ((unsigned long int)buf[0]<<24) |
           ((unsigned long int)buf[1]<<16) |
           ((unsigned long int)buf[2]<<8)  |
           buf[3];
}

/*
** unpacki64() -- unpack a 64-bit int from a char buffer (like
**                ntohl())
*/
long long int unpacki64(unsigned char *buf)
{
    unsigned long long int i2 =
        ((unsigned long long int)buf[0]<<56) |
        ((unsigned long long int)buf[1]<<48) |
        ((unsigned long long int)buf[2]<<40) |
        ((unsigned long long int)buf[3]<<32) |
        ((unsigned long long int)buf[4]<<24) |
        ((unsigned long long int)buf[5]<<16) |
        ((unsigned long long int)buf[6]<<8)  |
        buf[7];
    long long int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffffffffffu) { i = i2; }
    else { i = -1 -(long long int)(0xffffffffffffffffu - i2); }

    return i;
}

/*
** unpacku64() -- unpack a 64-bit unsigned from a char buffer (like
**                ntohl())
*/
unsigned long long int unpacku64(unsigned char *buf)
{
    return ((unsigned long long int)buf[0]<<56) |
           ((unsigned long long int)buf[1]<<48) |
           ((unsigned long long int)buf[2]<<40) |
           ((unsigned long long int)buf[3]<<32) |
           ((unsigned long long int)buf[4]<<24) |
           ((unsigned long long int)buf[5]<<16) |
           ((unsigned long long int)buf[6]<<8)  |
           buf[7];
}

/*
** pack() -- store data dictated by the format string in the buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (16-bit unsigned length is automatically prepended to strings)
*/

unsigned int pack(unsigned char *buf, char *format, ...)
{
    va_list ap;

    signed char c;              // 8-bit
    unsigned char C;

    int h;                      // 16-bit
    unsigned int H;

    long int l;                 // 32-bit
    unsigned long int L;

    long long int q;            // 64-bit
    unsigned long long int Q;

    float f;                    // floats
    double d;
    long double g;
    unsigned long long int fhold;

    char *s;                    // strings
    unsigned int len;

    unsigned int size = 0;

    va_start(ap, format);

    for(; *format != '\0'; format++) {
        switch(*format) {
        case 'c': // 8-bit
            size += 1;
            c = (signed char)va_arg(ap, int); // promoted
            *buf++ = c;
            break;

        case 'C': // 8-bit unsigned
            size += 1;
            C = (unsigned char)va_arg(ap, unsigned int); // promoted
            *buf++ = C;
            break;

        case 'h': // 16-bit
            size += 2;
            h = va_arg(ap, int);
            packi16(buf, h);
            buf += 2;
            break;

        case 'H': // 16-bit unsigned
            size += 2;
            H = va_arg(ap, unsigned int);
            packi16(buf, H);
            buf += 2;
            break;

        case 'l': // 32-bit
            size += 4;
            l = va_arg(ap, long int);
            packi32(buf, l);
            buf += 4;
            break;

        case 'L': // 32-bit unsigned
            size += 4;
            L = va_arg(ap, unsigned long int);
            packi32(buf, L);
            buf += 4;
            break;

        case 'q': // 64-bit
            size += 8;
            q = va_arg(ap, long long int);
            packi64(buf, q);
            buf += 8;
            break;

        case 'Q': // 64-bit unsigned
            size += 8;
            Q = va_arg(ap, unsigned long long int);
            packi64(buf, Q);
            buf += 8;
            break;

        case 'f': // float-16
            size += 2;
            f = (float)va_arg(ap, double); // promoted
            fhold = pack754_16(f); // convert to IEEE 754
            packi16(buf, fhold);
            buf += 2;
            break;

        case 'd': // float-32
            size += 4;
            d = va_arg(ap, double);
            fhold = pack754_32(d); // convert to IEEE 754
            packi32(buf, fhold);
            buf += 4;
            break;

        case 'g': // float-64
            size += 8;
            g = va_arg(ap, long double);
            fhold = pack754_64(g); // convert to IEEE 754
            packi64(buf, fhold);
            buf += 8;
            break;

        case 's': // string
            s = va_arg(ap, char*);
            len = strlen(s);
            size += len + 2;
            packi16(buf, len);
            buf += 2;
            memcpy(buf, s, len);
            buf += len;
            break;
        }
    }

    va_end(ap);

    return size;
}

/*
** unpack() -- unpack data dictated by the format string into the
**             buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (string is extracted based on its stored length, but 's' can be
**  prepended with a max length)
*/
void unpack(unsigned char *buf, char *format, ...)
{
    va_list ap;

    signed char *c;              // 8-bit
    unsigned char *C;

    int *h;                      // 16-bit
    unsigned int *H;

    long int *l;                 // 32-bit
    unsigned long int *L;

    long long int *q;            // 64-bit
    unsigned long long int *Q;

    float *f;                    // floats
    double *d;
    long double *g;
    unsigned long long int fhold;

    char *s;
    unsigned int len, maxstrlen=0, count;

    va_start(ap, format);

    for(; *format != '\0'; format++) {
        switch(*format) {
        case 'c': // 8-bit
            c = va_arg(ap, signed char*);
            if (*buf <= 0x7f) { *c = *buf;} // re-sign
            else { *c = -1 - (unsigned char)(0xffu - *buf); }
            buf++;
            break;

        case 'C': // 8-bit unsigned
            C = va_arg(ap, unsigned char*);
            *C = *buf++;
            break;

        case 'h': // 16-bit
            h = va_arg(ap, int*);
            *h = unpacki16(buf);
            buf += 2;
            break;

        case 'H': // 16-bit unsigned
            H = va_arg(ap, unsigned int*);
            *H = unpacku16(buf);
            buf += 2;
            break;

        case 'l': // 32-bit
            l = va_arg(ap, long int*);
            *l = unpacki32(buf);
            buf += 4;
            break;

        case 'L': // 32-bit unsigned
            L = va_arg(ap, unsigned long int*);
            *L = unpacku32(buf);
            buf += 4;
            break;

        case 'q': // 64-bit
            q = va_arg(ap, long long int*);
            *q = unpacki64(buf);
            buf += 8;
            break;

        case 'Q': // 64-bit unsigned
            Q = va_arg(ap, unsigned long long int*);
            *Q = unpacku64(buf);
            buf += 8;
            break;

        case 'f': // float
            f = va_arg(ap, float*);
            fhold = unpacku16(buf);
            *f = unpack754_16(fhold);
            buf += 2;
            break;

        case 'd': // float-32
            d = va_arg(ap, double*);
            fhold = unpacku32(buf);
            *d = unpack754_32(fhold);
            buf += 4;
            break;

        case 'g': // float-64
            g = va_arg(ap, long double*);
            fhold = unpacku64(buf);
            *g = unpack754_64(fhold);
            buf += 8;
            break;

        case 's': // string
            s = va_arg(ap, char*);
            len = unpacku16(buf);
            buf += 2;
            if (maxstrlen > 0 && len > maxstrlen)
                count = maxstrlen - 1;
            else
                count = len;
            memcpy(s, buf, count);
            s[count] = '\0';
            buf += len;
            break;

        default:
            if (isdigit(*format)) { // track max str len
                maxstrlen = maxstrlen * 10 + (*format-'0');
            }
        }

        if (!isdigit(*format)) maxstrlen = 0;
    }

    va_end(ap);
}
```

And [flx[here is a demonstration program|pack2.c]] of the above code
that packs some data into `buf` and then unpacks it into variables. Note
that when calling `unpack()` with a string argument (format specifier
"`s`"), it's wise to put a maximum length count in front of it to
prevent a buffer overrun, e.g. "`96s`". Be wary when unpacking data you
get over the network---a malicious user might send badly-constructed
packets in an effort to attack your system!

```{.c .numberLines}
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

// If you have a C23 compiler
#if __STDC_VERSION__ >= 202311L
#include <stdfloat.h>
#else
// Otherwise let's define our own.
// Varies for different architectures! But you're probably:
typedef float float32_t;
typedef double float64_t;
#endif

int main(void)
{
    uint8_t buf[1024];
    int8_t magic;
    int16_t monkeycount;
    int32_t altitude;
    float32_t absurdityfactor;
    char *s = "Great unmitigated Zot!  You've found the Runestaff!";
    char s2[96];
    int16_t packetsize, ps2;

    packetsize = pack(buf, "chhlsf", (int8_t)'B', (int16_t)0,
            (int16_t)37, (int32_t)-5, s, (float32_t)-3490.6677);
    packi16(buf+1, packetsize); // store packet size for kicks

    printf("packet is %" PRId32 " bytes\n", packetsize);

    unpack(buf, "chhl96sf", &magic, &ps2, &monkeycount, &altitude,
            s2, &absurdityfactor);

    printf("'%c' %" PRId32" %" PRId16 " %" PRId32
            " \"%s\" %f\n", magic, ps2, monkeycount,
            altitude, s2, absurdityfactor);
}
```

Whether you roll your own code or use someone else's, it's a good idea
to have a general set of data packing routines for the sake of keeping
bugs in check, rather than packing each bit by hand each time.

When packing the data, what's a good format to use? Excellent question.
Fortunately, [i[XDR]] [flrfc[RFC 4506|4506]], the External Data
Representation Standard, already defines binary formats for a bunch of
different types, like floating point types, integer types, arrays, raw
data, etc. I suggest conforming to that if you're going to roll the data
yourself. But you're not obligated to. The Packet Police are not right
outside your door. At least, I don't _think_ they are.

In any case, encoding the data somehow or another before you send it is
the right way of doing things!

[i[Serialization]>]

## Son of Data Encapsulation {#sonofdataencap}

What does it really mean to encapsulate data, anyway? In the simplest
case, it means you'll stick a header on there with either some
identifying information or a packet length, or both.

What should your header look like? Well, it's just some binary data that
represents whatever you feel is necessary to complete your project.

Wow. That's vague.

Okay. For instance, let's say you have a multi-user chat program that
uses `SOCK_STREAM`s. When a user types ("says") something, two pieces of
information need to be transmitted to the server: what was said and who
said it.

So far so good? "What's the problem?" you're asking.

The problem is that the messages can be of varying lengths. One person
named "tom" might say, "Hi", and another person named "Benjamin" might
say, "Hey guys what is up?"

So you `send()` all this stuff to the clients as it comes in. Your
outgoing data stream looks like this:

```
t o m H i B e n j a m i n H e y g u y s w h a t i s u p ?
```

And so on. How does the client know when one message starts and another
stops?  You could, if you wanted, make all messages the same length and
just call the [i[`sendall()` function]] `sendall()` we implemented,
[above](#sendall). But that wastes bandwidth! We don't want to `send()`
1024 bytes just so "tom" can say "Hi".

So we _encapsulate_ the data in a tiny header and packet structure. Both
the client and server know how to pack and unpack (sometimes referred to
as "marshal" and "unmarshal") this data. Don't look now, but we're
starting to define a _protocol_ that describes how a client and server
communicate!

In this case, let's assume the user name is a fixed length of 8
characters, padded with `'\0'`. And then let's assume the data is
variable length, up to a maximum of 128 characters. Let's have a look a
sample packet structure that we might use in this situation:

1. `len` (1 byte, unsigned)---The total length of the packet, counting
    the 8-byte user name and chat data.

2. `name` (8 bytes)---The user's name, NUL-padded if necessary.

3. `chatdata` (_n_-bytes)---The data itself, no more than 128 bytes. The
   length of the packet should be calculated as the length of this data
   plus 8 (the length of the name field, above).

Why did I choose the 8-byte and 128-byte limits for the fields? I pulled
them out of the air, assuming they'd be long enough. Maybe, though, 8
bytes is too restrictive for your needs, and you can have a 30-byte name
field, or whatever.  The choice is up to you.

Using the above packet definition, the first packet would consist of the
following information (in hex and ASCII):

```
   0A     74 6F 6D 00 00 00 00 00      48 69
(length)  T  o  m    (padding)         H  i
```

And the second is similar:

```
   18     42 65 6E 6A 61 6D 69 6E      48 65 79 20 67 75 79 73 20 77 ...
(length)  B  e  n  j  a  m  i  n       H  e  y     g  u  y  s     w  ...
```

(The length is stored in Network Byte Order, of course. In this case,
it's only one byte so it doesn't matter, but generally speaking you'll
want all your binary integers to be stored in Network Byte Order in your
packets.)

When you're sending this data, you should be safe and use a command
similar to [`sendall()`](#sendall), above, so you know all the data is
sent, even if it takes multiple calls to `send()` to get it all out.

Likewise, when you're receiving this data, you need to do a bit of extra
work.  To be safe, you should assume that you might receive a partial
packet (like maybe we receive "`18 42 65 6E 6A`" from Benjamin, above,
but that's all we get in this call to `recv()`). We need to call
`recv()` over and over again until the packet is completely received.

But how? Well, we know the number of bytes we need to receive in total
for the packet to be complete, since that number is tacked on the front
of the packet.  We also know the maximum packet size is 1+8+128, or 137
bytes (because that's how we defined the packet).

There are actually a couple things you can do here. Since you know every
packet starts off with a length, you can call `recv()` just to get the
packet length.  Then once you have that, you can call it again
specifying exactly the remaining length of the packet (possibly
repeatedly to get all the data) until you have the complete packet. The
advantage of this method is that you only need a buffer large enough for
one packet, while the disadvantage is that you need to call `recv()` at
least twice to get all the data.

Another option is just to call `recv()` and say the amount you're
willing to receive is the maximum number of bytes in a packet. Then
whatever you get, stick it onto the back of a buffer, and finally check
to see if the packet is complete. Of course, you might get some of the
next packet, so you'll need to have room for that.

What you can do is declare an array big enough for two packets. This is
your work array where you will reconstruct packets as they arrive.

Every time you `recv()` data, you'll append it into the work buffer and
check to see if the packet is complete. That is, the number of bytes in
the buffer is greater than or equal to the length specified in the
header (+1, because the length in the header doesn't include the byte
for the length itself). If the number of bytes in the buffer is less
than 1, the packet is not complete, obviously. You have to make a
special case for this, though, since the first byte is garbage and you
can't rely on it for the correct packet length.

Once the packet is complete, you can do with it what you will. Use it,
and remove it from your work buffer.

Whew! Are you juggling that in your head yet? Well, here's the second of
the one-two punch: you might have read past the end of one packet and
onto the next in a single `recv()` call. That is, you have a work buffer
with one complete packet, and an incomplete part of the next packet!
Bloody heck. (But this is why you made your work buffer large enough to
hold _two_ packets---in case this happened!)

Since you know the length of the first packet from the header, and
you've been keeping track of the number of bytes in the work buffer, you
can subtract and calculate how many of the bytes in the work buffer
belong to the second (incomplete) packet. When you've handled the first
one, you can clear it out of the work buffer and move the partial second
packet down the to front of the buffer so it's all ready to go for the
next `recv()`.

(Some of you readers will note that actually moving the partial second
packet to the beginning of the work buffer takes time, and the program
can be coded to not require this by using a circular buffer.
Unfortunately for the rest of you, a discussion on circular buffers is
beyond the scope of this article. If you're still curious, grab a data
structures book and go from there.)

I never said it was easy. Ok, I did say it was easy. And it is; you just
need practice and pretty soon it'll come to you naturally. By
[i[Excalibur]] Excalibur I swear it!


## Broadcast Packets---Hello, World!

So far, this guide has talked about sending data from one host to one
other host. But it is possible, I insist, that you can, with the proper
authority, send data to multiple hosts _at the same time_!

With [i[UDP]] UDP (only UDP, not TCP) and standard IPv4, this is done
through a mechanism called [i[Broadcast]] _broadcasting_. With IPv6,
broadcasting isn't supported, and you have to resort to the often
superior technique of _multicasting_, which, sadly I won't be discussing
at this time. But enough of the starry-eyed future---we're stuck in the
32-bit present.

But wait! You can't just run off and start broadcasting willy-nilly; You
have to [i[`setsockopt()` function]] set the socket option
[i[`SO_BROADCAST` macro]] `SO_BROADCAST` before you can send a broadcast
packet out on the network. It's like a one of those little plastic
covers they put over the missile launch switch! That's just how much
power you hold in your hands!

But seriously, though, there is a danger to using broadcast packets, and
that is: every system that receives a broadcast packet must undo all the
onion-skin layers of data encapsulation until it finds out what port the
data is destined to. And then it hands the data over or discards it. In
either case, it's a lot of work for each machine that receives the
broadcast packet, and since it is all of them on the local network, that
could be a lot of machines doing a lot of unnecessary work. When the
game Doom first came out, this was a complaint about its network code.

Now, there is more than one way to skin a cat... wait a minute. Is there
really more than one way to skin a cat? What kind of expression is that?
Uh, and likewise, there is more than one way to send a broadcast packet.
So, to get to the meat and potatoes of the whole thing: how do you
specify the destination address for a broadcast message? There are two
common ways:

1. Send the data to a specific subnet's broadcast address. This is the
   subnet's network number with all one-bits set for the host portion of
   the address. For instance, at home my network is `192.168.1.0`, my
   netmask is `255.255.255.0`, so the last byte of the address is my
   host number (because the first three bytes, according to the netmask,
   are the network number). So my broadcast address is `192.168.1.255`.
   Under Unix, the `ifconfig` command will actually give you all this
   data. (If you're curious, the bitwise logic to get your broadcast
   address is `network_number` OR (NOT `netmask`).) You can send this
   type of broadcast packet to remote networks as well as your local
   network, but you run the risk of the packet being dropped by the
   destination's router.  (If they didn't drop it, then some random
   smurf could start flooding their LAN with broadcast traffic.)

2. Send the data to the "global" broadcast address. This is
   [i[`255.255.255.255`]] `255.255.255.255`, aka [i[`INADDR_BROADCAST`
   macro]] `INADDR_BROADCAST`. Many machines will automatically bitwise
   AND this with your network number to convert it to a network
   broadcast address, but some won't. It varies. Routers do not forward
   this type of broadcast packet off your local network, ironically
   enough.

So what happens if you try to send data on the broadcast address without
first setting the `SO_BROADCAST` socket option? Well, let's fire up good
old [`talker` and `listener`](#datagram) and see what happens.

```
$ talker 192.168.1.2 foo
sent 3 bytes to 192.168.1.2
$ talker 192.168.1.255 foo
sendto: Permission denied
$ talker 255.255.255.255 foo
sendto: Permission denied
```

Yes, it's not happy at all...because we didn't set the `SO_BROADCAST`
socket option. Do that, and now you can `sendto()` anywhere you want!

In fact, that's the _only difference_ between a UDP application that can
broadcast and one that can't. So let's take the old `talker` application
and add one section that sets the `SO_BROADCAST` socket option. We'll
call this program [flx[`broadcaster.c`|broadcaster.c]]:

```{.c .numberLines}
/*
** broadcaster.c -- a datagram "client" like talker.c, except
**                  this one can broadcast
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

#define SERVERPORT 4950    // the port users will be connecting to

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // connector's address info
    struct hostent *he;
    int numbytes;
    int broadcast = 1;
    //char broadcast = '1'; // if that doesn't work, try this

    if (argc != 3) {
        fprintf(stderr,"usage: broadcaster hostname message\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // this call is what allows broadcast packets to be sent:
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
        sizeof broadcast) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    their_addr.sin_family = AF_INET;     // host byte order
    their_addr.sin_port = htons(SERVERPORT); // network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
             (struct sockaddr *)&their_addr, sizeof their_addr);

    if (numbytes == -1) {
        perror("sendto");
        exit(1);
    }

    printf("sent %d bytes to %s\n", numbytes,
        inet_ntoa(their_addr.sin_addr));

    close(sockfd);

    return 0;
}
```

What's different between this and a "normal" UDP client/server
situation?  Nothing! (With the exception of the client being allowed to
send broadcast packets in this case.) As such, go ahead and run the old
UDP [`listener`](#datagram) program in one window, and `broadcaster` in
another. You should be now be able to do all those sends that failed,
above.

```
$ broadcaster 192.168.1.2 foo
sent 3 bytes to 192.168.1.2
$ broadcaster 192.168.1.255 foo
sent 3 bytes to 192.168.1.255
$ broadcaster 255.255.255.255 foo
sent 3 bytes to 255.255.255.255
```

And you should see `listener` responding that it got the packets. (If
`listener` doesn't respond, it could be because it's bound to an IPv6
address. Try changing the `AF_INET6` in `listener.c` to `AF_INET` to
force IPv4.)

Well, that's kind of exciting. But now fire up `listener` on another
machine next to you on the same network so that you have two copies
going, one on each machine, and run `broadcaster` again with your
broadcast address... Hey! Both `listener`s get the packet even though
you only called `sendto()` once! Cool!

If the `listener` gets data you send directly to it, but not data on the
broadcast address, it could be that you have a [i[Firewall]] firewall on
your local machine that is blocking the packets. (Yes, [i[Pat]] Pat and
[i[Bapper]] Bapper, thank you for realizing before I did that this is
why my sample code wasn't working. I told you I'd mention you in the
guide, and here you are. So _nyah_.)

Again, be careful with broadcast packets. Since every machine on the LAN
will be forced to deal with the packet whether it `recvfrom()`s it or
not, it can present quite a load to the entire computing network. They
are definitely to be used sparingly and appropriately.
