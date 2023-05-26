FROM quay.io/openshift-release-dev/ocp-v4.0-art-dev@sha256:cd29255bd4e6634404ef8e0f4a7f19b4ae5a57a3afc7d67cbe00934ba998d6ac

# add NetworkManager-libreswan
RUN rpm-ostree install dnf && dnf -y install NetworkManager-libreswan && rpm-ostree cleanup -m

