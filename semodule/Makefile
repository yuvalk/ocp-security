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
