#!/bin/bash
red_clr="\033[31m"
grn_clr="\033[32m"
end_clr="\033[0m"
cd `dirname $0`
cwd=`pwd`

lockfile='./bin/daemon.pid'
appname='async-proxy'
conf_path='./conf/bench.conf'
#svr_path="/opt/home/maggie/refactor/public/test_backend/backend00"
#svr_path="/opt/home/maggie/test/refactor3/refactor/public/async-proxy-without-thrift"
#svr_path="/home/maggie/test/refactor3/refactor/public/async-proxy-without-thrift"

function add_crontab()
{
    tmpfile=crontab-ori.tempXX
    item1='*/1 * * * * cd '${cwd}' && ./keep-alive.sh '${appname}' >> keep-alive.log 2>&1'

    crontab -l >$tmpfile 2>/dev/null

    fgrep "${item1}" $tmpfile &>/dev/null
    if [ $? -ne 0 ]
    then
        echo "${item1}" >> $tmpfile
    fi

    fgrep "${item2}" $tmpfile &>/dev/null
    if [ $? -ne 0 ]
    then
        echo "${item2}" >> $tmpfile
    fi

    crontab $tmpfile
	rm -f $tmpfile
}

function delete_crontab()
{
	tmpfile1=crontab-ori.temp1XX
	tmpfile2=crontab-ori.temp2XX
	tmpfile3=crontab-ori.temp3XX
	item1='*/1 * * * * cd '${cwd}' && ./keep-alive.sh '${appname}' >> keep-alive.log 2>&1'

	crontab -l >$tmpfile1 2>/dev/null

	fgrep "${item1}" $tmpfile1 &>/dev/null
	if [ $? -eq 0 ]
	then
		fgrep -v "${item1}" $tmpfile1 &> $tmpfile2
		crontab $tmpfile2
	fi

	fgrep "${item2}" $tmpfile1 &>/dev/null
	if [ $? -eq 0 ]
	then
		fgrep -v "${item2}" $tmpfile2 &> $tmpfile3
		crontab $tmpfile3
	fi

	rm -f $tmpfile1
	rm -f $tmpfile2
	rm -f $tmpfile3
}


function start()
{
	./bin/check-single $lockfile

	if [ $? -eq 1 ]
	then
		printf "$red_clr%50s$end_clr\n" "${appname} is already running"
		exit 1
	fi
	
	if [ "$1" = "file" ]
	then
		LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:./bin/" ./bin/${appname} ./bin/${appname}.so 0 ${conf_path}
	else 
		LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:./bin/" ./bin/${appname} ./bin/${appname}.so 1 ${svr_path}
	fi

	sleep 1
	./bin/check-single $lockfile
	if [ $? -eq 0 ]
	then
		printf "$red_clr%50s$end_clr\n" "start ${appname} failed."
		exit 1
	fi
}

function stop()
{
    ./bin/check-single $lockfile
    running=$?
    if [ $running -eq 0 ]
    then
        printf "$red_clr%50s$end_clr\n" "$appname is not running"
        exit 1
    fi

    while [ $running -eq 1 ]
    do
        kill `cat $lockfile`
        sleep 1
        ./bin/check-single $lockfile
        running=$?
    done
}

function restart()
{
	stop
	start
}

function state()
{
    ./bin/check-single $lockfile
    running=$?
    if [ $running -eq 0 ]
    then
        printf "$red_clr%50s$end_clr\n" "$appname is not running"
        exit 1
    fi

    ps -fs `cat $lockfile`
}

function usage()
{
	echo "$0 <start|stop|restart|state|> <center|file>"
}

if [[ $# -ne 1 && $# -ne 2 ]]; then
    usage
    exit 1
fi

case $1 in
    start)
        start $2
		add_crontab
        ;;
    stop)
        stop
		delete_crontab
        ;;
    restart)
        restart 
        ;;
    state)
        state 
        ;;
    *)
        usage 
        ;;
    esac

exit 0
