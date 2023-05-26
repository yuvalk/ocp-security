OCP layered libreswan
======================

to run a POC with my existing image (based on 4.12.9) you can simply
`kcli create cluster openshift -P ctlplanes=1 -P version=stable -P tag=4.12.9 ykipsec -P clusterprofile=sample-openshift-sno --force`

to build your own, see get-os-image.sh, then modify Dockerfile accordingly,
build it
`podman build -t quay.io/ykashtan/coreos-8:libreswan . --authfile openshift_pull.json`
`podman push quay.io/ykashtan/coreos-8:libreswan`
modify the mainfest with your image name
then create a cluster with that
