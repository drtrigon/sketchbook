[![Build Status](https://travis-ci.org/drtrigon/sketchbook.svg?branch=master)](https://travis-ci.org/drtrigon/sketchbook)

## Setup

I would like to use features like CI (continous integration) keep a copy/mirror on e.g. github and more.

Maintaining a copy/mirror (more precise: with 2 remotes overloaded origin) on GitHub also allows
to use GitLab and GitHub together and by that all the nice featurs on GitHub.
See also; https://steveperkins.com/migrating-projects-from-github-to-gitlab/

"Option 2: Overload Origin with Both Remotes (needs one single pull/push only)"; in order to clone
and set this repo up use:
```
$ git clone file://// /data/mount/gvfs/smb-share:server=.../01git/sketchbook.git
$ cd sketchbook
$ git remote set-url --add origin https://github.com/drtrigon/sketchbook.git
```
you can check the settings with:
```
$ git remote -v
origin  /data/mount/gvfs/smb-share:server=.../01git/sketchbook.git/ (fetch)
origin  /data/mount/gvfs/smb-share:server=.../01git/sketchbook.git/ (push)
origin  https://github.com/drtrigon/sketchbook.git (push)
```

### Enable Travis CI for a Repository (for Arduino)
https://learn.adafruit.com/continuous-integration-arduino-and-you/testing-your-project

Login to Travis-CI using the GitHub account and enable Travis-CI for the given repo.

Add a .travis.yml file to your project, commit, pull/push and enjoy.
