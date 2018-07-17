#!/bin/bash

cd `dirname $0`
appname="server"

./${appname}.sh state > /dev/null 2>&1

if [ $? -ne 0 ];then
    echo "${appname} stopped abnormally, try to start it."
    ./${appname}.sh stop
    sleep 5
    echo "restart ${appname} at $(date)"
    ./${appname}.sh start
fi

find . -name "core*" | xargs rm -vf 

exit 0
