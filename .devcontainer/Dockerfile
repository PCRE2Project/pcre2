ARG CI_IMAGE=ghcr.io/pcre2project/pcre2-ci:latest
FROM ${CI_IMAGE}

RUN bash /opt/pcre2-container/.devcontainer/setup.sh --env=interactive

ENV PATH=/home/vscode/.local/bin:${PATH}
USER vscode
WORKDIR /workspaces
