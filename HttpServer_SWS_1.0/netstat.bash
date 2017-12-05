#!/bin/sh
port=$1
netstat -na | grep ${port}
