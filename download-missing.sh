#!/bin/bash

EXTERN_LIBS_DIR=./extra-libs

POLARSSL_VERSION=1.3.2
POLARSSL_PATH=polarssl-${POLARSSL_VERSION}
POLARSSL_FILE=${POLARSSL_PATH}-gpl.tgz

if [ ! -d ${EXTERN_LIBS_DIR} ]
then
mkdir ${EXTERN_LIBS_DIR}
fi

cd ${EXTERN_LIBS_DIR}

wget https://polarssl.org/download/${POLARSSL_FILE} -O ${POLARSSL_FILE}
tar xf ${POLARSSL_FILE}
rm ${POLARSSL_FILE}

cd ..

ln -s ${EXTERN_LIBS_DIR}/${POLARSSL_PATH}/ polarssl
