registry = quay.io/ykashtan
project = argsprinter

.PHONY: build-argsprinter
build-argsprinter:
	@echo "build argsprinter"
	go build -o argsprinter main.go

.PHONY: build-image-nothing
build-image-nothing:
	@echo "build Dockerfile.nothing container image"
	podman build . -f Dockerfile.nothing --tag $(registry)/$(project):nothing

.PHONY: build-image-cmd
build-image-cmd:
	@echo "build Dockerfile.cmd container image"
	podman build . -f Dockerfile.cmd --tag $(registry)/$(project):cmd

.PHONY: build-image-entrypoint
build-image-entrypoint:
	@echo "build Dockerfile.entrypoint container image"
	podman build . -f Dockerfile.entrypoint --tag $(registry)/$(project):entrypoint

.PHONY: build-image-cmdentrypoint
build-image-cmdentrypoint:
	@echo "build Dockerfile.cmdentrypoint container image"
	podman build . -f Dockerfile.cmdentrypoint --tag $(registry)/$(project):cmdentrypoint

build-images: build-image-nothing build-image-cmd build-image-entrypoint build-image-cmdentrypoint

push-images: build-images
	@echo "pushing images"
	podman push $(registry)/$(project):nothing
	podman push $(registry)/$(project):cmd
	podman push $(registry)/$(project):entrypoint
	podman push $(registry)/$(project):cmdentrypoint
