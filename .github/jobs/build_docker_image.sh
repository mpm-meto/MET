#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=$(get_dockerhub_tag)

DOCKERFILE_PATH=${GITHUB_WORKSPACE}/internal/scripts/docker/Dockerfile.copy

CMD_LOGFILE=${GITHUB_WORKSPACE}/docker_build.log

time_command docker build -t ${DOCKERHUB_TAG} \
    --build-arg SOURCE_BRANCH \
    --build-arg MET_BASE_REPO \
    --build-arg MET_BASE_TAG \
    -f $DOCKERFILE_PATH ${GITHUB_WORKSPACE}
if [ $? != 0 ]; then
  cat ${GITHUB_WORKSPACE}/docker_build.log
  exit 1
fi

# Copy the log directory from the image
id=$(docker create ${DOCKERHUB_TAG})
time_command docker cp $id:/met/logs met_logs
mv met_logs/*.log ${GITHUB_WORKSPACE}/.
docker rm -v $id
