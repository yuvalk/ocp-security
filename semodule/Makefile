.PHONY: build-image
build-image:
	@echo "building container image"
	podman build . --tag quay.io/ykashtan/ubi8-ip

.PHONY: openshift-deploy
openshift-deploy:
	@echo "deploy demo on cluster"
	oc adm new-project semodule-test
	oc project semodule-test
	oc adm policy add-scc-to-user privileged -z default -n semodule-test
	oc apply -f deployment.yaml

.PHONY: pod-ping
pod-ping:
	@echo "pinging from pod with different src"
	POD_NAME=`oc get pod -o json | jq -r '.items[0].metadata.name'`; \
	oc exec -ti $$POD_NAME -- ping -c 1 -I 1.1.1.1 1.1.1.1
