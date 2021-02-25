.PHONY: build dev docs pages lab test

CONDA_ACTIVATE=source $$(conda info --base)/etc/profile.d/conda.sh ; conda activate ; conda activate
ENV_FILE=environment.yml
ENV_NAME=pyfitenv
ENV=$(CONDA_ACTIVATE) $(ENV_NAME) ;

DEV_ENV_FILE=environment-dev.yml
DEV_NAME=pyfitenv-dev
DEV=$(CONDA_ACTIVATE) $(DEV_NAME) ;

build:
	@conda env remove --name $(ENV_NAME)
	@conda env create -f $(ENV_FILE)
	@$(ENV) python test/test_pyfitparquet.py

dev:
	@conda env remove --name $(DEV_NAME)
	@conda env create -f $(DEV_ENV_FILE)

docs:
	@$(DEV) mkdocs build
	@$(DEV) mkdocs serve

lab:
	@$(DEV) jupyter lab

pages:
	@$(DEV) mkdocs gh-deploy

test:
	@$(DEV) python test/test_pyfitparquet.py

