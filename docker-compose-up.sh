#! /usr/bin/env bash

# if ysyx container is already running, do nothing
if [[ -z $(docker container ls -q --filter=name=$(cat docker-compose.yaml | sed -n "s/    container_name: //p") --format "{{.Names}}") ]]
then
    # for graphic
    XAUTH=/tmp/.docker.xauth
    sudo rm $XAUTH
    touch $XAUTH
    xauth nlist $DISPLAY | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -

    # copy ssh private key from host machine
    [[ ! -d configs/.ssh ]] && mkdir -p configs/.ssh
    [[ ! -f configs/.ssh/id_rsa ]] && cp ~/.ssh/id_rsa configs/.ssh/

    # if container created but stopped, start it, otherwise create one with docker-compose
    ysyx_container=$(docker container ls -aq --filter=name=$(cat docker-compose.yaml | sed -n "s/    container_name: //p") --format "{{.Names}}")
    if [[ -n $ysyx_container ]]
    then
        docker start $ysyx_container
    else
        export HOST_IP=$(ip route get 8.8.8.8 | head -1 | cut -d' ' -f7)
        docker-compose -f docker-compose.yaml up --build -d --remove-orphans
    fi
    echo Container started
else
    echo Container already running
fi