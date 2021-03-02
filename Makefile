.PHONY: all install sdist bdist uninstall clean test docs pages create remove

ENV_NAME=pyfitenv
CONDA_CONFIG=source $$(conda info --base)/etc/profile.d/conda.sh
ENV=$(CONDA_CONFIG) ; conda activate ; conda activate $(ENV_NAME) ;

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
ifneq ($(dir $(mkfile_path)), $(shell pwd)/)
$(error "ERROR: Must run from repo root directory")
endif

all : create test

install:
	# Builds from source and installs into $(ENV_NAME)
	@$(ENV) pip install . 

build:
	# Builds from source and installs in local cmake-build/ directory
	@$(ENV) python setup.py install 

sdist:
	# Packages a source distribution in local dist/ directory (prints SHA256)
	@$(ENV) python setup.py sdist 
	@$(ENV) openssl sha256 dist/pyfitparquet-*.tar.gz 
	
bdist:
	# Builds a binary distribution in local dist/
	# build/ and pyfitparquet.egg-info directories
	@$(ENV) python setup.py bdist 

uninstall:
	# Completely uninstalls pyfitparquet from $(ENV_NAME)
	@$(ENV) .scripts/uninstall.sh 

clean:
	# Removes all temporary local build, dist, etc files/dirs
	@$(ENV) .scripts/clean.sh 

test:
	# Runs unit test sequence on $(ENV_NAME) installed pyfitparquet
	@$(ENV) python test/test_pyfitparquet.py 

docs:
	# Builds and locally serves documentation
	@$(ENV) conda install -c conda-forge mkdocs --yes
	@$(ENV) mkdocs build 
	@$(ENV) mkdocs serve 

pages:
	# Deploys documentation to pyfitparquet GitHub Pages
	@$(ENV) conda install -c conda-forge mkdocs --yes
	@$(ENV) mkdocs gh-deploy 

create:
	# Freshly creates the conda $(ENV_NAME), this includes by 
	# default pyfitparquet build from source and install
	@conda env remove --name $(ENV_NAME) 
	@conda env create -f environment.yml  

remove:
	# Completely removes the conda $(ENV_NAME) 
	@conda remove --name $(ENV_NAME) --all --yes 
