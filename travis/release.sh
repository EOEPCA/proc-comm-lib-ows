#!/usr/bin/env bash

set -euov pipefail


source travis/variables.sh

pullTag=${EOEPCA_REPOSITORY}/${SERVICE_NAME}:$buildTag
releaseTag=${EOEPCA_REPOSITORY}/${SERVICE_NAME}:release_${buildTag}
latestTag=${EOEPCA_REPOSITORY}/${SERVICE_NAME}:latest

echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin

docker pull ${pullTag}

docker tag ${pullTag} ${releaseTag}
docker push ${releaseTag}

docker tag ${pullTag} ${latestTag}
docker push ${latestTag}
