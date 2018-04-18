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

@htmlonly
    <table>
      <tr>
        <td>CAD</td>
        <td>
        </td>
      </tr>
      <tr>
        <td>PCB</td>
        <td>
          <a href="https://edrc.me/g/drtrigon/eagle">
            <img alt="design rule check status" src="https://edrc.me/api/v1/user/drtrigon/project/eagle/img/status.svg" />
          </a>
          <a href="https://travis-ci.org/drtrigon/eagle">
            <img alt="GERBER Build Status" src="https://travis-ci.org/drtrigon/eagle.svg?branch=master" />
          </a><br>
        </td>
      </tr>
      <tr>
        <td>code</td>
        <td>
          <a href="https://travis-ci.org/drtrigon/sketchbook">
            <img alt="Build Status" src="https://travis-ci.org/drtrigon/sketchbook.svg?branch=master" />
          </a><br>
        </td>
      </tr>
      <tr>
        <td>docu</td>
        <td>
          <a href="http://www.doxygen.org/index.html">
            <img src="https://raw.githubusercontent.com/drtrigon/sketchbook/result/docu/doc/html/doxygen.png" alt="doxygen">
          </a><br>
        </td>
      </tr>
    </table>
    <hr>
    <table>
      <tr>
        <td>CAD</td>
        <td>
          <script src="https://embed.github.com/view/3d/drtrigon/sketchbook/master/MultiWii_2_4/Drone-MultiWii_FlightController.stl"></script><br>
        </td>
      </tr>
      <tr>
        <td>PCB</td>
        <td>
          <!--<a href="https://edrc.me/g/drtrigon/eagle">
            <img alt="design rule check status" src="https://edrc.me/api/v1/user/drtrigon/project/eagle/img/status.svg" />
          </a><br>-->
          <a href="https://edrc.me/g/drtrigon/eagle">
            <img alt="projects/Drone_MultiWii/FlightController.brd from EDRC.me" src="https://edrc.me/api/v1/user/drtrigon/project/eagle/img/file/projects%2FDrone_MultiWii%2FFlightController.png?ref=refs%2Fheads%2Fmaster" />
          </a><br>
          <a href="https://github.com/drtrigon/eagle/blob/result/pcb/projects/Drone_MultiWii/FlightController.png">
            <img alt="projects/Drone_MultiWii/FlightController.png" width="600" src="https://raw.githubusercontent.com/drtrigon/eagle/result/pcb/projects/Drone_MultiWii/FlightController.png" />
          </a><br>
          GERBER: <a href="https://github.com/drtrigon/eagle/blob/result/pcb/projects/Drone_MultiWii/FlightController.zip">FlightController.zip</a><br>
          partlist: <a href="https://github.com/drtrigon/eagle/blob/result/pcb/projects/Drone_MultiWii/FlightController.txt">FlightController.txt</a><br>
          <br>
          <a href="https://github.com/drtrigon/eagle/tree/result/pcb/projects/Drone_MultiWii">drtrigon/eagle/tree/result/pcb/projects/Drone_MultiWii</a> logs:<br>
          <ul>
            <li><a href="https://github.com/drtrigon/eagle/blob/result/pcb/projects/Drone_MultiWii/FlightController.gpi">FlightController.gpi</a></li>
            <li><a href="https://github.com/drtrigon/eagle/blob/result/pcb/projects/Drone_MultiWii/FlightController.dri">FlightController.dri</a></li>
          </ul>
        </td>
      </tr>
      <tr>
        <td>code</td>
        <td>
          <!--<a href="https://travis-ci.org/drtrigon/sketchbook">
            <img alt="Build Status" src="https://travis-ci.org/drtrigon/sketchbook.svg?branch=master" />
          </a><br>-->
        </td>
      </tr>
      <tr>
        <td>docu</td>
        <td>
          <ul>
            <li><a href="https://rawgit.com/drtrigon/sketchbook/result/docu/doc/html/index.html">doxygen html</a></li>
            <!--<li><a href="https://github.com/drtrigon/sketchbook/blob/result/docu/doc/latex/refman.pdf">doxygen pdf</a></li>-->
          </ul>
        </td>
      </tr>
    </table>
@endhtmlonly

#### changelog pcb (eagle)

todo 1.0 -> 1.1

* more info to silk screen; GND, 5V, PIN NUMBERS (at least first/last in row), 
* name/value the connectors (insead of manually putting text on silk screen)
* use milling??
* ...

main changes 1.0 -> 1.1

* silk screen; plugin board shapes
* copper top plane; inverted font for info field
* geometry; size and position of screw holes
* geometry; milled hole to reduce weight

## Setup
(might make sense to split projects off into separate repos/submodules)

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
* add doxygen to repository
  * The Well-Automated Arduino Library - Adafruit Learning System: https://learn.adafruit.com/the-well-automated-arduino-library/doxygen
  * https://stackoverflow.com/questions/35500277/doxygen-for-ino-files-arduino
