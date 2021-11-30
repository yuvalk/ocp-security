#!/bin/bash

SCRIPT_NAME=$1
shift
chroot /host ./$SCRIPT_NAME $*
