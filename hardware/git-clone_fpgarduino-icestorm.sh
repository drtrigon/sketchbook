#!/bin/bash

# https://stackoverflow.com/questions/2411031/how-do-i-clone-into-a-non-empty-directory

# Clone just the repository's .git folder (excluding files as they are already in
# `existing-dir`) into an empty temporary directory
#git clone --no-checkout repo-to-clone existing-dir/existing-dir.tmp # might want --no-hardlinks for cloning local repo
git clone --no-checkout https://github.com/drtrigon/fpgarduino-icestorm.git

# Move the .git folder to the directory with the files.
# This makes `existing-dir` a git repo.
#mv existing-dir/existing-dir.tmp/.git existing-dir/
mv fpgarduino-icestorm/.git .

# Delete the temporary directory
rmdir fpgarduino-icestorm
#cd existing-dir

# git thinks all files are deleted, this reverts the state of the repo to HEAD.
# WARNING: any local changes to the files will be lost.
#git reset --hard HEAD
git add AlhambraII
git diff HEAD
