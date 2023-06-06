# 개요

이 폴더에는 한국어 번역과 관련된 파일이 들어있습니다.

# 구조

## fonts

pretendard 폰트 관련 파일이 들어있습니다.

## src

루트 아래의 src와 마찬가지이며 메이크파일의 설정변수들이 약간 변경되어 있습니다.

# 빌드

한국어용으로 빌드하기 위해서 루트 경로에서 아래와 같이 입력합니다.

```
docker run --rm -v "$PWD":/guide -ti {루트 폴더의 Dockerfile로 만든 이미지 태그} make -e SHELL=/bin/bash pristine all stage SRCDIR=i18n/ko/src
```

빌드 결과물은 영어와 동일하게 루트의 stage폴더에 생성됩니다.