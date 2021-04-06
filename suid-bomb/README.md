suid-bomb POC
==============
this is a POC for Openshift Container Platform
showing that even a restricted SCC (unauthorized) pod
can get uid 0, if the container image include a binary with setuid bit turned on

NOTE: this is not a security flaw !

Inventory:
-----------
main.c - is a simple program that try to read `secret.txt` twice. once before setuid() call and once afterwards.
secret.txt- is our secret file
Dockerfile - is the recepie on how to get this all into a container image
bomb-pod.yaml - is a pod defintion that uses my compiled and pushed image from quay

how to reproduce:
------------------
1. compile the c program with gcc to produce a.out
2. build the container with `podman build .`
3. push it to a registry available from your OCP cluster
4. edit the `bomb-pod.yaml` so it'll pull the image from the right place
5. apply the pod definition, with a non-privileged user
6. observe output with `oc logs suid-bomb

if all fine output should look like:
```
start
try to read secert
this is a secret file


setuid: 0
setuid=0, errno=0
try to read secert
this is a secret file


exit
```
