#!/usr/bin/env bash

RELEASETYPE=''


export SERVICE_NAME="template-service-cpp"
export LOCAL_SERVICE_NAME="local_${SERVICE_NAME}"

#null definition
export NULL='none'

#internal DokerImage name
export LOCAL_DOCKERIMAGE='eoepca/eoepca-build-cpp'
#eoepca repository
export EOEPCA_REPOSITORY='eoepca'
#eoepca name
export EOEPCA_IMAGE="eoepca-build-cpp"

#get branch name
TRAVIS_BRANCH="${TRAVIS_BRANCH:-develop}"

#change name for branch feature ex feature/EOEPCA-38 change in  feature_EOEPCA_38
TRAVIS_BRANCH=$(echo ${TRAVIS_BRANCH}| sed -e 's@/@_@g')
TRAVIS_BRANCH=$(echo ${TRAVIS_BRANCH}| sed -e 's@-@_@g')

export TRAVIS_BRANCH


export CMAKERELEASE="Debug"

#simple anchor
if [ "${TRAVIS_BRANCH}" == 'master' ]
then
  CMAKERELEASE="Release"
  RELEASETYPE='release'
	echo 'Branch selected: master '
fi

#simple anchor
if [ "${TRAVIS_BRANCH}" == 'develop' ]
then
  RELEASETYPE='develop'
	echo 'Branch selected: develop' 
fi

#new definitions
export TRAVIS_BUILD_NUMBER="${TRAVIS_BUILD_NUMBER:-0}"
export buildTag=${TRAVIS_BRANCH}_${TRAVIS_BUILD_NUMBER}
export BUILDER_ENV_IMAGE="${BUILDER_ENV_IMAGE:-${NULL}}"
export BUILDER_ENV_IMAGE_NEW_TAG="${BUILDER_ENV_IMAGE_NEW_TAG:-${NULL}}"

