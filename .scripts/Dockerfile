FROM quay.io/condaforge/linux-anvil-comp7
WORKDIR /home/conda-builder
ADD cbuilderenv.yml .
RUN /bin/bash -c 'source /etc/profile; \
conda env create -f cbuilderenv.yml'

COPY dist dist/
COPY recipes recipes/
COPY docker_endpoint.sh .
ENTRYPOINT [ "./docker_endpoint.sh" ]
