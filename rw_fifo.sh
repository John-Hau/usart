#!/bin/bash
#
m_fifo="/tmp/p_fifo"
echo "hello world" > $m_fifo
read msg  < $m_fifo

for item in $msg
do
echo $item
done


echo "done"


