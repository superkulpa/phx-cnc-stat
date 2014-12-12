#!/bin/sh
#echo $1
tar -xzf  $1 -Clogs --wildcards CNC/logs/log*/report.xml
#logs/logs.xml.tar.gz