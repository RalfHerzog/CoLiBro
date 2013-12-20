#!/bin/bash

EXTERN_LIBS_DIR=./extern

POLARSSL_VERSION=1.3.2
POLARSSL_PATH=polarssl-${POLARSSL_VERSION}
POLARSSL_FILE=${POLARSSL_PATH}-gpl.tgz
POLARSSL_SYMLINK=polarssl

FIREDNS_PATH=firedns

if [ ! -d ${EXTERN_LIBS_DIR} ]
then
  mkdir ${EXTERN_LIBS_DIR}
fi

cd ${EXTERN_LIBS_DIR}

if [ ! -d ${POLARSSL_SYMLINK} ]
then
  printf "Downloading polarssl ... "

  wget --quiet https://polarssl.org/download/${POLARSSL_FILE} -O ${POLARSSL_FILE}
  tar xf ${POLARSSL_FILE}
  rm ${POLARSSL_FILE}
  ln -s ${POLARSSL_PATH}/ ${POLARSSL_SYMLINK}

  printf "done\n"
fi

if [ ! -d ${FIREDNS_PATH} ]
then
  printf "Downloading firedns ... \n"
  git clone https://github.com/rofl0r/firedns ${FIREDNS_PATH}
  printf "done\n"
fi

cd ..

echo done
