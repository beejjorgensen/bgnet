FROM python:3.8.2

RUN apt-get update && apt-get install -y pandoc texlive-xetex fonts-liberation && apt-get clean

WORKDIR /guide

CMD make -e SHELL=/bin/bash pristine all stage
