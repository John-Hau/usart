#!/bin/bash
#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
m_fifo="/tmp/p_fifo"
echo "hello world" > $m_fifo
read msg  < $m_fifo

for item in $msg
do
echo $item
done


echo "done"


