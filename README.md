## Projekte
### Drone_MultiWii / FlightController
[![Build Status](https://travis-ci.org/drtrigon/sketchbook.svg?branch=master)](https://travis-ci.org/drtrigon/sketchbook)
[![design rule check status](https://edrc.me/api/v1/user/drtrigon/project/eagle/img/status.svg)](https://edrc.me/g/drtrigon/eagle)
[![GERBER Build Status](https://travis-ci.org/drtrigon/eagle.svg?branch=master)](https://travis-ci.org/drtrigon/eagle)
([detail...](https://rawgit.com/drtrigon/sketchbook/master/MultiWii_2_4/status.html))

[![Build Status](https://travis-ci.org/drtrigon/docker-sketchbook.svg?branch=master)](https://travis-ci.org/drtrigon/docker-sketchbook)

https://github.com/drtrigon/eagle

* https://github.com/drtrigon/sketchbook/blob/master/MultiWii_2_4/Drone-MultiWii_FlightController.stl

[![projects/Drone_MultiWii/FlightController.brd from EDRC.me](https://edrc.me/api/v1/user/drtrigon/project/eagle/img/file/projects%2FDrone_MultiWii%2FFlightController.png?ref=refs%2Fheads%2Fmaster)](https://edrc.me/g/drtrigon/eagle)

(might make sense to split projects off into separate repos/submodules)

## Setup

I would like to use features like CI (continous integration) keep a copy/mirror on e.g. github and more.

Maintaining a copy/mirror (more precise: with 2 remotes overloaded origin) on GitHub also allows
to use GitLab and GitHub together and by that all the nice featurs on GitHub.
See also; https://steveperkins.com/migrating-projects-from-github-to-gitlab/

"Option 2: Overload Origin with Both Remotes (needs one single pull/push only)"; in order to clone
and set this repo up use:
```
$ git clone file://///data/mount/gvfs/smb-share:server=.../01git/sketchbook.git
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

May be split off some sub-directories like MultiWii (for simpler testing),
see also: https://help.github.com/articles/splitting-a-subfolder-out-into-a-new-repository/.

Consider creating a docker for testing and may be upload of the skechtes,
see also: https://github.com/tombenke/darduino, https://github.com/suculent/arduino-docker-build

### Enable EDRC.ME for a Repository (for Eagle)
See https://github.com/drtrigon/eagle

### Enable CAD View (for Catia)
* https://help.github.com/articles/3d-file-viewer/
* https://blog.github.com/2013-04-09-stl-file-viewing/
* https://blog.github.com/2013-09-17-3d-file-diffs/

#### Create CAD (.stl) Designs online easily
See https://www.tinkercad.com/

### Further Info
* convert STEP file to .stl online: https://www.makexyz.com/convert/step-to-stl
* convert Catia file to .stl online: https://www.convertcadfiles.com/de/konvertieren/
