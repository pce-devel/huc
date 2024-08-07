#!/bin/sh

export PCE_INCLUDE=`pwd`/../include/hucc
export PCE_PCEAS=`pwd`/../bin/pceas
echo $PCE_INCLUDE $PCE_PCEAS

tests="$@"
test -z "$tests" && tests="tests/*.c dg/*.c"
test -n "$1" && tests="$@"

fails=0
nocompiles=0
exesuffix=

if [ "$OS" = "Windows_NT" ]; then
	exesuffix=.exe
fi

echo exesuffix="$exesuffix"

for d in small norec noopt
do
	fails=0
	nocompiles=0
	passes=0
	test "$d" = "small" && opt="-DSTACK_SIZE=128 -DSMALL -msmall"
	test "$d" = "norec" && opt="-DSTACK_SIZE=128 -DNORECURSE -fno-recursive"
	test "$d" = "noopt" && opt="-DSTACK_SIZE=128 -DNOOPT -O0"
	echo "testing $d"
	echo opt="$opt"
	for i in $tests
	do
		echo "HuCC Type: $d   Test: $i"
		if ! ../bin/hucc${exesuffix} -DNO_LABEL_VALUES $opt $i >/dev/null ; then
			echo NOCOMPILE
			../bin/hucc${exesuffix} $opt $i
			exit
#			nocompiles=$((nocompiles + 1))
#			continue
		fi
		if ../tgemu/tgemu${exesuffix} "${i%.c}.pce" 2>/dev/null >/dev/null ; then
			echo PASS
			passes=$((passes + 1))
		else
			echo "FAIL (exit code $?)"
			../tgemu/tgemu${exesuffix} "${i%.c}.pce"
			exit
#			mkdir -p failtraces
#			mv "${i%.c}".{sym,s,pce} failtraces/
#			fails=$((fails + 1))
		fi
	done
	echo "$d passes: $passes; fails: $fails, nocompiles: $nocompiles"
	total_fails=$((total_fails + fails))
	total_nocompiles=$((total_nocompiles + nocompiles))
	total_passes=$((total_passes + passes))
done
echo "Total passes: $total_passes; fails: $total_fails, nocompiles: $total_nocompiles"
exit $((total_fails + total_nocompiles))
