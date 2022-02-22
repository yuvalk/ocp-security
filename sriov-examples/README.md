SRIOV Examples
===============
This simple example demonstrate how to create a pod with multiple NICs from multiple networks. Some of the nics are vfio for dpdk and some netdevice for kernel networking.

the pod uses env variables and utilze the data put in by the Network Resources Injector to find out info about these network definitions from within the pod
