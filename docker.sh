#!/usr/bin/env bash
set -euo pipefail

ARCH=$(uname -m)
ALL_ARCHS="x86_64 armv7l aarch64"
TAG=${TAG:-"latest"}
ARCHTAG="${TAG}-${ARCH}"

usage () {
  echo "Usage: ${0} [command]"
  echo "Available commands:"
  echo "  help       Show this usage information"
  echo "  build      Build docker image"
  echo "  push       Push built docker image to the docker hub"
  echo "  manifest   Compile the docker hub manifest (combines arm and x86 tags into one)"
  echo "  tag        Tag a release"
}

build () {
  docker build --pull -t jketterl/codecserver:${ARCHTAG} .
}

push () {
  docker push jketterl/codecserver:${ARCHTAG}
}

manifest () {
  # there's no docker manifest rm command, and the create --amend does not work, so we have to clean up manually
  rm -rf "${HOME}/.docker/manifests/docker.io_jketterl_codecserver-${TAG}"
  IMAGE_LIST=""
  for a in ${ALL_ARCHS}; do
    IMAGE_LIST="${IMAGE_LIST} jketterl/codecserver:${TAG}-${a}"
  done
  docker manifest create jketterl/codecserver:${TAG} ${IMAGE_LIST}
  docker manifest push --purge jketterl/codecserver:${TAG}
}

tag () {
  if [[ -x ${1:-} || -z ${2:-} ]] ; then
    echo "Usage: ${0} tag [SRC_TAG] [TARGET_TAG]"
    return
  fi

  local SRC_TAG=${1}
  local TARGET_TAG=${2}

  # there's no docker manifest rm command, and the create --amend does not work, so we have to clean up manually
  rm -rf "${HOME}/.docker/manifests/docker.io_jketterl_codecserver-${TARGET_TAG}"
  IMAGE_LIST=""
  for a in ${ALL_ARCHS}; do
    docker pull jketterl/codecserver:${SRC_TAG}-${a}
    docker tag jketterl/codecserver:${SRC_TAG}-${a} jketterl/codecserver:${TARGET_TAG}-${a}
    docker push jketterl/codecserver:${TARGET_TAG}-${a}
    IMAGE_LIST="${IMAGE_LIST} jketterl/codecserver:${TARGET_TAG}-${a}"
  done
  docker manifest create jketterl/codecserver:${TARGET_TAG} ${IMAGE_LIST}
  docker manifest push --purge jketterl/codecserver:${TARGET_TAG}
  docker pull jketterl/codecserver:${TARGET_TAG}
}

case ${1:-} in
  build)
    build
    ;;
  push)
    push
    ;;
  manifest)
    manifest
    ;;
  tag)
    tag ${@:2}
    ;;
  *)
    usage
    ;;
esac