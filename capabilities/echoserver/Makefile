.PHONY: build-go-echo
build-go-echo:
	@echo "build echo golang version"
	go build echo.go
	podman build . -f Dockerfile.go --tag quay.io/ykashtan/goecho
