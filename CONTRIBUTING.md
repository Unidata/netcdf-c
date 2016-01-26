![](http://www.unidata.ucar.edu/images/logos/thredds_tds-150x150.png)

Welcome contributors!
=====================

First off, thank you for your interest in contributing to the THREDDS
project! This repository contains the code for both netCDF-Java and the
THREDDS Data Server (TDS) projects. The other projects held under the
THREDDS umbrella are
[Siphon](https://github.com/unidata/rosetta%3ERosetta%3C/a%3E%20and%20the%20latest%20addition,%20%3Ca%20href=)
(a python interface to the TDS).

Process Overview
----------------

-   [GitHub Setup](#gh-setup)
    -   [Join Github!](#gh-join)
    -   [Fork the Unidata THREDDS project](#gh-fork)
    -   [Pull down local copy of the Unidata THREDDS
        project](#gh-pull-ud-thredds)
    -   [Add and pull down a local copy of your THREDDS project
        fork](#gh-pull-personal-thredds)
-   [Contribution workflow](#gh-contrib-workflow)
    -   [Make sure you have the latest changes from Unidata THREDDS
        repository](#gh-sync-ud)
    -   [Make a new branch for your work and start hacking](#gh-branch)
    -   [Clean up your git commit history](#gh-history-cleanup)
    -   [Push changes to your fork to use for the pull
        request](#gh-final-commit-for-pr)
    -   [Make the pull request](#gh-pr)
-   [Now what?](#gh-now-what)

[GitHub Setup]()
----------------

### [Join Github!]()

To get started contributing to the THREDDS project, the first thing you
should do is [signup for a free account on
GitHub](https://github.com/join).

### [Fork the Unidata THREDDS project]()

Once you have an account, go ahead and
[fork](https://github.com/unidata/thredds#fork-destination-box) the
THREDDS project. By forking the project, you will have a complete copy
of the THREDDS project, history and all, under your personal account.
This will allow you to make pull requests against the Unidata THREDDS
repository, which is the primairy mechanism used to add new code to the
project (even Unidata developers do this!).

### [Pull down local copy of the Unidata THREDDS project]()

After cloning the Unidata repository, you can pull down the source code
to your local machine by using git:

    git clone --origin unidata git@github.com:Unidata/thredds.git (for ssh)

or

    git clone --origin unidata https://github.com/Unidata/thredds.git (for http)

Note that these commands reference the Unidata repository.

Normally in git, the remote repository you clone from is automatically
named 'origin'. To help with any confusion when making pull requests,
this commands above rename the remote repository to 'unidata'.

### [Add and pull down a local copy of your THREDDS project fork]()

Next, move into the source directory git has created, and add your
personal fork of the THREDDS code as a remote"

    git clone --origin me git@github.com:/thredds.git (for ssh)

or

    git clone --origin me https://github.com/,my-github-user-name>/thredds.git (for http)

Now you are all set!

[Contribution workflow]()
-------------------------

### [Make sure you have the latest changes from Unidata THREDDS repository]()

First, make sure you have the most recent changes to the THREDDS code by
using git pull:

    git pull unidata master

### [Make a new branch for your work and start hacking]()

Next, make a new branch where you will actually do the hacking:

    git checkout -b mywork

As of this point, the branch 'mywork' is local. To make this branch part
of your personal GitHub Remote repository, use the following command:

    git push -u me mywork

Now git (on your local machine) is setup to a point where you can start
hacking on the code and commit changes to your personal GitHub
repository. At any point, you may add commits to your local copy of the
repository by using:

    git commit

If you would like these changes to be stored on your personal remote
repository, simply use:

    git push me mywork

Once you are satisified with your work, there is one last step to
complete before submitting the pull request - clean up the history.

### [Clean up your git commit history]()

Commit history can often be full of temporiariy commit messages, of
commits with code changes that ultimately didn't make the final cut.

To clean up your history, use the

    git rebase -i

command, which will open an editor:

    sarms@flip: [mywork] git rebase -i
    pick 083508e first commit of my really cool feature or bug fix!
    pick 9bcba01 Oops missed this one thing. This commit fixes that.

    # Rebase 083508e..9bcba01 onto 083508e (2 command(s))
    #
    # Commands:
    # p, pick = use commit
    # r, reword = use commit, but edit the commit message
    # e, edit = use commit, but stop for amending
    # s, squash = use commit, but meld into previous commit
    # f, fixup = like "squash", but discard this commit

message \# x, exec = run command (the rest of the line) using shell \#
d, drop = remove commit \# \# These lines can be re-ordered; they are
executed from top to bottom. \# \# If you remove a line here THAT COMMIT
WILL BE LOST. \# \# However, if you remove everything, the rebase will
be aborted. \# \# Note that empty commits are commented out


    Based on my commit messages, you can see that commit 

1' fixed a mistake from my first commit. It would be nice to 'squash'
those changes into the first commit, so that the official history does
not show my mistake..uhhh...this extra commit. To do so, edit the text
to change the second commits 'pick' to 'squash':

    h
    pick 083508e first commit of my really cool feature or bug fix!
    squash 9bcba01 Oops missed this one thing. This commit fixes that.

    # Rebase 083508e..9bcba01 onto 083508e (2 command(s))
    #
    # Commands:
    # p, pick = use commit
    # r, reword = use commit, but edit the commit message
    # e, edit = use commit, but stop for amending
    # s, squash = use commit, but meld into previous commit
    # f, fixup = like "squash", but discard this commit

message \# x, exec = run command (the rest of the line) using shell \#
d, drop = remove commit \# \# These lines can be re-ordered; they are
executed from top to bottom. \# \# If you remove a line here THAT COMMIT
WILL BE LOST. \# \# However, if you remove everything, the rebase will
be aborted. \# \# Note that empty commits are commented out


    Once you have marked the commits to be squashed and exited the edit, you will prompted to change the commit message for the new, squashed, mega commit:

\# This is a combination of 2 commits. \# The first commit's message is:
first commit of my really cool feature or bug fix! \# This is the 2nd
commit message: Oops missed this one thing. This commit fixes that.
\#Please enter the commit message for your changes. Lines starting \#
with '\#' will be ignored, and an empty message aborts the commit. \# \#
Date: Thu Oct 15 09:59:23 2015 -0600 \# \# interactive rebase in
progress; onto 083508e \# Last commands done (2 commands done): \# pick
09134d5 first commit of my really cool feature or bug fix! \# squash
9bcba01 Oops missed this one thing. This commit fixes that. \# No
commands remaining. \# You are currently editing a commit while rebasing
branch 'mywork' on '0835 08e'. \# \# Changes to be committed: ...


    Edit the two commit messages into a single message that describes the overall change:

Once you have and exit, you will have a change to change the commit
message for the new, squashed, mega commit:

    h

    Really cool feature or bug fix. Addresses the github issue Unidata/thredds#1

    #Please enter the commit message for your changes. Lines starting
    # with 

l be ignored, and an empty message aborts the commit. \# \# Date: Thu
Oct 15 09:59:23 2015 -0600 \# \# interactive rebase in progress; onto
083508e \# Last commands done (2 commands done): \# pick 09134d5 first
commit of my really cool feature or bug fix! \# squash 9bcba01 Oops
missed this one thing. This commit fixes that. \# No commands remaining.
\# You are currently editing a commit while rebasing branch 'mywork' on
'0835 08e'. \# \# Changes to be committed: ...


    Now, when you look at your git commit logs, you will see:

commit 805b4723c4a2cbbed240354332cd7af57559a1b9 Author: Sean Arms Date:
Thu Oct 15 09:59:23 2015 -0600 Really cool feature or bug fix. Addresses
the github issue Unidata/thredds\#1


    Note that the commit conains the text 

a/thredds\#1'. This is a cool github trick that will allow you to
reference GitHub issues within your commit messages. When viewed on
github.com, this will be turned into a hyperlink to the issue. While not
every contribution will address an issue, please use this feature if
your contribution does!

### [Push changes to your fork to use for the pull request]()

Now that you have cleaned up the history, you will need to make a final
push to your personal GitHub repository. However, the rebase has changed
the history of your local branch, which means you will need to use the
'--force' flag in your push:

    ush --force me mywork

### [Make the pull request]()

Finally, go to your personal remote repository on github.com and switch
to your 'mywork' branch. Once you are on your work branch, you will see
a button that says "Pull request", which will allow you to make a pull
request. The github pull request page will allow you to select which
repository and branch you would like to submit the pull request to (the
'base fork', which should be 'Unidata/thredds', and 'base', which should
be 'master'), as well as the 'head fork' and 'compare' (which should be
'' and 'mywork', respectivly). Once this is setup, you can make the pull
request.

[Now what?]()
-------------

The Unidata THREDDS project is setup such that automated testing for all
pull requests is done via TravisCI. The status of the tests can be seen
on the pull request page. For example, see
[Unidata/thredds\#231](https://github.com/Unidata/thredds/pull/231) by
selecting the 'View Details' button. This pull request was tested on
[TravisCI](https://travis-ci.org/Unidata/thredds/builds/84433104) and
passed on all versions of Java supported by the current master branch.
We have setup the THREDDS repository such that changes that do not pass
these tests cannot be merged. One of the Unidata THREDDS team members
will work with you to make sure your work is ready for merging once the
tests have passed on TravisCI. Note that as changes to your pull request
may be required, you can simply push thos changes to your personal
GitHub repository and the pull request will automatically be updated and
new TravisCI tests will be initiated. If your pull request addresses a
bug, we kindly ask that you include a test in your pull request. If you
do not know how to write tests in Java, we will be more than happy to
work with you!
