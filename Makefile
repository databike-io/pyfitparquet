.PHONY: docs env pages lab test uninstall

CONDA_ACTIVATE=source $$(conda info --base)/etc/profile.d/conda.sh ; conda activate ; conda activate
DEV_ENV_FILE=environment.yml
DEV_NAME=pyfitenv
DEV=$(CONDA_ACTIVATE) $(DEV_NAME) ;

docs:
	@$(DEV) mkdocs build
	@$(DEV) mkdocs serve

env:
	@conda env remove --name $(DEV_NAME)
	@conda env create -f $(DEV_ENV_FILE)

lab:
	@$(DEV) jupyter lab

pages:
	@$(DEV) mkdocs gh-deploy

test:
	@$(DEV) python test/test_pyfitparquet.py

uninstall:
	@conda env remove --name pyfitenv
