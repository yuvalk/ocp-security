SRIOV Examples
===============
This simple example demonstrate how to create a pod with multiple NICs from multiple networks. Some of the nics are vfio for dpdk and some netdevice for kernel networking.

the pod uses env variables and utilze the data put in by the Network Resources Injector to find out info about these network definitions from within the pod

Installation
-------------
1. modify nodeSelector labels if needed
1. apply in policies
`oc apply -f 01_policies.yaml`
1. wait for node to settle (after reboot, because of the policy)
1. apply network definitions
`oc apply -f 02_networks.yaml`
1. deploy pod
`oc apply -f 03_pod.yaml`

then you can either
`oc logs client`
or experiment and wander around with
`oc exec -ti client /bin/bash`

