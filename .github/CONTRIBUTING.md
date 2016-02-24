# How to contribute

Thanks for your interest in contributing to the netCDF project.  There are many different platforms and configurations which we don't have access to directly, and your contributions help ensure that things continue to run smoothly for **all** of our users.  We'll strive to keep the process simple for you, the contributor, by establishing a handful of guidelines up front.

> We will assume that you are familiar with `git` and `GitHub`.  If not, you may go through the GitHub tutorial found at [https://guides.github.com/activities/hello-world/](https://guides.github.com/activities/hello-world/).  This tutorial should only take around 10 minutes.

## Table of Contents

* [Workflow](#workflow)
* [Characteristics of an accepted pull request](#chars)
	* [Small bug-and-typo fixes](#smallissues)
* [Testing your contribution](#testing)
	* [Spot-checks](#spotcheck)
	* [Continuous Integration testing](#contint)
	* [Regression testing with Docker](#regression)
* [Final Remarks](#conclusion)

# Workflow <A NAME="workflow"></A>

The process for contributing a patch to netCDF roughly follows the following workflow:

* (Optional) Open an issue [on GitHub](http://github.com/Unidata/netcdf-c/issues).

> This step is ***strongly*** encouraged.  The larger the scope of the proposed change, the more likely it is that we will need to have a discussion about the ramifications before accepting the pull request.  We don't want anybody to waste their time developing a patch only to have it rejected because we disagree with the premise or goal!  By opening an issue you can start a public dialog with us about the proposed changes, ensuring that everybody is on the same page and that no time/effort will be wasted!

* Fork the [Unidata/netcdf-c](http://github.com/Unidata/netcdf-c) repository.
* Make the desired changes to the netcdf-c project.
* Compile your code and test your changes.  See the section on testing your changes below for more information; we provide a lot of options!
* Push your changes to GitHub.
* Issue a pull request.

Once a pull request has been received, it will be reviewed and evaluated by the netCDF developers at Unidata.  If there are any questions or comments, they will be appended to the discussion area of the pull request.

# The characteristics of an accepted pull request <A NAME="chars"></A>

Not every pull request is created equally; while we appreciate every contribution we receive, we do not accept them all.  Those that we *do* accept generally have the following in common:

* **They possess a clear purpose** - What is the point of the pull request? How does it benefit the netCDF community at large? If it is not beneficial in an apparent fashion, we will likely reject the pull request.
* **The code is documented** - The netCDF developers must understand not only *what* a change is doing, but *how* it is doing it.  Documenting your code makes it easier for us to understand the workflow of your patch.  If we can't figure out how a patch works, we will ask for clarification before accepting it.
* **They pass QA** - If new functionality has been added, corresponding unit tests have been created and wired in to the build systems.  We do not expect you to perform comprehensive testing across a multitude of platforms, but at the very least your code should **compile** and no existing tests should be broken.  A pull request that doesn't compile will be rejected on principle, and a pull request which breaks tests will have to be corrected before it is accepted.  See "Testing your changes" below for more information.
* **They do not compromise the principles behind netCDF** - NetCDF has a 100% iron-clad commitment to backwards compatibility.  Any file ever created with netCDF must be readable by any future version of netCDF.  The netCDF data models and file formats are **well** defined.  If your purpose is to modify the data model or file format, **please** discuss this with us first.  ***If*** we are in agreement regarding the changes, we will need to discuss how to add them as a **new** data-model/file-format.  Similarly, netCDF has a commitment to remaining *machine-independent*; data created on one platform/environment/architecture **must** remain readable by any other `libnetcdf` compiled with similar functionality.  

## What about Typo/Bug Fixes/Incremental Fixes? <A NAME="smallissues"></A>

Many of the pull requests we receive do little other than to fix a typo or correct a bug in our code.  This is great, and we encourage as many of these as we can get!  "Fixing mistakes" falls within the principles guiding netCDF, and these changes are 1) Typically self-describing rarely result in any issues with QA

# Testing your changes <A NAME="testing"></A>

There are several ways to test your changes to ensure that your pull request passes QA.  There are manual *spot-checks* which test the code "on-the-spot", and there are automated *continuous integration* tests which will run and evaluate any contributes to ensure nothing breaks.  **Advanced** users may also use Unidata-maintained *Docker* images for running *regression* tests.

## Spot-check tests <A NAME="spotcheck"></A>

* `make check` if you are using configure/autotools.
* `make test` if you are using cmake.

## Continuous Integration testing with Travis CI <A NAME="contint"></A>

Travis CI is a system for providing **automated** continuous integration testing, without requiring any effort on the part of the contributor.  Any time a change is pushed out to GitHub, a series of tests will run automatically and will ultimately report a success or a failure.  You may view the Unidata Travis dashboard [here](https://travis-ci.org/Unidata/).  Each project listed may be "drilled down" into, where specific information may be found.

## Regression testing with Docker <A NAME="regression"></A>

If you are not familiar with Docker, feel free to skip this section *or* read up on Docker [at their website](http://www.docker.io).  If you are familiar with Docker, please read on.

We provide several Docker images for performing regression tests against netCDF-C.  These tests build and test `libnetcdf`, and then use it as the basis for testing the following packages:

* NetCDF-C++
* NetCDF-Fortran
* NetCDF-Python
* NetCDF Operators (NCO)

By performing these comprehensive tests, we're able to see if any change in the core library results in unexpected behavior with the common interfaces.  For full documentation, please see [this page](https://github.com/Unidata/docker-nctests/tree/master/tests-regression).  

These tests will be run against any pull request submitted; you are encouraged to make use of them if you so like.

# We are here to help <A NAME="conclusion"></A>

We want as many contributions as we can get, and are here to help ensure that your efforts are not wasted!  We will work with you to ensure that your code works on more esoteric platforms (`ARM`, `Windows`) and that the pull request possesses the characteristics described above.  Feel free to reach out to us if you have any questions!
