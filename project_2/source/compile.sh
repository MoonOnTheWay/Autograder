#!/bin/bash

cp -r source/harness_code/* /tmp
cp -r submission/* /tmp/userprog

cd /tmp && make
