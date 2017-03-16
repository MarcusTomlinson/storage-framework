#!/bin/sh

#
# Copyright (C) 2013 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Michi Henning <michi.henning@canonical.com>
#

#
# Check that we have acceptable license information in our source files.
#

set -eu

usage()
{
    echo "usage: check_copyright dir [ignore_dir]" >&2
    exit 2
}

[ $# -lt 1 ] && usage
[ $# -gt 2 ] && usage

source_dir="$1"
ignore_dir="${2:-}"

ignore_pat="/parts/|/stage/|/prime/|\\.sci$|\\.swp$|\\.bzr|debian|qmldir|HACKING|ubsan-suppress|valgrind-suppress|\\.txt$|\\.xml$|\\.in$|\\.dox$|\\.yaml$"

#
# We don't use the -i option of licensecheck to add ignore_dir to the pattern because Jenkins creates directories
# with names that contain regex meta-characters, such as "." and "+". Instead, if ignore_dir is set, we post-filter
# the output with grep -F, so we don't get false positives from licensecheck.
#

licensecheck -i "$ignore_pat" -r "$source_dir" > licensecheck.log
if [ -n "$ignore_dir" ]; then
    cat licensecheck.log | grep -v -F "$ignore_dir" | grep "No copyright" > filtered.log || :
else
    cat licensecheck.log | grep "No copyright" > filtered.log || :
fi

if [ -s filtered.log ]; then
    cat filtered.log
    exit 1
fi

exit 0
