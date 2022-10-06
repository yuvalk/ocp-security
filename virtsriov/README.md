# virtsriov
virtsriov is an attempt to run dpdk applications on top of openshift running in virtual machines.

This includes all the configurations needed to 
- create VFs on the hypervisor
- deploy a virtual ocp cluster
- configure the worker vms with vIOMMU and sriov VFs (passthrough from the host)
- deploy sriov device plugin
- and run unprivileged dpdk-testpmd

currently support intel nics, with intel iommu only.

## SRIOV background
The thing that apparently confuses some of us is related to better understanding of how sriov NIC works.
I dont want to go into too much details here, and there is plenty info around (will ad dlink later)
the important detail to remember about sriov and vms
is that the physical nic (PF) will create new virtual nics (VFs) on the main host (hypervisor) PCI bus.
that is problematic for vms becaue the vm have no way of knowing that.
so if we move a pf to a vm (with pci passthrough for example) the VFs will still be created on the HV. in many cases resulting in a kernel panic of the vm. (bc driver expect to see the vf?) 
so, the only viable solution is to create VFs on the HV and then move them to VM.

this repo will try to suggest a way to do that properly with openshift.

## the gory details:
### hv config:
1. create vfs
2. create vms. and here's the tricky part, for the workers we need:
a. use uefi capable firmware (pc-q35)
b. add a iommu device:
```
    <iommu model='intel'>
      <driver intremap='on' caching_mode='on'/>
    </iommu>
```
c. add vfs as hostdev vfio pci devices, for example:
```
    <hostdev mode='subsystem' type='pci' managed='yes'>
      <driver name='vfio'/>
      <source>
        <address domain='0x0000' bus='0x3b' slot='0x03' function='0x0'/>
      </source>
      <alias name='hostdev0'/>
      <address type='pci' domain='0x0000' bus='0x05' slot='0x00' function='0x0'/>
    </hostdev>
```
d. add a memtune with high hard_limit, for example:
```
  <memtune>
    <hard_limit unit='KiB'>104857600</hard_limit>
  </memtune>
```
if not done, qemu will crash out of memory. this is because iommu allocate extra memory.

### vm node config:
1. kernel args:
a. set hugepages
b. iommu
2. disable iavf
3. to allow unprivileged dpdk, we need to raise the limit on amount of locked memoery for our runtime.
4. to enable vfio-pci we need to set it with the right ids
5. then we deploy the sriov-device-plugin which will be used to move the VFs from node to pod namespace, including their iommu deive.
6. deploy the dpdk with both hugepages and the sriov resources.
7. from pod, can look at env variables (add which) to get the ids of the attached vfs pci ids


## Openshift implementation:
1. I'm using kcli to interact with libvirt and also to deploy openshift.
1. performance profile is used to get hugepages, set intel iommu and disable iavf
1. to allow unprivileged dpdk, we need to raise the limit on amount of locked memory. this is done by changing the crio runtime ulimit. https://access.redhat.com/solutions/6243491
1. to enable correct vfio-pci attachment we configure module load with correct ids
1. then we deploy sriov device plugin which will be used to move the VFs from node to pod namespace, including their iommu device. to do that:
   1. create namespace
   1. create service account (sa)
   1. make sa privileged
   1. create config map, including desired devices
   1. deploy device plugin daemonset on all nodes
1. and now we can deploy our application. we are using testpmd to show that everything works as expected. for non privileged, dpdk must be used in iova mode (which can use vmm addresses)

