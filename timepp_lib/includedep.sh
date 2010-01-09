#!/bin/bash

cmd=${1:-dot}
opt=""
if [[ ${2} == "showsys" ]]; then opt=--include=forceshow; fi

dir=/d/greensoft
if [ -d /e/greensoft ]; then dir=/e/greensoft; fi

rm -f ./tplib.dot
$dir/cmdline/cinclude2dot.pl --src=include $opt > ./tplib.dot

if [ -d $dir/graphviz ]
then
	rm -f /tmp/tplib.ps
	$dir/graphviz/bin/$cmd.exe ./tplib.dot -Tps -o /tmp/tplib.ps
	if [ -f /tmp/tplib.ps ]
	then 
		rm -f ./tplib.dot
		start /tmp/tplib.ps
	fi
fi

