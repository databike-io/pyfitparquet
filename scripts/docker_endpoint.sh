#!/usr/bin/env bash
source /etc/profile

conda activate cbuilderenv
conda build -c conda-forge recipes/

# Block to keep container running
tail -f /dev/null