# IPv4에서 IPv6으로 점프하기

[i[IPv6]]

"아무튼 저는 IPv6에서 동작하려면 제 코드의 어디를 바꿔야 하는지 알고싶단 말입니다!
당장 알려주세요!"

종아요! 좋습니다!

여기 적을 내용의 대부분은 제가 위에서 다룬 내용들이지만 이것은 참을성 없는
분들을 위한 짧은 버전입니다. (물론 이것보다 더 많은 내용이 있겠지만, 이 안내서에
있는 내용은 이정도입니다.)

1. 우선 `struct sockaddr` 정보를 얻어내기 위해서 수작업 대신
   [i[`getaddrinfo()` function]] [`getaddrinfo()`](#structs) 함수를 사용하십시오.
   이렇게 하면 여러분은 IP버전에 신경쓰지 않을 수 있고, 뒤따르는 많은 후속
   작업을 할 필요가 없게 해 줍니다.

2. IP 버전에 관계된 것을 하드코딩하는 곳을 찾아낼 때마다 헬퍼 함수로
   감싸두는 처리를 해 두십시오.

3. `AF_INET`를 `AF_INET6`로 바꾸십시오.

4. `PF_INET`를 `PF_INET6`로 바꾸십시오.

5. `INADDR_ANY` 대입을 `in6addr_any`대입으로 바꾸십시오. 이런 차이가 있습니다:

   ```{.c}
   struct sockaddr_in sa;
   struct sockaddr_in6 sa6;
   
   sa.sin_addr.s_addr = INADDR_ANY;  // use my IPv4 address
   sa6.sin6_addr = in6addr_any; // use my IPv6 address
   ```

   또한 `struct in6_addr`을 선언할 때 `IN6ADDR_ANY_INIT`을 초기값으로
   사용할 수 있습니다. 아래와 같이 합니다.

   ```{.c}
   struct in6_addr ia6 = IN6ADDR_ANY_INIT;
   ```

6. `struct sockaddr_in` 대신에 `struct sockaddr_in6`을 사용하시고, 필요한
   필드에 "6"을 적절히 덧붙이십시오. (위의 [`struct`s](#structs)을
   참고하십시오) `sin6_zero`필드는 없습니다.

7. `struct in_addr` 대신에 `struct in6_addr`를 사용하시고, 필요한
   필드에 "6"을 적절히 덧붙이십시오. (위의 [`struct`s](#structs)을
   참고하십시오)

8. `inet_aton()`이나 `inet_addr()` 대신에 `inet_pton()`을 사용하십시오.

9. `inet_ntoa()` 대신에 `inet_ntop()`을 사용하십시오.

10. `gethostbyname()`대신에 더 뛰어난 `getaddrinfo()`를 사용하십시오.

11. `gethostbyaddr()` 대신에 더 뛰어난 [i[`getnameinfo()` function]]
    `getnameinfo()`를 사용하십시오. (`gethostbyaddr()`가 IPv6을 위해서도
    여전히 작동하기는 합니다).

12. `INADDR_BROADCAST`는 더 이상 작동하지 않습니다. 대신 IPv6 멀티캐스트를
    사용하십시오.

_끝_!
