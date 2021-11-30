#!/bin/bash -x

for policy_file in /tmp/selinux-policies/*.te; do
	policy="${policy_file%.*}"
	checkmodule -M -m -o $policy.mod $policy.te
	semodule_package -o $policy.pp -m $policy.mod

	semodule -i $policy.pp
done

