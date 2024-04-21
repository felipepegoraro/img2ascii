#!/bin/bash
CC=gcc
FILE=img2ascii

configure(){
  wget https://raw.githubusercontent.com/nothings/stb/master/stb_ds.h
  wget https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
  wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_resize2.h
  wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
  mv *.h ./inc/.
}

test(){
  if [ "$#" -ne 3 ]; then
    echo "Uso: $0 R G B"
    exit 1
  fi

  R=$1
  G=$2
  B=$3

  convert -size 50x50 xc:"rgb($R,$G,$B)" background.png

  display background.png

  rm background.png
}

compile(){
  configure
  $CC -Wall -o $FILE ${FILE}.c -lm
}

compile
