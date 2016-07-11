#!/bin/sh

project_version="$1"
srcdir="$2"

echo "Project version is $project_version"

if [ ! -f "$srcdir/debian/changelog" ]; then
    echo "Skipping test: Debian packaging files not found"
    exit 0
fi

parsed_changelog="$(dpkg-parsechangelog -l "$srcdir/debian/changelog")"
if [ $? -ne 0 ]; then
    echo "Skipping test: could not parse change log"
    exit 0
fi
debian_version="$(echo "$parsed_changelog" | sed -n 's/^Version: //p')"

debian_upstream="$(echo "$debian_version" | sed 's/-.*//')"
debian_release="$(echo "$debian_version" | sed 's/^[^-]*-//')"

echo "Debian package version is ${debian_upstream} with release ${debian_release}"

# The CI system augments the upstream portion of the version number
# with something like "+16.04.20160701", which we want to ignore for
# the sake of this comparison.
stripped_upstream="$(echo "$debian_upstream" | sed 's/\+.*//')"

if [ "$project_version" != "$stripped_upstream" ]; then
    echo "Debian package version does not match project version"
    exit 1
fi
