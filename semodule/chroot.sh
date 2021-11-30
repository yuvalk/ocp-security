#!/bin/bash

SCRIPT_NAME=$1
shift
rm -fr /host/tmp/selinux-policies
cp -a /selinux-policies/ /host/tmp/selinux-policies
cp $SCRIPT_NAME /host/tmp/semodule-loader-entrypoint.sh
chroot /host /tmp/semodule-loader-entrypoint.sh $*
